
#include "webview.h"

#include <Ecore_IMF_Evas.h>
#include <Ecore_Input_Evas.h>
#include <flutter_platform_view.h>
#include <flutter_texture_registrar.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

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
  NavigationRequestResult(std::string url, WebView* webView)
      : url_(url), webView_(webView) {}

  void SuccessInternal(const T* shouldLoad) override {
    if (std::holds_alternative<bool>(*shouldLoad)) {
      if (std::get<bool>(*shouldLoad)) {
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
    if (webView_ && webView_->GetWebViewInstance()) {
      webView_->GetWebViewInstance()->LoadURL(url_);
    }
  }

  std::string url_;
  WebView* webView_;
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

static std::string ErrorCodeToString(int errorCode) {
  switch (errorCode) {
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
      "Could not find a string for errorCode: " + std::to_string(errorCode);
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
                 FlutterTextureRegistrar* textureRegistrar, double width,
                 double height, flutter::EncodableMap& params)
    : PlatformView(registrar, viewId),
      textureRegistrar_(textureRegistrar),
      webViewInstance_(nullptr),
      width_(width),
      height_(height),
      tbmSurface_(nullptr),
      isMouseLButtonDown_(false),
      hasNavigationDelegate_(false) {
  SetTextureId(FlutterRegisterExternalTexture(textureRegistrar_));
  InitWebView();

  channel_ = std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
      GetPluginRegistrar()->messenger(), GetChannelName(),
      &flutter::StandardMethodCodec::GetInstance());
  channel_->SetMethodCallHandler(
      [webview = this](const auto& call, auto result) {
        webview->HandleMethodCall(call, std::move(result));
      });

  std::string url;
  auto initialUrl = params[flutter::EncodableValue("initialUrl")];
  if (std::holds_alternative<std::string>(initialUrl)) {
    url = std::get<std::string>(initialUrl);
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
    auto nameList = std::get<flutter::EncodableList>(names);
    for (size_t i = 0; i < nameList.size(); i++) {
      if (std::holds_alternative<std::string>(nameList[i])) {
        RegisterJavaScriptChannelName(std::get<std::string>(nameList[i]));
      }
    }
  }

  webViewInstance_->RegisterOnPageStartedHandler(
      [this](LWE::WebContainer* container, const std::string& url) {
        LOG_DEBUG("RegisterOnPageStartedHandler(url: %s)\n", url.c_str());
        flutter::EncodableMap map;
        map.insert(
            std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
                flutter::EncodableValue("url"), flutter::EncodableValue(url)));
        auto args = std::make_unique<flutter::EncodableValue>(map);
        channel_->InvokeMethod("onPageStarted", std::move(args));
      });
  webViewInstance_->RegisterOnPageLoadedHandler(
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
  webViewInstance_->RegisterOnReceivedErrorHandler(
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
        channel_->InvokeMethod("onPageFinished", std::move(args));
      });

  webViewInstance_->RegisterShouldOverrideUrlLoadingHandler(
      [this](LWE::WebContainer* view, const std::string& url) -> bool {
        if (!hasNavigationDelegate_) {
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
        auto onResult =
            std::make_unique<NavigationRequestResult<flutter::EncodableValue>>(
                url, this);
        channel_->InvokeMethod("navigationRequest", std::move(args),
                               std::move(onResult));

        return true;
      });

  webViewInstance_->LoadURL(url);
}

void WebView::ApplySettings(flutter::EncodableMap settings) {
  for (auto const& [key, val] : settings) {
    if (std::holds_alternative<std::string>(key)) {
      std::string k = std::get<std::string>(key);
      if ("jsMode" == k) {
        // NOTE: Not supported by LWE on Tizen.
      } else if ("hasNavigationDelegate" == k) {
        if (std::holds_alternative<bool>(val)) {
          hasNavigationDelegate_ = std::get<bool>(val);
        }
      } else if ("debuggingEnabled" == k) {
        // NOTE: Not supported by LWE on Tizen.
      } else if ("gestureNavigationEnabled" == k) {
        // NOTE: Not supported by LWE on Tizen.
      } else if ("userAgent" == k) {
        if (std::holds_alternative<std::string>(val)) {
          auto settings = webViewInstance_->GetSettings();
          settings.SetUserAgentString(std::get<std::string>(val));
          webViewInstance_->SetSettings(settings);
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

  webViewInstance_->AddJavaScriptInterface(name, "postMessage", cb);
}

WebView::~WebView() { Dispose(); }

std::string WebView::GetChannelName() {
  return "plugins.flutter.io/webview_" + std::to_string(GetViewId());
}

void WebView::Dispose() {
  FlutterUnregisterExternalTexture(textureRegistrar_, GetTextureId());

  if (webViewInstance_) {
    webViewInstance_->Destroy();
    webViewInstance_ = nullptr;
  }
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
            // LWE::WebContainer* self = (LWE::WebContainer*)data;
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

void WebView::SetSoftwareKeyboardContext(Ecore_IMF_Context* context) {
  webViewInstance_->RegisterOnShowSoftwareKeyboardIfPossibleHandler(
      [context](LWE::WebContainer* v) {
        LOG_DEBUG("WebView - Show Keyboard()\n");
        if (!context) {
          LOG_ERROR("Ecore_IMF_Context NULL\n");
          return;
        }
        ecore_imf_context_input_panel_show(context);
        ecore_imf_context_focus_in(context);
      });

  webViewInstance_->RegisterOnHideSoftwareKeyboardIfPossibleHandler(
      [context](LWE::WebContainer*) {
        LOG_INFO("WebView - Hide Keyboard()\n");
        if (!context) {
          LOG_INFO("Ecore_IMF_Context NULL\n");
          return;
        }
        ecore_imf_context_reset(context);
        ecore_imf_context_focus_out(context);
        ecore_imf_context_input_panel_hide(context);
      });
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

  webViewInstance_ = (LWE::WebContainer*)createWebViewInstance(
      0, 0, width_, height_, scaleFactor, "SamsungOneUI", "ko-KR", "Asia/Seoul",
      [this]() -> LWE::WebContainer::ExternalImageInfo {
        LWE::WebContainer::ExternalImageInfo result;
        tbmSurface_ = tbm_surface_create(width_, height_, TBM_FORMAT_ARGB8888);
        result.imageAddress = (void*)tbmSurface_;
        return result;
      },
      [this](LWE::WebContainer* c, bool isRendered) {
        if (isRendered) {
          FlutterMarkExternalTextureFrameAvailable(textureRegistrar_,
                                                   GetTextureId(), tbmSurface_);
        }
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

  LOG_DEBUG("WebView::HandleMethodCall : %s \n ", methodName.c_str());

  if (methodName.compare("loadUrl") == 0) {
    std::string url = ExtractStringFromMap(arguments, "url");
    webViewInstance_->LoadURL(url);
    result->Success();
  } else if (methodName.compare("updateSettings") == 0) {
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
    result->Success(flutter::EncodableValue(webViewInstance_->GetURL()));
  } else if (methodName.compare("evaluateJavascript") == 0) {
    if (std::holds_alternative<std::string>(arguments)) {
      std::string jsString = std::get<std::string>(arguments);
      webViewInstance_->EvaluateJavaScript(
          jsString, [res = result.release()](std::string value) {
            LOG_DEBUG("value: %s\n", value.c_str());
            if (res) {
              res->Success(flutter::EncodableValue(value));
              delete res;
            }
          });
    } else {
      result->Error("Invalid Arguments", "Invalid Arguments");
    }
  } else if (methodName.compare("addJavascriptChannels") == 0) {
    if (std::holds_alternative<flutter::EncodableList>(arguments)) {
      auto nameList = std::get<flutter::EncodableList>(arguments);
      for (size_t i = 0; i < nameList.size(); i++) {
        if (std::holds_alternative<std::string>(nameList[i])) {
          RegisterJavaScriptChannelName(std::get<std::string>(nameList[i]));
        }
      }
    }
    result->Success();
  } else if (methodName.compare("removeJavascriptChannels") == 0) {
    if (std::holds_alternative<flutter::EncodableList>(arguments)) {
      auto nameList = std::get<flutter::EncodableList>(arguments);
      for (size_t i = 0; i < nameList.size(); i++) {
        if (std::holds_alternative<std::string>(nameList[i])) {
          webViewInstance_->RemoveJavascriptInterface(
              std::get<std::string>(nameList[i]), "postMessage");
        }
      }
    }
    result->Success();

  } else if (methodName.compare("clearCache") == 0) {
    webViewInstance_->ClearCache();
    result->Success();
  } else if (methodName.compare("getTitle") == 0) {
    result->Success(flutter::EncodableValue(webViewInstance_->GetTitle()));
  } else if (methodName.compare("scrollTo") == 0) {
    int x = ExtractIntFromMap(arguments, "x");
    int y = ExtractIntFromMap(arguments, "y");
    webViewInstance_->ScrollTo(x, y);
    result->Success();
  } else if (methodName.compare("scrollBy") == 0) {
    int x = ExtractIntFromMap(arguments, "x");
    int y = ExtractIntFromMap(arguments, "y");
    webViewInstance_->ScrollBy(x, y);
    result->Success();
  } else if (methodName.compare("getScrollX") == 0) {
    result->NotImplemented();
  } else if (methodName.compare("getScrollY") == 0) {
    result->NotImplemented();
  } else {
    result->NotImplemented();
  }
}
