// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview.h"

#include <Ecore_IMF_Evas.h>
#include <Ecore_Input_Evas.h>
#include <flutter/texture_registrar.h>
#include <flutter_platform_view.h>
#include <flutter_texture_registrar.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "buffer_pool.h"
#include "log.h"
#include "lwe/LWEWebView.h"
#include "lwe/PlatformIntegrationData.h"
#include "webview_factory.h"

#define LWE_EXPORT
extern "C" size_t LWE_EXPORT createWebViewInstance(
    unsigned x, unsigned y, unsigned width, unsigned height,
    float devicePixelRatio, const char* defaultFontName, const char* locale,
    const char* timezoneID,
    const std::function<::LWE::WebContainer::ExternalImageInfo(void)>&
        prepareImageCb,
    const std::function<void(::LWE::WebContainer*, bool isRendered)>&
        renderedCb);

template <typename T = flutter::EncodableValue>
class NavigationRequestResult : public flutter::MethodResult<T> {
 public:
  NavigationRequestResult(std::string url, WebView* webview)
      : url_(url), webview_(webview) {}

  void SuccessInternal(const T* should_load) override {
    if (std::holds_alternative<bool>(*should_load)) {
      if (std::get<bool>(*should_load)) {
        LoadUrl();
      }
    }
  }

  void ErrorInternal(const std::string& error_code,
                     const std::string& error_message,
                     const T* error_details) override {
    throw std::invalid_argument("navigationRequest calls must succeed [code:" +
                                error_code + "][msg:" + error_message + "]");
  }

  void NotImplementedInternal() override {
    throw std::invalid_argument(
        "navigationRequest must be implemented by the webview method channel");
  }

 private:
  void LoadUrl() {
    if (webview_ && webview_->GetWebViewInstance()) {
      webview_->GetWebViewInstance()->LoadURL(url_);
    }
  }

  std::string url_;
  WebView* webview_;
};

enum RequestErrorType {
  NoError,
  UnknownError,
  HostLookupError,
  UnsupportedAuthSchemeError,
  AuthenticationError,
  ProxyAuthenticationError,
  ConnectError,
  IOError,
  TimeoutError,
  RedirectLoopError,
  UnsupportedSchemeError,
  FailedSSLHandshakeError,
  BadURLError,
  FileError,
  FileNotFoundError,
  TooManyRequestError,
};

static std::string ErrorCodeToString(int error_code) {
  switch (error_code) {
    case RequestErrorType::AuthenticationError:
      return "authentication";
    case RequestErrorType::BadURLError:
      return "badUrl";
    case RequestErrorType::ConnectError:
      return "connect";
    case RequestErrorType::FailedSSLHandshakeError:
      return "failedSslHandshake";
    case RequestErrorType::FileError:
      return "file";
    case RequestErrorType::FileNotFoundError:
      return "fileNotFound";
    case RequestErrorType::HostLookupError:
      return "hostLookup";
    case RequestErrorType::IOError:
      return "io";
    case RequestErrorType::ProxyAuthenticationError:
      return "proxyAuthentication";
    case RequestErrorType::RedirectLoopError:
      return "redirectLoop";
    case RequestErrorType::TimeoutError:
      return "timeout";
    case RequestErrorType::TooManyRequestError:
      return "tooManyRequests";
    case RequestErrorType::UnknownError:
      return "unknown";
    case RequestErrorType::UnsupportedAuthSchemeError:
      return "unsupportedAuthScheme";
    case RequestErrorType::UnsupportedSchemeError:
      return "unsupportedScheme";
  }

  std::string message =
      "Could not find a string for errorCode: " + std::to_string(error_code);
  throw std::invalid_argument(message);
}

std::string ExtractStringFromMap(const flutter::EncodableValue& arguments,
                                 const char* key) {
  if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
    flutter::EncodableMap values = std::get<flutter::EncodableMap>(arguments);
    flutter::EncodableValue value = values[flutter::EncodableValue(key)];
    if (std::holds_alternative<std::string>(value))
      return std::get<std::string>(value);
  }
  return std::string();
}
int ExtractIntFromMap(const flutter::EncodableValue& arguments,
                      const char* key) {
  if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
    flutter::EncodableMap values = std::get<flutter::EncodableMap>(arguments);
    flutter::EncodableValue value = values[flutter::EncodableValue(key)];
    if (std::holds_alternative<int>(value)) return std::get<int>(value);
  }
  return -1;
}
double ExtractDoubleFromMap(const flutter::EncodableValue& arguments,
                            const char* key) {
  if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
    flutter::EncodableMap values = std::get<flutter::EncodableMap>(arguments);
    flutter::EncodableValue value = values[flutter::EncodableValue(key)];
    if (std::holds_alternative<double>(value)) return std::get<double>(value);
  }
  return -1;
}

WebView::WebView(flutter::PluginRegistrar* registrar, int viewId,
                 flutter::TextureRegistrar* texture_registrar, double width,
                 double height, flutter::EncodableMap& params)
    : PlatformView(registrar, viewId),
      texture_registrar_(texture_registrar),
      webview_instance_(nullptr),
      width_(width),
      height_(height),
      candidate_surface_(nullptr),
      rendered_surface_(nullptr),
      is_mouse_lbutton_down_(false),
      has_navigation_delegate_(false),
      has_progress_tracking_(false),
      context_(nullptr),
      texture_variant_(nullptr),
      gpu_buffer_(nullptr) {
  tbm_pool_ = std::make_unique<BufferPool>(width, height);
  texture_variant_ = new flutter::TextureVariant(flutter::GpuBufferTexture(
      [this](size_t width, size_t height) -> const FlutterDesktopGpuBuffer* {
        return this->ObtainGpuBuffer(width, height);
      },
      [this](void* buffer) -> void { this->DestructBuffer(buffer); }));
  SetTextureId(texture_registrar_->RegisterTexture(texture_variant_));
  InitWebView();

  channel_ = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
      GetPluginRegistrar()->messenger(), GetChannelName(),
      &flutter::StandardMethodCodec::GetInstance());
  channel_->SetMethodCallHandler(
      [webview = this](const auto& call, auto result) {
        webview->HandleMethodCall(call, std::move(result));
      });

  auto cookie_channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          GetPluginRegistrar()->messenger(),
          "plugins.flutter.io/cookie_manager",
          &flutter::StandardMethodCodec::GetInstance());
  cookie_channel->SetMethodCallHandler(
      [webview = this](const auto& call, auto result) {
        webview->HandleCookieMethodCall(call, std::move(result));
      });

  std::string url;
  auto initial_url = params[flutter::EncodableValue("initialUrl")];
  if (std::holds_alternative<std::string>(initial_url)) {
    url = std::get<std::string>(initial_url);
  } else {
    url = "about:blank";
  }

  auto settings = params[flutter::EncodableValue("settings")];
  if (std::holds_alternative<flutter::EncodableMap>(settings)) {
    auto settingList = std::get<flutter::EncodableMap>(settings);
    if (settingList.size() > 0) {
      ApplySettings(settingList);
    }
  }

  auto names = params[flutter::EncodableValue("javascriptChannelNames")];
  if (std::holds_alternative<flutter::EncodableList>(names)) {
    auto name_list = std::get<flutter::EncodableList>(names);
    for (size_t i = 0; i < name_list.size(); i++) {
      if (std::holds_alternative<std::string>(name_list[i])) {
        RegisterJavaScriptChannelName(std::get<std::string>(name_list[i]));
      }
    }
  }

  // TODO: Not implemented
  // auto media = params[flutter::EncodableValue("autoMediaPlaybackPolicy")];

  auto user_agent = params[flutter::EncodableValue("userAgent")];
  if (std::holds_alternative<std::string>(user_agent)) {
    auto settings = webview_instance_->GetSettings();
    settings.SetUserAgentString(std::get<std::string>(user_agent));
    webview_instance_->SetSettings(settings);
  }

  webview_instance_->RegisterOnPageStartedHandler(
      [this](LWE::WebContainer* container, const std::string& url) {
        LOG_DEBUG("RegisterOnPageStartedHandler(url: %s)\n", url.c_str());
        flutter::EncodableMap map;
        map.insert(
            std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
                flutter::EncodableValue("url"), flutter::EncodableValue(url)));
        auto args = std::make_unique<flutter::EncodableValue>(map);
        channel_->InvokeMethod("onPageStarted", std::move(args));
      });
  webview_instance_->RegisterOnPageLoadedHandler(
      [this](LWE::WebContainer* container, const std::string& url) {
        LOG_DEBUG("RegisterOnPageLoadedHandler(url: %s)(title:%s)\n",
                  url.c_str(), container->GetTitle().c_str());
        flutter::EncodableMap map;
        map.insert(
            std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
                flutter::EncodableValue("url"), flutter::EncodableValue(url)));
        auto args = std::make_unique<flutter::EncodableValue>(map);
        channel_->InvokeMethod("onPageFinished", std::move(args));
      });
  webview_instance_->RegisterOnProgressChangedHandler(
      [this](LWE::WebContainer* container, int progress) {
        LOG_DEBUG("RegisterOnProgressChangedHandler(progress:%d)\n", progress);
        if (!has_progress_tracking_) {
          return;
        }
        flutter::EncodableMap map;
        map.insert(
            std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
                flutter::EncodableValue("progress"),
                flutter::EncodableValue(progress)));
        auto args = std::make_unique<flutter::EncodableValue>(map);
        channel_->InvokeMethod("onProgress", std::move(args));
      });
  webview_instance_->RegisterOnReceivedErrorHandler(
      [this](LWE::WebContainer* container, LWE::ResourceError e) {
        flutter::EncodableMap map;
        map.insert(
            std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
                flutter::EncodableValue("errorCode"),
                flutter::EncodableValue(e.GetErrorCode())));
        map.insert(
            std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
                flutter::EncodableValue("description"),
                flutter::EncodableValue(e.GetDescription())));
        map.insert(
            std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
                flutter::EncodableValue("errorType"),
                flutter::EncodableValue(ErrorCodeToString(e.GetErrorCode()))));
        map.insert(
            std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
                flutter::EncodableValue("failingUrl"),
                flutter::EncodableValue(e.GetUrl())));
        auto args = std::make_unique<flutter::EncodableValue>(map);
        channel_->InvokeMethod("onWebResourceError", std::move(args));
      });

  webview_instance_->RegisterShouldOverrideUrlLoadingHandler(
      [this](LWE::WebContainer* view, const std::string& url) -> bool {
        if (!has_navigation_delegate_) {
          return false;
        }
        flutter::EncodableMap map;
        map.insert(
            std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
                flutter::EncodableValue("url"), flutter::EncodableValue(url)));
        map.insert(
            std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
                flutter::EncodableValue("isForMainFrame"),
                flutter::EncodableValue(true)));
        auto args = std::make_unique<flutter::EncodableValue>(map);
        auto on_result =
            std::make_unique<NavigationRequestResult<flutter::EncodableValue>>(
                url, this);
        channel_->InvokeMethod("navigationRequest", std::move(args),
                               std::move(on_result));

        return true;
      });

  webview_instance_->LoadURL(url);
}

void WebView::ApplySettings(flutter::EncodableMap settings) {
  for (auto const& [key, val] : settings) {
    if (std::holds_alternative<std::string>(key)) {
      std::string k = std::get<std::string>(key);
      if ("jsMode" == k) {
        // NOTE: Not supported by Lightweight Web Engine (LWE) on Tizen.
      } else if ("hasNavigationDelegate" == k) {
        if (std::holds_alternative<bool>(val)) {
          has_navigation_delegate_ = std::get<bool>(val);
        }
      } else if ("hasProgressTracking" == k) {
        if (std::holds_alternative<bool>(val)) {
          has_progress_tracking_ = std::get<bool>(val);
        }
      } else if ("debuggingEnabled" == k) {
        // NOTE: Not supported by LWE on Tizen.
      } else if ("gestureNavigationEnabled" == k) {
        // NOTE: Not supported by LWE on Tizen.
      } else if ("allowsInlineMediaPlayback") {
        // no-op inline media playback is always allowed on Tizen.
      } else if ("userAgent" == k) {
        if (std::holds_alternative<std::string>(val)) {
          auto settings = webview_instance_->GetSettings();
          settings.SetUserAgentString(std::get<std::string>(val));
          webview_instance_->SetSettings(settings);
        }
      } else {
        throw std::invalid_argument("Unknown WebView setting: " + k);
      }
    }
  }
}

/**
 * Added as a JavaScript interface to the WebView for any JavaScript channel
 * that the Dart code sets up.
 *
 * Exposes a single method named `postMessage` to JavaScript, which sends a
 * message over a method channel to the Dart code.
 */
void WebView::RegisterJavaScriptChannelName(const std::string& name) {
  LOG_DEBUG("RegisterJavaScriptChannelName(channelName: %s)\n", name.c_str());

  std::function<std::string(const std::string&)> cb =
      [this, name](const std::string& message) -> std::string {
    LOG_DEBUG("Invoke JavaScriptChannel(message: %s)\n", message.c_str());
    flutter::EncodableMap map;
    map.insert(std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
        flutter::EncodableValue("channel"), flutter::EncodableValue(name)));
    map.insert(std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
        flutter::EncodableValue("message"), flutter::EncodableValue(message)));

    std::unique_ptr<flutter::EncodableValue> args =
        std::make_unique<flutter::EncodableValue>(map);
    channel_->InvokeMethod("javascriptChannelMessage", std::move(args));
    return "success";
  };

  webview_instance_->AddJavaScriptInterface(name, "postMessage", cb);
}

WebView::~WebView() { Dispose(); }

std::string WebView::GetChannelName() {
  return "plugins.flutter.io/webview_" + std::to_string(GetViewId());
}

void WebView::Dispose() {
  texture_registrar_->UnregisterTexture(GetTextureId());

  if (webview_instance_) {
    webview_instance_->Destroy();
    webview_instance_ = nullptr;
  }

  if (texture_variant_) {
    delete texture_variant_;
    texture_variant_ = nullptr;
  }

  if (gpu_buffer_) {
    delete gpu_buffer_;
    gpu_buffer_ = nullptr;
  }
}

void WebView::Resize(double width, double height) {
  LOG_DEBUG("WebView::Resize width: %f height: %f \n", width, height);
  width_ = width;
  height_ = height;

  if (candidate_surface_) {
    candidate_surface_ = nullptr;
  }
  tbm_pool_->Prepare(width_, height_);
  webview_instance_->ResizeTo(width_, height_);
}

void WebView::Touch(int type, int button, double x, double y, double dx,
                    double dy) {
  if (type == 0) {  // down event
    webview_instance_->DispatchMouseDownEvent(
        LWE::MouseButtonValue::LeftButton,
        LWE::MouseButtonsValue::LeftButtonDown, x, y);
    is_mouse_lbutton_down_ = true;
  } else if (type == 1) {  // move event
    webview_instance_->DispatchMouseMoveEvent(
        is_mouse_lbutton_down_ ? LWE::MouseButtonValue::LeftButton
                               : LWE::MouseButtonValue::NoButton,
        is_mouse_lbutton_down_ ? LWE::MouseButtonsValue::LeftButtonDown
                               : LWE::MouseButtonsValue::NoButtonDown,
        x, y);
  } else if (type == 2) {  // up event
    webview_instance_->DispatchMouseUpEvent(
        LWE::MouseButtonValue::NoButton, LWE::MouseButtonsValue::NoButtonDown,
        x, y);
    is_mouse_lbutton_down_ = false;
  } else {
    // TODO: Not implemented
  }
}

static LWE::KeyValue EcoreEventKeyToKeyValue(const char* ecore_key_string,
                                             bool is_shift_pressed) {
  if (strcmp("Left", ecore_key_string) == 0) {
    return LWE::KeyValue::ArrowLeftKey;
  } else if (strcmp("Right", ecore_key_string) == 0) {
    return LWE::KeyValue::ArrowRightKey;
  } else if (strcmp("Up", ecore_key_string) == 0) {
    return LWE::KeyValue::ArrowUpKey;
  } else if (strcmp("Down", ecore_key_string) == 0) {
    return LWE::KeyValue::ArrowDownKey;
  } else if (strcmp("space", ecore_key_string) == 0) {
    return LWE::KeyValue::SpaceKey;
  } else if (strcmp("Return", ecore_key_string) == 0) {
    return LWE::KeyValue::EnterKey;
  } else if (strcmp("Tab", ecore_key_string) == 0) {
    return LWE::KeyValue::TabKey;
  } else if (strcmp("BackSpace", ecore_key_string) == 0) {
    return LWE::KeyValue::BackspaceKey;
  } else if (strcmp("Escape", ecore_key_string) == 0) {
    return LWE::KeyValue::EscapeKey;
  } else if (strcmp("Delete", ecore_key_string) == 0) {
    return LWE::KeyValue::DeleteKey;
  } else if (strcmp("at", ecore_key_string) == 0) {
    return LWE::KeyValue::AtMarkKey;
  } else if (strcmp("minus", ecore_key_string) == 0) {
    if (is_shift_pressed) {
      return LWE::KeyValue::UnderScoreMarkKey;
    } else {
      return LWE::KeyValue::MinusMarkKey;
    }
  } else if (strcmp("equal", ecore_key_string) == 0) {
    if (is_shift_pressed) {
      return LWE::KeyValue::PlusMarkKey;
    } else {
      return LWE::KeyValue::EqualitySignKey;
    }
  } else if (strcmp("bracketleft", ecore_key_string) == 0) {
    if (is_shift_pressed) {
      return LWE::KeyValue::LeftCurlyBracketMarkKey;
    } else {
      return LWE::KeyValue::LeftSquareBracketKey;
    }
  } else if (strcmp("bracketright", ecore_key_string) == 0) {
    if (is_shift_pressed) {
      return LWE::KeyValue::RightCurlyBracketMarkKey;
    } else {
      return LWE::KeyValue::RightSquareBracketKey;
    }
  } else if (strcmp("semicolon", ecore_key_string) == 0) {
    if (is_shift_pressed) {
      return LWE::KeyValue::ColonMarkKey;
    } else {
      return LWE::KeyValue::SemiColonMarkKey;
    }
  } else if (strcmp("apostrophe", ecore_key_string) == 0) {
    if (is_shift_pressed) {
      return LWE::KeyValue::DoubleQuoteMarkKey;
    } else {
      return LWE::KeyValue::SingleQuoteMarkKey;
    }
  } else if (strcmp("comma", ecore_key_string) == 0) {
    if (is_shift_pressed) {
      return LWE::KeyValue::LessThanMarkKey;
    } else {
      return LWE::KeyValue::CommaMarkKey;
    }
  } else if (strcmp("period", ecore_key_string) == 0) {
    if (is_shift_pressed) {
      return LWE::KeyValue::GreaterThanSignKey;
    } else {
      return LWE::KeyValue::PeriodKey;
    }
  } else if (strcmp("slash", ecore_key_string) == 0) {
    if (is_shift_pressed) {
      return LWE::KeyValue::QuestionMarkKey;
    } else {
      return LWE::KeyValue::SlashKey;
    }
  } else if (strlen(ecore_key_string) == 1) {
    char ch = ecore_key_string[0];
    if (ch >= '0' && ch <= '9') {
      if (is_shift_pressed) {
        switch (ch) {
          case '1':
            return LWE::KeyValue::ExclamationMarkKey;
          case '2':
            return LWE::KeyValue::AtMarkKey;
          case '3':
            return LWE::KeyValue::SharpMarkKey;
          case '4':
            return LWE::KeyValue::DollarMarkKey;
          case '5':
            return LWE::KeyValue::PercentMarkKey;
          case '6':
            return LWE::KeyValue::CaretMarkKey;
          case '7':
            return LWE::KeyValue::AmpersandMarkKey;
          case '8':
            return LWE::KeyValue::AsteriskMarkKey;
          case '9':
            return LWE::KeyValue::LeftParenthesisMarkKey;
          case '0':
            return LWE::KeyValue::RightParenthesisMarkKey;
        }
      }
      return (LWE::KeyValue)(LWE::KeyValue::Digit0Key + ch - '0');
    } else if (ch >= 'a' && ch <= 'z') {
      if (is_shift_pressed) {
        return (LWE::KeyValue)(LWE::KeyValue::LowerAKey + ch - 'a' - 32);
      } else {
        return (LWE::KeyValue)(LWE::KeyValue::LowerAKey + ch - 'a');
      }
    } else if (ch >= 'A' && ch <= 'Z') {
      if (is_shift_pressed) {
        return (LWE::KeyValue)(LWE::KeyValue::AKey + ch - 'A' + 32);
      } else {
        return (LWE::KeyValue)(LWE::KeyValue::AKey + ch - 'A');
      }
    }
  } else if (strcmp("XF86AudioRaiseVolume", ecore_key_string) == 0) {
    return LWE::KeyValue::TVVolumeUpKey;
  } else if (strcmp("XF86AudioLowerVolume", ecore_key_string) == 0) {
    return LWE::KeyValue::TVVolumeDownKey;
  } else if (strcmp("XF86AudioMute", ecore_key_string) == 0) {
    return LWE::KeyValue::TVMuteKey;
  } else if (strcmp("XF86RaiseChannel", ecore_key_string) == 0) {
    return LWE::KeyValue::TVChannelUpKey;
  } else if (strcmp("XF86LowerChannel", ecore_key_string) == 0) {
    return LWE::KeyValue::TVChannelDownKey;
  } else if (strcmp("XF86AudioRewind", ecore_key_string) == 0) {
    return LWE::KeyValue::MediaTrackPreviousKey;
  } else if (strcmp("XF86AudioNext", ecore_key_string) == 0) {
    return LWE::KeyValue::MediaTrackNextKey;
  } else if (strcmp("XF86AudioPause", ecore_key_string) == 0) {
    return LWE::KeyValue::MediaPauseKey;
  } else if (strcmp("XF86AudioRecord", ecore_key_string) == 0) {
    return LWE::KeyValue::MediaRecordKey;
  } else if (strcmp("XF86AudioPlay", ecore_key_string) == 0) {
    return LWE::KeyValue::MediaPlayKey;
  } else if (strcmp("XF86AudioStop", ecore_key_string) == 0) {
    return LWE::KeyValue::MediaStopKey;
  } else if (strcmp("XF86Info", ecore_key_string) == 0) {
    return LWE::KeyValue::TVInfoKey;
  } else if (strcmp("XF86Back", ecore_key_string) == 0) {
    return LWE::KeyValue::TVReturnKey;
  } else if (strcmp("XF86Red", ecore_key_string) == 0) {
    return LWE::KeyValue::TVRedKey;
  } else if (strcmp("XF86Green", ecore_key_string) == 0) {
    return LWE::KeyValue::TVGreenKey;
  } else if (strcmp("XF86Yellow", ecore_key_string) == 0) {
    return LWE::KeyValue::TVYellowKey;
  } else if (strcmp("XF86Blue", ecore_key_string) == 0) {
    return LWE::KeyValue::TVBlueKey;
  } else if (strcmp("XF86SysMenu", ecore_key_string) == 0) {
    return LWE::KeyValue::TVMenuKey;
  } else if (strcmp("XF86Home", ecore_key_string) == 0) {
    return LWE::KeyValue::TVHomeKey;
  } else if (strcmp("XF86Exit", ecore_key_string) == 0) {
    return LWE::KeyValue::TVExitKey;
  } else if (strcmp("XF86PreviousChannel", ecore_key_string) == 0) {
    return LWE::KeyValue::TVPreviousChannel;
  } else if (strcmp("XF86ChannelList", ecore_key_string) == 0) {
    return LWE::KeyValue::TVChannelList;
  } else if (strcmp("XF86ChannelGuide", ecore_key_string) == 0) {
    return LWE::KeyValue::TVChannelGuide;
  } else if (strcmp("XF86SimpleMenu", ecore_key_string) == 0) {
    return LWE::KeyValue::TVSimpleMenu;
  } else if (strcmp("XF86EManual", ecore_key_string) == 0) {
    return LWE::KeyValue::TVEManual;
  } else if (strcmp("XF86ExtraApp", ecore_key_string) == 0) {
    return LWE::KeyValue::TVExtraApp;
  } else if (strcmp("XF86Search", ecore_key_string) == 0) {
    return LWE::KeyValue::TVSearch;
  } else if (strcmp("XF86PictureSize", ecore_key_string) == 0) {
    return LWE::KeyValue::TVPictureSize;
  } else if (strcmp("XF86Sleep", ecore_key_string) == 0) {
    return LWE::KeyValue::TVSleep;
  } else if (strcmp("XF86Caption", ecore_key_string) == 0) {
    return LWE::KeyValue::TVCaption;
  } else if (strcmp("XF86More", ecore_key_string) == 0) {
    return LWE::KeyValue::TVMore;
  } else if (strcmp("XF86BTVoice", ecore_key_string) == 0) {
    return LWE::KeyValue::TVBTVoice;
  } else if (strcmp("XF86Color", ecore_key_string) == 0) {
    return LWE::KeyValue::TVColor;
  } else if (strcmp("XF86PlayBack", ecore_key_string) == 0) {
    return LWE::KeyValue::TVPlayBack;
  }

  LOG_DEBUG("WebViewEFL - unimplemented key %s\n", ecore_key_string);
  return LWE::KeyValue::UnidentifiedKey;
}

void WebView::DispatchKeyDownEvent(Ecore_Event_Key* key_event) {
  std::string key_name = key_event->keyname;
  LOG_DEBUG("ECORE_EVENT_KEY_DOWN [%s, %d]\n", key_name.c_str(),
            (key_event->modifiers & 1) || (key_event->modifiers & 2));

  if (!IsFocused()) {
    LOG_DEBUG("ignore keydown because we dont have focus");
    return;
  }

  if ((strcmp(key_name.c_str(), "XF86Exit") == 0) ||
      (strcmp(key_name.c_str(), "Select") == 0) ||
      (strcmp(key_name.c_str(), "Cancel") == 0)) {
    if (strcmp(key_name.c_str(), "Select") == 0) {
      webview_instance_->AddIdleCallback(
          [](void* data) {
            WebView* view = (WebView*)data;
            LWE::WebContainer* self = view->GetWebViewInstance();
            LWE::KeyValue kv = LWE::KeyValue::EnterKey;
            self->DispatchKeyDownEvent(kv);
            self->DispatchKeyPressEvent(kv);
            self->DispatchKeyUpEvent(kv);
            view->HidePanel();
          },
          this);
    } else {
      webview_instance_->AddIdleCallback(
          [](void* data) {
            WebView* view = (WebView*)data;
            view->HidePanel();
          },
          this);
    }
  }

  struct Param {
    LWE::WebContainer* webview_instance;
    LWE::KeyValue key_value;
  };
  Param* p = new Param();
  p->webview_instance = webview_instance_;
  p->key_value =
      EcoreEventKeyToKeyValue(key_name.c_str(), (key_event->modifiers & 1));

  webview_instance_->AddIdleCallback(
      [](void* data) {
        Param* p = (Param*)data;
        p->webview_instance->DispatchKeyDownEvent(p->key_value);
        p->webview_instance->DispatchKeyPressEvent(p->key_value);
        delete p;
      },
      p);
}

void WebView::DispatchKeyUpEvent(Ecore_Event_Key* key_event) {
  std::string key_name = key_event->keyname;
  LOG_DEBUG("ECORE_EVENT_KEY_UP [%s, %d]\n", key_name.c_str(),
            (key_event->modifiers & 1) || (key_event->modifiers & 2));

  if (!IsFocused()) {
    LOG_DEBUG("ignore keyup because we dont have focus");
    return;
  }

  struct Param {
    LWE::WebContainer* webview_instance;
    LWE::KeyValue key_value;
  };
  Param* p = new Param();
  p->webview_instance = webview_instance_;
  p->key_value =
      EcoreEventKeyToKeyValue(key_name.c_str(), (key_event->modifiers & 1));

  webview_instance_->AddIdleCallback(
      [](void* data) {
        Param* p = (Param*)data;
        p->webview_instance->DispatchKeyUpEvent(p->key_value);
        delete p;
      },
      p);
}

void WebView::DispatchCompositionUpdateEvent(const char* str, int size) {
  if (str) {
    LOG_DEBUG("WebView::DispatchCompositionUpdateEvent [%s]", str);
    webview_instance_->DispatchCompositionUpdateEvent(std::string(str, size));
  }
}

void WebView::DispatchCompositionEndEvent(const char* str, int size) {
  if (str) {
    LOG_DEBUG("WebView::DispatchCompositionEndEvent [%s]", str);
    webview_instance_->DispatchCompositionEndEvent(std::string(str, size));
  }
}

void WebView::ShowPanel() {
  LOG_DEBUG("WebView::ShowPanel()");
  if (!context_) {
    LOG_ERROR("Ecore_IMF_Context nullptr");
    return;
  }
  ecore_imf_context_input_panel_show(context_);
  ecore_imf_context_focus_in(context_);
}

void WebView::HidePanel() {
  LOG_DEBUG("WebView::HidePanel()");
  if (!context_) {
    LOG_ERROR("Ecore_IMF_Context nullptr");
    return;
  }
  ecore_imf_context_reset(context_);
  ecore_imf_context_focus_out(context_);
  ecore_imf_context_input_panel_hide(context_);
}

void WebView::SetSoftwareKeyboardContext(Ecore_IMF_Context* context) {
  context_ = context;

  webview_instance_->RegisterOnShowSoftwareKeyboardIfPossibleHandler(
      [this](LWE::WebContainer* v) { ShowPanel(); });

  webview_instance_->RegisterOnHideSoftwareKeyboardIfPossibleHandler(
      [this](LWE::WebContainer*) { HidePanel(); });
}

void WebView::ClearFocus() {
  LOG_DEBUG("WebView::ClearFocus()");
  HidePanel();
}

void WebView::SetDirection(int direction) {
  LOG_DEBUG("WebView::SetDirection direction: %d\n", direction);
  // TODO: implement this if necessary
}

void WebView::InitWebView() {
  if (webview_instance_ != nullptr) {
    webview_instance_->Destroy();
    webview_instance_ = nullptr;
  }
  if (!gpu_buffer_) {
    gpu_buffer_ = new FlutterDesktopGpuBuffer();
  }

  float scale_factor = 1;

  webview_instance_ = (LWE::WebContainer*)createWebViewInstance(
      0, 0, width_, height_, scale_factor, "SamsungOneUI", "ko-KR",
      "Asia/Seoul",
      [this]() -> LWE::WebContainer::ExternalImageInfo {
        std::lock_guard<std::mutex> lock(mutex_);
        LWE::WebContainer::ExternalImageInfo result;
        if (!candidate_surface_) {
          candidate_surface_ = tbm_pool_->AllocateBuffer();
        }
        if (candidate_surface_) {
          result.imageAddress = (void*)candidate_surface_->Surface();
        } else {
          result.imageAddress = nullptr;
        }
        return result;
      },
      [this](LWE::WebContainer* c, bool isRendered) {
        if (isRendered) {
          texture_registrar_->MarkTextureFrameAvailable(GetTextureId());
        }
      });
#ifndef TV_PROFILE
  auto settings = webview_instance_->GetSettings();
  settings.SetUserAgentString(
      "Mozilla/5.0 (like Gecko/54.0 Firefox/54.0) Mobile");
  webview_instance_->SetSettings(settings);
#endif
}

void WebView::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (!webview_instance_) {
    return;
  }
  const auto method_name = method_call.method_name();
  const auto& arguments = *method_call.arguments();

  LOG_DEBUG("WebView::HandleMethodCall : %s \n ", method_name.c_str());

  if (method_name.compare("loadUrl") == 0) {
    std::string url = ExtractStringFromMap(arguments, "url");
    webview_instance_->LoadURL(url);
    result->Success();
  } else if (method_name.compare("updateSettings") == 0) {
    if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
      auto settings = std::get<flutter::EncodableMap>(arguments);
      if (settings.size() > 0) {
        try {
          ApplySettings(settings);
        } catch (const std::invalid_argument& ex) {
          LOG_ERROR("[Exception] %s\n", ex.what());
          result->Error(ex.what());
          return;
        }
      }
    }
    result->Success();
  } else if (method_name.compare("canGoBack") == 0) {
    result->Success(flutter::EncodableValue(webview_instance_->CanGoBack()));
  } else if (method_name.compare("canGoForward") == 0) {
    result->Success(flutter::EncodableValue(webview_instance_->CanGoForward()));
  } else if (method_name.compare("goBack") == 0) {
    webview_instance_->GoBack();
    result->Success();
  } else if (method_name.compare("goForward") == 0) {
    webview_instance_->GoForward();
    result->Success();
  } else if (method_name.compare("reload") == 0) {
    webview_instance_->Reload();
    result->Success();
  } else if (method_name.compare("currentUrl") == 0) {
    result->Success(flutter::EncodableValue(webview_instance_->GetURL()));
  } else if (method_name.compare("evaluateJavascript") == 0) {
    if (std::holds_alternative<std::string>(arguments)) {
      std::string js_string = std::get<std::string>(arguments);
      webview_instance_->EvaluateJavaScript(
          js_string, [res = result.release()](std::string value) {
            LOG_DEBUG("value: %s\n", value.c_str());
            if (res) {
              res->Success(flutter::EncodableValue(value));
              delete res;
            }
          });
    } else {
      result->Error("Invalid Arguments", "Invalid Arguments");
    }
  } else if (method_name.compare("addJavascriptChannels") == 0) {
    if (std::holds_alternative<flutter::EncodableList>(arguments)) {
      auto name_list = std::get<flutter::EncodableList>(arguments);
      for (size_t i = 0; i < name_list.size(); i++) {
        if (std::holds_alternative<std::string>(name_list[i])) {
          RegisterJavaScriptChannelName(std::get<std::string>(name_list[i]));
        }
      }
    }
    result->Success();
  } else if (method_name.compare("removeJavascriptChannels") == 0) {
    if (std::holds_alternative<flutter::EncodableList>(arguments)) {
      auto name_list = std::get<flutter::EncodableList>(arguments);
      for (size_t i = 0; i < name_list.size(); i++) {
        if (std::holds_alternative<std::string>(name_list[i])) {
          webview_instance_->RemoveJavascriptInterface(
              std::get<std::string>(name_list[i]), "postMessage");
        }
      }
    }
    result->Success();
  } else if (method_name.compare("clearCache") == 0) {
    webview_instance_->ClearCache();
    result->Success();
  } else if (method_name.compare("getTitle") == 0) {
    result->Success(flutter::EncodableValue(webview_instance_->GetTitle()));
  } else if (method_name.compare("scrollTo") == 0) {
    int x = ExtractIntFromMap(arguments, "x");
    int y = ExtractIntFromMap(arguments, "y");
    webview_instance_->ScrollTo(x, y);
    result->Success();
  } else if (method_name.compare("scrollBy") == 0) {
    int x = ExtractIntFromMap(arguments, "x");
    int y = ExtractIntFromMap(arguments, "y");
    webview_instance_->ScrollBy(x, y);
    result->Success();
  } else if (method_name.compare("getScrollX") == 0) {
    result->Success(flutter::EncodableValue(webview_instance_->GetScrollX()));
  } else if (method_name.compare("getScrollY") == 0) {
    result->Success(flutter::EncodableValue(webview_instance_->GetScrollY()));
  } else {
    result->NotImplemented();
  }
}

void WebView::HandleCookieMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (webview_instance_ == nullptr) {
    result->Error("Not Webview created");
    return;
  }

  const auto method_name = method_call.method_name();

  LOG_DEBUG("WebView::HandleMethodCall : %s \n ", method_name.c_str());

  if (method_name.compare("clearCookies") == 0) {
    LWE::CookieManager* cookie = LWE::CookieManager::GetInstance();
    cookie->ClearCookies();
    result->Success(flutter::EncodableValue(true));
  } else {
    result->NotImplemented();
  }
}

FlutterDesktopGpuBuffer* WebView::ObtainGpuBuffer(size_t width, size_t height) {
  if (!candidate_surface_) {
    if (!rendered_surface_) {
      return nullptr;
    } else {
      return gpu_buffer_;
    }
  }
  std::lock_guard<std::mutex> lock(mutex_);
  rendered_surface_ = candidate_surface_;
  candidate_surface_ = nullptr;
  if (gpu_buffer_) {
    gpu_buffer_->buffer = (void*)rendered_surface_->Surface();
    gpu_buffer_->width = width;
    gpu_buffer_->height = height;
  }
  return gpu_buffer_;
}

void WebView::DestructBuffer(void* buffer) {
  if (buffer) {
    BufferUnit* unit = tbm_pool_->Get((tbm_surface_h)buffer);
    if (unit) {
      tbm_pool_->Release(unit);
    }
  }
}
