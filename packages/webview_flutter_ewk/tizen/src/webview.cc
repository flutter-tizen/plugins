// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webview.h"

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>

#include "log.h"
#include "webview_factory.h"

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
      ewk_view_url_set(webview_->GetWebViewInstance(), url_.c_str());
    }
  }

  std::string url_;
  WebView* webview_;
};

static std::string ErrorCodeToString(int error_code) {
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
                 double height, flutter::EncodableMap& params, void* winHandle)
    : PlatformView(registrar, viewId, winHandle),
      texture_registrar_(texture_registrar),
      webview_instance_(nullptr),
      width_(width),
      height_(height),
      candidate_surface_(nullptr),
      rendered_surface_(nullptr),
      has_navigation_delegate_(false),
      has_progress_tracking_(false),
      texture_variant_(nullptr),
      gpu_buffer_(nullptr)

{
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

  if (!webview_instance_) return;

  auto user_agent = params[flutter::EncodableValue("userAgent")];
  if (std::holds_alternative<std::string>(user_agent)) {
    ewk_view_user_agent_set(webview_instance_,
                            std::get<std::string>(user_agent).c_str());
  }
  ewk_view_url_set(webview_instance_, url.c_str());
}

void WebView::ApplySettings(flutter::EncodableMap settings) {
  for (auto const& [key, val] : settings) {
    if (std::holds_alternative<std::string>(key)) {
      std::string k = std::get<std::string>(key);
      if ("jsMode" == k) {
      } else if ("hasNavigationDelegate" == k) {
        if (std::holds_alternative<bool>(val)) {
          has_navigation_delegate_ = std::get<bool>(val);
        }
      } else if ("debuggingEnabled" == k) {
      } else if ("gestureNavigationEnabled" == k) {
      } else if ("userAgent" == k) {
        if (std::holds_alternative<std::string>(val)) {
          ewk_view_user_agent_set(webview_instance_,
                                  std::get<std::string>(val).c_str());
        }
      }
    }
  }
}

void WebView::RegisterJavaScriptChannelName(const std::string& name) {
  LOG_DEBUG("RegisterJavaScriptChannelName(channelName: %s)\n", name.c_str());
}

WebView::~WebView() { Dispose(); }

std::string WebView::GetChannelName() {
  return "plugins.flutter.io/webview_" + std::to_string(GetViewId());
}

void WebView::Dispose() {
  texture_registrar_->UnregisterTexture(GetTextureId());

  if (texture_variant_) {
    delete texture_variant_;
    texture_variant_ = nullptr;
  }

  if (gpu_buffer_) {
    delete gpu_buffer_;
    gpu_buffer_ = nullptr;
  }

  if (webview_instance_) {
    evas_object_del(webview_instance_);
    webview_instance_ = nullptr;
  }
}

void WebView::Resize(double width, double height) {
  LOG_DEBUG("WebView::Resize width: %f height: %f \n", width, height);
  width_ = width;
  height_ = height;
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

  ewk_view_feed_touch_event(webview_instance_, mouse_event_type, pointList, 0);
  eina_list_free(pointList);
}

void WebView::ClearFocus() {
  LOG_DEBUG("WebView::ClearFocus()");
  ewk_view_focus_set(webview_instance_, false);
}

void WebView::SetDirection(int direction) {
  LOG_DEBUG("WebView::SetDirection direction: %d\n", direction);
  // TODO: implement this if necessary
}

void WebView::InitWebView() {
  if (!gpu_buffer_) {
    gpu_buffer_ = new FlutterDesktopGpuBuffer();
  }

  ewk_init();
  Ecore_Evas* evas = ecore_evas_new("wayland_egl", 0, 0, 1, 1, 0);
  Ewk_Context* context = ewk_context_default_get();

  webview_instance_ = ewk_view_add(ecore_evas_get(evas));
  ecore_evas_focus_set(evas, true);
  ewk_view_focus_set(webview_instance_, true);
  ewk_view_offscreen_rendering_enabled_set(webview_instance_, true);

  Ewk_Settings* settings = ewk_view_settings_get(webview_instance_);

  context = ewk_view_context_get(webview_instance_);
  Ewk_Cookie_Manager* manager = ewk_context_cookie_manager_get(context);
  ewk_cookie_manager_accept_policy_set(manager,
                                       EWK_COOKIE_ACCEPT_POLICY_NO_THIRD_PARTY);
  ewk_view_ime_window_set(webview_instance_,
                          (Ecore_Wl2_Window*)platform_window_);
  ewk_settings_viewport_meta_tag_set(settings, false);
  ewk_view_key_events_enabled_set(webview_instance_, true);
  ewk_context_cache_model_set(context, EWK_CACHE_MODEL_PRIMARY_WEBBROWSER);

  evas_object_smart_callback_add(webview_instance_, "offscreen,frame,rendered",
                                 &WebView::OnFrameRendered, this);
  evas_object_smart_callback_add(webview_instance_, "load,started",
                                 &WebView::OnLoadStarted, this);
  evas_object_smart_callback_add(webview_instance_, "load,progress",
                                 &WebView::OnLoadInProgress, this);
  evas_object_smart_callback_add(webview_instance_, "load,finished",
                                 &WebView::OnLoadFinished, this);
  evas_object_smart_callback_add(webview_instance_, "load,error",
                                 &WebView::OnLoadError, this);
  evas_object_smart_callback_add(webview_instance_, "url,changed",
                                 &WebView::OnUrlChanged, this);
  evas_object_smart_callback_add(webview_instance_, "console,message",
                                 &WebView::OnConsoleMessage, this);
  Resize(width_, height_);
  evas_object_show(webview_instance_);
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
    ewk_view_url_set(webview_instance_, url.c_str());

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
    result->Success(
        flutter::EncodableValue(ewk_view_back_possible(webview_instance_)));
  } else if (method_name.compare("canGoForward") == 0) {
    result->Success(
        flutter::EncodableValue(ewk_view_forward_possible(webview_instance_)));
  } else if (method_name.compare("goBack") == 0) {
    ewk_view_back(webview_instance_);
    result->Success();
  } else if (method_name.compare("goForward") == 0) {
    ewk_view_forward(webview_instance_);
    result->Success();
  } else if (method_name.compare("reload") == 0) {
    ewk_view_reload(webview_instance_);
    result->Success();
  } else if (method_name.compare("currentUrl") == 0) {
    result->Success(
        flutter::EncodableValue(ewk_view_url_get(webview_instance_)));
  } else if (method_name.compare("evaluateJavascript") == 0) {
    if (std::holds_alternative<std::string>(arguments)) {
      std::string js_string = std::get<std::string>(arguments);
      ewk_view_script_execute(webview_instance_, js_string.c_str(),
                              &WebView::OnEvaluateJavaScript, nullptr);
    } else {
      result->Error("Invalid Arguments", "Invalid Arguments");
    }
  } else if (method_name.compare("addJavascriptChannels") == 0) {
    result->NotImplemented();
  } else if (method_name.compare("removeJavascriptChannels") == 0) {
    result->NotImplemented();
  } else if (method_name.compare("clearCache") == 0) {
    result->NotImplemented();
  } else if (method_name.compare("getTitle") == 0) {
    result->Success(flutter::EncodableValue(
        std::string(ewk_view_title_get(webview_instance_))));
  } else if (method_name.compare("scrollTo") == 0) {
    int x = ExtractIntFromMap(arguments, "x");
    int y = ExtractIntFromMap(arguments, "y");
    ewk_view_scroll_set(webview_instance_, x, y);
    result->Success();
  } else if (method_name.compare("scrollBy") == 0) {
    int x = ExtractIntFromMap(arguments, "x");
    int y = ExtractIntFromMap(arguments, "y");
    ewk_view_scroll_by(webview_instance_, x, y);
    result->Success();
  } else if (method_name.compare("getScrollX") == 0) {
    result->NotImplemented();
  } else if (method_name.compare("getScrollY") == 0) {
    result->NotImplemented();
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
    result->NotImplemented();
  } else {
    result->NotImplemented();
  }
}

void WebView::OnFrameRendered(void* data, Evas_Object*, void* buffer) {
  if (buffer) {
    WebView* webview = (WebView*)data;
    webview->candidate_surface_ = static_cast<tbm_surface_h>(buffer);
    webview->texture_registrar_->MarkTextureFrameAvailable(
        webview->GetTextureId());
  }
}

void WebView::OnLoadStarted(void* data, Evas_Object*, void*) {
  WebView* webview = (WebView*)data;
  std::string url = std::string(ewk_view_url_get(webview->webview_instance_));
  LOG_DEBUG("RegisterOnPageStartedHandler(url: %s)\n", url.c_str());
  flutter::EncodableMap map;
  map.insert(std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
      flutter::EncodableValue("url"), flutter::EncodableValue(url)));
  auto args = std::make_unique<flutter::EncodableValue>(map);
  webview->channel_->InvokeMethod("onPageStarted", std::move(args));
}

void WebView::OnLoadInProgress(void* data, Evas_Object*, void*) {}

void WebView::OnLoadFinished(void* data, Evas_Object*, void*) {
  WebView* webview = (WebView*)data;
  std::string url = std::string(ewk_view_url_get(webview->webview_instance_));
  flutter::EncodableMap map;
  map.insert(std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
      flutter::EncodableValue("url"), flutter::EncodableValue(url)));
  auto args = std::make_unique<flutter::EncodableValue>(map);
  webview->channel_->InvokeMethod("onPageFinished", std::move(args));
}

void WebView::OnLoadError(void* data, Evas_Object*, void* rawError) {
  WebView* webview = (WebView*)data;
  Ewk_Error* error = static_cast<Ewk_Error*>(rawError);
  flutter::EncodableMap map;

  map.insert(std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
      flutter::EncodableValue("errorCode"),
      flutter::EncodableValue(ewk_error_code_get(error))));
  map.insert(std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
      flutter::EncodableValue("description"),
      flutter::EncodableValue(ewk_error_description_get(error))));
  map.insert(std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
      flutter::EncodableValue("errorType"),
      flutter::EncodableValue(ErrorCodeToString(ewk_error_code_get(error)))));
  map.insert(std::make_pair<flutter::EncodableValue, flutter::EncodableValue>(
      flutter::EncodableValue("failingUrl"),
      flutter::EncodableValue(ewk_error_url_get(error))));
  auto args = std::make_unique<flutter::EncodableValue>(map);
  webview->channel_->InvokeMethod("onWebResourceError", std::move(args));
}

void WebView::OnUrlChanged(void* data, Evas_Object*, void* newUrl) {}

void WebView::OnConsoleMessage(void*, Evas_Object*, void* eventInfo) {
  Ewk_Console_Message* message = (Ewk_Console_Message*)eventInfo;
  LOG_DEBUG("console message:%s: %d: %d: %s",
            ewk_console_message_source_get(message),
            ewk_console_message_line_get(message),
            ewk_console_message_level_get(message),
            ewk_console_message_text_get(message));
}

void WebView::OnEvaluateJavaScript(Evas_Object* o, const char* result,
                                   void* data) {}

void WebView::OnJavaScriptMessage(Evas_Object* o, Ewk_Script_Message message) {}

Eina_Bool WebView::OnJavaScriptAlert(Evas_Object* o, const char* alert_text,
                                     void*) {
  return false;
}

Eina_Bool WebView::OnJavaScriptConfirm(Evas_Object* o, const char* message,
                                       void*) {
  return false;
}

Eina_Bool WebView::OnJavaScriptPrompt(Evas_Object* o, const char* message,
                                      const char* default_value, void*) {
  return false;
}

FlutterDesktopGpuBuffer* WebView::ObtainGpuBuffer(size_t width, size_t height) {
  if (!candidate_surface_) {
    if (!rendered_surface_) {
      return nullptr;
    } else {
      return gpu_buffer_;
    }
  }

  rendered_surface_ = candidate_surface_;
  candidate_surface_ = nullptr;
  if (gpu_buffer_) {
    gpu_buffer_->buffer = static_cast<void*>(rendered_surface_);
    gpu_buffer_->width = width;
    gpu_buffer_->height = height;
  }
  return gpu_buffer_;
}

void WebView::DestructBuffer(void* buffer) {
  // TODO
}
