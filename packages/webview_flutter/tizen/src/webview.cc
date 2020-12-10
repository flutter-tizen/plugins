
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <flutter/standard_message_codec.h>
#include <flutter_platform_view.h>
#include <flutter_texture_registrar.h>
#include "webview_flutter_tizen_plugin.h"
#include "webview.h"

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "log.h"
#include "lwe/LWEWebView.h"
#include "lwe/PlatformIntegrationData.h"
#include "webview_factory.h"

#include <Ecore_Input_Evas.h>
#include <Ecore_IMF_Evas.h>

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
                 FlutterTextureRegistrar* textureRegistrar, double width,
                 double height, const std::string initialUrl)
    : PlatformView(registrar, viewId),
      tbmSurface_(nullptr),
      textureRegistrar_(textureRegistrar),
      webViewInstance_(nullptr),
      currentUrl_(initialUrl),
      width_(width),
      height_(height) {
  SetTextureId(FlutterRegisterExternalTexture(textureRegistrar_));
  InitWebView();
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          GetPluginRegistrar()->messenger(), GetChannelName(),
          &flutter::StandardMethodCodec::GetInstance());
  channel->SetMethodCallHandler(
      [webview = this](const auto& call, auto result) {
        webview->HandleMethodCall(call, std::move(result));
      });
  webViewInstance_->LoadURL(currentUrl_);
}

WebView::~WebView() { Dispose(); }

std::string WebView::GetChannelName() {
  return "plugins.flutter.io/webview_" + std::to_string(GetViewId());
}

void WebView::Dispose() {
  FlutterUnregisterExternalTexture(textureRegistrar_, GetTextureId());

  // Should move to starfish
  // glDeleteFramebuffers(1, &fbo_id_);
  webViewInstance_->Destroy();
  webViewInstance_ = nullptr;
}

void WebView::Resize(double width, double height) {
  LOG_DEBUG("WebView::Resize width: %f height: %f \n", width, height);
}

void WebView::Touch(int type, int button, double x, double y, double dx,
                    double dy) {
  // LOG_DEBUG(
  //   "Widget::Native::Touch type[%s],btn[%d],x[%f],y[%f],dx[%f],dy[%f]",
  //           type == 0 ? "DownEvent" : type == 1 ? "MoveEvent" : "UpEvent",
  //           button, x, y, dx, dy);
  if (type == 0) {  // down event
    webViewInstance_->DispatchMouseDownEvent(
        LWE::MouseButtonValue::LeftButton,
        LWE::MouseButtonsValue::LeftButtonDown, x, y);
    isMouseLButtonDown_ = true;
  } else if (type == 1) {  // move event
    webViewInstance_->DispatchMouseMoveEvent(
        isMouseLButtonDown_ ? LWE::MouseButtonValue::LeftButton
                            : LWE::MouseButtonValue::NoButton,
        isMouseLButtonDown_ ? LWE::MouseButtonsValue::LeftButtonDown
                            : LWE::MouseButtonsValue::NoButtonDown,
        x, y);
  } else if (type == 2) {  // up event
    webViewInstance_->DispatchMouseUpEvent(LWE::MouseButtonValue::NoButton,
                                           LWE::MouseButtonsValue::NoButtonDown,
                                           x, y);
    isMouseLButtonDown_ = false;
  } else {
    // TODO: Not implemented
  }
}

static LWE::KeyValue EcoreEventKeyToKeyValue(const char* ecoreKeyString,
                                             bool isShiftPressed) {
  if (strcmp("Left", ecoreKeyString) == 0) {
    return LWE::KeyValue::ArrowLeftKey;
  } else if (strcmp("Right", ecoreKeyString) == 0) {
    return LWE::KeyValue::ArrowRightKey;
  } else if (strcmp("Up", ecoreKeyString) == 0) {
    return LWE::KeyValue::ArrowUpKey;
  } else if (strcmp("Down", ecoreKeyString) == 0) {
    return LWE::KeyValue::ArrowDownKey;
  } else if (strcmp("space", ecoreKeyString) == 0) {
    return LWE::KeyValue::SpaceKey;
  } else if (strcmp("Return", ecoreKeyString) == 0) {
    return LWE::KeyValue::EnterKey;
  } else if (strcmp("Tab", ecoreKeyString) == 0) {
    return LWE::KeyValue::TabKey;
  } else if (strcmp("BackSpace", ecoreKeyString) == 0) {
    return LWE::KeyValue::BackspaceKey;
  } else if (strcmp("Escape", ecoreKeyString) == 0) {
    return LWE::KeyValue::EscapeKey;
  } else if (strcmp("Delete", ecoreKeyString) == 0) {
    return LWE::KeyValue::DeleteKey;
  } else if (strcmp("at", ecoreKeyString) == 0) {
    return LWE::KeyValue::AtMarkKey;
  } else if (strcmp("minus", ecoreKeyString) == 0) {
    if (isShiftPressed) {
      return LWE::KeyValue::UnderScoreMarkKey;
    } else {
      return LWE::KeyValue::MinusMarkKey;
    }
  } else if (strcmp("equal", ecoreKeyString) == 0) {
    if (isShiftPressed) {
      return LWE::KeyValue::PlusMarkKey;
    } else {
      return LWE::KeyValue::EqualitySignKey;
    }
  } else if (strcmp("bracketleft", ecoreKeyString) == 0) {
    if (isShiftPressed) {
      return LWE::KeyValue::LeftCurlyBracketMarkKey;
    } else {
      return LWE::KeyValue::LeftSquareBracketKey;
    }
  } else if (strcmp("bracketright", ecoreKeyString) == 0) {
    if (isShiftPressed) {
      return LWE::KeyValue::RightCurlyBracketMarkKey;
    } else {
      return LWE::KeyValue::RightSquareBracketKey;
    }
  } else if (strcmp("semicolon", ecoreKeyString) == 0) {
    if (isShiftPressed) {
      return LWE::KeyValue::ColonMarkKey;
    } else {
      return LWE::KeyValue::SemiColonMarkKey;
    }
  } else if (strcmp("apostrophe", ecoreKeyString) == 0) {
    if (isShiftPressed) {
      return LWE::KeyValue::DoubleQuoteMarkKey;
    } else {
      return LWE::KeyValue::SingleQuoteMarkKey;
    }
  } else if (strcmp("comma", ecoreKeyString) == 0) {
    if (isShiftPressed) {
      return LWE::KeyValue::LessThanMarkKey;
    } else {
      return LWE::KeyValue::CommaMarkKey;
    }
  } else if (strcmp("period", ecoreKeyString) == 0) {
    if (isShiftPressed) {
      return LWE::KeyValue::GreaterThanSignKey;
    } else {
      return LWE::KeyValue::PeriodKey;
    }
  } else if (strcmp("slash", ecoreKeyString) == 0) {
    if (isShiftPressed) {
      return LWE::KeyValue::QuestionMarkKey;
    } else {
      return LWE::KeyValue::SlashKey;
    }
  } else if (strlen(ecoreKeyString) == 1) {
    char ch = ecoreKeyString[0];
    if (ch >= '0' && ch <= '9') {
      if (isShiftPressed) {
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
      return (LWE::KeyValue)(LWE::KeyValue::LowerAKey + ch - 'a');
    } else if (ch >= 'A' && ch <= 'Z') {
      return (LWE::KeyValue)(LWE::KeyValue::AKey + ch - 'A');
    }
  } else if (strcmp("XF86AudioRaiseVolume", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVVolumeUpKey;
  } else if (strcmp("XF86AudioLowerVolume", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVVolumeDownKey;
  } else if (strcmp("XF86AudioMute", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVMuteKey;
  } else if (strcmp("XF86RaiseChannel", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVChannelUpKey;
  } else if (strcmp("XF86LowerChannel", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVChannelDownKey;
  } else if (strcmp("XF86AudioRewind", ecoreKeyString) == 0) {
    return LWE::KeyValue::MediaTrackPreviousKey;
  } else if (strcmp("XF86AudioNext", ecoreKeyString) == 0) {
    return LWE::KeyValue::MediaTrackNextKey;
  } else if (strcmp("XF86AudioPause", ecoreKeyString) == 0) {
    return LWE::KeyValue::MediaPauseKey;
  } else if (strcmp("XF86AudioRecord", ecoreKeyString) == 0) {
    return LWE::KeyValue::MediaRecordKey;
  } else if (strcmp("XF86AudioPlay", ecoreKeyString) == 0) {
    return LWE::KeyValue::MediaPlayKey;
  } else if (strcmp("XF86AudioStop", ecoreKeyString) == 0) {
    return LWE::KeyValue::MediaStopKey;
  } else if (strcmp("XF86Info", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVInfoKey;
  } else if (strcmp("XF86Back", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVReturnKey;
  } else if (strcmp("XF86Red", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVRedKey;
  } else if (strcmp("XF86Green", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVGreenKey;
  } else if (strcmp("XF86Yellow", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVYellowKey;
  } else if (strcmp("XF86Blue", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVBlueKey;
  } else if (strcmp("XF86SysMenu", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVMenuKey;
  } else if (strcmp("XF86Home", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVHomeKey;
  } else if (strcmp("XF86Exit", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVExitKey;
  } else if (strcmp("XF86PreviousChannel", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVPreviousChannel;
  } else if (strcmp("XF86ChannelList", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVChannelList;
  } else if (strcmp("XF86ChannelGuide", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVChannelGuide;
  } else if (strcmp("XF86SimpleMenu", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVSimpleMenu;
  } else if (strcmp("XF86EManual", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVEManual;
  } else if (strcmp("XF86ExtraApp", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVExtraApp;
  } else if (strcmp("XF86Search", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVSearch;
  } else if (strcmp("XF86PictureSize", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVPictureSize;
  } else if (strcmp("XF86Sleep", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVSleep;
  } else if (strcmp("XF86Caption", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVCaption;
  } else if (strcmp("XF86More", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVMore;
  } else if (strcmp("XF86BTVoice", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVBTVoice;
  } else if (strcmp("XF86Color", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVColor;
  } else if (strcmp("XF86PlayBack", ecoreKeyString) == 0) {
    return LWE::KeyValue::TVPlayBack;
  }

  LOG_DEBUG("WebViewEFL - unimplemented key %s\n", ecoreKeyString);
  return LWE::KeyValue::UnidentifiedKey;
}

void WebView::DispatchKeyDownEvent(Ecore_Event_Key* keyEvent) {
  std::string keyName = keyEvent->keyname;
  LOG_DEBUG("ECORE_EVENT_KEY_DOWN [%s, %d]\n", keyName.data(),
            (keyEvent->modifiers & 1) || (keyEvent->modifiers & 2));

  bool lastInputTimeWasZeroBefore = false;
  // if (webViewInstance_->m_lastInputTime == 0) {
  //   lastInputTimeWasZeroBefore = true;
  //   webViewInstance_->m_lastInputTime = Starfish::longTickCount();
  // }

  if (!IsFocused()) {
    LOG_DEBUG("ignore keydown because we dont have focus");
    return;
  }

#ifdef TV_PROFILE
  if ((strncmp(keyName.data(), "XF86Back", 8) == 0)) {
    keyName = "Escape";
  }
#endif

  if ((strcmp(keyName.data(), "XF86Exit") == 0) ||
      (strcmp(keyName.data(), "Select") == 0) ||
      (strcmp(keyName.data(), "Cancel") == 0)) {
    if (strcmp(keyName.data(), "Select") == 0) {
      webViewInstance_->AddIdleCallback(
          [](void* data) {
            LWE::WebContainer* self = (LWE::WebContainer*)data;
            LWE::KeyValue kv = LWE::KeyValue::EnterKey;
            self->DispatchKeyDownEvent(kv);
            self->DispatchKeyPressEvent(kv);
            self->DispatchKeyUpEvent(kv);
            // self->HideSoftwareKeyboardIfPossible();
          },
          webViewInstance_);
    } else {
      webViewInstance_->AddIdleCallback(
          [](void* data) {
            LWE::WebContainer* self = (LWE::WebContainer*)data;
            // self->HideSoftwareKeyboardIfPossible();
          },
          webViewInstance_);
    }
  }

  auto keyValue = EcoreEventKeyToKeyValue(keyName.data(), false);

  // if (keyValue >= LWE::KeyValue::ArrowDownKey &&
  //     keyValue <= LWE::KeyValue::ArrowRightKey) {
  //   int currentTimestamp = keyEvent->timestamp;
  //   if (currentTimestamp -
  //           g_arrowKeyDownTimestamp[keyValue - LWE::KeyValue::ArrowDownKey] <
  //       g_arrowKeyDownMinimumDelayInMS) {
  //     return;
  //   }
  //   g_arrowKeyDownTimestamp[keyValue - LWE::KeyValue::ArrowDownKey] =
  //       currentTimestamp;
  // }

  if (lastInputTimeWasZeroBefore) {
    webViewInstance_->DispatchKeyDownEvent(keyValue);
    webViewInstance_->DispatchKeyPressEvent(keyValue);
  } else {
    struct Param {
      LWE::WebContainer* webViewInstance;
      LWE::KeyValue keyValue;
    };
    Param* p = new Param();
    p->webViewInstance = webViewInstance_;
    p->keyValue = keyValue;

    webViewInstance_->AddIdleCallback(
        [](void* data) {
          Param* p = (Param*)data;
          p->webViewInstance->DispatchKeyDownEvent(p->keyValue);
          p->webViewInstance->DispatchKeyPressEvent(p->keyValue);
          delete p;
        },
        p);
  }
}

void WebView::DispatchKeyUpEvent(Ecore_Event_Key* keyEvent) {
  std::string keyName = keyEvent->keyname;
  LOG_DEBUG("ECORE_EVENT_KEY_UP [%s, %d]\n", keyName.data(),
            (keyEvent->modifiers & 1) || (keyEvent->modifiers & 2));

  if (!IsFocused()) {
    LOG_DEBUG("ignore keyup because we dont have focus");
    return;
  }

#ifdef TV_PROFILE
  if ((strncmp(keyName.data(), "XF86Back", 8) == 0)) {
    keyName = "Escape";
  }
#endif
  auto keyValue = EcoreEventKeyToKeyValue(keyName.data(), false);

  // if (keyValue >= LWE::KeyValue::ArrowDownKey &&
  //     keyValue <= LWE::KeyValue::ArrowRightKey) {
  //   g_arrowKeyDownTimestamp[keyValue - LWE::KeyValue::ArrowDownKey] = 0;
  // }

  struct Param {
    LWE::WebContainer* webView;
    LWE::KeyValue keyValue;
  };
  Param* p = new Param();
  p->webView = webViewInstance_;
  p->keyValue = keyValue;

  webViewInstance_->AddIdleCallback(
      [](void* data) {
        Param* p = (Param*)data;
        p->webView->DispatchKeyUpEvent(p->keyValue);
        delete p;
      },
      p);
}

void WebView::ClearFocus() { LOG_DEBUG("WebView::clearFocus \n"); }

void WebView::SetDirection(int direction) {
  LOG_DEBUG("WebView::SetDirection direction: %d\n", direction);
}

void WebView::InitWebView() {
  if (webViewInstance_ != nullptr) {
    webViewInstance_->Destroy();
    webViewInstance_ = nullptr;
  }
  float scaleFactor = 1;

  webViewInstance_ = (LWE::WebContainer*)LWE::WebView::Create(nullptr,0,0,width_, height_,scaleFactor, "SamsungOneUI", "ko-KR", "Asia/Seoul");
  webViewInstance_->RegisterPreRenderingHandler(
      [this]() -> LWE::WebContainer::RenderInfo {
        LWE::WebContainer::RenderInfo result;
        {
          // Should move to starfish
          // glBindFramebuffer(GL_FRAMEBUFFER, fbo_id_);

          tbmSurface_ = tbm_surface_create(width_, height_, TBM_FORMAT_ARGB8888);
          tbm_surface_info_s tbmSurfaceInfo;
          if (tbm_surface_map(tbmSurface_, TBM_SURF_OPTION_WRITE,
                              &tbmSurfaceInfo) == TBM_SURFACE_ERROR_NONE) {
            result.updatedBufferAddress = tbmSurfaceInfo.planes[0].ptr;
            result.bufferStride = tbmSurfaceInfo.planes[0].stride;

            // Should move to starfish

            // EGLint attribs[] = {EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE};
            // eglImage_ = g_eglCreateImageKHRProc(
            //     egl_display_, EGL_NO_CONTEXT, EGL_NATIVE_SURFACE_TIZEN,
            //     (void*)(intptr_t)tbmSurface_, attribs);
            // glGenTextures(1, &textureID_);
            // glBindTexture(GL_TEXTURE_2D, textureID_);
            // g_glEGLImageTargetTexture2DOESProc(GL_TEXTURE_2D, eglImage_);
            // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            //                        GL_TEXTURE_2D, textureID_, 0);
          }
        }
        return result;
      });

  webViewInstance_->RegisterOnRenderedHandler(
      [this](LWE::WebContainer* c, LWE::WebContainer::RenderResult r) {
        // Should move to starfish
        // glFlush();

        FlutterMarkExternalTextureFrameAvailable(textureRegistrar_,
                                                 GetTextureId(), tbmSurface_);
        // Should move to starfish
        // g_eglDestroyImageKHRProc(egl_display_, eglImage_);
        // eglImage_ = nullptr;
        // glDeleteTextures(1, &textureID_);
        tbm_surface_destroy(tbmSurface_);
        tbmSurface_ = nullptr;
      });
#ifndef TV_PROFILE
  auto settings = webViewInstance_->GetSettings();
  settings.SetUserAgentString(
      "Mozilla/5.0 (like Gecko/54.0 Firefox/54.0) Mobile");
  webViewInstance_->SetSettings(settings);
#endif
}

void WebView::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  if (webViewInstance_ == nullptr) return;

  const auto methodName = method_call.method_name();
  const auto& arguments = *method_call.arguments();

  LOG_DEBUG("HandleMethodCall : %s \n ", methodName.c_str());

  if (methodName.compare("loadUrl") == 0) {
    currentUrl_ = ExtractStringFromMap(arguments, "url");
    webViewInstance_->LoadURL(GetCurrentUrl());
    result->Success();
  } else if (methodName.compare("updateSettings") == 0) {
    result->NotImplemented();
  } else if (methodName.compare("canGoBack") == 0) {
    result->Success(flutter::EncodableValue(webViewInstance_->CanGoBack()));
  } else if (methodName.compare("canGoForward") == 0) {
    result->Success(flutter::EncodableValue(webViewInstance_->CanGoForward()));
  } else if (methodName.compare("goBack") == 0) {
    webViewInstance_->GoBack();
    result->Success();
  } else if (methodName.compare("goForward") == 0) {
    webViewInstance_->GoForward();
    result->Success();
  } else if (methodName.compare("reload") == 0) {
    webViewInstance_->Reload();
    result->Success();
  } else if (methodName.compare("currentUrl") == 0) {
    result->Success(flutter::EncodableValue(GetCurrentUrl().c_str()));
  } else if (methodName.compare("evaluateJavascript") == 0) {
    result->NotImplemented();
  } else if (methodName.compare("addJavascriptChannels") == 0) {
    result->NotImplemented();
  } else if (methodName.compare("removeJavascriptChannels") == 0) {
    result->NotImplemented();
  } else if (methodName.compare("clearCache") == 0) {
    webViewInstance_->ClearCache();
    result->Success();
  } else if (methodName.compare("getTitle") == 0) {
    result->NotImplemented();
  } else if (methodName.compare("scrollTo") == 0) {
    result->NotImplemented();
  } else if (methodName.compare("scrollBy") == 0) {
    result->NotImplemented();
  } else if (methodName.compare("getScrollX") == 0) {
    result->NotImplemented();
  } else if (methodName.compare("getScrollY") == 0) {
    result->NotImplemented();
  } else {
    result->NotImplemented();
  }
}
