// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview.h"

#include <app_common.h>
#include <flutter/standard_method_codec.h>
#include <flutter_texture_registrar.h>
#include <system_info.h>
#include <tbm_surface.h>

#include <stdexcept>
#include <variant>

#include "buffer_pool.h"
#include "log.h"
#include "lwe/LWEWebView.h"
#include "lwe/PlatformIntegrationData.h"
#include "webview_factory.h"

static constexpr size_t kBufferPoolSize = 5;

extern "C" size_t LWE_EXPORT createWebViewInstance(
    unsigned x, unsigned y, unsigned width, unsigned height,
    float devicePixelRatio, const char* defaultFontName, const char* locale,
    const char* timezoneID,
    const std::function<::LWE::WebContainer::ExternalImageInfo(void)>&
        prepareImageCb,
    const std::function<void(::LWE::WebContainer*, bool needsFlush)>& flushCb,
    bool useSWBackend);

class NavigationRequestResult
    : public flutter::MethodResult<flutter::EncodableValue> {
 public:
  NavigationRequestResult(std::string url, WebView* webview)
      : url_(url), webview_(webview) {}

  void SuccessInternal(const flutter::EncodableValue* should_load) override {
    if (std::holds_alternative<bool>(*should_load)) {
      if (std::get<bool>(*should_load)) {
        LoadUrl();
      }
    }
  }

  void ErrorInternal(const std::string& error_code,
                     const std::string& error_message,
                     const flutter::EncodableValue* error_details) override {
    LOG_ERROR("The request unexpectedly completed with an error.");
  }

  void NotImplementedInternal() override {
    LOG_ERROR("The target method was unexpectedly unimplemented.");
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

enum class ResourceErrorType {
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
  switch (ResourceErrorType(error_code)) {
    case ResourceErrorType::AuthenticationError:
      return "authentication";
    case ResourceErrorType::BadURLError:
      return "badUrl";
    case ResourceErrorType::ConnectError:
      return "connect";
    case ResourceErrorType::FailedSSLHandshakeError:
      return "failedSslHandshake";
    case ResourceErrorType::FileError:
      return "file";
    case ResourceErrorType::FileNotFoundError:
      return "fileNotFound";
    case ResourceErrorType::HostLookupError:
      return "hostLookup";
    case ResourceErrorType::IOError:
      return "io";
    case ResourceErrorType::ProxyAuthenticationError:
      return "proxyAuthentication";
    case ResourceErrorType::RedirectLoopError:
      return "redirectLoop";
    case ResourceErrorType::TimeoutError:
      return "timeout";
    case ResourceErrorType::TooManyRequestError:
      return "tooManyRequests";
    case ResourceErrorType::UnknownError:
      return "unknown";
    case ResourceErrorType::UnsupportedAuthSchemeError:
      return "unsupportedAuthScheme";
    case ResourceErrorType::UnsupportedSchemeError:
      return "unsupportedScheme";
    default:
      LOG_ERROR("Unknown error type: %d", error_code);
      return std::to_string(error_code);
  }
}

template <typename T>
static bool GetValueFromEncodableMap(const flutter::EncodableValue* arguments,
                                     std::string key, T* out) {
  if (auto* map = std::get_if<flutter::EncodableMap>(arguments)) {
    auto iter = map->find(flutter::EncodableValue(key));
    if (iter != map->end() && !iter->second.IsNull()) {
      if (auto* value = std::get_if<T>(&iter->second)) {
        *out = *value;
        return true;
      }
    }
  }
  return false;
}

static bool IsRunningOnEmulator() {
  bool result = false;
  char* value = nullptr;
  int ret = system_info_get_platform_string(
      "http://tizen.org/system/model_name", &value);
  if (ret == SYSTEM_INFO_ERROR_NONE && strcmp(value, "Emulator") == 0) {
    result = true;
  }
  if (value) {
    free(value);
  }
  return result;
}

WebView::WebView(flutter::PluginRegistrar* registrar, int view_id,
                 flutter::TextureRegistrar* texture_registrar, double width,
                 double height, const flutter::EncodableValue& params)
    : PlatformView(registrar, view_id, nullptr),
      texture_registrar_(texture_registrar),
      width_(width),
      height_(height) {
  use_sw_backend_ = IsRunningOnEmulator();
  if (use_sw_backend_) {
    tbm_pool_ = std::make_unique<SingleBufferPool>(width, height);
  } else {
    tbm_pool_ = std::make_unique<BufferPool>(width, height, kBufferPoolSize);
  }

  texture_variant_ =
      std::make_unique<flutter::TextureVariant>(flutter::GpuBufferTexture(
          [this](size_t width,
                 size_t height) -> const FlutterDesktopGpuBuffer* {
            return ObtainGpuBuffer(width, height);
          }));
  SetTextureId(texture_registrar_->RegisterTexture(texture_variant_.get()));

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
  if (!GetValueFromEncodableMap(&params, "initialUrl", &url)) {
    url = "about:blank";
  }

  int color;
  if (GetValueFromEncodableMap(&params, "backgroundColor", &color)) {
    LWE::Settings settings = webview_instance_->GetSettings();
    settings.SetBaseBackgroundColor(color >> 16 & 0xff, color >> 8 & 0xff,
                                    color & 0xff, color >> 24 & 0xff);
    webview_instance_->SetSettings(settings);
  }

  flutter::EncodableMap settings;
  if (GetValueFromEncodableMap(&params, "settings", &settings)) {
    ApplySettings(settings);
  }

  flutter::EncodableList names;
  if (GetValueFromEncodableMap(&params, "javascriptChannelNames", &names)) {
    for (flutter::EncodableValue name : names) {
      if (std::holds_alternative<std::string>(name)) {
        RegisterJavaScriptChannelName(std::get<std::string>(name));
      }
    }
  }

  // TODO: Implement autoMediaPlaybackPolicy.

  std::string user_agent;
  if (GetValueFromEncodableMap(&params, "userAgent", &user_agent)) {
    LWE::Settings settings = webview_instance_->GetSettings();
    settings.SetUserAgentString(user_agent);
    webview_instance_->SetSettings(settings);
  }

  webview_instance_->RegisterOnPageStartedHandler(
      [this](LWE::WebContainer* container, const std::string& url) {
        flutter::EncodableMap args = {
            {flutter::EncodableValue("url"), flutter::EncodableValue(url)}};
        channel_->InvokeMethod("onPageStarted",
                               std::make_unique<flutter::EncodableValue>(args));
      });
  webview_instance_->RegisterOnPageLoadedHandler(
      [this](LWE::WebContainer* container, const std::string& url) {
        flutter::EncodableMap args = {
            {flutter::EncodableValue("url"), flutter::EncodableValue(url)}};
        channel_->InvokeMethod("onPageFinished",
                               std::make_unique<flutter::EncodableValue>(args));
      });
  webview_instance_->RegisterOnProgressChangedHandler(
      [this](LWE::WebContainer* container, int progress) {
        if (!has_progress_tracking_) {
          return;
        }
        flutter::EncodableMap args = {{flutter::EncodableValue("progress"),
                                       flutter::EncodableValue(progress)}};
        channel_->InvokeMethod("onProgress",
                               std::make_unique<flutter::EncodableValue>(args));
      });
  webview_instance_->RegisterOnReceivedErrorHandler(
      [this](LWE::WebContainer* container, LWE::ResourceError error) {
        flutter::EncodableMap args = {
            {flutter::EncodableValue("errorCode"),
             flutter::EncodableValue(error.GetErrorCode())},
            {flutter::EncodableValue("description"),
             flutter::EncodableValue(error.GetDescription())},
            {flutter::EncodableValue("errorType"),
             flutter::EncodableValue(ErrorCodeToString(error.GetErrorCode()))},
            {flutter::EncodableValue("failingUrl"),
             flutter::EncodableValue(error.GetUrl())},
        };
        channel_->InvokeMethod("onWebResourceError",
                               std::make_unique<flutter::EncodableValue>(args));
      });
  webview_instance_->RegisterShouldOverrideUrlLoadingHandler(
      [this](LWE::WebContainer* view, const std::string& url) -> bool {
        if (!has_navigation_delegate_) {
          return false;
        }
        flutter::EncodableMap args = {
            {flutter::EncodableValue("url"), flutter::EncodableValue(url)},
            {flutter::EncodableValue("isForMainFrame"),
             flutter::EncodableValue(true)},
        };
        auto result = std::make_unique<NavigationRequestResult>(url, this);
        channel_->InvokeMethod("navigationRequest",
                               std::make_unique<flutter::EncodableValue>(args),
                               std::move(result));
        return true;
      });

  webview_instance_->LoadURL(url);
}

void WebView::ApplySettings(const flutter::EncodableMap& settings) {
  for (const auto& [key, value] : settings) {
    if (std::holds_alternative<std::string>(key)) {
      std::string string_key = std::get<std::string>(key);
      if (string_key == "jsMode") {
        // NOTE: Not supported by LWE on Tizen.
      } else if (string_key == "hasNavigationDelegate") {
        if (std::holds_alternative<bool>(value)) {
          has_navigation_delegate_ = std::get<bool>(value);
        }
      } else if (string_key == "hasProgressTracking") {
        if (std::holds_alternative<bool>(value)) {
          has_progress_tracking_ = std::get<bool>(value);
        }
      } else if (string_key == "debuggingEnabled") {
        // NOTE: Not supported by LWE on Tizen.
      } else if (string_key == "gestureNavigationEnabled") {
        // NOTE: Not supported by LWE on Tizen.
      } else if (string_key == "allowsInlineMediaPlayback") {
        // no-op inline media playback is always allowed on Tizen.
      } else if (string_key == "userAgent") {
        if (std::holds_alternative<std::string>(value)) {
          LWE::Settings settings = webview_instance_->GetSettings();
          settings.SetUserAgentString(std::get<std::string>(value));
          webview_instance_->SetSettings(settings);
        }
      } else if (string_key == "zoomEnabled") {
        // NOTE: Not supported by LWE on Tizen.
      } else {
        LOG_WARN("Unknown settings key: %s", string_key.c_str());
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
  LOG_DEBUG("Register a JavaScript channel: %s", name.c_str());

  auto on_message = [this, name](const std::string& message) -> std::string {
    LOG_DEBUG("JavaScript channel message: %s", message.c_str());
    flutter::EncodableMap args = {
        {flutter::EncodableValue("channel"), flutter::EncodableValue(name)},
        {flutter::EncodableValue("message"), flutter::EncodableValue(message)},
    };
    channel_->InvokeMethod("javascriptChannelMessage",
                           std::make_unique<flutter::EncodableValue>(args));
    return "success";
  };
  webview_instance_->AddJavaScriptInterface(name, "postMessage", on_message);
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
}

void WebView::Resize(double width, double height) {
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
    LOG_WARN("Unknown touch event type: %d", type);
  }
}

static LWE::KeyValue KeyToKeyValue(const std::string& key,
                                   bool is_shift_pressed) {
  if (key == "Left") {
    return LWE::KeyValue::ArrowLeftKey;
  } else if (key == "Right") {
    return LWE::KeyValue::ArrowRightKey;
  } else if (key == "Up") {
    return LWE::KeyValue::ArrowUpKey;
  } else if (key == "Down") {
    return LWE::KeyValue::ArrowDownKey;
  } else if (key == "space") {
    return LWE::KeyValue::SpaceKey;
  } else if (key == "Select") {
    return LWE::KeyValue::EnterKey;
  } else if (key == "Return") {
    return LWE::KeyValue::EnterKey;
  } else if (key == "Tab") {
    return LWE::KeyValue::TabKey;
  } else if (key == "BackSpace") {
    return LWE::KeyValue::BackspaceKey;
  } else if (key == "Escape") {
    return LWE::KeyValue::EscapeKey;
  } else if (key == "Delete") {
    return LWE::KeyValue::DeleteKey;
  } else if (key == "at") {
    return LWE::KeyValue::AtMarkKey;
  } else if (key == "minus") {
    if (is_shift_pressed) {
      return LWE::KeyValue::UnderScoreMarkKey;
    } else {
      return LWE::KeyValue::MinusMarkKey;
    }
  } else if (key == "equal") {
    if (is_shift_pressed) {
      return LWE::KeyValue::PlusMarkKey;
    } else {
      return LWE::KeyValue::EqualitySignKey;
    }
  } else if (key == "bracketleft") {
    if (is_shift_pressed) {
      return LWE::KeyValue::LeftCurlyBracketMarkKey;
    } else {
      return LWE::KeyValue::LeftSquareBracketKey;
    }
  } else if (key == "bracketright") {
    if (is_shift_pressed) {
      return LWE::KeyValue::RightCurlyBracketMarkKey;
    } else {
      return LWE::KeyValue::RightSquareBracketKey;
    }
  } else if (key == "semicolon") {
    if (is_shift_pressed) {
      return LWE::KeyValue::ColonMarkKey;
    } else {
      return LWE::KeyValue::SemiColonMarkKey;
    }
  } else if (key == "apostrophe") {
    if (is_shift_pressed) {
      return LWE::KeyValue::DoubleQuoteMarkKey;
    } else {
      return LWE::KeyValue::SingleQuoteMarkKey;
    }
  } else if (key == "comma") {
    if (is_shift_pressed) {
      return LWE::KeyValue::LessThanMarkKey;
    } else {
      return LWE::KeyValue::CommaMarkKey;
    }
  } else if (key == "period") {
    if (is_shift_pressed) {
      return LWE::KeyValue::GreaterThanSignKey;
    } else {
      return LWE::KeyValue::PeriodKey;
    }
  } else if (key == "slash") {
    if (is_shift_pressed) {
      return LWE::KeyValue::QuestionMarkKey;
    } else {
      return LWE::KeyValue::SlashKey;
    }
  } else if (key.length() == 1) {
    const char ch = key.at(0);
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
      return LWE::KeyValue(LWE::KeyValue::Digit0Key + ch - '0');
    } else if (ch >= 'a' && ch <= 'z') {
      if (is_shift_pressed) {
        return LWE::KeyValue(LWE::KeyValue::LowerAKey + ch - 'a' - 32);
      } else {
        return LWE::KeyValue(LWE::KeyValue::LowerAKey + ch - 'a');
      }
    } else if (ch >= 'A' && ch <= 'Z') {
      if (is_shift_pressed) {
        return LWE::KeyValue(LWE::KeyValue::AKey + ch - 'A' + 32);
      } else {
        return LWE::KeyValue(LWE::KeyValue::AKey + ch - 'A');
      }
    }
  } else if (key == "XF86AudioRaiseVolume") {
    return LWE::KeyValue::TVVolumeUpKey;
  } else if (key == "XF86AudioLowerVolume") {
    return LWE::KeyValue::TVVolumeDownKey;
  } else if (key == "XF86AudioMute") {
    return LWE::KeyValue::TVMuteKey;
  } else if (key == "XF86RaiseChannel") {
    return LWE::KeyValue::TVChannelUpKey;
  } else if (key == "XF86LowerChannel") {
    return LWE::KeyValue::TVChannelDownKey;
  } else if (key == "XF86AudioRewind") {
    return LWE::KeyValue::MediaTrackPreviousKey;
  } else if (key == "XF86AudioNext") {
    return LWE::KeyValue::MediaTrackNextKey;
  } else if (key == "XF86AudioPause") {
    return LWE::KeyValue::MediaPauseKey;
  } else if (key == "XF86AudioRecord") {
    return LWE::KeyValue::MediaRecordKey;
  } else if (key == "XF86AudioPlay") {
    return LWE::KeyValue::MediaPlayKey;
  } else if (key == "XF86AudioStop") {
    return LWE::KeyValue::MediaStopKey;
  } else if (key == "XF86Info") {
    return LWE::KeyValue::TVInfoKey;
  } else if (key == "XF86Back") {
    return LWE::KeyValue::TVReturnKey;
  } else if (key == "XF86Red") {
    return LWE::KeyValue::TVRedKey;
  } else if (key == "XF86Green") {
    return LWE::KeyValue::TVGreenKey;
  } else if (key == "XF86Yellow") {
    return LWE::KeyValue::TVYellowKey;
  } else if (key == "XF86Blue") {
    return LWE::KeyValue::TVBlueKey;
  } else if (key == "XF86SysMenu") {
    return LWE::KeyValue::TVMenuKey;
  } else if (key == "XF86Home") {
    return LWE::KeyValue::TVHomeKey;
  } else if (key == "XF86Exit") {
    return LWE::KeyValue::TVExitKey;
  } else if (key == "XF86PreviousChannel") {
    return LWE::KeyValue::TVPreviousChannel;
  } else if (key == "XF86ChannelList") {
    return LWE::KeyValue::TVChannelList;
  } else if (key == "XF86ChannelGuide") {
    return LWE::KeyValue::TVChannelGuide;
  } else if (key == "XF86SimpleMenu") {
    return LWE::KeyValue::TVSimpleMenu;
  } else if (key == "XF86EManual") {
    return LWE::KeyValue::TVEManual;
  } else if (key == "XF86ExtraApp") {
    return LWE::KeyValue::TVExtraApp;
  } else if (key == "XF86Search") {
    return LWE::KeyValue::TVSearch;
  } else if (key == "XF86PictureSize") {
    return LWE::KeyValue::TVPictureSize;
  } else if (key == "XF86Sleep") {
    return LWE::KeyValue::TVSleep;
  } else if (key == "XF86Caption") {
    return LWE::KeyValue::TVCaption;
  } else if (key == "XF86More") {
    return LWE::KeyValue::TVMore;
  } else if (key == "XF86BTVoice") {
    return LWE::KeyValue::TVBTVoice;
  } else if (key == "XF86Color") {
    return LWE::KeyValue::TVColor;
  } else if (key == "XF86PlayBack") {
    return LWE::KeyValue::TVPlayBack;
  }
  LOG_WARN("Unknown key name: %s", key.c_str());
  return LWE::KeyValue::UnidentifiedKey;
}

void WebView::SendKey(const char* key, const char* string, const char* compose,
                      uint32_t modifiers, uint32_t scan_code, bool is_down) {
  if (!IsFocused()) {
    return;
  }

  bool is_shift_pressed = modifiers & 1;

  struct Param {
    LWE::WebContainer* webview_instance;
    LWE::KeyValue key_value;
    bool is_down;
  };

  Param* param = new Param();
  param->webview_instance = webview_instance_;
  param->key_value = KeyToKeyValue(key, is_shift_pressed);
  param->is_down = is_down;

  webview_instance_->AddIdleCallback(
      [](void* data) {
        Param* param = reinterpret_cast<Param*>(data);
        if (param->is_down) {
          param->webview_instance->DispatchKeyDownEvent(param->key_value);
          param->webview_instance->DispatchKeyPressEvent(param->key_value);
        } else {
          param->webview_instance->DispatchKeyUpEvent(param->key_value);
        }
        delete param;
      },
      param);
}

void WebView::SetDirection(int direction) {
  // TODO: Implement if necessary.
}

void WebView::InitWebView() {
  if (webview_instance_) {
    webview_instance_->Destroy();
    webview_instance_ = nullptr;
  }

  float pixel_ratio = 1.0;

  auto on_prepare_image = [this]() -> LWE::WebContainer::ExternalImageInfo {
    std::lock_guard<std::mutex> lock(mutex_);
    LWE::WebContainer::ExternalImageInfo result;
    if (!working_surface_) {
      working_surface_ = tbm_pool_->GetAvailableBuffer();
    }
    if (working_surface_) {
      result.imageAddress = working_surface_->Surface();
    } else {
      result.imageAddress = nullptr;
    }
    return result;
  };
  auto on_flush = [this](LWE::WebContainer* container, bool is_rendered) {
    if (is_rendered) {
      std::lock_guard<std::mutex> lock(mutex_);
      if (candidate_surface_) {
        tbm_pool_->Release(candidate_surface_);
        candidate_surface_ = nullptr;
      }
      candidate_surface_ = working_surface_;
      working_surface_ = nullptr;
      texture_registrar_->MarkTextureFrameAvailable(GetTextureId());
    }
  };

  webview_instance_ =
      reinterpret_cast<LWE::WebContainer*>(createWebViewInstance(
          0, 0, width_, height_, pixel_ratio, "SamsungOneUI", "ko-KR",
          "Asia/Seoul", on_prepare_image, on_flush, use_sw_backend_));

#ifndef TV_PROFILE
  LWE::Settings settings = webview_instance_->GetSettings();
  settings.SetUserAgentString(
      "Mozilla/5.0 (like Gecko/54.0 Firefox/54.0) Mobile");
  webview_instance_->SetSettings(settings);
#endif
}

void WebView::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (!webview_instance_) {
    result->Error("Invalid operation",
                  "The webview instance has not been initialized.");
    return;
  }

  const std::string& method_name = method_call.method_name();
  const flutter::EncodableValue* arguments = method_call.arguments();

  LOG_DEBUG("Handle a method call: %s", method_name.c_str());

  if (method_name == "loadUrl") {
    std::string url;
    if (GetValueFromEncodableMap(arguments, "url", &url)) {
      webview_instance_->LoadURL(url);
      result->Success();
    } else {
      result->Error("Invalid argument", "No url provided.");
    }
  } else if (method_name == "updateSettings") {
    const auto* settings = std::get_if<flutter::EncodableMap>(arguments);
    if (settings) {
      ApplySettings(*settings);
    }
    result->Success();
  } else if (method_name == "canGoBack") {
    result->Success(flutter::EncodableValue(webview_instance_->CanGoBack()));
  } else if (method_name == "canGoForward") {
    result->Success(flutter::EncodableValue(webview_instance_->CanGoForward()));
  } else if (method_name == "goBack") {
    webview_instance_->GoBack();
    result->Success();
  } else if (method_name == "goForward") {
    webview_instance_->GoForward();
    result->Success();
  } else if (method_name == "reload") {
    webview_instance_->Reload();
    result->Success();
  } else if (method_name == "currentUrl") {
    result->Success(flutter::EncodableValue(webview_instance_->GetURL()));
  } else if (method_name == "evaluateJavascript" ||
             method_name == "runJavascriptReturningResult" ||
             method_name == "runJavascript") {
    const auto* javascript = std::get_if<std::string>(arguments);
    if (javascript) {
      bool should_return = method_name != "runJavascript";
      auto on_result = [result = result.release(),
                        should_return](std::string value) {
        LOG_DEBUG("JavaScript evaluation result: %s", value.c_str());
        if (result) {
          if (should_return) {
            result->Success(flutter::EncodableValue(value));
          } else {
            result->Success();
          }
          delete result;
        }
      };
      webview_instance_->EvaluateJavaScript(*javascript, on_result);
    } else {
      result->Error("Invalid argument", "The argument must be a string.");
    }
  } else if (method_name == "addJavascriptChannels") {
    const auto* channels = std::get_if<flutter::EncodableList>(arguments);
    if (channels) {
      for (flutter::EncodableValue channel : *channels) {
        if (std::holds_alternative<std::string>(channel)) {
          RegisterJavaScriptChannelName(std::get<std::string>(channel));
        }
      }
    }
    result->Success();
  } else if (method_name == "removeJavascriptChannels") {
    const auto* channels = std::get_if<flutter::EncodableList>(arguments);
    if (channels) {
      for (flutter::EncodableValue channel : *channels) {
        if (std::holds_alternative<std::string>(channel)) {
          webview_instance_->RemoveJavascriptInterface(
              std::get<std::string>(channel), "postMessage");
        }
      }
    }
    result->Success();
  } else if (method_name == "clearCache") {
    webview_instance_->ClearCache();
    result->Success();
  } else if (method_name == "getTitle") {
    result->Success(flutter::EncodableValue(webview_instance_->GetTitle()));
  } else if (method_name == "scrollTo") {
    int x = 0, y = 0;
    if (GetValueFromEncodableMap(arguments, "x", &x) &&
        GetValueFromEncodableMap(arguments, "y", &y)) {
      webview_instance_->ScrollTo(x, y);
      result->Success();
    } else {
      result->Error("Invalid argument", "No x or y provided.");
    }
  } else if (method_name == "scrollBy") {
    int x = 0, y = 0;
    if (GetValueFromEncodableMap(arguments, "x", &x) &&
        GetValueFromEncodableMap(arguments, "y", &y)) {
      webview_instance_->ScrollBy(x, y);
      result->Success();
    } else {
      result->Error("Invalid argument", "No x or y provided.");
    }
  } else if (method_name == "getScrollX") {
    result->Success(flutter::EncodableValue(webview_instance_->GetScrollX()));
  } else if (method_name == "getScrollY") {
    result->Success(flutter::EncodableValue(webview_instance_->GetScrollY()));
  } else if (method_name == "loadFlutterAsset") {
    const auto* key = std::get_if<std::string>(arguments);
    if (key) {
      char* res_path = app_get_resource_path();
      if (res_path) {
        std::string url =
            std::string("file://") + res_path + "flutter_assets/" + *key;
        free(res_path);
        webview_instance_->LoadURL(url);
        result->Success();
      } else {
        result->Error("Operation failed",
                      "Could not get the flutter_assets path.");
      }
    } else {
      result->Error("Invalid argument", "The argument must be a string.");
    }
  } else if (method_name == "loadHtmlString") {
    std::string html, base_url;
    if (!GetValueFromEncodableMap(arguments, "html", &html)) {
      result->Error("Invalid argument", "No html provided.");
      return;
    }
    if (GetValueFromEncodableMap(arguments, "baseUrl", &base_url)) {
      LOG_WARN("loadHtmlString: baseUrl is not supported and will be ignored.");
    }
    webview_instance_->LoadData(html);
    result->Success();
  } else if (method_name == "loadFile") {
    const auto* file_path = std::get_if<std::string>(arguments);
    if (file_path) {
      std::string url = std::string("file://") + *file_path;
      webview_instance_->LoadURL(url);
      result->Success();
    } else {
      result->Error("Invalid argument", "The argument must be a string.");
    }
  } else if (method_name == "loadRequest") {
    result->NotImplemented();
  } else if (method_name == "setCookie") {
    result->NotImplemented();
  } else {
    result->NotImplemented();
  }
}

void WebView::HandleCookieMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (!webview_instance_) {
    result->Error("Invalid operation",
                  "The webview instance has not been initialized.");
    return;
  }

  const std::string& method_name = method_call.method_name();

  if (method_name == "clearCookies") {
    LWE::CookieManager* cookie = LWE::CookieManager::GetInstance();
    cookie->ClearCookies();
    result->Success(flutter::EncodableValue(true));
  } else {
    result->NotImplemented();
  }
}

FlutterDesktopGpuBuffer* WebView::ObtainGpuBuffer(size_t width, size_t height) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!candidate_surface_) {
    if (rendered_surface_) {
      return rendered_surface_->GpuBuffer();
    }
    return nullptr;
  }
  if (rendered_surface_ && rendered_surface_->IsUsed()) {
    tbm_pool_->Release(rendered_surface_);
  }
  rendered_surface_ = candidate_surface_;
  candidate_surface_ = nullptr;
  return rendered_surface_->GpuBuffer();
}
