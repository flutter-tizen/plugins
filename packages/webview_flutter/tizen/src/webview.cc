// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview.h"

#include <app_common.h>
#include <flutter/standard_method_codec.h>
#include <glib.h>
#include <tbm_surface.h>

#include <functional>
#include <map>
#include <vector>

#include "buffer_pool.h"
#include "log.h"
#include "webview_backend_factory.h"
#include "webview_factory.h"

namespace {

constexpr size_t kBufferPoolSize = 5;
constexpr char kTizenWebViewChannelName[] = "plugins.flutter.io/tizen_webview_";
constexpr char kTizenWebViewControllerChannelName[] =
    "plugins.flutter.io/tizen_webview_controller_";
constexpr char kTizenNavigationDelegateChannelName[] =
    "plugins.flutter.io/tizen_webview_navigation_delegate_";

class NavigationRequestResult : public FlMethodResult {
 public:
  // |alive| gates every dereference below: Dart resolves this call
  // asynchronously, so it can complete after |webview| is destroyed.
  NavigationRequestResult(WebView* webview, std::shared_ptr<bool> alive)
      : webview_(webview), alive_(std::move(alive)) {}

 private:
  void SuccessInternal(const flutter::EncodableValue* should_load) override {
    if (!*alive_) {
      return;
    }
    if (std::holds_alternative<bool>(*should_load) &&
        std::get<bool>(*should_load)) {
      webview_->Resume();
      return;
    }
    webview_->Stop();
  }

  void ErrorInternal(const std::string& error_code,
                     const std::string& error_message,
                     const flutter::EncodableValue* error_details) override {
    LOG_ERROR("The request unexpectedly completed with an error.");
    if (!*alive_) {
      return;
    }
    webview_->Stop();
  }

  void NotImplementedInternal() override {
    LOG_ERROR("The target method was unexpectedly unimplemented.");
    if (!*alive_) {
      return;
    }
    webview_->Stop();
  }

  WebView* webview_;
  std::shared_ptr<bool> alive_;
};

template <typename T>
bool GetValueFromEncodableMap(const flutter::EncodableValue* arguments,
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

}  // namespace

WebView::WebView(flutter::PluginRegistrar* registrar, int view_id,
                 flutter::TextureRegistrar* texture_registrar, double width,
                 double height, const flutter::EncodableValue& params,
                 void* window)
    : PlatformView(registrar, view_id, nullptr),
      texture_registrar_(texture_registrar),
      width_(width),
      height_(height),
      window_(window) {
  backend_ = WebViewBackendFactory::Create(this);
  if (!backend_) {
    LOG_ERROR("Failed to create a webview backend.");
    return;
  }

  tbm_pool_ = std::make_shared<SingleBufferPool>(width, height);

  texture_variant_ =
      std::make_unique<flutter::TextureVariant>(flutter::GpuSurfaceTexture(
          kFlutterDesktopGpuSurfaceTypeNone,
          [this](size_t width,
                 size_t height) -> const FlutterDesktopGpuSurfaceDescriptor* {
            return ObtainGpuSurface(width, height);
          }));
  SetTextureId(texture_registrar_->RegisterTexture(texture_variant_.get()));

  webview_channel_ = std::make_unique<FlMethodChannel>(
      GetPluginRegistrar()->messenger(), GetWebViewChannelName(),
      &flutter::StandardMethodCodec::GetInstance());
  webview_channel_->SetMethodCallHandler(
      [webview = this](const auto& call, auto result) {
        webview->HandleWebViewMethodCall(call, std::move(result));
      });

  webview_controller_channel_ = std::make_unique<FlMethodChannel>(
      GetPluginRegistrar()->messenger(), GetWebViewControllerChannelName(),
      &flutter::StandardMethodCodec::GetInstance());

  navigation_delegate_channel_ = std::make_unique<FlMethodChannel>(
      GetPluginRegistrar()->messenger(), GetNavigationDelegateChannelName(),
      &flutter::StandardMethodCodec::GetInstance());

  auto cookie_channel = std::make_unique<FlMethodChannel>(
      GetPluginRegistrar()->messenger(),
      "plugins.flutter.io/tizen_cookie_manager",
      &flutter::StandardMethodCodec::GetInstance());
  cookie_channel->SetMethodCallHandler(
      [webview = this](const auto& call, auto result) {
        webview->HandleCookieMethodCall(call, std::move(result));
      });
}

WebView::~WebView() { Dispose(); }

std::string WebView::GetWebViewChannelName() {
  return std::string(kTizenWebViewChannelName) + std::to_string(GetViewId());
}

std::string WebView::GetWebViewControllerChannelName() {
  return std::string(kTizenWebViewControllerChannelName) +
         std::to_string(GetViewId());
}

std::string WebView::GetNavigationDelegateChannelName() {
  return std::string(kTizenNavigationDelegateChannelName) +
         std::to_string(GetViewId());
}

void WebView::Dispose() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (disposed_) {
      return;
    }
    disposed_ = true;
  }
  *is_alive_ = false;

  if (!backend_) {
    return;
  }

  std::shared_ptr<BufferPool> pool;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    working_surface_ = nullptr;
    candidate_surface_ = nullptr;
    rendered_surface_ = nullptr;
    pool = std::move(tbm_pool_);
  }

  std::function<void()> teardown = backend_->PrepareTeardown(std::move(pool));

  texture_registrar_->UnregisterTexture(GetTextureId(), [teardown]() {
    // Must stay a high-priority timeout: g_idle_add() runs too late and the
    // delete then races the raster thread on the TV emulator.
    g_timeout_add_full(
        G_PRIORITY_HIGH, 0,
        [](gpointer data) -> gboolean {
          auto* fn = static_cast<std::function<void()>*>(data);
          (*fn)();
          return G_SOURCE_REMOVE;
        },
        new std::function<void()>(teardown),
        [](gpointer data) {
          delete static_cast<std::function<void()>*>(data);
        });
  });
}

void WebView::InitializeEngine() { WebViewBackendFactory::InitializeEngine(); }

void WebView::ShutdownEngine() { WebViewBackendFactory::ShutdownEngine(); }

void WebView::Offset(double left, double top) {
  if (!backend_) {
    return;
  }
  backend_->Offset(left, top);
}

void WebView::Resize(double width, double height) {
  if (!backend_) {
    return;
  }
  width_ = width;
  height_ = height;

  if (candidate_surface_) {
    candidate_surface_ = nullptr;
  }

  tbm_pool_->Prepare(width_, height_);
  backend_->Resize(width_, height_);
}

void WebView::Touch(int event_type, int button_type, double x, double y,
                    double dx, double dy) {
  if (!backend_) {
    return;
  }
  backend_->Touch(event_type, button_type, x, y, dx, dy);
}

bool WebView::SendKey(const char* key, const char* string, const char* compose,
                      uint32_t modifiers, uint32_t scan_code, bool is_down) {
  if (!backend_) {
    return false;
  }
  if (!IsFocused()) {
    return false;
  }
  return backend_->SendKey(key, string, compose, modifiers, scan_code, is_down);
}

void WebView::Resume() {
  if (!backend_) {
    return;
  }
  backend_->Resume();
}

void WebView::Stop() {
  if (!backend_) {
    return;
  }
  backend_->Stop();
}

void WebView::SetDirection(int direction) {
  // TODO: Implement if necessary.
}

template <typename T>
void WebView::SetBackgroundColor(const T& color) {
  backend_->SetBackgroundColor(color >> 16 & 0xff, color >> 8 & 0xff,
                               color & 0xff, color >> 24 & 0xff);
}

void WebView::HandleWebViewMethodCall(const FlMethodCall& method_call,
                                      std::unique_ptr<FlMethodResult> result) {
  const std::string& method_name = method_call.method_name();
  const flutter::EncodableValue* arguments = method_call.arguments();

  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (disposed_) {
      result->Error("Invalid operation",
                    "The webview instance has been disposed.");
      return;
    }
  }

  if (method_name == "setEnginePolicy") {
    const auto* engine_policy = std::get_if<bool>(arguments);
    if (engine_policy) {
      engine_policy_ = *engine_policy;
    }
    result->Success();
    return;
  }

  if (!webview_created_) {
    if (!backend_->Create(width_, height_, window_, engine_policy_)) {
      result->Error("Invalid operation",
                    "The webview instance initialize failed.");
      return;
    }
    webview_created_ = true;
  }

  if (method_name == "javaScriptMode") {
    const auto* mode = std::get_if<int32_t>(arguments);
    if (mode) {
      backend_->SetJavaScriptEnabled(*mode == 1);
    }
    result->Success();
  } else if (method_name == "hasNavigationDelegate") {
    const auto* has_navigation_delegate = std::get_if<bool>(arguments);
    if (has_navigation_delegate) {
      backend_->SetHasNavigationDelegate(*has_navigation_delegate);
    }
    result->Success();
  } else if (method_name == "loadRequest") {
    std::string url;
    if (GetValueFromEncodableMap(arguments, "url", &url)) {
      backend_->LoadUrl(url);
      result->Success();
    } else {
      result->Error("Invalid argument", "No url provided.");
    }
  } else if (method_name == "loadRequestWithParams") {
    std::string url;
    if (!GetValueFromEncodableMap(arguments, "url", &url)) {
      result->Error("Invalid argument", "No url provided.");
      return;
    }

    int32_t method = 0;
    GetValueFromEncodableMap(arguments, "method", &method);

    flutter::EncodableMap encodable_headers;
    GetValueFromEncodableMap(arguments, "headers", &encodable_headers);
    std::map<std::string, std::string> headers;
    for (const auto& header : encodable_headers) {
      auto key = std::get_if<std::string>(&header.first);
      auto value = std::get_if<std::string>(&header.second);
      if (key && value) {
        headers[*key] = *value;
      }
    }

    std::vector<uint8_t> body;
    if (GetValueFromEncodableMap(arguments, "body", &body)) {
      body.push_back('\0');
    }

    if (backend_->LoadUrlRequest(url, method, headers, body)) {
      result->Success();
    } else {
      result->Error("Operation failed",
                    "Failed to load request with parameters.");
    }
  } else if (method_name == "canGoBack") {
    result->Success(flutter::EncodableValue(backend_->CanGoBack()));
  } else if (method_name == "canGoForward") {
    result->Success(flutter::EncodableValue(backend_->CanGoForward()));
  } else if (method_name == "goBack") {
    backend_->GoBack();
    result->Success();
  } else if (method_name == "goForward") {
    backend_->GoForward();
    result->Success();
  } else if (method_name == "reload") {
    backend_->Reload();
    result->Success();
  } else if (method_name == "currentUrl") {
    result->Success(flutter::EncodableValue(backend_->GetCurrentUrl()));
  } else if (method_name == "evaluateJavaScript" ||
             method_name == "runJavaScriptReturningResult" ||
             method_name == "runJavaScript") {
    const auto* javascript = std::get_if<std::string>(arguments);
    if (javascript) {
      FlMethodResult* raw_result = result.release();
      backend_->EvaluateJavaScript(
          *javascript, [raw_result](bool success, const char* result_value) {
            if (!success) {
              raw_result->Error("Failed to execute JavaScript");
            } else if (result_value) {
              raw_result->Success(flutter::EncodableValue(result_value));
            } else {
              raw_result->Success();
            }
            delete raw_result;
          });
    } else {
      result->Error("Invalid argument", "The argument must be a string.");
    }
  } else if (method_name == "addJavaScriptChannel") {
    const auto* channel = std::get_if<std::string>(arguments);
    if (channel) {
      backend_->RegisterJavaScriptChannel(*channel);
      result->Success();
    } else {
      result->Error("Invalid argument", "The argument must be a string.");
    }
  } else if (method_name == "clearCache") {
    backend_->ClearCache();
    result->Success();
  } else if (method_name == "clearLocalStorage") {
    backend_->ClearLocalStorage();
    result->Success();
  } else if (method_name == "getTitle") {
    result->Success(flutter::EncodableValue(backend_->GetTitle()));
  } else if (method_name == "scrollTo") {
    int32_t x = 0, y = 0;
    if (GetValueFromEncodableMap(arguments, "x", &x) &&
        GetValueFromEncodableMap(arguments, "y", &y)) {
      backend_->ScrollTo(x, y);
      result->Success();
    } else {
      result->Error("Invalid argument", "No x or y provided.");
    }
  } else if (method_name == "scrollBy") {
    int32_t x = 0, y = 0;
    if (GetValueFromEncodableMap(arguments, "x", &x) &&
        GetValueFromEncodableMap(arguments, "y", &y)) {
      backend_->ScrollBy(x, y);
      result->Success();
    } else {
      result->Error("Invalid argument", "No x or y provided.");
    }
  } else if (method_name == "getScrollPosition") {
    int32_t x = 0, y = 0;
    backend_->GetScrollPosition(&x, &y);
    flutter::EncodableMap args = {
        {flutter::EncodableValue("x"),
         flutter::EncodableValue(static_cast<double>(x))},
        {flutter::EncodableValue("y"),
         flutter::EncodableValue(static_cast<double>(y))}};
    result->Success(flutter::EncodableValue(args));
  } else if (method_name == "loadFlutterAsset") {
    const auto* key = std::get_if<std::string>(arguments);
    if (key) {
      char* res_path = app_get_resource_path();
      if (res_path) {
        std::string url =
            std::string("file://") + res_path + "flutter_assets/" + *key;
        free(res_path);
        backend_->LoadUrl(url);
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
    GetValueFromEncodableMap(arguments, "baseUrl", &base_url);
    backend_->LoadHtmlString(html, base_url);
    result->Success();
  } else if (method_name == "loadFile") {
    const auto* file_path = std::get_if<std::string>(arguments);
    if (file_path) {
      backend_->LoadUrl(std::string("file://") + *file_path);
      result->Success();
    } else {
      result->Error("Invalid argument", "The argument must be a string.");
    }
  } else if (method_name == "backgroundColor") {
    if (std::holds_alternative<int32_t>(*arguments)) {
      SetBackgroundColor(std::get<int32_t>(*arguments));
      result->Success();
    } else if (std::holds_alternative<int64_t>(*arguments)) {
      SetBackgroundColor(std::get<int64_t>(*arguments));
      result->Success();
    } else {
      result->Error("Invalid argument",
                    "The argument must be a int32_t or int64_t.");
    }
  } else if (method_name == "setUserAgent") {
    const auto* user_agent = std::get_if<std::string>(arguments);
    if (user_agent) {
      backend_->SetUserAgent(*user_agent);
    }
    result->Success();
  } else if (method_name == "getUserAgent") {
    result->Success(flutter::EncodableValue(backend_->GetUserAgent()));
  } else if (method_name == "enableZoom") {
    const auto* support = std::get_if<bool>(arguments);
    if (!support) {
      result->Error("Invalid argument", "The argument must be a bool.");
      return;
    }
    backend_->EnableZoom(*support);
    result->Success();
  } else if (method_name == "javaScriptAlertReply") {
    backend_->JavaScriptAlertReply();
    result->Success();
  } else if (method_name == "javaScriptConfirmReply") {
    const auto* value = std::get_if<bool>(arguments);
    if (value) {
      backend_->JavaScriptConfirmReply(*value);
      result->Success();
    } else {
      result->Error("Invalid argument", "The argument must be a bool.");
    }
  } else if (method_name == "javaScriptPromptReply") {
    const auto* value = std::get_if<std::string>(arguments);
    if (value) {
      backend_->JavaScriptPromptReply(*value);
      result->Success();
    } else {
      result->Error("Invalid argument", "The argument must be a string.");
    }
  } else if (method_name == "setVerticalScrollBarEnabled" ||
             method_name == "setHorizontalScrollBarEnabled") {
    const auto* value = std::get_if<bool>(arguments);
    if (value) {
      backend_->SetScrollbarVisible(*value);
      result->Success();
    } else {
      result->Error("Invalid argument", "The argument must be a bool.");
    }
  } else {
    result->NotImplemented();
  }
}

void WebView::HandleCookieMethodCall(const FlMethodCall& method_call,
                                     std::unique_ptr<FlMethodResult> result) {
  if (!webview_created_) {
    result->Error("Invalid operation",
                  "The webview instance has not been initialized.");
    return;
  }

  const std::string& method_name = method_call.method_name();

  if (method_name == "clearCookies") {
    if (backend_->ClearCookies()) {
      result->Success(flutter::EncodableValue(true));
    } else {
      result->Error("Operation failed", "Failed to get cookie manager");
    }
  } else {
    result->NotImplemented();
  }
}

FlutterDesktopGpuSurfaceDescriptor* WebView::ObtainGpuSurface(size_t width,
                                                              size_t height) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (disposed_ || !tbm_pool_) {
    return nullptr;
  }
  if (!candidate_surface_) {
    if (rendered_surface_) {
      return rendered_surface_->GpuSurface();
    }
    return nullptr;
  }
  if (rendered_surface_ && rendered_surface_->IsUsed()) {
    tbm_pool_->Release(rendered_surface_);
  }
  rendered_surface_ = candidate_surface_;
  candidate_surface_ = nullptr;
  return rendered_surface_->GpuSurface();
}

void WebView::OnFrameRendered(void* tbm_surface) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (disposed_ || !tbm_pool_) {
    return;
  }
  if (!working_surface_) {
    working_surface_ = tbm_pool_->GetAvailableBuffer();
    working_surface_->UseExternalBuffer();
  }
  working_surface_->SetExternalBuffer(static_cast<tbm_surface_h>(tbm_surface));

  if (candidate_surface_) {
    tbm_pool_->Release(candidate_surface_);
    candidate_surface_ = nullptr;
  }
  candidate_surface_ = working_surface_;
  working_surface_ = nullptr;
  texture_registrar_->MarkTextureFrameAvailable(GetTextureId());
}

void WebView::OnLoadStarted(const std::string& url) {
  flutter::EncodableMap args = {
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)}};
  navigation_delegate_channel_->InvokeMethod(
      "onPageStarted", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnLoadFinished(const std::string& url) {
  flutter::EncodableMap args = {
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)}};
  navigation_delegate_channel_->InvokeMethod(
      "onPageFinished", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnProgress(int32_t progress) {
  flutter::EncodableMap args = {
      {flutter::EncodableValue("progress"), flutter::EncodableValue(progress)}};
  navigation_delegate_channel_->InvokeMethod(
      "onProgress", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnLoadError(int32_t error_code, const std::string& description,
                          const std::string& failing_url) {
  flutter::EncodableMap args = {
      {flutter::EncodableValue("errorCode"),
       flutter::EncodableValue(error_code)},
      {flutter::EncodableValue("description"),
       flutter::EncodableValue(description)},
      {flutter::EncodableValue("failingUrl"),
       flutter::EncodableValue(failing_url)},
  };
  navigation_delegate_channel_->InvokeMethod(
      "onWebResourceError", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnConsoleMessage(const std::string& level,
                               const std::string& message) {
  if (webview_controller_channel_) {
    flutter::EncodableMap args = {
        {flutter::EncodableValue("level"), flutter::EncodableValue(level)},
        {flutter::EncodableValue("message"), flutter::EncodableValue(message)},
    };
    webview_controller_channel_->InvokeMethod(
        "onConsoleMessage", std::make_unique<flutter::EncodableValue>(args));
  }
}

void WebView::OnNavigationPolicyDecide(const std::string& url) {
  flutter::EncodableMap args = {
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)},
      {flutter::EncodableValue("isForMainFrame"),
       flutter::EncodableValue(true)},
  };
  auto result = std::make_unique<NavigationRequestResult>(this, is_alive_);
  navigation_delegate_channel_->InvokeMethod(
      "navigationRequest", std::make_unique<flutter::EncodableValue>(args),
      std::move(result));
}

void WebView::OnResponsePolicyDecide(const std::string& url,
                                     int32_t status_code) {
  flutter::EncodableMap args = {
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)},
      {flutter::EncodableValue("statusCode"),
       flutter::EncodableValue(status_code)},
  };
  navigation_delegate_channel_->InvokeMethod(
      "onHttpError", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnUrlChanged(const std::string& url) {
  flutter::EncodableMap args = {
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)}};
  navigation_delegate_channel_->InvokeMethod(
      "onUrlChange", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnJavaScriptMessage(const std::string& channel,
                                  const std::string& message) {
  if (webview_channel_) {
    flutter::EncodableMap args = {
        {flutter::EncodableValue("channel"), flutter::EncodableValue(channel)},
        {flutter::EncodableValue("message"), flutter::EncodableValue(message)},
    };
    webview_channel_->InvokeMethod(
        "javaScriptChannelMessage",
        std::make_unique<flutter::EncodableValue>(args));
  }
}

void WebView::OnJavaScriptAlertDialog(const std::string& message,
                                      const std::string& url) {
  flutter::EncodableMap args = {
      {flutter::EncodableValue("message"), flutter::EncodableValue(message)},
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)}};
  webview_controller_channel_->InvokeMethod(
      "onJavaScriptAlert", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnJavaScriptConfirmDialog(const std::string& message,
                                        const std::string& url) {
  flutter::EncodableMap args = {
      {flutter::EncodableValue("message"), flutter::EncodableValue(message)},
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)}};
  webview_controller_channel_->InvokeMethod(
      "onJavaScriptConfirm", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnJavaScriptPromptDialog(const std::string& message,
                                       const std::string& default_text,
                                       const std::string& url) {
  flutter::EncodableMap args = {
      {flutter::EncodableValue("message"), flutter::EncodableValue(message)},
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)},
      {flutter::EncodableValue("defaultText"),
       flutter::EncodableValue(default_text)}};
  webview_controller_channel_->InvokeMethod(
      "onJavaScriptPrompt", std::make_unique<flutter::EncodableValue>(args));
}
