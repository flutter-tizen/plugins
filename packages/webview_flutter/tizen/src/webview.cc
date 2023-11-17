// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview.h"

#include <Ecore_Evas.h>
#include <app_common.h>
#include <flutter/standard_method_codec.h>
#include <flutter_texture_registrar.h>
#include <tbm_surface.h>

#include <ostream>

#include "buffer_pool.h"
#include "ewk_internal_api_binding.h"
#include "log.h"
#include "webview_factory.h"

namespace {

constexpr size_t kBufferPoolSize = 5;
constexpr char kEwkInstance[] = "ewk_instance";
constexpr char kTizenWebViewChannelName[] = "plugins.flutter.io/tizen_webview_";
constexpr char kTizenWebViewControllerChannelName[] =
    "plugins.flutter.io/tizen_webview_controller_";
constexpr char kTizenNavigationDelegateChannelName[] =
    "plugins.flutter.io/tizen_webview_navigation_delegate_";

std::string ConvertLogLevelToString(Ewk_Console_Message_Level level) {
  switch (level) {
    case EWK_CONSOLE_MESSAGE_LEVEL_NULL:
    case EWK_CONSOLE_MESSAGE_LEVEL_LOG:
      return "log";
    case EWK_CONSOLE_MESSAGE_LEVEL_WARNING:
      return "warning";
    case EWK_CONSOLE_MESSAGE_LEVEL_ERROR:
      return "error";
    case EWK_CONSOLE_MESSAGE_LEVEL_DEBUG:
      return "debug";
    case EWK_CONSOLE_MESSAGE_LEVEL_INFO:
      return "info";
    default:
      return "log";
  }
}

class NavigationRequestResult : public FlMethodResult {
 public:
  NavigationRequestResult(WebView* webview) : webview_(webview) {}

  void SuccessInternal(const flutter::EncodableValue* should_load) override {
    if (std::holds_alternative<bool>(*should_load)) {
      if (std::get<bool>(*should_load)) {
        webview_->Resume();
        return;
      }
    }
    webview_->Stop();
  }

  void ErrorInternal(const std::string& error_code,
                     const std::string& error_message,
                     const flutter::EncodableValue* error_details) override {
    LOG_ERROR("The request unexpectedly completed with an error.");
    webview_->Stop();
  }

  void NotImplementedInternal() override {
    LOG_ERROR("The target method was unexpectedly unimplemented.");
    webview_->Stop();
  }

 private:
  WebView* webview_;
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
  if (!EwkInternalApiBinding::GetInstance().Initialize()) {
    LOG_ERROR("Failed to initialize EWK internal APIs.");
    return;
  }

  tbm_pool_ = std::make_unique<SingleBufferPool>(width, height);

  texture_variant_ =
      std::make_unique<flutter::TextureVariant>(flutter::GpuSurfaceTexture(
          kFlutterDesktopGpuSurfaceTypeNone,
          [this](size_t width,
                 size_t height) -> const FlutterDesktopGpuSurfaceDescriptor* {
            return ObtainGpuSurface(width, height);
          }));
  SetTextureId(texture_registrar_->RegisterTexture(texture_variant_.get()));

  InitWebView();

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

/**
 * Added as a JavaScript interface to the WebView for any JavaScript channel
 * that the Dart code sets up.
 *
 * Exposes a single method named `postMessage` to JavaScript, which sends a
 * message over a method channel to the Dart code.
 */
void WebView::RegisterJavaScriptChannelName(const std::string& name) {
  ewk_view_javascript_message_handler_add(
      webview_instance_, &WebView::OnJavaScriptMessage, name.c_str());
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
  texture_registrar_->UnregisterTexture(GetTextureId(), nullptr);

  if (webview_instance_) {
    evas_object_smart_callback_del(webview_instance_,
                                   "offscreen,frame,rendered",
                                   &WebView::OnFrameRendered);
    evas_object_smart_callback_del(webview_instance_, "load,started",
                                   &WebView::OnLoadStarted);
    evas_object_smart_callback_del(webview_instance_, "load,finished",
                                   &WebView::OnLoadFinished);
    evas_object_smart_callback_del(webview_instance_, "load,progress",
                                   &WebView::OnProgress);
    evas_object_smart_callback_del(webview_instance_, "load,error",
                                   &WebView::OnLoadError);
    evas_object_smart_callback_del(webview_instance_, "console,message",
                                   &WebView::OnConsoleMessage);
    evas_object_smart_callback_del(webview_instance_,
                                   "policy,navigation,decide",
                                   &WebView::OnNavigationPolicy);
    evas_object_smart_callback_del(webview_instance_, "url,changed",
                                   &WebView::OnUrlChange);
    evas_object_del(webview_instance_);
  }
}

void WebView::Offset(double left, double top) {
  left_ = left;
  top_ = top;

  evas_object_move(webview_instance_, static_cast<int>(left_),
                   static_cast<int>(top_));
}

void WebView::Resize(double width, double height) {
  width_ = width;
  height_ = height;

  if (candidate_surface_) {
    candidate_surface_ = nullptr;
  }

  tbm_pool_->Prepare(width_, height_);
  evas_object_resize(webview_instance_, width_, height_);
}

void WebView::Touch(int type, int button, double x, double y, double dx,
                    double dy) {
  Ewk_Touch_Event_Type mouse_event_type = EWK_TOUCH_START;
  Evas_Touch_Point_State state = EVAS_TOUCH_POINT_DOWN;
  if (type == 0) {  // down event
    mouse_event_type = EWK_TOUCH_START;
    state = EVAS_TOUCH_POINT_DOWN;
  } else if (type == 1) {  // move event
    mouse_event_type = EWK_TOUCH_MOVE;
    state = EVAS_TOUCH_POINT_MOVE;
  } else if (type == 2) {  // up event
    mouse_event_type = EWK_TOUCH_END;
    state = EVAS_TOUCH_POINT_UP;
  } else {
    LOG_WARN("Unknown touch event type: %d", type);
  }

  Eina_List* points = 0;
  Ewk_Touch_Point* point = new Ewk_Touch_Point;
  point->id = 0;
  point->x = x + left_;
  point->y = y + top_;
  point->state = state;
  points = eina_list_append(points, point);

  EwkInternalApiBinding::GetInstance().view.FeedTouchEvent(
      webview_instance_, mouse_event_type, points, 0);
  eina_list_free(points);
}

bool WebView::SendKey(const char* key, const char* string, const char* compose,
                      uint32_t modifiers, uint32_t scan_code, bool is_down) {
  if (!IsFocused()) {
    return false;
  }

  if (strcmp(key, "XF86Back") == 0 && !is_down &&
      ewk_view_back_possible(webview_instance_)) {
    ewk_view_back(webview_instance_);
    return true;
  }

  if (is_down) {
    // TODO(swift-kim): Deal with other members of the structure.
    Evas_Event_Key_Down down_event = {};
    down_event.key = key;
    down_event.string = string;
    EwkInternalApiBinding::GetInstance().view.SendKeyEvent(
        webview_instance_, &down_event, is_down);
  } else {
    Evas_Event_Key_Up up_event = {};
    up_event.key = key;
    up_event.string = string;
    EwkInternalApiBinding::GetInstance().view.SendKeyEvent(webview_instance_,
                                                           &up_event, is_down);
  }
  return true;
}

void WebView::Resume() { ewk_view_resume(webview_instance_); }

void WebView::Stop() { ewk_view_stop(webview_instance_); }

void WebView::SetDirection(int direction) {
  // TODO: Implement if necessary.
}

void WebView::InitWebView() {
  char* chromium_argv[] = {
      const_cast<char*>("--disable-pinch"),
      const_cast<char*>("--js-flags=--expose-gc"),
      const_cast<char*>("--single-process"),
      const_cast<char*>("--no-zygote"),
  };
  int chromium_argc = sizeof(chromium_argv) / sizeof(chromium_argv[0]);
  EwkInternalApiBinding::GetInstance().main.SetArguments(chromium_argc,
                                                         chromium_argv);

  ewk_init();
  Ecore_Evas* evas = ecore_evas_new("wayland_egl", 0, 0, 1, 1, 0);

  webview_instance_ = ewk_view_add(ecore_evas_get(evas));
  ecore_evas_focus_set(evas, true);
  ewk_view_focus_set(webview_instance_, true);
  EwkInternalApiBinding::GetInstance().view.OffscreenRenderingEnabledSet(
      webview_instance_, true);

  Ewk_Context* context = ewk_view_context_get(webview_instance_);
  Ewk_Cookie_Manager* cookie_manager = ewk_context_cookie_manager_get(context);
  if (cookie_manager) {
    ewk_cookie_manager_accept_policy_set(
        cookie_manager, EWK_COOKIE_ACCEPT_POLICY_NO_THIRD_PARTY);
  }
  ewk_context_cache_model_set(context, EWK_CACHE_MODEL_PRIMARY_WEBBROWSER);

  EwkInternalApiBinding::GetInstance().settings.ImePanelEnabledSet(
      ewk_view_settings_get(webview_instance_), true);
  EwkInternalApiBinding::GetInstance().view.ImeWindowSet(webview_instance_,
                                                         window_);
  EwkInternalApiBinding::GetInstance().view.KeyEventsEnabledSet(
      webview_instance_, true);

#ifdef TV_PROFILE
  EwkInternalApiBinding::GetInstance().view.SupportVideoHoleSet(
      webview_instance_, window_, true, false);
#endif

  evas_object_smart_callback_add(webview_instance_, "offscreen,frame,rendered",
                                 &WebView::OnFrameRendered, this);
  evas_object_smart_callback_add(webview_instance_, "load,started",
                                 &WebView::OnLoadStarted, this);
  evas_object_smart_callback_add(webview_instance_, "load,finished",
                                 &WebView::OnLoadFinished, this);
  evas_object_smart_callback_add(webview_instance_, "load,progress",
                                 &WebView::OnProgress, this);
  evas_object_smart_callback_add(webview_instance_, "load,error",
                                 &WebView::OnLoadError, this);
  evas_object_smart_callback_add(webview_instance_, "console,message",
                                 &WebView::OnConsoleMessage, this);
  evas_object_smart_callback_add(webview_instance_, "policy,navigation,decide",
                                 &WebView::OnNavigationPolicy, this);
  evas_object_smart_callback_add(webview_instance_, "url,changed",
                                 &WebView::OnUrlChange, this);

  Resize(width_, height_);
  evas_object_show(webview_instance_);

  evas_object_data_set(webview_instance_, kEwkInstance, this);
}

void WebView::HandleWebViewMethodCall(const FlMethodCall& method_call,
                                      std::unique_ptr<FlMethodResult> result) {
  if (!webview_instance_) {
    result->Error("Invalid operation",
                  "The webview instance has not been initialized.");
    return;
  }

  const std::string& method_name = method_call.method_name();
  const flutter::EncodableValue* arguments = method_call.arguments();

  if (method_name == "javaScriptMode") {
    const auto* mode = std::get_if<int32_t>(arguments);
    if (mode) {
      bool enabled = (*mode == 1);
      ewk_settings_javascript_enabled_set(
          ewk_view_settings_get(webview_instance_), enabled);
    }
    result->Success();
  } else if (method_name == "hasNavigationDelegate") {
    const auto* has_navigation_delegate = std::get_if<bool>(arguments);
    if (has_navigation_delegate) {
      has_navigation_delegate_ = *has_navigation_delegate;
    }
    result->Success();
  } else if (method_name == "loadRequest") {
    std::string url;
    if (GetValueFromEncodableMap(arguments, "url", &url)) {
      ewk_view_url_set(webview_instance_, url.c_str());
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

    Ewk_Http_Method ewk_method = EWK_HTTP_METHOD_GET;
    int32_t method = 0;
    GetValueFromEncodableMap(arguments, "method", &method);
    if (method == 1) {  // Post request.
      ewk_method = EWK_HTTP_METHOD_POST;
    }

    Eina_Hash* ewk_headers = eina_hash_new(
        [](const void* key) -> unsigned int {
          return key ? strlen(static_cast<const char*>(key)) + 1 : 0;
        },
        [](const void* key1, int key1_length, const void* key2,
           int key2_length) -> int {
          return strcmp(static_cast<const char*>(key1),
                        static_cast<const char*>(key2));
        },
        EINA_KEY_HASH(eina_hash_superfast), [](void* data) { free(data); }, 10);
    flutter::EncodableMap headers;
    GetValueFromEncodableMap(arguments, "headers", &headers);
    for (const auto& header : headers) {
      auto key = std::get_if<std::string>(&header.first);
      auto value = std::get_if<std::string>(&header.second);
      if (key && value) {
        eina_hash_add(ewk_headers, key->c_str(), strdup(value->c_str()));
      }
    }

    std::vector<uint8_t> body;
    if (GetValueFromEncodableMap(arguments, "body", &body)) {
      body.push_back('\0');
    }

    bool ret = ewk_view_url_request_set(
        webview_instance_, url.c_str(), ewk_method, ewk_headers,
        reinterpret_cast<const char*>(body.data()));
    eina_hash_free(ewk_headers);
    if (ret) {
      result->Success();
    } else {
      result->Error("Operation failed",
                    "Failed to load request with parameters.");
    }
  } else if (method_name == "canGoBack") {
    result->Success(flutter::EncodableValue(
        static_cast<bool>(ewk_view_back_possible(webview_instance_))));
  } else if (method_name == "canGoForward") {
    result->Success(flutter::EncodableValue(
        static_cast<bool>(ewk_view_forward_possible(webview_instance_))));
  } else if (method_name == "goBack") {
    ewk_view_back(webview_instance_);
    result->Success();
  } else if (method_name == "goForward") {
    ewk_view_forward(webview_instance_);
    result->Success();
  } else if (method_name == "reload") {
    ewk_view_reload(webview_instance_);
    result->Success();
  } else if (method_name == "currentUrl") {
    result->Success(
        flutter::EncodableValue(ewk_view_url_get(webview_instance_)));
  } else if (method_name == "evaluateJavaScript" ||
             method_name == "runJavaScriptReturningResult" ||
             method_name == "runJavaScript") {
    const auto* javascript = std::get_if<std::string>(arguments);
    if (javascript) {
      ewk_view_script_execute(webview_instance_, javascript->c_str(),
                              &WebView::OnEvaluateJavaScript, result.release());
    } else {
      result->Error("Invalid argument", "The argument must be a string.");
    }
  } else if (method_name == "addJavaScriptChannel") {
    const auto* channel = std::get_if<std::string>(arguments);
    if (channel) {
      RegisterJavaScriptChannelName(*channel);
      result->Success();
    } else {
      result->Error("Invalid argument", "The argument must be a string.");
    }
  } else if (method_name == "clearCache") {
    Ewk_Context* context = ewk_view_context_get(webview_instance_);
    ewk_context_resource_cache_clear(context);
    result->Success();
  } else if (method_name == "getTitle") {
    result->Success(flutter::EncodableValue(
        std::string(ewk_view_title_get(webview_instance_))));
  } else if (method_name == "scrollTo") {
    int32_t x = 0, y = 0;
    if (GetValueFromEncodableMap(arguments, "x", &x) &&
        GetValueFromEncodableMap(arguments, "y", &y)) {
      ewk_view_scroll_set(webview_instance_, x, y);
      result->Success();
    } else {
      result->Error("Invalid argument", "No x or y provided.");
    }
  } else if (method_name == "scrollBy") {
    int32_t x = 0, y = 0;
    if (GetValueFromEncodableMap(arguments, "x", &x) &&
        GetValueFromEncodableMap(arguments, "y", &y)) {
      ewk_view_scroll_by(webview_instance_, x, y);
      result->Success();
    } else {
      result->Error("Invalid argument", "No x or y provided.");
    }
  } else if (method_name == "getScrollPosition") {
    int32_t x = 0, y = 0;
    // TODO(jsuya) : ewk_view_scroll_pos_get() returns the position set in
    // ewk_view_scroll_set(). Therefore, it currently does not work as intended.
    ewk_view_scroll_pos_get(webview_instance_, &x, &y);
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
        ewk_view_url_set(webview_instance_, url.c_str());
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
    ewk_view_html_string_load(webview_instance_, html.c_str(), base_url.c_str(),
                              nullptr);
    result->Success();
  } else if (method_name == "loadFile") {
    const auto* file_path = std::get_if<std::string>(arguments);
    if (file_path) {
      std::string url = std::string("file://") + *file_path;
      ewk_view_url_set(webview_instance_, url.c_str());
      result->Success();
    } else {
      result->Error("Invalid argument", "The argument must be a string.");
    }
  } else if (method_name == "backgroundColor") {
    const auto* color = std::get_if<int32_t>(arguments);
    if (color) {
      EwkInternalApiBinding::GetInstance().view.SetBackgroundColor(
          webview_instance_, *color >> 16 & 0xff, *color >> 8 & 0xff,
          *color & 0xff, *color >> 24 & 0xff);
      result->Success();
    }
  } else if (method_name == "setUserAgent") {
    const auto* userAgent = std::get_if<std::string>(arguments);
    if (userAgent) {
      ewk_view_user_agent_set(webview_instance_, userAgent->c_str());
    }
    result->Success();
  } else if (method_name == "getUserAgent") {
    result->Success(flutter::EncodableValue(
        std::string(ewk_view_user_agent_get(webview_instance_))));
  } else {
    result->NotImplemented();
  }
}

void WebView::HandleCookieMethodCall(const FlMethodCall& method_call,
                                     std::unique_ptr<FlMethodResult> result) {
  if (!webview_instance_) {
    result->Error("Invalid operation",
                  "The webview instance has not been initialized.");
    return;
  }

  const std::string& method_name = method_call.method_name();

  if (method_name == "clearCookies") {
    Ewk_Context* context = ewk_view_context_get(webview_instance_);
    Ewk_Cookie_Manager* cookie_manager =
        ewk_context_cookie_manager_get(context);
    if (cookie_manager) {
      ewk_cookie_manager_cookies_clear(cookie_manager);
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

void WebView::OnFrameRendered(void* data, Evas_Object* obj, void* event_info) {
  if (event_info) {
    WebView* webview = static_cast<WebView*>(data);

    std::lock_guard<std::mutex> lock(webview->mutex_);
    if (!webview->working_surface_) {
      webview->working_surface_ = webview->tbm_pool_->GetAvailableBuffer();
      webview->working_surface_->UseExternalBuffer();
    }
    webview->working_surface_->SetExternalBuffer(
        static_cast<tbm_surface_h>(event_info));

    if (webview->candidate_surface_) {
      webview->tbm_pool_->Release(webview->candidate_surface_);
      webview->candidate_surface_ = nullptr;
    }
    webview->candidate_surface_ = webview->working_surface_;
    webview->working_surface_ = nullptr;
    webview->texture_registrar_->MarkTextureFrameAvailable(
        webview->GetTextureId());
  }
}

void WebView::OnLoadStarted(void* data, Evas_Object* obj, void* event_info) {
  WebView* webview = static_cast<WebView*>(data);
  std::string url = std::string(ewk_view_url_get(webview->webview_instance_));
  flutter::EncodableMap args = {
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)}};
  webview->navigation_delegate_channel_->InvokeMethod(
      "onPageStarted", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnLoadFinished(void* data, Evas_Object* obj, void* event_info) {
  WebView* webview = static_cast<WebView*>(data);
  std::string url = std::string(ewk_view_url_get(webview->webview_instance_));
  flutter::EncodableMap args = {
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)}};
  webview->navigation_delegate_channel_->InvokeMethod(
      "onPageFinished", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnProgress(void* data, Evas_Object* obj, void* event_info) {
  WebView* webview = static_cast<WebView*>(data);
  std::string url = std::string(ewk_view_url_get(webview->webview_instance_));
  int32_t progress =
      static_cast<int32_t>((*static_cast<double*>(event_info)) * 100);
  flutter::EncodableMap args = {
      {flutter::EncodableValue("progress"), flutter::EncodableValue(progress)}};
  webview->navigation_delegate_channel_->InvokeMethod(
      "onProgress", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnLoadError(void* data, Evas_Object* obj, void* event_info) {
  WebView* webview = static_cast<WebView*>(data);
  Ewk_Error* error = static_cast<Ewk_Error*>(event_info);

  flutter::EncodableMap args = {
      {flutter::EncodableValue("errorCode"),
       flutter::EncodableValue(ewk_error_code_get(error))},
      {flutter::EncodableValue("description"),
       flutter::EncodableValue(ewk_error_description_get(error))},
      {flutter::EncodableValue("failingUrl"),
       flutter::EncodableValue(ewk_error_url_get(error))},
  };
  webview->navigation_delegate_channel_->InvokeMethod(
      "onWebResourceError", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnConsoleMessage(void* data, Evas_Object* obj, void* event_info) {
  Ewk_Console_Message* message = static_cast<Ewk_Console_Message*>(event_info);
  Ewk_Console_Message_Level log_level =
      EwkInternalApiBinding::GetInstance().console_message.LevelGet(message);
  std::string source =
      EwkInternalApiBinding::GetInstance().console_message.SourceGet(message);
  int32_t line =
      EwkInternalApiBinding::GetInstance().console_message.LineGet(message);
  std::string text =
      EwkInternalApiBinding::GetInstance().console_message.TextGet(message);
  std::ostream& stream =
      log_level == EWK_CONSOLE_MESSAGE_LEVEL_ERROR ? std::cerr : std::cout;
  stream << "WebView: ";
  if (!source.empty() && line > 0) {
    stream << source << "(" << line << ") > ";
  }
  stream << text << std::endl;

  if (obj) {
    WebView* webview =
        static_cast<WebView*>(evas_object_data_get(obj, kEwkInstance));
    if (webview->webview_controller_channel_) {
      flutter::EncodableMap args = {
          {flutter::EncodableValue("level"),
           flutter::EncodableValue(ConvertLogLevelToString(log_level))},
          {flutter::EncodableValue("message"), flutter::EncodableValue(text)},
      };
      webview->webview_controller_channel_->InvokeMethod(
          "onConsoleMessage", std::make_unique<flutter::EncodableValue>(args));
    }
  }
}

void WebView::OnNavigationPolicy(void* data, Evas_Object* obj,
                                 void* event_info) {
  WebView* webview = static_cast<WebView*>(data);
  Ewk_Policy_Decision* policy_decision =
      static_cast<Ewk_Policy_Decision*>(event_info);
  ewk_policy_decision_use(policy_decision);
  if (!webview->has_navigation_delegate_) {
    return;
  }
  ewk_view_suspend(webview->webview_instance_);

  const char* url = ewk_policy_decision_url_get(policy_decision);
  flutter::EncodableMap args = {
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)},
      {flutter::EncodableValue("isForMainFrame"),
       flutter::EncodableValue(true)},
  };
  auto result = std::make_unique<NavigationRequestResult>(webview);
  webview->navigation_delegate_channel_->InvokeMethod(
      "navigationRequest", std::make_unique<flutter::EncodableValue>(args),
      std::move(result));
}

void WebView::OnUrlChange(void* data, Evas_Object* obj, void* event_info) {
  WebView* webview = static_cast<WebView*>(data);
  std::string url = std::string(ewk_view_url_get(webview->webview_instance_));
  flutter::EncodableMap args = {
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)}};
  webview->navigation_delegate_channel_->InvokeMethod(
      "onUrlChange", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnEvaluateJavaScript(Evas_Object* obj, const char* result_value,
                                   void* user_data) {
  FlMethodResult* result = static_cast<FlMethodResult*>(user_data);
  if (result_value) {
    result->Success(flutter::EncodableValue(result_value));
  } else {
    result->Success();
  }
  delete result;
}

void WebView::OnJavaScriptMessage(Evas_Object* obj,
                                  Ewk_Script_Message message) {
  if (obj) {
    WebView* webview =
        static_cast<WebView*>(evas_object_data_get(obj, kEwkInstance));
    if (webview->webview_channel_) {
      std::string channel_name(message.name);
      std::string message_body(static_cast<char*>(message.body));

      flutter::EncodableMap args = {
          {flutter::EncodableValue("channel"),
           flutter::EncodableValue(channel_name)},
          {flutter::EncodableValue("message"),
           flutter::EncodableValue(message_body)},
      };
      webview->webview_channel_->InvokeMethod(
          "javaScriptChannelMessage",
          std::make_unique<flutter::EncodableValue>(args));
    }
  }
}
