
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

std::string extractStringFromMap(const flutter::EncodableValue& arguments,
                                 const char* key) {
  if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
    flutter::EncodableMap values = std::get<flutter::EncodableMap>(arguments);
    flutter::EncodableValue value = values[flutter::EncodableValue(key)];
    if (std::holds_alternative<std::string>(value))
      return std::get<std::string>(value);
  }
  return std::string();
}
int extractIntFromMap(const flutter::EncodableValue& arguments,
                      const char* key) {
  if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
    flutter::EncodableMap values = std::get<flutter::EncodableMap>(arguments);
    flutter::EncodableValue value = values[flutter::EncodableValue(key)];
    if (std::holds_alternative<int>(value)) return std::get<int>(value);
  }
  return -1;
}
double extractDoubleFromMap(const flutter::EncodableValue& arguments,
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
      textureRegistrar_(textureRegistrar),
      webViewInstance_(nullptr),
      currentUrl_(initialUrl),
      width_(width),
      height_(height) {
  setTextureId(FlutterRegisterExternalTexture(textureRegistrar_));
  initWebView();
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          getPluginRegistrar()->messenger(), getChannelName(),
          &flutter::StandardMethodCodec::GetInstance());
  channel->SetMethodCallHandler(
      [webview = this](const auto& call, auto result) {
        webview->HandleMethodCall(call, std::move(result));
      });
  webViewInstance_->LoadURL(currentUrl_);
}

WebView::~WebView() { dispose(); }

std::string WebView::getChannelName() {
  return "plugins.flutter.io/webview_" + std::to_string(getViewId());
}

void WebView::dispose() {
  FlutterUnregisterExternalTexture(textureRegistrar_, getTextureId());

  webViewInstance_->Destroy();
  webViewInstance_ = nullptr;
}

void WebView::resize(double width, double height) {
  LOG_DEBUG("WebView::resize width: %f height: %f \n", width, height);
}

void WebView::touch(int type, int button, double x, double y, double dx,
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

void WebView::clearFocus() { LOG_DEBUG("WebView::clearFocus \n"); }

void WebView::setDirection(int direction) {
  LOG_DEBUG("WebView::setDirection direction: %d\n", direction);
}

void WebView::initWebView() {
  if (webViewInstance_ != nullptr) {
    webViewInstance_->Destroy();
    webViewInstance_ = nullptr;
  }
  float scaleFactor = 1;
  webViewInstance_ = LWE::WebContainer::Create(
      width_, height_, scaleFactor, "SamsungOneUI", "ko-KR", "Asia/Seoul");
  webViewInstance_->RegisterPreRenderingHandler(
      [this]() -> LWE::WebContainer::RenderInfo {
        LWE::WebContainer::RenderInfo result;
        {
          tbmSurface_ =
              tbm_surface_create(width_, height_, TBM_FORMAT_ARGB8888);
          tbm_surface_info_s tbmSurfaceInfo;
          if (tbm_surface_map(tbmSurface_, TBM_SURF_OPTION_WRITE,
                              &tbmSurfaceInfo) == TBM_SURFACE_ERROR_NONE) {
            result.updatedBufferAddress = tbmSurfaceInfo.planes[0].ptr;
            result.bufferStride = tbmSurfaceInfo.planes[0].stride;
          }
        }
        return result;
      });
  webViewInstance_->RegisterOnRenderedHandler(
      [this](LWE::WebContainer* c, LWE::WebContainer::RenderResult r) {
        FlutterMarkExternalTextureFrameAvailable(textureRegistrar_,
                                                 getTextureId(), tbmSurface_);
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
    currentUrl_ = extractStringFromMap(arguments, "url");
    webViewInstance_->LoadURL(getCurrentUrl());
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
    result->Success(flutter::EncodableValue(getCurrentUrl().c_str()));
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
