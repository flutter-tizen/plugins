// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ewk_webview_backend.h"

#include <Ecore_Evas.h>

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <utility>

#include "buffer_pool.h"
#include "log.h"

namespace {

constexpr char kEwkInstance[] = "ewk_instance";

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

std::string ToString(const char* value) {
  return value ? std::string(value) : std::string();
}

// NOTE: Freeing this host terminates the EGL display shared by the app.
Ecore_Evas* g_offscreen_host = nullptr;

}  // namespace

EwkWebViewBackend::EwkWebViewBackend(Delegate* delegate)
    : delegate_(delegate) {}

EwkWebViewBackend::~EwkWebViewBackend() {
  if (Evas_Object* instance = DetachView()) {
    evas_object_del(instance);
  }
}

void EwkWebViewBackend::GlobalInitialize() { ewk_init(); }

void EwkWebViewBackend::GlobalShutdown() {
  FlushPendingTeardowns();
  ewk_shutdown();
}

Ecore_Evas* EwkWebViewBackend::GetOffscreenHost() {
  if (!g_offscreen_host) {
    g_offscreen_host = ecore_evas_new("wayland_egl", 0, 0, 1, 1, 0);
  }
  return g_offscreen_host;
}

bool EwkWebViewBackend::Create(double width, double height, void* window) {
  static std::once_flag ewk_args_once;
  std::call_once(ewk_args_once, []() {
    char* chromium_argv[] = {
        const_cast<char*>("--disable-pinch"),
        const_cast<char*>("--js-flags=--expose-gc"),
        const_cast<char*>("--single-process"),
        const_cast<char*>("--no-zygote"),
    };
    int chromium_argc = sizeof(chromium_argv) / sizeof(chromium_argv[0]);
    EwkInternalApiBinding::GetInstance().main.SetArguments(chromium_argc,
                                                           chromium_argv);
  });

  Ecore_Evas* evas = GetOffscreenHost();
  if (!evas) {
    LOG_ERROR("Failed to create Ecore_Evas for the WebView.");
    return false;
  }

  view_ = ewk_view_add(ecore_evas_get(evas));
  if (!view_) {
    return false;
  }
  ecore_evas_focus_set(evas, true);
  ewk_view_focus_set(view_, true);
  EwkInternalApiBinding::GetInstance().view.OffscreenRenderingEnabledSet(view_,
                                                                         true);

  Ewk_Context* context = ewk_view_context_get(view_);
  if (context) {
    Ewk_Cookie_Manager* cookie_manager =
        ewk_context_cookie_manager_get(context);
    if (cookie_manager) {
      ewk_cookie_manager_accept_policy_set(
          cookie_manager, EWK_COOKIE_ACCEPT_POLICY_NO_THIRD_PARTY);
    }
    ewk_context_cache_model_set(context, EWK_CACHE_MODEL_PRIMARY_WEBBROWSER);
  } else {
    LOG_WARN("Unable to access the EWK context; skipping cookie/cache setup.");
  }

  EwkInternalApiBinding::GetInstance().settings.ImePanelEnabledSet(
      ewk_view_settings_get(view_), true);
  EwkInternalApiBinding::GetInstance().settings.ForceZoomSet(
      ewk_view_settings_get(view_), true);
  EwkInternalApiBinding::GetInstance().view.ImeWindowSet(view_, window);
  EwkInternalApiBinding::GetInstance().view.KeyEventsEnabledSet(view_, true);
#ifdef WEBVIEW_TIZEN_TOUCH_EVENTS_ENABLED
  EwkInternalApiBinding::GetInstance().view.TouchEventsEnabledSet(view_, true);
  EwkInternalApiBinding::GetInstance().view.MouseEventsEnabledSet(view_, false);
#else
  EwkInternalApiBinding::GetInstance().view.TouchEventsEnabledSet(view_, false);
  EwkInternalApiBinding::GetInstance().view.MouseEventsEnabledSet(view_, true);
#endif

  EwkInternalApiBinding::GetInstance().view.OnJavaScriptAlert(
      view_, &EwkWebViewBackend::OnJavaScriptAlertDialog, this);
  EwkInternalApiBinding::GetInstance().view.OnJavaScriptConfirm(
      view_, &EwkWebViewBackend::OnJavaScriptConfirmDialog, this);
  EwkInternalApiBinding::GetInstance().view.OnJavaScriptPrompt(
      view_, &EwkWebViewBackend::OnJavaScriptPromptDialog, this);

#ifdef TV_PROFILE
  EwkInternalApiBinding::GetInstance().view.SupportVideoHoleSet(view_, window,
                                                                true, false);
#endif

  evas_object_smart_callback_add(view_, "offscreen,frame,rendered",
                                 &EwkWebViewBackend::OnFrameRendered, this);
  evas_object_smart_callback_add(view_, "load,started",
                                 &EwkWebViewBackend::OnLoadStarted, this);
  evas_object_smart_callback_add(view_, "load,finished",
                                 &EwkWebViewBackend::OnLoadFinished, this);
  evas_object_smart_callback_add(view_, "load,progress",
                                 &EwkWebViewBackend::OnProgress, this);
  evas_object_smart_callback_add(view_, "load,error",
                                 &EwkWebViewBackend::OnLoadError, this);
  evas_object_smart_callback_add(view_, "console,message",
                                 &EwkWebViewBackend::OnConsoleMessage, this);
  evas_object_smart_callback_add(view_, "policy,navigation,decide",
                                 &EwkWebViewBackend::OnNavigationPolicy, this);
  evas_object_smart_callback_add(view_, "url,changed",
                                 &EwkWebViewBackend::OnUrlChange, this);
  evas_object_smart_callback_add(view_, "title,changed",
                                 &EwkWebViewBackend::OnTitleChange, this);

  evas_object_data_set(view_, kEwkInstance, this);

  evas_object_resize(view_, static_cast<int>(std::round(width)),
                     static_cast<int>(std::round(height)));
  evas_object_show(view_);

  return true;
}

Evas_Object* EwkWebViewBackend::DetachView() {
  Evas_Object* instance = view_;
  view_ = nullptr;
  if (!instance) {
    return nullptr;
  }

  // NOTE: A suspended view must be resumed before it can be stopped.
  ewk_view_resume(instance);
  ewk_view_stop(instance);
  evas_object_hide(instance);

  evas_object_smart_callback_del(instance, "offscreen,frame,rendered",
                                 &EwkWebViewBackend::OnFrameRendered);
  evas_object_smart_callback_del(instance, "load,started",
                                 &EwkWebViewBackend::OnLoadStarted);
  evas_object_smart_callback_del(instance, "load,finished",
                                 &EwkWebViewBackend::OnLoadFinished);
  evas_object_smart_callback_del(instance, "load,progress",
                                 &EwkWebViewBackend::OnProgress);
  evas_object_smart_callback_del(instance, "load,error",
                                 &EwkWebViewBackend::OnLoadError);
  evas_object_smart_callback_del(instance, "console,message",
                                 &EwkWebViewBackend::OnConsoleMessage);
  evas_object_smart_callback_del(instance, "policy,navigation,decide",
                                 &EwkWebViewBackend::OnNavigationPolicy);
  evas_object_smart_callback_del(instance, "url,changed",
                                 &EwkWebViewBackend::OnUrlChange);
  evas_object_smart_callback_del(instance, "title,changed",
                                 &EwkWebViewBackend::OnTitleChange);

  auto& ewk_view = EwkInternalApiBinding::GetInstance().view;
  if (ewk_view.OnJavaScriptAlert) {
    ewk_view.OnJavaScriptAlert(instance, nullptr, nullptr);
  }
  if (ewk_view.OnJavaScriptConfirm) {
    ewk_view.OnJavaScriptConfirm(instance, nullptr, nullptr);
  }
  if (ewk_view.OnJavaScriptPrompt) {
    ewk_view.OnJavaScriptPrompt(instance, nullptr, nullptr);
  }

  evas_object_data_del(instance, kEwkInstance);

  return instance;
}

std::function<void()> EwkWebViewBackend::PrepareTeardown(
    std::shared_ptr<BufferPool> pool) {
  Evas_Object* instance = DetachView();

  std::function<void()> destroy;
  if (instance) {
    destroy = [instance]() { evas_object_del(instance); };
  }

  return RegisterPendingTeardown(std::move(pool), std::move(destroy));
}

void EwkWebViewBackend::Offset(double left, double top) {
  left_ = left;
  top_ = top;
  evas_object_move(view_, static_cast<int>(std::round(left_)),
                   static_cast<int>(std::round(top_)));
}

void EwkWebViewBackend::Resize(double width, double height) {
  evas_object_resize(view_, static_cast<int>(std::round(width)),
                     static_cast<int>(std::round(height)));
}

void EwkWebViewBackend::Touch(int event_type, int button_type, double x,
                              double y, double dx, double dy) {
#ifdef WEBVIEW_TIZEN_TOUCH_EVENTS_ENABLED
  SendTouchEvent(event_type, x, y);
#else
  SendMouseEvent(event_type, button_type, x, y, dx, dy);
#endif
}

void EwkWebViewBackend::SendTouchEvent(int event_type, double x, double y) {
  Ewk_Touch_Event_Type mouse_event_type = EWK_TOUCH_START;
  Evas_Touch_Point_State state = EVAS_TOUCH_POINT_DOWN;
  if (event_type == 0) {  // down event
    mouse_event_type = EWK_TOUCH_START;
    state = EVAS_TOUCH_POINT_DOWN;
  } else if (event_type == 1) {  // move event
    mouse_event_type = EWK_TOUCH_MOVE;
    state = EVAS_TOUCH_POINT_MOVE;
  } else if (event_type == 2) {  // up event
    mouse_event_type = EWK_TOUCH_END;
    state = EVAS_TOUCH_POINT_UP;
  } else {
    LOG_WARN("Unknown touch event type: %d", event_type);
  }

  Eina_List* points = 0;
  Ewk_Touch_Point* point = new Ewk_Touch_Point;
  point->id = 0;
  point->x = static_cast<int>(std::round(x + left_));
  point->y = static_cast<int>(std::round(y + top_));
  point->state = state;
  points = eina_list_append(points, point);

  EwkInternalApiBinding::GetInstance().view.FeedTouchEvent(
      view_, mouse_event_type, points, 0);
  eina_list_free(points);
  delete point;
}

void EwkWebViewBackend::SendMouseEvent(int event_type, int button_type,
                                       double x, double y, double dx,
                                       double dy) {
  Ewk_Mouse_Button_Type mouse_button_type = (Ewk_Mouse_Button_Type)0;
  switch (button_type) {
    case 1:
      mouse_button_type = EWK_MOUSE_BUTTON_LEFT;
      break;
    case 2:
      mouse_button_type = EWK_MOUSE_BUTTON_RIGHT;
      break;
    case 4:
      mouse_button_type = EWK_MOUSE_BUTTON_MIDDLE;
      break;
  }

  int px = static_cast<int>(std::round(x + left_));
  int py = static_cast<int>(std::round(y + top_));

  if (event_type == 0) {  // down event
    mouse_button_type_ = mouse_button_type;
    EwkInternalApiBinding::GetInstance().view.FeedMouseDown(
        view_, mouse_button_type_, px, py);
  } else if (event_type == 1) {
    if (dy != 0) {
      EwkInternalApiBinding::GetInstance().view.FeedMouseWheel(
          view_, true, dy > 0 ? 1 : -1, px, py);
    }
  } else if (event_type == 2) {  // up event
    EwkInternalApiBinding::GetInstance().view.FeedMouseUp(
        view_, mouse_button_type_, px, py);
    mouse_button_type_ = mouse_button_type;
  } else {
    LOG_WARN("Unknown mouse event type: %d", event_type);
  }
}

bool EwkWebViewBackend::SendKey(const char* key, const char* string,
                                const char* compose, uint32_t modifiers,
                                uint32_t scan_code, bool is_down) {
  if (key && strcmp(key, "XF86Exit") == 0 && !is_down) {
    return false;
  }

  if (key && strcmp(key, "XF86Back") == 0 && !is_down) {
    if (ewk_view_back_possible(view_)) {
      ewk_view_back(view_);
      return true;
    }
    return false;
  }

  if (is_down) {
    // TODO(swift-kim): Deal with other members of the structure.
    Evas_Event_Key_Down down_event = {};
    down_event.key = key;
    down_event.string = string;
    EwkInternalApiBinding::GetInstance().view.SendKeyEvent(view_, &down_event,
                                                           is_down);
  } else {
    Evas_Event_Key_Up up_event = {};
    up_event.key = key;
    up_event.string = string;
    EwkInternalApiBinding::GetInstance().view.SendKeyEvent(view_, &up_event,
                                                           is_down);
  }
  return true;
}

void EwkWebViewBackend::Resume() {
  if (view_) {
    ewk_view_resume(view_);
  }
}

void EwkWebViewBackend::Stop() {
  if (view_) {
    ewk_view_stop(view_);
  }
}

void EwkWebViewBackend::Suspend() {
  if (view_) {
    ewk_view_suspend(view_);
  }
}

void EwkWebViewBackend::SetJavaScriptEnabled(bool enabled) {
  ewk_settings_javascript_enabled_set(ewk_view_settings_get(view_), enabled);
}

bool EwkWebViewBackend::LoadUrl(const std::string& url) {
  return ewk_view_url_set(view_, url.c_str());
}

bool EwkWebViewBackend::LoadUrlRequest(
    const std::string& url, int32_t method,
    const std::map<std::string, std::string>& headers,
    const std::vector<uint8_t>& body) {
  Ewk_Http_Method ewk_method = EWK_HTTP_METHOD_GET;
  if (method == 1) {
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
  for (const auto& header : headers) {
    eina_hash_add(ewk_headers, header.first.c_str(),
                  strdup(header.second.c_str()));
  }

  // NOTE: The EWK API requires a NUL-terminated request body.
  std::string body_str(body.begin(), body.end());
  bool ret =
      ewk_view_url_request_set(view_, url.c_str(), ewk_method, ewk_headers,
                               body.empty() ? nullptr : body_str.c_str());
  eina_hash_free(ewk_headers);
  return ret;
}

bool EwkWebViewBackend::LoadHtmlString(const std::string& html,
                                       const std::string& base_url) {
  return ewk_view_html_string_load(view_, html.c_str(), base_url.c_str(),
                                   nullptr);
}

bool EwkWebViewBackend::CanGoBack() { return ewk_view_back_possible(view_); }

bool EwkWebViewBackend::CanGoForward() {
  return ewk_view_forward_possible(view_);
}

bool EwkWebViewBackend::GoBack() { return ewk_view_back(view_); }

bool EwkWebViewBackend::GoForward() { return ewk_view_forward(view_); }

bool EwkWebViewBackend::Reload() { return ewk_view_reload(view_); }

std::string EwkWebViewBackend::GetCurrentUrl() {
  return ToString(ewk_view_url_get(view_));
}

void EwkWebViewBackend::EvaluateJavaScript(
    const std::string& javascript,
    std::function<void(bool success, const char* result_value)> callback) {
  auto* callback_ptr =
      new std::function<void(bool, const char*)>(std::move(callback));
  if (!ewk_view_script_execute(view_, javascript.c_str(),
                               &EwkWebViewBackend::OnEvaluateJavaScript,
                               callback_ptr)) {
    LOG_WARN("ewk_view_script_execute failed.");
    (*callback_ptr)(false, nullptr);
    delete callback_ptr;
  }
}

void EwkWebViewBackend::ClearCache() {
  Ewk_Context* context = ewk_view_context_get(view_);
  if (context) {
    ewk_context_resource_cache_clear(context);
  }
}

std::string EwkWebViewBackend::GetTitle() {
  return ToString(ewk_view_title_get(view_));
}

void EwkWebViewBackend::ScrollTo(int32_t x, int32_t y) {
  ewk_view_scroll_set(view_, x, y);
}

void EwkWebViewBackend::GetScrollPosition(int32_t* x, int32_t* y) {
  // TODO(jsuya): ewk_view_scroll_pos_get() returns the position set in
  // ewk_view_scroll_set(). Therefore, it currently does not work as intended.
  ewk_view_scroll_pos_get(view_, x, y);
}

void EwkWebViewBackend::SetBackgroundColor(int r, int g, int b, int a) {
  EwkInternalApiBinding::GetInstance().view.SetBackgroundColor(view_, r, g, b,
                                                               a);
}

void EwkWebViewBackend::SetUserAgent(const std::string& user_agent) {
  ewk_view_user_agent_set(view_, user_agent.c_str());
}

std::string EwkWebViewBackend::GetUserAgent() {
  return ToString(ewk_view_user_agent_get(view_));
}

void EwkWebViewBackend::EnableZoom(bool enabled) {
  EwkInternalApiBinding::GetInstance().settings.ForceZoomSet(
      ewk_view_settings_get(view_), enabled);
}

void EwkWebViewBackend::JavaScriptAlertReply() {
  EwkInternalApiBinding::GetInstance().view.JavaScriptAlertReply(view_);
}

void EwkWebViewBackend::JavaScriptConfirmReply(bool result) {
  EwkInternalApiBinding::GetInstance().view.JavaScriptConfirmReply(view_,
                                                                   result);
}

void EwkWebViewBackend::JavaScriptPromptReply(const char* result) {
  EwkInternalApiBinding::GetInstance().view.JavaScriptPromptReply(view_,
                                                                  result);
}

bool EwkWebViewBackend::ClearCookies() {
  if (!view_) {
    return false;
  }
  Ewk_Context* context = ewk_view_context_get(view_);
  Ewk_Cookie_Manager* cookie_manager =
      context ? ewk_context_cookie_manager_get(context) : nullptr;
  if (cookie_manager) {
    ewk_cookie_manager_cookies_clear(cookie_manager);
    return true;
  }
  return false;
}

int32_t EwkWebViewBackend::GetProgress() {
  if (!view_) {
    return 0;
  }
  double progress = ewk_view_load_progress_get(view_);
  if (progress < 0) {
    return 0;
  }
  return static_cast<int32_t>(progress * 100);
}

double EwkWebViewBackend::GetScale() {
  if (!view_) {
    return 1.0;
  }
  double scale = ewk_view_scale_get(view_);
  return scale > 0 ? scale : 1.0;
}

void EwkWebViewBackend::SetScale(double scale, int32_t x, int32_t y) {
  ewk_view_scale_set(view_, scale, x, y);
}

void EwkWebViewBackend::OnFrameRendered(void* data, Evas_Object* obj,
                                        void* event_info) {
  if (event_info) {
    static_cast<EwkWebViewBackend*>(data)->delegate_->OnFrameRendered(
        event_info);
  }
}

void EwkWebViewBackend::OnLoadStarted(void* data, Evas_Object* obj,
                                      void* event_info) {
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  backend->delegate_->OnLoadStarted(ToString(ewk_view_url_get(backend->view_)));
}

void EwkWebViewBackend::OnLoadFinished(void* data, Evas_Object* obj,
                                       void* event_info) {
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  backend->delegate_->OnLoadFinished(
      ToString(ewk_view_url_get(backend->view_)));
}

void EwkWebViewBackend::OnProgress(void* data, Evas_Object* obj,
                                   void* event_info) {
  if (!event_info) {
    return;
  }
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  int32_t progress =
      static_cast<int32_t>((*static_cast<double*>(event_info)) * 100);
  backend->delegate_->OnProgress(progress);
}

void EwkWebViewBackend::OnLoadError(void* data, Evas_Object* obj,
                                    void* event_info) {
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  Ewk_Error* error = static_cast<Ewk_Error*>(event_info);
  backend->delegate_->OnLoadError(ewk_error_code_get(error),
                                  ToString(ewk_error_description_get(error)),
                                  ToString(ewk_error_url_get(error)));
}

void EwkWebViewBackend::OnConsoleMessage(void* data, Evas_Object* obj,
                                         void* event_info) {
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  Ewk_Console_Message* message = static_cast<Ewk_Console_Message*>(event_info);
  Ewk_Console_Message_Level log_level =
      EwkInternalApiBinding::GetInstance().console_message.LevelGet(message);
  std::string text = ToString(
      EwkInternalApiBinding::GetInstance().console_message.TextGet(message));
  backend->delegate_->OnConsoleMessage(ConvertLogLevelToString(log_level),
                                       text);
}

void EwkWebViewBackend::OnNavigationPolicy(void* data, Evas_Object* obj,
                                           void* event_info) {
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  Ewk_Policy_Decision* policy_decision =
      static_cast<Ewk_Policy_Decision*>(event_info);

  const std::string current_url = ToString(ewk_view_url_get(backend->view_));
  ewk_policy_decision_use(policy_decision);

  backend->delegate_->OnNavigationPolicyDecide(
      ToString(ewk_policy_decision_url_get(policy_decision)), current_url);
}

void EwkWebViewBackend::OnUrlChange(void* data, Evas_Object* obj,
                                    void* event_info) {
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  backend->delegate_->OnUrlChanged(ToString(ewk_view_url_get(backend->view_)));
}

void EwkWebViewBackend::OnTitleChange(void* data, Evas_Object* obj,
                                      void* event_info) {
  const char* title = static_cast<const char*>(event_info);
  if (!title) {
    return;
  }
  static_cast<EwkWebViewBackend*>(data)->delegate_->OnTitleChanged(title);
}

void EwkWebViewBackend::OnEvaluateJavaScript(Evas_Object* obj,
                                             const char* result_value,
                                             void* user_data) {
  auto* callback =
      static_cast<std::function<void(bool, const char*)>*>(user_data);
  (*callback)(true, result_value);
  delete callback;
}

Eina_Bool EwkWebViewBackend::OnJavaScriptAlertDialog(Evas_Object* o,
                                                     const char* message,
                                                     void* data) {
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  backend->delegate_->OnJavaScriptAlertDialog(
      ToString(message), ToString(ewk_view_url_get(backend->view_)));
  return true;
}

Eina_Bool EwkWebViewBackend::OnJavaScriptConfirmDialog(Evas_Object* o,
                                                       const char* message,
                                                       void* data) {
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  backend->delegate_->OnJavaScriptConfirmDialog(
      ToString(message), ToString(ewk_view_url_get(backend->view_)));
  return true;
}

Eina_Bool EwkWebViewBackend::OnJavaScriptPromptDialog(Evas_Object* o,
                                                      const char* message,
                                                      const char* default_text,
                                                      void* data) {
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  backend->delegate_->OnJavaScriptPromptDialog(
      ToString(message), ToString(default_text),
      ToString(ewk_view_url_get(backend->view_)));
  return true;
}
