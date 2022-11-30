// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview.h"

#include <Ecore_Evas.h>
#include <app_common.h>
#include <flutter/standard_method_codec.h>
#include <flutter_texture_registrar.h>
#include <tbm_surface.h>

#include "buffer_pool.h"
#include "ewk_internal_api_binding.h"
#include "log.h"
#include "webview_factory.h"

namespace {

typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;

constexpr size_t kBufferPoolSize = 5;
constexpr char kEwkInstance[] = "ewk_instance";

class NavigationRequestResult
    : public flutter::MethodResult<flutter::EncodableValue> {
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
    webview_->Stop();
    LOG_ERROR("The request unexpectedly completed with an error.");
  }

  void NotImplementedInternal() override {
    webview_->Stop();
    LOG_ERROR("The target method was unexpectedly unimplemented.");
  }

 private:
  WebView* webview_;
};

std::string ErrorCodeToString(int error_code) {
  switch (error_code) {
    case EWK_ERROR_CODE_AUTHENTICATION:
      return "authentication";
    case EWK_ERROR_CODE_BAD_URL:
      return "badUrl";
    case EWK_ERROR_CODE_FAILED_TLS_HANDSHAKE:
      return "failedSslHandshake";
    case EWK_ERROR_CODE_FAILED_FILE_IO:
      return "file";
    case EWK_ERROR_CODE_CANT_LOOKUP_HOST:
      return "hostLookup";
    case EWK_ERROR_CODE_REQUEST_TIMEOUT:
      return "timeout";
    case EWK_ERROR_CODE_TOO_MANY_REQUESTS:
      return "tooManyRequests";
    case EWK_ERROR_CODE_UNKNOWN:
      return "unknown";
    case EWK_ERROR_CODE_UNSUPPORTED_SCHEME:
      return "unsupportedScheme";
    default:
      LOG_ERROR("Unknown error type: %d", error_code);
      return std::to_string(error_code);
  }
}

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
                 void* win)
    : PlatformView(registrar, view_id, nullptr),
      texture_registrar_(texture_registrar),
      width_(width),
      height_(height),
      win_(win) {
  if (!EwkInternalApiBinding::GetInstance().Initialize()) {
    LOG_ERROR("Failed to Initialize EWK internal APIs.");
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

  channel_ = std::make_unique<FlMethodChannel>(
      GetPluginRegistrar()->messenger(), GetChannelName(),
      &flutter::StandardMethodCodec::GetInstance());
  channel_->SetMethodCallHandler(
      [webview = this](const auto& call, auto result) {
        webview->HandleMethodCall(call, std::move(result));
      });

  auto cookie_channel = std::make_unique<FlMethodChannel>(
      GetPluginRegistrar()->messenger(), "plugins.flutter.io/cookie_manager",
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
    EwkInternalApiBinding::GetInstance().view.SetBackgroundColor(
        webview_instance_, color >> 16 & 0xff, color >> 8 & 0xff, color & 0xff,
        color >> 24 & 0xff);
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
    ewk_view_user_agent_set(webview_instance_, user_agent.c_str());
  }
  ewk_view_url_set(webview_instance_, url.c_str());
}

void WebView::ApplySettings(const flutter::EncodableMap& settings) {
  for (const auto& [key, value] : settings) {
    if (std::holds_alternative<std::string>(key)) {
      std::string string_key = std::get<std::string>(key);
      if (string_key == "jsMode") {
      } else if (string_key == "hasNavigationDelegate") {
        if (std::holds_alternative<bool>(value)) {
          has_navigation_delegate_ = std::get<bool>(value);
        }
      } else if (string_key == "hasProgressTracking") {
        if (std::holds_alternative<bool>(value)) {
          has_progress_tracking_ = std::get<bool>(value);
        }
      } else if (string_key == "debuggingEnabled") {
      } else if (string_key == "gestureNavigationEnabled") {
      } else if (string_key == "allowsInlineMediaPlayback") {
      } else if (string_key == "userAgent") {
        if (std::holds_alternative<std::string>(value)) {
          ewk_view_user_agent_set(webview_instance_,
                                  std::get<std::string>(value).c_str());
        }
      } else if (string_key == "zoomEnabled") {
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
  ewk_view_javascript_message_handler_add(
      webview_instance_, &WebView::OnJavaScriptMessage, name.c_str());
}

WebView::~WebView() { Dispose(); }

std::string WebView::GetChannelName() {
  return "plugins.flutter.io/webview_" + std::to_string(GetViewId());
}

void WebView::Dispose() {
  evas_object_smart_callback_del(webview_instance_, "offscreen,frame,rendered",
                                 &WebView::OnFrameRendered);
  evas_object_smart_callback_del(webview_instance_, "load,started",
                                 &WebView::OnLoadStarted);
  evas_object_smart_callback_del(webview_instance_, "load,finished",
                                 &WebView::OnLoadFinished);
  evas_object_smart_callback_del(webview_instance_, "load,error",
                                 &WebView::OnLoadError);
  evas_object_smart_callback_del(webview_instance_, "console,message",
                                 &WebView::OnConsoleMessage);
  evas_object_smart_callback_del(webview_instance_, "policy,navigation,decide",
                                 &WebView::OnNavigationPolicy);
  texture_registrar_->UnregisterTexture(GetTextureId());
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
    // TODO: Not implemented
  }
  Eina_List* pointList = 0;
  Ewk_Touch_Point* point = new Ewk_Touch_Point;
  point->id = 0;
  point->x = x;
  point->y = y;
  point->state = state;
  pointList = eina_list_append(pointList, point);

  EwkInternalApiBinding::GetInstance().view.FeedTouchEvent(
      webview_instance_, mouse_event_type, pointList, 0);
  eina_list_free(pointList);
}

bool WebView::SendKey(const char* key, const char* string, const char* compose,
                      uint32_t modifiers, uint32_t scan_code, bool is_down) {
  if (!IsFocused()) {
    return false;
  }

  if (is_down) {
    Evas_Event_Key_Down downEvent;
    memset(&downEvent, 0, sizeof(Evas_Event_Key_Down));
    downEvent.key = key;
    downEvent.string = string;
    void* evasKeyEvent = static_cast<void*>(&downEvent);
    EwkInternalApiBinding::GetInstance().view.SendKeyEvent(
        webview_instance_, evasKeyEvent, is_down);
    return true;
  } else {
    Evas_Event_Key_Up upEvent;
    memset(&upEvent, 0, sizeof(Evas_Event_Key_Up));
    upEvent.key = key;
    upEvent.string = string;
    void* evasKeyEvent = static_cast<void*>(&upEvent);
    EwkInternalApiBinding::GetInstance().view.SendKeyEvent(
        webview_instance_, evasKeyEvent, is_down);
    return true;
  }

  return false;
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

  Ewk_Settings* settings = ewk_view_settings_get(webview_instance_);

  Ewk_Context* context = ewk_view_context_get(webview_instance_);
  Ewk_Cookie_Manager* manager = ewk_context_cookie_manager_get(context);
  if (manager) {
    ewk_cookie_manager_accept_policy_set(
        manager, EWK_COOKIE_ACCEPT_POLICY_NO_THIRD_PARTY);
  }
  EwkInternalApiBinding::GetInstance().settings.ImePanelEnabledSet(settings,
                                                                   true);
  ewk_settings_javascript_enabled_set(settings, true);

  EwkInternalApiBinding::GetInstance().view.ImeWindowSet(webview_instance_,
                                                         win_);
  EwkInternalApiBinding::GetInstance().view.KeyEventsEnabledSet(
      webview_instance_, true);
  ewk_context_cache_model_set(context, EWK_CACHE_MODEL_PRIMARY_WEBBROWSER);

  evas_object_smart_callback_add(webview_instance_, "offscreen,frame,rendered",
                                 &WebView::OnFrameRendered, this);
  evas_object_smart_callback_add(webview_instance_, "load,started",
                                 &WebView::OnLoadStarted, this);
  evas_object_smart_callback_add(webview_instance_, "load,finished",
                                 &WebView::OnLoadFinished, this);
  evas_object_smart_callback_add(webview_instance_, "load,error",
                                 &WebView::OnLoadError, this);
  evas_object_smart_callback_add(webview_instance_, "console,message",
                                 &WebView::OnConsoleMessage, this);
  evas_object_smart_callback_add(webview_instance_, "policy,navigation,decide",
                                 &WebView::OnNavigationPolicy, this);
  Resize(width_, height_);
  evas_object_show(webview_instance_);

  evas_object_data_set(webview_instance_, kEwkInstance, this);
}

void WebView::HandleMethodCall(const FlMethodCall& method_call,
                               std::unique_ptr<FlMethodResult> result) {
  if (!webview_instance_) {
    result->Error("Invalid operation",
                  "The webview instance has not been initialized.");
    return;
  }

  const std::string& method_name = method_call.method_name();
  const flutter::EncodableValue* arguments = method_call.arguments();

  if (method_name == "loadUrl") {
    std::string url;
    if (GetValueFromEncodableMap(arguments, "url", &url)) {
      ewk_view_url_set(webview_instance_, url.c_str());
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
  } else if (method_name == "evaluateJavascript" ||
             method_name == "runJavascriptReturningResult" ||
             method_name == "runJavascript") {
    const auto* javascript = std::get_if<std::string>(arguments);
    if (javascript) {
      auto p_result = result.release();
      ewk_view_script_execute(webview_instance_, javascript->c_str(),
                              &WebView::OnEvaluateJavaScript, p_result);
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
    result->NotImplemented();
  } else if (method_name == "clearCache") {
    result->NotImplemented();
  } else if (method_name == "getTitle") {
    result->Success(flutter::EncodableValue(
        std::string(ewk_view_title_get(webview_instance_))));
  } else if (method_name == "scrollTo") {
    int x = 0, y = 0;
    if (GetValueFromEncodableMap(arguments, "x", &x) &&
        GetValueFromEncodableMap(arguments, "y", &y)) {
      ewk_view_scroll_set(webview_instance_, x, y);
      result->Success();
    } else {
      result->Error("Invalid argument", "No x or y provided.");
    }
  } else if (method_name == "scrollBy") {
    int x = 0, y = 0;
    if (GetValueFromEncodableMap(arguments, "x", &x) &&
        GetValueFromEncodableMap(arguments, "y", &y)) {
      ewk_view_scroll_by(webview_instance_, x, y);
      result->Success();
    } else {
      result->Error("Invalid argument", "No x or y provided.");
    }
  } else if (method_name == "getScrollX") {
    int x = 0;
    ewk_view_scroll_pos_get(webview_instance_, &x, nullptr);
    result->Success(flutter::EncodableValue(x));
  } else if (method_name == "getScrollY") {
    int y = 0;
    ewk_view_scroll_pos_get(webview_instance_, nullptr, &y);
    result->Success(flutter::EncodableValue(y));
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
    if (GetValueFromEncodableMap(arguments, "baseUrl", &base_url)) {
      LOG_WARN("loadHtmlString: baseUrl is not supported and will be ignored.");
    }
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
  } else if (method_name == "loadRequest") {
    result->NotImplemented();
  } else if (method_name == "setCookie") {
    result->NotImplemented();
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
    Ewk_Cookie_Manager* manager =
        ewk_context_cookie_manager_get(ewk_view_context_get(webview_instance_));
    if (manager) {
      ewk_cookie_manager_cookies_clear(manager);
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
  flutter::EncodableMap args;
  args.insert(std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
      flutter::EncodableValue("url"), flutter::EncodableValue(url)));
  webview->channel_->InvokeMethod(
      "onPageStarted", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnLoadFinished(void* data, Evas_Object* obj, void* event_info) {
  WebView* webview = static_cast<WebView*>(data);
  std::string url = std::string(ewk_view_url_get(webview->webview_instance_));
  flutter::EncodableMap args;
  args.insert(std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
      flutter::EncodableValue("url"), flutter::EncodableValue(url)));
  webview->channel_->InvokeMethod(
      "onPageFinished", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnLoadError(void* data, Evas_Object* obj, void* event_info) {
  WebView* webview = static_cast<WebView*>(data);
  Ewk_Error* error = static_cast<Ewk_Error*>(event_info);
  flutter::EncodableMap args = {
      {flutter::EncodableValue("errorCode"),
       flutter::EncodableValue(ewk_error_code_get(error))},
      {flutter::EncodableValue("description"),
       flutter::EncodableValue(ewk_error_description_get(error))},
      {flutter::EncodableValue("errorType"),
       flutter::EncodableValue(ErrorCodeToString(ewk_error_code_get(error)))},
      {flutter::EncodableValue("failingUrl"),
       flutter::EncodableValue(ewk_error_url_get(error))},
  };
  webview->channel_->InvokeMethod(
      "onWebResourceError", std::make_unique<flutter::EncodableValue>(args));
}

void WebView::OnConsoleMessage(void* data, Evas_Object* obj, void* event_info) {
  Ewk_Console_Message* message = static_cast<Ewk_Console_Message*>(event_info);
  LOG_INFO(
      "console message:%s: %d: %d: %s",
      EwkInternalApiBinding::GetInstance().console_message.SourceGet(message),
      EwkInternalApiBinding::GetInstance().console_message.LineGet(message),
      EwkInternalApiBinding::GetInstance().console_message.LevelGet(message),
      EwkInternalApiBinding::GetInstance().console_message.TextGet(message));
}

void WebView::OnNavigationPolicy(void* data, Evas_Object* obj,
                                 void* event_info) {
  WebView* webview = static_cast<WebView*>(data);
  Ewk_Policy_Decision* policy_decision =
      static_cast<Ewk_Policy_Decision*>(event_info);

  const char* url = ewk_policy_decision_url_get(policy_decision);
  if (!webview->has_navigation_delegate_) {
    ewk_policy_decision_use(policy_decision);
    return;
  }
  ewk_view_suspend(webview->webview_instance_);
  flutter::EncodableMap args = {
      {flutter::EncodableValue("url"), flutter::EncodableValue(url)},
      {flutter::EncodableValue("isForMainFrame"),
       flutter::EncodableValue(true)},
  };
  auto result = std::make_unique<NavigationRequestResult>(webview);
  webview->channel_->InvokeMethod(
      "navigationRequest", std::make_unique<flutter::EncodableValue>(args),
      std::move(result));
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
    if (webview->channel_) {
      std::string channel_name(message.name);
      std::string channel_message(static_cast<char*>(message.body));

      flutter::EncodableMap args = {
          {flutter::EncodableValue("channel"),
           flutter::EncodableValue(channel_name)},
          {flutter::EncodableValue("message"),
           flutter::EncodableValue(channel_message)},
      };
      webview->channel_->InvokeMethod(
          "javascriptChannelMessage",
          std::make_unique<flutter::EncodableValue>(args));
    }
  }
}
