// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview.h"

#include <app_common.h>
#include <flutter/standard_method_codec.h>
#include <flutter_texture_registrar.h>
#include <glib.h>
#include <tbm_surface.h>

#include <algorithm>
#include <atomic>
#include <map>
#include <utility>
#include <vector>

#include "buffer_pool.h"
#include "log.h"
#include "webview_backend_factory.h"
#include "webview_factory.h"

struct WebViewLifetimeState {
  std::atomic_bool disposed = false;
};

// Owned by a shared_ptr so the raster thread's GpuSurfaceTexture callback
// never touches the WebView, which the embedder deletes one statement after
// Dispose() while frames can still be in flight.
struct RenderState {
  std::mutex mutex;
  BufferUnit* working = nullptr;
  BufferUnit* candidate = nullptr;
  BufferUnit* rendered = nullptr;
  std::shared_ptr<BufferPool> pool;

  FlutterDesktopGpuSurfaceDescriptor* ObtainGpuSurface(size_t width,
                                                       size_t height);
};

class JavaScriptReply {
 public:
  explicit JavaScriptReply(std::unique_ptr<FlMethodResult> result)
      : result_(std::move(result)) {}
  ~JavaScriptReply() { Fail(); }

  void Succeed(const char* value) {
    if (!result_) {
      return;
    }
    if (value) {
      result_->Success(flutter::EncodableValue(value));
    } else {
      result_->Success();
    }
    result_.reset();
  }

  void Fail() {
    if (!result_) {
      return;
    }
    result_->Error("Operation failed", "Failed to execute JavaScript.");
    result_.reset();
  }

 private:
  std::unique_ptr<FlMethodResult> result_;
};

namespace {

constexpr char kInAppWebViewChannelName[] =
    "com.pichillilorenzo/flutter_inappwebview_";
constexpr int kConsoleMessageLog = 1;
constexpr int kConsoleMessageWarning = 2;
constexpr int kConsoleMessageError = 3;
constexpr int kConsoleMessageDebug = 4;
constexpr int kConsoleMessageInfo = 0;
constexpr int kWebResourceErrorUnknown = -1;

// The backend reports the level as one of the strings defined by
// WebViewBackend::Delegate::OnConsoleMessage.
int ConvertLogLevel(const std::string& level) {
  if (level == "warning") {
    return kConsoleMessageWarning;
  }
  if (level == "error") {
    return kConsoleMessageError;
  }
  if (level == "debug") {
    return kConsoleMessageDebug;
  }
  if (level == "info") {
    return kConsoleMessageInfo;
  }
  return kConsoleMessageLog;
}

class NavigationRequestResult : public FlMethodResult {
 public:
  NavigationRequestResult(WebView* webview,
                          std::weak_ptr<WebViewLifetimeState> lifetime)
      : webview_(webview), lifetime_(std::move(lifetime)) {}

  void SuccessInternal(const flutter::EncodableValue* should_load) override {
    if (!IsWebViewAlive()) {
      return;
    }
    // The Dart side returns NavigationActionPolicy.toNativeValue():
    // 0 = CANCEL, 1 = ALLOW. Treat anything else as a cancel.
    if (should_load && std::holds_alternative<int32_t>(*should_load) &&
        std::get<int32_t>(*should_load) == 1) {
      webview_->ResumeNavigation();
      return;
    }
    webview_->StopNavigation();
  }

  void ErrorInternal(const std::string& error_code,
                     const std::string& error_message,
                     const flutter::EncodableValue* error_details) override {
    LOG_ERROR("The shouldOverrideUrlLoading reply errored: %s",
              error_message.c_str());
    if (!IsWebViewAlive()) {
      return;
    }
    webview_->StopNavigation();
  }

  void NotImplementedInternal() override {
    LOG_ERROR("The shouldOverrideUrlLoading reply was unimplemented.");
    if (!IsWebViewAlive()) {
      return;
    }
    webview_->StopNavigation();
  }

 private:
  bool IsWebViewAlive() {
    auto lifetime = lifetime_.lock();
    return lifetime && !lifetime->disposed.load();
  }

  WebView* webview_;
  std::weak_ptr<WebViewLifetimeState> lifetime_;
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

template <typename T>
bool GetValueFromEncodableMap(const flutter::EncodableMap& arguments,
                              std::string key, T* out) {
  auto iter = arguments.find(flutter::EncodableValue(key));
  if (iter != arguments.end() && !iter->second.IsNull()) {
    if (auto* value = std::get_if<T>(&iter->second)) {
      *out = *value;
      return true;
    }
  }
  return false;
}

flutter::EncodableMap CreateRequestMap(const std::string& url,
                                       const std::string& method = "GET") {
  flutter::EncodableMap map;
  map[flutter::EncodableValue("url")] = flutter::EncodableValue(url);
  map[flutter::EncodableValue("headers")] =
      flutter::EncodableValue(flutter::EncodableMap());
  map[flutter::EncodableValue("method")] = flutter::EncodableValue(method);
  map[flutter::EncodableValue("hasGesture")] = flutter::EncodableValue(false);
  map[flutter::EncodableValue("isForMainFrame")] =
      flutter::EncodableValue(true);
  map[flutter::EncodableValue("isRedirect")] = flutter::EncodableValue(false);
  return map;
}

flutter::EncodableMap CreateNavigationActionMap(const std::string& url) {
  flutter::EncodableMap map;
  map[flutter::EncodableValue("hasGesture")] = flutter::EncodableValue(false);
  map[flutter::EncodableValue("isForMainFrame")] =
      flutter::EncodableValue(true);
  map[flutter::EncodableValue("isRedirect")] = flutter::EncodableValue(false);
  map[flutter::EncodableValue("navigationType")] = flutter::EncodableValue();
  map[flutter::EncodableValue("request")] =
      flutter::EncodableValue(CreateRequestMap(url));
  map[flutter::EncodableValue("shouldPerformDownload")] =
      flutter::EncodableValue(false);
  map[flutter::EncodableValue("sourceFrame")] = flutter::EncodableValue();
  map[flutter::EncodableValue("targetFrame")] = flutter::EncodableValue();
  return map;
}

flutter::EncodableMap CreateErrorMap(
    const std::string& description, int error_code = kWebResourceErrorUnknown) {
  flutter::EncodableMap map;
  map[flutter::EncodableValue("description")] =
      flutter::EncodableValue(description);
  map[flutter::EncodableValue("errorCode")] =
      flutter::EncodableValue(error_code);
  map[flutter::EncodableValue("type")] = flutter::EncodableValue(error_code);
  return map;
}

// The engine reports a missing URL or title as an empty string; the Dart API
// expects null in that case.
flutter::EncodableValue ToNullableString(const std::string& value) {
  return value.empty() ? flutter::EncodableValue()
                       : flutter::EncodableValue(value);
}

}  // namespace

std::set<WebView*> WebView::instances_;
std::mutex WebView::instances_mutex_;
std::string WebView::default_user_agent_;

void WebView::ClearAllCache() {
  std::lock_guard<std::mutex> lock(instances_mutex_);
  for (auto* instance : instances_) {
    if (!instance || !instance->backend_) {
      continue;
    }
    instance->backend_->ClearCache();
  }
}

bool WebView::ClearAllCookies() {
  std::lock_guard<std::mutex> lock(instances_mutex_);
  for (auto* instance : instances_) {
    if (!instance || !instance->backend_) {
      continue;
    }
    // Views in this plugin share the default engine context, so any live view
    // can provide the process-wide cookie manager.
    if (instance->backend_->ClearCookies()) {
      return true;
    }
  }
  return false;
}

void WebView::InitializeEngine() { WebViewBackendFactory::InitializeEngine(); }

void WebView::ShutdownEngine() {
  // The engine shutdown fatally CHECKs (SIGTRAP) on a live view. Dispose()
  // normally empties instances_ already; past the deadline, force-dispose
  // the stragglers instead of shutting down anyway.
  constexpr gint64 kDeadlineUsec = 2 * G_USEC_PER_SEC;
  const gint64 deadline = g_get_monotonic_time() + kDeadlineUsec;
  for (;;) {
    std::vector<WebView*> stragglers;
    {
      std::lock_guard<std::mutex> lock(instances_mutex_);
      if (instances_.empty()) {
        break;
      }
      if (g_get_monotonic_time() >= deadline) {
        stragglers.assign(instances_.begin(), instances_.end());
      }
    }
    if (!stragglers.empty()) {
      LOG_WARN(
          "ShutdownEngine: WebView instance(s) still alive past the "
          "deadline; force-disposing them before shutting the engine down.");
      for (auto* instance : stragglers) {
        instance->Dispose();
      }
      continue;
    }
    g_usleep(1000);
  }
  WebViewBackendFactory::ShutdownEngine();
}

std::string WebView::GetDefaultUserAgent() {
  std::lock_guard<std::mutex> lock(instances_mutex_);
  for (auto* instance : instances_) {
    if (!instance || !instance->backend_) {
      continue;
    }
    std::string user_agent = instance->backend_->GetUserAgent();
    if (!user_agent.empty()) {
      return user_agent;
    }
  }
  // Fall back to the value cached during the first WebView creation. Empty if
  // no InAppWebView has ever been created in this process.
  return default_user_agent_;
}

WebView::WebView(flutter::PluginRegistrar* registrar, int view_id,
                 flutter::TextureRegistrar* texture_registrar, double width,
                 double height, const flutter::EncodableValue& params,
                 void* window)
    : PlatformView(registrar, view_id, nullptr),
      texture_registrar_(texture_registrar),
      width_(width),
      height_(height),
      window_(window),
      render_state_(std::make_shared<RenderState>()),
      lifetime_(std::make_shared<WebViewLifetimeState>()) {
  render_state_->pool = std::make_shared<SingleBufferPool>(width, height);

  texture_variant_ =
      std::make_unique<flutter::TextureVariant>(flutter::GpuSurfaceTexture(
          kFlutterDesktopGpuSurfaceTypeNone,
          [state = render_state_](size_t width, size_t height)
              -> const FlutterDesktopGpuSurfaceDescriptor* {
            return state->ObtainGpuSurface(width, height);
          }));
  int64_t texture_id =
      texture_registrar_->RegisterTexture(texture_variant_.get());
  if (texture_id < 0) {
    LOG_ERROR("Failed to register the WebView texture.");
    return;
  }
  SetTextureId(static_cast<int>(texture_id));
  texture_registered_ = true;

  webview_channel_ = std::make_unique<FlMethodChannel>(
      GetPluginRegistrar()->messenger(), GetWebViewChannelName(),
      &flutter::StandardMethodCodec::GetInstance());
  webview_channel_->SetMethodCallHandler(
      [webview = this](const auto& call, auto result) {
        webview->HandleWebViewMethodCall(call, std::move(result));
      });

  if (!InitWebView()) {
    LOG_ERROR("Failed to initialize the webview backend.");
    return;
  }

  {
    std::lock_guard<std::mutex> lock(instances_mutex_);
    if (default_user_agent_.empty()) {
      default_user_agent_ = backend_->GetUserAgent();
    }
    instances_.insert(this);
  }

  ApplyInitialParams(params);
}

WebView::~WebView() { Dispose(); }

std::string WebView::GetWebViewChannelName() {
  return std::string(kInAppWebViewChannelName) + std::to_string(GetViewId());
}

void WebView::ResumeNavigation() {
  if (disposed_ || !backend_) {
    return;
  }
  backend_->Resume();
}

void WebView::StopNavigation() {
  if (disposed_ || !backend_) {
    return;
  }
  is_navigation_cancelled_ = true;
  if (!url_before_navigation_.empty()) {
    committed_url_ = url_before_navigation_;
  }
  // Stop() has no effect while the view is suspended.
  backend_->Resume();
  backend_->Stop();
}

bool WebView::NavigateProgrammatically(
    const std::function<bool()>& backend_call) {
  is_programmatic_navigation_ = true;
  const bool started = backend_call();
  if (!started) {
    is_programmatic_navigation_ = false;
  }
  return started;
}

void WebView::Dispose() {
  if (disposed_) {
    return;
  }
  disposed_ = true;
  lifetime_->disposed.store(true);

  {
    std::lock_guard<std::mutex> lock(instances_mutex_);
    instances_.erase(this);
  }

  if (webview_channel_) {
    webview_channel_->SetMethodCallHandler(nullptr);
  }

  for (auto& reply : pending_js_replies_) {
    if (auto pending = reply.lock()) {
      pending->Fail();
    }
  }
  pending_js_replies_.clear();

  // NOTE: Engine destruction must wait until texture buffers are released.
  std::function<void()> teardown;
  if (backend_) {
    teardown = backend_->PrepareTeardown(render_state_->pool);
  }

  if (texture_registered_) {
    texture_registrar_->UnregisterTexture(GetTextureId(), [teardown]() {
      if (!teardown) {
        return;
      }
      // Must stay a high-priority timeout: the completion runs off the
      // platform thread, and g_idle_add() runs too late -- the delete then
      // races the raster thread on the TV emulator.
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
    texture_registered_ = false;
  } else if (teardown) {
    // No texture was ever registered, so nothing can still be reading the
    // buffers.
    teardown();
  }

  backend_.reset();
}

void WebView::Offset(double left, double top) {
  if (backend_) {
    backend_->Offset(left, top);
  }
}

void WebView::Resize(double width, double height) {
  {
    RenderState& state = *render_state_;
    std::lock_guard<std::mutex> lock(state.mutex);
    width_ = width;
    height_ = height;

    if (state.working) {
      state.pool->Release(state.working);
      state.working = nullptr;
    }
    if (state.candidate) {
      state.pool->Release(state.candidate);
      state.candidate = nullptr;
    }
    state.rendered = nullptr;
    state.pool->Prepare(width_, height_);
  }

  if (backend_) {
    backend_->Resize(width_, height_);
  }
}

void WebView::Touch(int event_type, int button_type, double x, double y,
                    double dx, double dy) {
  if (backend_) {
    backend_->Touch(event_type, button_type, x, y, dx, dy);
  }
}

bool WebView::SendKey(const char* key, const char* string, const char* compose,
                      uint32_t modifiers, uint32_t scan_code, bool is_down) {
  if (!IsFocused() || !backend_) {
    return false;
  }
  return backend_->SendKey(key, string, compose, modifiers, scan_code, is_down);
}

void WebView::SetDirection(int direction) {
  // TODO: Implement if necessary.
}

bool WebView::InitWebView() {
  auto backend = WebViewBackendFactory::Create(this);
  if (!backend) {
    return false;
  }
  if (!backend->Create(width_, height_, window_)) {
    return false;
  }
  backend_ = std::move(backend);
  return true;
}

void WebView::ApplySettings(const flutter::EncodableMap& settings) {
  bool bool_value = false;
  if (GetValueFromEncodableMap(settings, "javaScriptEnabled", &bool_value)) {
    backend_->SetJavaScriptEnabled(bool_value);
  }

  if (GetValueFromEncodableMap(settings, "supportZoom", &bool_value)) {
    backend_->EnableZoom(bool_value);
  }

  if (GetValueFromEncodableMap(settings, "useShouldOverrideUrlLoading",
                               &bool_value)) {
    has_navigation_delegate_ = bool_value;
  }

  std::string user_agent;
  if (GetValueFromEncodableMap(settings, "userAgent", &user_agent) &&
      !user_agent.empty()) {
    backend_->SetUserAgent(user_agent);
  }

  if (GetValueFromEncodableMap(settings, "transparentBackground",
                               &bool_value) &&
      bool_value) {
    backend_->SetBackgroundColor(0, 0, 0, 0);
  }
}

void WebView::ApplyInitialParams(const flutter::EncodableValue& params) {
  const auto* creation_params = std::get_if<flutter::EncodableMap>(&params);
  if (!creation_params) {
    return;
  }

  flutter::EncodableMap initial_settings;
  if (GetValueFromEncodableMap(*creation_params, "initialSettings",
                               &initial_settings)) {
    ApplySettings(initial_settings);
  }

  std::string initial_file;
  if (GetValueFromEncodableMap(*creation_params, "initialFile",
                               &initial_file) &&
      !initial_file.empty()) {
    char* res_path = app_get_resource_path();
    if (res_path) {
      std::string url =
          std::string("file://") + res_path + "flutter_assets/" + initial_file;
      free(res_path);
      NavigateProgrammatically([this, &url] { return backend_->LoadUrl(url); });
      return;
    }
  }

  flutter::EncodableMap initial_data;
  if (GetValueFromEncodableMap(*creation_params, "initialData",
                               &initial_data)) {
    std::string data;
    std::string base_url = "about:blank";
    if (GetValueFromEncodableMap(initial_data, "data", &data)) {
      GetValueFromEncodableMap(initial_data, "baseUrl", &base_url);
      NavigateProgrammatically([this, &data, &base_url] {
        return backend_->LoadHtmlString(data, base_url);
      });
      return;
    }
  }

  flutter::EncodableMap url_request;
  if (GetValueFromEncodableMap(*creation_params, "initialUrlRequest",
                               &url_request)) {
    std::string url;
    if (GetValueFromEncodableMap(url_request, "url", &url) && !url.empty()) {
      NavigateProgrammatically([this, &url] { return backend_->LoadUrl(url); });
    }
  }
}

void WebView::HandleWebViewMethodCall(const FlMethodCall& method_call,
                                      std::unique_ptr<FlMethodResult> result) {
  const std::string& method_name = method_call.method_name();
  const flutter::EncodableValue* arguments = method_call.arguments();

  if (!backend_) {
    result->Error("Invalid operation",
                  "The webview instance has not been initialized.");
    return;
  }

  if (method_name == "loadUrl") {
    flutter::EncodableMap url_request;
    if (!GetValueFromEncodableMap(arguments, "urlRequest", &url_request)) {
      result->Error("Invalid argument", "No urlRequest provided.");
      return;
    }

    std::string url;
    if (!GetValueFromEncodableMap(url_request, "url", &url)) {
      result->Error("Invalid argument", "No url provided.");
      return;
    }

    std::string method = "GET";
    GetValueFromEncodableMap(url_request, "method", &method);
    flutter::EncodableMap headers;
    GetValueFromEncodableMap(url_request, "headers", &headers);
    std::vector<uint8_t> body;
    GetValueFromEncodableMap(url_request, "body", &body);

    if (method == "POST" || !headers.empty() || !body.empty()) {
      std::map<std::string, std::string> request_headers;
      for (const auto& header : headers) {
        auto key = std::get_if<std::string>(&header.first);
        auto value = std::get_if<std::string>(&header.second);
        if (key && value) {
          request_headers[*key] = *value;
        }
      }
      const int32_t request_method = method == "POST" ? 1 : 0;
      const bool ret = NavigateProgrammatically([&] {
        return backend_->LoadUrlRequest(url, request_method, request_headers,
                                        body);
      });
      if (!ret) {
        result->Error("Operation failed", "Failed to load URL request.");
        return;
      }
    } else {
      if (!NavigateProgrammatically(
              [this, &url] { return backend_->LoadUrl(url); })) {
        result->Error("Operation failed", "Failed to load URL.");
        return;
      }
    }
    result->Success();
  } else if (method_name == "postUrl") {
    std::string url;
    std::vector<uint8_t> body;
    if (!GetValueFromEncodableMap(arguments, "url", &url)) {
      result->Error("Invalid argument", "No url provided.");
      return;
    }
    GetValueFromEncodableMap(arguments, "postData", &body);
    const bool ret = NavigateProgrammatically(
        [&] { return backend_->LoadUrlRequest(url, 1, {}, body); });
    if (ret) {
      result->Success();
    } else {
      result->Error("Operation failed", "Failed to submit POST request.");
    }
  } else if (method_name == "loadData") {
    std::string data, base_url;
    if (!GetValueFromEncodableMap(arguments, "data", &data)) {
      result->Error("Invalid argument", "No data provided.");
      return;
    }
    GetValueFromEncodableMap(arguments, "baseUrl", &base_url);
    // Bypasses the navigation policy, so clear any stale cancellation here.
    is_navigation_cancelled_ = false;
    if (!NavigateProgrammatically([this, &data, &base_url] {
          return backend_->LoadHtmlString(data, base_url);
        })) {
      result->Error("Operation failed", "Failed to load data.");
      return;
    }
    result->Success();
  } else if (method_name == "loadFile") {
    std::string file_path;
    if (!GetValueFromEncodableMap(arguments, "assetFilePath", &file_path)) {
      result->Error("Invalid argument", "No assetFilePath provided.");
      return;
    }
    std::string url;
    if (!file_path.empty() && file_path[0] == '/') {
      url = std::string("file://") + file_path;
    } else {
      char* res_path = app_get_resource_path();
      if (!res_path) {
        result->Error("Operation failed", "Could not get app resource path.");
        return;
      }
      url = std::string("file://") + res_path + "flutter_assets/" + file_path;
      free(res_path);
    }
    if (!NavigateProgrammatically(
            [this, &url] { return backend_->LoadUrl(url); })) {
      result->Error("Operation failed", "Failed to load file.");
      return;
    }
    result->Success();
  } else if (method_name == "canGoBack") {
    result->Success(flutter::EncodableValue(backend_->CanGoBack()));
  } else if (method_name == "canGoForward") {
    result->Success(flutter::EncodableValue(backend_->CanGoForward()));
  } else if (method_name == "goBack") {
    NavigateProgrammatically([this] { return backend_->GoBack(); });
    result->Success();
  } else if (method_name == "goForward") {
    NavigateProgrammatically([this] { return backend_->GoForward(); });
    result->Success();
  } else if (method_name == "reload") {
    NavigateProgrammatically([this] { return backend_->Reload(); });
    result->Success();
  } else if (method_name == "getUrl") {
    if (!committed_url_.empty()) {
      result->Success(flutter::EncodableValue(committed_url_));
    } else {
      result->Success(ToNullableString(backend_->GetCurrentUrl()));
    }
  } else if (method_name == "getTitle") {
    result->Success(ToNullableString(backend_->GetTitle()));
  } else if (method_name == "getProgress") {
    result->Success(
        flutter::EncodableValue(static_cast<int>(backend_->GetProgress())));
  } else if (method_name == "stopLoading") {
    backend_->Stop();
    result->Success();
  } else if (method_name == "evaluateJavascript") {
    std::string javascript;
    if (!GetValueFromEncodableMap(arguments, "source", &javascript)) {
      result->Error("Invalid argument", "No source provided.");
      return;
    }
    auto pending = std::make_shared<JavaScriptReply>(std::move(result));
    pending_js_replies_.erase(
        std::remove_if(pending_js_replies_.begin(), pending_js_replies_.end(),
                       [](const std::weak_ptr<JavaScriptReply>& reply) {
                         return reply.expired();
                       }),
        pending_js_replies_.end());
    pending_js_replies_.push_back(pending);
    backend_->EvaluateJavaScript(
        javascript, [pending](bool success, const char* result_value) {
          if (success) {
            pending->Succeed(result_value);
          } else {
            pending->Fail();
          }
        });
  } else if (method_name == "clearCache") {
    backend_->ClearCache();
    result->Success();
  } else if (method_name == "scrollTo" || method_name == "scrollBy") {
    int32_t x = 0, y = 0;
    if (!GetValueFromEncodableMap(arguments, "x", &x) ||
        !GetValueFromEncodableMap(arguments, "y", &y)) {
      result->Error("Invalid argument", "No x or y provided.");
      return;
    }
    if (method_name == "scrollTo") {
      backend_->ScrollTo(x, y);
      target_scroll_x_ = x;
      target_scroll_y_ = y;
      target_scroll_set_time_ = std::chrono::steady_clock::now();
    } else {
      int32_t current_x = 0, current_y = 0;
      backend_->GetScrollPosition(&current_x, &current_y);
      // Only trust a pending target briefly; it can go stale (manual
      // scroll, engine clamping) since ScrollTo() applies asynchronously.
      constexpr auto kTargetTtl = std::chrono::milliseconds(100);
      const bool target_fresh =
          std::chrono::steady_clock::now() - target_scroll_set_time_ <
          kTargetTtl;
      int32_t base_x = (target_fresh && target_scroll_x_ >= 0)
                           ? target_scroll_x_
                           : current_x;
      int32_t base_y = (target_fresh && target_scroll_y_ >= 0)
                           ? target_scroll_y_
                           : current_y;
      target_scroll_x_ = base_x + x;
      target_scroll_y_ = base_y + y;
      target_scroll_set_time_ = std::chrono::steady_clock::now();
      backend_->ScrollTo(target_scroll_x_, target_scroll_y_);
    }
    int32_t new_x = target_scroll_x_;
    int32_t new_y = target_scroll_y_;
    flutter::EncodableMap args = {
        {flutter::EncodableValue("x"), flutter::EncodableValue(new_x)},
        {flutter::EncodableValue("y"), flutter::EncodableValue(new_y)},
    };
    webview_channel_->InvokeMethod(
        "onScrollChanged", std::make_unique<flutter::EncodableValue>(args));
    result->Success();
  } else if (method_name == "getScrollX" || method_name == "getScrollY") {
    int32_t x = 0, y = 0;
    backend_->GetScrollPosition(&x, &y);
    if (method_name == "getScrollX") {
      if (target_scroll_x_ >= 0) {
        x = target_scroll_x_;
        target_scroll_x_ = -1;
      }
    } else {
      if (target_scroll_y_ >= 0) {
        y = target_scroll_y_;
        target_scroll_y_ = -1;
      }
    }
    result->Success(
        flutter::EncodableValue(method_name == "getScrollX" ? x : y));
  } else if (method_name == "zoomBy") {
    double zoom_factor = 1.0;
    if (!GetValueFromEncodableMap(arguments, "zoomFactor", &zoom_factor)) {
      result->Error("Invalid argument", "No zoomFactor provided.");
      return;
    }
    const double old_scale = backend_->GetScale();
    const double new_scale = old_scale * zoom_factor;
    backend_->SetScale(new_scale, 0, 0);
    flutter::EncodableMap args = {
        {flutter::EncodableValue("oldScale"),
         flutter::EncodableValue(old_scale)},
        {flutter::EncodableValue("newScale"),
         flutter::EncodableValue(new_scale)},
    };
    webview_channel_->InvokeMethod(
        "onZoomScaleChanged", std::make_unique<flutter::EncodableValue>(args));
    result->Success();
  } else if (method_name == "setSettings") {
    flutter::EncodableMap settings;
    if (GetValueFromEncodableMap(arguments, "settings", &settings)) {
      ApplySettings(settings);
      result->Success();
    } else {
      result->Error("Invalid argument", "No settings provided.");
    }
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
    // A null argument signals that the prompt was cancelled; pass nullptr so
    // the JavaScript prompt() call resolves to null.
    const auto* value = std::get_if<std::string>(arguments);
    backend_->JavaScriptPromptReply(value ? value->c_str() : nullptr);
    result->Success();
  } else {
    result->NotImplemented();
  }
}

FlutterDesktopGpuSurfaceDescriptor* RenderState::ObtainGpuSurface(
    size_t width, size_t height) {
  std::lock_guard<std::mutex> lock(mutex);
  if (!candidate) {
    if (rendered) {
      if (!rendered->MarkInUse()) {
        return nullptr;
      }
      return rendered->GpuSurface();
    }
    return nullptr;
  }
  rendered = candidate;
  candidate = nullptr;
  return rendered->GpuSurface();
}

void WebView::OnFrameRendered(void* tbm_surface) {
  RenderState& state = *render_state_;
  std::lock_guard<std::mutex> lock(state.mutex);
  if (!state.working) {
    if (state.candidate) {
      state.pool->Release(state.candidate);
      state.candidate = nullptr;
    }
    state.working = state.pool->GetAvailableBuffer();
    if (!state.working) {
      return;
    }
    state.working->UseExternalBuffer();
  }
  state.working->SetExternalBuffer(static_cast<tbm_surface_h>(tbm_surface));

  if (state.candidate) {
    state.pool->Release(state.candidate);
    state.candidate = nullptr;
  }
  state.candidate = state.working;
  state.working = nullptr;
  texture_registrar_->MarkTextureFrameAvailable(GetTextureId());
}

void WebView::OnLoadStarted(const std::string& url) {
  is_programmatic_navigation_ = false;
  target_scroll_x_ = -1;
  target_scroll_y_ = -1;
  flutter::EncodableMap args = {
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)}};
  webview_channel_->InvokeMethod(
      "onLoadStart", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnLoadFinished(const std::string& url) {
  is_programmatic_navigation_ = false;
  flutter::EncodableMap args = {
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)}};
  webview_channel_->InvokeMethod(
      "onLoadStop", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnProgress(int32_t progress) {
  flutter::EncodableMap args = {
      {flutter::EncodableValue("progress"), flutter::EncodableValue(progress)}};
  webview_channel_->InvokeMethod(
      "onProgressChanged", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnLoadError(int32_t error_code, const std::string& description,
                          const std::string& failing_url) {
  is_programmatic_navigation_ = false;
  target_scroll_x_ = -1;
  target_scroll_y_ = -1;

  flutter::EncodableMap args = {
      {flutter::EncodableValue("request"),
       flutter::EncodableValue(CreateRequestMap(failing_url))},
      {flutter::EncodableValue("error"),
       flutter::EncodableValue(CreateErrorMap(description, error_code))},
  };
  webview_channel_->InvokeMethod(
      "onReceivedError", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnConsoleMessage(const std::string& level,
                               const std::string& message) {
  if (!webview_channel_) {
    return;
  }
  flutter::EncodableMap args = {
      {flutter::EncodableValue("messageLevel"),
       flutter::EncodableValue(ConvertLogLevel(level))},
      {flutter::EncodableValue("message"), flutter::EncodableValue(message)},
  };
  webview_channel_->InvokeMethod(
      "onConsoleMessage", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnNavigationPolicyDecide(const std::string& url,
                                       const std::string& current_url) {
  // A new decision means any prior cancellation is now stale.
  is_navigation_cancelled_ = false;

  if (is_programmatic_navigation_) {
    // App-initiated navigations skip shouldOverrideUrlLoading, and must not be
    // suspended.
    is_programmatic_navigation_ = false;
    return;
  }

  if (!has_navigation_delegate_) {
    return;
  }

  // Captured before the decision was accepted, so it is the URL to roll back
  // to if Dart cancels this navigation.
  url_before_navigation_ = current_url;

  // Suspended until NavigationRequestResult resumes or stops it.
  backend_->Suspend();

  flutter::EncodableMap args = CreateNavigationActionMap(url);
  auto result = std::make_unique<NavigationRequestResult>(this, lifetime_);
  webview_channel_->InvokeMethod(
      "shouldOverrideUrlLoading",
      std::make_unique<flutter::EncodableValue>(args), std::move(result));
}

void WebView::OnUrlChanged(const std::string& url) {
  if (is_navigation_cancelled_) {
    // Drop only this one stale event, then clear the flag, or a later
    // same-document change (pushState/replaceState) would be ignored too.
    is_navigation_cancelled_ = false;
    return;
  }
  committed_url_ = url;
  flutter::EncodableMap args = {
      {flutter::EncodableValue("url"), flutter::EncodableValue(committed_url_)},
      {flutter::EncodableValue("isReload"), flutter::EncodableValue(false)}};
  webview_channel_->InvokeMethod(
      "onUpdateVisitedHistory",
      std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnTitleChanged(const std::string& title) {
  flutter::EncodableMap args = {
      {flutter::EncodableValue("title"), flutter::EncodableValue(title)}};
  webview_channel_->InvokeMethod(
      "onTitleChanged", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnJavaScriptAlertDialog(const std::string& message,
                                      const std::string& url) {
  flutter::EncodableMap args = {
      {flutter::EncodableValue("message"), flutter::EncodableValue(message)},
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)},
      {flutter::EncodableValue("isMainFrame"), flutter::EncodableValue(true)}};
  webview_channel_->InvokeMethod(
      "onJsAlert", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnJavaScriptConfirmDialog(const std::string& message,
                                        const std::string& url) {
  flutter::EncodableMap args = {
      {flutter::EncodableValue("message"), flutter::EncodableValue(message)},
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)},
      {flutter::EncodableValue("isMainFrame"), flutter::EncodableValue(true)}};
  webview_channel_->InvokeMethod(
      "onJsConfirm", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnJavaScriptPromptDialog(const std::string& message,
                                       const std::string& default_text,
                                       const std::string& url) {
  flutter::EncodableMap args = {
      {flutter::EncodableValue("message"), flutter::EncodableValue(message)},
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)},
      {flutter::EncodableValue("defaultValue"),
       flutter::EncodableValue(default_text)},
      {flutter::EncodableValue("isMainFrame"), flutter::EncodableValue(true)}};
  webview_channel_->InvokeMethod(
      "onJsPrompt", std::make_unique<flutter::EncodableValue>(args));
}
