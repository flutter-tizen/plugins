// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ewk_webview_backend.h"

#include <Ecore_Evas.h>
#include <Ecore_Input.h>

#include <cmath>
#include <cstdlib>
#include <cstring>

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

void SyncEvasModifiers(Evas* evas, uint32_t modifiers) {
  auto set_modifier = [evas](const char* name, bool on) {
    if (on) {
      evas_key_modifier_on(evas, name);
    } else {
      evas_key_modifier_off(evas, name);
    }
  };
  set_modifier("Shift", modifiers & ECORE_EVENT_MODIFIER_SHIFT);
  set_modifier("Control", modifiers & ECORE_EVENT_MODIFIER_CTRL);
  set_modifier("Alt", modifiers & ECORE_EVENT_MODIFIER_ALT);
  set_modifier("Super", modifiers & ECORE_EVENT_MODIFIER_WIN);
  set_modifier("Hyper", modifiers & ECORE_EVENT_MODIFIER_ALTGR);

  auto set_lock = [evas](const char* name, bool on) {
    if (on) {
      evas_key_lock_on(evas, name);
    } else {
      evas_key_lock_off(evas, name);
    }
  };
  set_lock("Caps_Lock", modifiers & ECORE_EVENT_LOCK_CAPS);
  set_lock("Num_Lock", modifiers & ECORE_EVENT_LOCK_NUM);
}

Ecore_Evas* g_offscreen_host = nullptr;

}  // namespace

EwkWebViewBackend::EwkWebViewBackend(Delegate* delegate)
    : delegate_(delegate) {}

void EwkWebViewBackend::GlobalInitialize() { ewk_init(); }

void EwkWebViewBackend::GlobalShutdown() {
  FlushPendingTeardowns();
  FreeOffscreenHost();
  ewk_shutdown();
}

Ecore_Evas* EwkWebViewBackend::GetOffscreenHost() {
  if (!g_offscreen_host) {
    g_offscreen_host = ecore_evas_new("wayland_egl", 0, 0, 1, 1, 0);
  }
  return g_offscreen_host;
}

void EwkWebViewBackend::FreeOffscreenHost() {
  if (g_offscreen_host) {
    ecore_evas_free(g_offscreen_host);
    g_offscreen_host = nullptr;
  }
}

bool EwkWebViewBackend::Create(double width, double height, void* window,
                               bool engine_policy) {
  window_ = window;

  if (engine_policy) {
    LOG_INFO("Upgrade web engine used.");
    EwkInternalApiBinding::GetInstance().main.SetVersionPolicy(1);
  }

  char* chromium_argv[] = {
      const_cast<char*>("--disable-pinch"),
      const_cast<char*>("--js-flags=--expose-gc"),
      const_cast<char*>("--single-process"),
      const_cast<char*>("--no-zygote"),
  };
  int chromium_argc = sizeof(chromium_argv) / sizeof(chromium_argv[0]);
  EwkInternalApiBinding::GetInstance().main.SetArguments(chromium_argc,
                                                         chromium_argv);

  Ecore_Evas* evas = GetOffscreenHost();
  if (!evas) {
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
  Ewk_Cookie_Manager* cookie_manager = ewk_context_cookie_manager_get(context);
  if (cookie_manager) {
    ewk_cookie_manager_accept_policy_set(
        cookie_manager, EWK_COOKIE_ACCEPT_POLICY_NO_THIRD_PARTY);
  }
  ewk_context_cache_model_set(context, EWK_CACHE_MODEL_PRIMARY_WEBBROWSER);

  EwkInternalApiBinding::GetInstance().settings.ImePanelEnabledSet(
      ewk_view_settings_get(view_), true);
  EwkInternalApiBinding::GetInstance().settings.ForceZoomSet(
      ewk_view_settings_get(view_), true);
  EwkInternalApiBinding::GetInstance().view.ImeWindowSet(view_, window_);
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
  EwkInternalApiBinding::GetInstance().view.SetSupportVideoHole(view_, window_,
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
  evas_object_smart_callback_add(view_, "policy,response,decide",
                                 &EwkWebViewBackend::OnResponsePolicy, this);
  evas_object_smart_callback_add(view_, "url,changed",
                                 &EwkWebViewBackend::OnUrlChange, this);

  evas_object_resize(view_, static_cast<int>(std::round(width)),
                     static_cast<int>(std::round(height)));
  evas_object_show(view_);

  evas_object_data_set(view_, kEwkInstance, this);

  return true;
}

std::function<void()> EwkWebViewBackend::PrepareTeardown(
    std::shared_ptr<BufferPool> pool) {
  Evas_Object* instance = view_;
  view_ = nullptr;

  std::function<void()> destroy;
  if (instance) {
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
    evas_object_smart_callback_del(instance, "policy,response,decide",
                                   &EwkWebViewBackend::OnResponsePolicy);
    evas_object_smart_callback_del(instance, "url,changed",
                                   &EwkWebViewBackend::OnUrlChange);

    EwkInternalApiBinding::GetInstance().view.OnJavaScriptAlert(
        instance, nullptr, nullptr);
    EwkInternalApiBinding::GetInstance().view.OnJavaScriptConfirm(
        instance, nullptr, nullptr);
    EwkInternalApiBinding::GetInstance().view.OnJavaScriptPrompt(
        instance, nullptr, nullptr);

    evas_object_data_del(instance, kEwkInstance);
    ewk_view_stop(instance);
    ewk_view_suspend(instance);

    destroy = [instance]() { evas_object_del(instance); };
  }

  return RegisterPendingTeardown(std::move(pool), std::move(destroy));
}

void EwkWebViewBackend::Offset(double left, double top) {
  left_ = left;
  top_ = top;
  evas_object_move(view_, static_cast<int>(left_), static_cast<int>(top_));
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
  if (event_type == 0) {
    mouse_event_type = EWK_TOUCH_START;
    state = EVAS_TOUCH_POINT_DOWN;
  } else if (event_type == 1) {
    mouse_event_type = EWK_TOUCH_MOVE;
    state = EVAS_TOUCH_POINT_MOVE;
  } else if (event_type == 2) {
    mouse_event_type = EWK_TOUCH_END;
    state = EVAS_TOUCH_POINT_UP;
  } else {
    LOG_WARN("Unknown touch event type: %d", event_type);
  }

  Eina_List* points = 0;
  Ewk_Touch_Point* point = new Ewk_Touch_Point;
  point->id = 0;
  point->x = x + left_;
  point->y = y + top_;
  point->state = state;
  points = eina_list_append(points, point);

  EwkInternalApiBinding::GetInstance().view.FeedTouchEvent(
      view_, mouse_event_type, points, 0);
  eina_list_free(points);
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

  int px = x + left_;
  int py = y + top_;

  if (event_type == 0) {
    mouse_button_type_ = mouse_button_type;
    EwkInternalApiBinding::GetInstance().view.FeedMouseDown(
        view_, mouse_button_type_, px, py);
  } else if (event_type == 1) {
    if (dy != 0) {
      EwkInternalApiBinding::GetInstance().view.FeedMouseWheel(
          view_, true, dy > 0 ? 1 : -1, px, py);
    }
  } else if (event_type == 2) {
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
  if (strcmp(key, "XF86Exit") == 0 && !is_down) {
    return false;
  }

  if (strcmp(key, "XF86Back") == 0 && !is_down) {
    if (ewk_view_back_possible(view_)) {
      ewk_view_back(view_);
      return true;
    }
    return false;
  }

  Evas* evas = evas_object_evas_get(view_);
  SyncEvasModifiers(evas, modifiers);
  Evas_Modifier* evas_modifiers =
      const_cast<Evas_Modifier*>(evas_key_modifier_get(evas));
  Evas_Lock* evas_locks = const_cast<Evas_Lock*>(evas_key_lock_get(evas));

  if (is_down) {
    Evas_Event_Key_Down down_event = {};
    down_event.key = key;
    down_event.string = string;
    down_event.modifiers = evas_modifiers;
    down_event.locks = evas_locks;
    EwkInternalApiBinding::GetInstance().view.SendKeyEvent(view_, &down_event,
                                                           is_down);
  } else {
    Evas_Event_Key_Up up_event = {};
    up_event.key = key;
    up_event.modifiers = evas_modifiers;
    up_event.locks = evas_locks;
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

void EwkWebViewBackend::SetJavaScriptEnabled(bool enabled) {
  ewk_settings_javascript_enabled_set(ewk_view_settings_get(view_), enabled);
}

void EwkWebViewBackend::SetHasNavigationDelegate(bool has_navigation_delegate) {
  has_navigation_delegate_ = has_navigation_delegate;
}

void EwkWebViewBackend::LoadUrl(const std::string& url) {
  ewk_view_url_set(view_, url.c_str());
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

  bool ret =
      ewk_view_url_request_set(view_, url.c_str(), ewk_method, ewk_headers,
                               reinterpret_cast<const char*>(body.data()));
  eina_hash_free(ewk_headers);
  return ret;
}

void EwkWebViewBackend::LoadHtmlString(const std::string& html,
                                       const std::string& base_url) {
  ewk_view_html_string_load(view_, html.c_str(), base_url.c_str(), nullptr);
}

bool EwkWebViewBackend::CanGoBack() { return ewk_view_back_possible(view_); }

bool EwkWebViewBackend::CanGoForward() {
  return ewk_view_forward_possible(view_);
}

void EwkWebViewBackend::GoBack() { ewk_view_back(view_); }

void EwkWebViewBackend::GoForward() { ewk_view_forward(view_); }

void EwkWebViewBackend::Reload() { ewk_view_reload(view_); }

std::string EwkWebViewBackend::GetCurrentUrl() {
  return std::string(ewk_view_url_get(view_));
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

void EwkWebViewBackend::RegisterJavaScriptChannel(const std::string& name) {
  ewk_view_javascript_message_handler_add(
      view_, &EwkWebViewBackend::OnJavaScriptMessage, name.c_str());
}

void EwkWebViewBackend::ClearCache() {
  Ewk_Context* context = ewk_view_context_get(view_);
  ewk_context_resource_cache_clear(context);
}

void EwkWebViewBackend::ClearLocalStorage() {
  Ewk_Context* context = ewk_view_context_get(view_);
  ewk_context_web_storage_delete_all(context);
}

std::string EwkWebViewBackend::GetTitle() {
  return std::string(ewk_view_title_get(view_));
}

void EwkWebViewBackend::ScrollTo(int32_t x, int32_t y) {
  ewk_view_scroll_set(view_, x, y);
}

void EwkWebViewBackend::ScrollBy(int32_t x, int32_t y) {
  ewk_view_scroll_by(view_, x, y);
}

void EwkWebViewBackend::GetScrollPosition(int32_t* x, int32_t* y) {
  // TODO(jsuya): ewk_view_scroll_pos_get() returns the position set in
  // ewk_view_scroll_set(). Therefore, it currently does not work as
  // intended.
  ewk_view_scroll_pos_get(view_, x, y);
}

void EwkWebViewBackend::SetBackgroundColor(int r, int g, int b, int a) {
  EwkInternalApiBinding::GetInstance().view.BgColorSet(view_, r, g, b, a);
}

void EwkWebViewBackend::SetUserAgent(const std::string& user_agent) {
  ewk_view_user_agent_set(view_, user_agent.c_str());
}

std::string EwkWebViewBackend::GetUserAgent() {
  return std::string(ewk_view_user_agent_get(view_));
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

void EwkWebViewBackend::JavaScriptPromptReply(const std::string& result) {
  EwkInternalApiBinding::GetInstance().view.JavaScriptPromptReply(
      view_, result.c_str());
}

void EwkWebViewBackend::SetScrollbarVisible(bool visible) {
  scrollbar_enabled_ = visible;
  EwkInternalApiBinding::GetInstance().view.MainFrameScrollbarVisibleSet(
      view_, scrollbar_enabled_);
}

bool EwkWebViewBackend::ClearCookies() {
  Ewk_Context* context = ewk_view_context_get(view_);
  Ewk_Cookie_Manager* cookie_manager = ewk_context_cookie_manager_get(context);
  if (cookie_manager) {
    ewk_cookie_manager_cookies_clear(cookie_manager);
    return true;
  }
  return false;
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
  EwkInternalApiBinding::GetInstance().view.MainFrameScrollbarVisibleSet(
      backend->view_, backend->scrollbar_enabled_);
  backend->delegate_->OnLoadStarted(
      std::string(ewk_view_url_get(backend->view_)));
}

void EwkWebViewBackend::OnLoadFinished(void* data, Evas_Object* obj,
                                       void* event_info) {
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  backend->delegate_->OnLoadFinished(
      std::string(ewk_view_url_get(backend->view_)));
}

void EwkWebViewBackend::OnProgress(void* data, Evas_Object* obj,
                                   void* event_info) {
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
                                  ewk_error_description_get(error),
                                  ewk_error_url_get(error));
}

void EwkWebViewBackend::OnConsoleMessage(void* data, Evas_Object* obj,
                                         void* event_info) {
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  Ewk_Console_Message* message = static_cast<Ewk_Console_Message*>(event_info);
  Ewk_Console_Message_Level log_level =
      EwkInternalApiBinding::GetInstance().console_message.LevelGet(message);
  std::string text =
      EwkInternalApiBinding::GetInstance().console_message.TextGet(message);
  backend->delegate_->OnConsoleMessage(ConvertLogLevelToString(log_level),
                                       text);
}

void EwkWebViewBackend::OnNavigationPolicy(void* data, Evas_Object* obj,
                                           void* event_info) {
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  Ewk_Policy_Decision* policy_decision =
      static_cast<Ewk_Policy_Decision*>(event_info);
  ewk_policy_decision_use(policy_decision);

  if (!backend->has_navigation_delegate_) {
    return;
  }
  ewk_view_suspend(backend->view_);
  const char* url = ewk_policy_decision_url_get(policy_decision);
  backend->delegate_->OnNavigationPolicyDecide(url);
}

void EwkWebViewBackend::OnResponsePolicy(void* data, Evas_Object* obj,
                                         void* event_info) {
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  Ewk_Policy_Decision* policy_decision =
      static_cast<Ewk_Policy_Decision*>(event_info);
  int status_code =
      ewk_policy_decision_response_status_code_get(policy_decision);
  const char* url = ewk_policy_decision_url_get(policy_decision);
  ewk_policy_decision_use(policy_decision);

  if (!backend->has_navigation_delegate_ || status_code < 400) {
    return;
  }
  backend->delegate_->OnResponsePolicyDecide(url ? url : "", status_code);
}

void EwkWebViewBackend::OnUrlChange(void* data, Evas_Object* obj,
                                    void* event_info) {
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  backend->delegate_->OnUrlChanged(
      std::string(ewk_view_url_get(backend->view_)));
}

void EwkWebViewBackend::OnEvaluateJavaScript(Evas_Object* obj,
                                             const char* result_value,
                                             void* user_data) {
  auto* callback =
      static_cast<std::function<void(bool, const char*)>*>(user_data);
  (*callback)(true, result_value);
  delete callback;
}

void EwkWebViewBackend::OnJavaScriptMessage(Evas_Object* obj,
                                            Ewk_Script_Message message) {
  if (obj) {
    EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(
        evas_object_data_get(obj, kEwkInstance));
    if (backend) {
      std::string channel_name(message.name);
      std::string message_body(static_cast<char*>(message.body));
      backend->delegate_->OnJavaScriptMessage(channel_name, message_body);
    }
  }
}

Eina_Bool EwkWebViewBackend::OnJavaScriptAlertDialog(Evas_Object* o,
                                                     const char* message,
                                                     void* data) {
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  backend->delegate_->OnJavaScriptAlertDialog(message,
                                              ewk_view_url_get(backend->view_));
  return true;
}

Eina_Bool EwkWebViewBackend::OnJavaScriptConfirmDialog(Evas_Object* o,
                                                       const char* message,
                                                       void* data) {
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  backend->delegate_->OnJavaScriptConfirmDialog(
      message, ewk_view_url_get(backend->view_));
  return true;
}

Eina_Bool EwkWebViewBackend::OnJavaScriptPromptDialog(Evas_Object* o,
                                                      const char* message,
                                                      const char* default_text,
                                                      void* data) {
  EwkWebViewBackend* backend = static_cast<EwkWebViewBackend*>(data);
  backend->delegate_->OnJavaScriptPromptDialog(
      message, default_text, ewk_view_url_get(backend->view_));
  return true;
}
