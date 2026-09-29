// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wv_webview_backend.h"

#include <glib.h>

#include <cmath>
#include <cstring>
#include <mutex>
#include <vector>

#include "buffer_pool.h"
#include "log.h"

namespace {

std::string ConvertLogLevelToString(wv_console_message_level_e level) {
  switch (level) {
    case WV_CONSOLE_MESSAGE_LEVEL_NULL:
    case WV_CONSOLE_MESSAGE_LEVEL_LOG:
      return "log";
    case WV_CONSOLE_MESSAGE_LEVEL_WARNING:
      return "warning";
    case WV_CONSOLE_MESSAGE_LEVEL_ERROR:
      return "error";
    case WV_CONSOLE_MESSAGE_LEVEL_DEBUG:
      return "debug";
    case WV_CONSOLE_MESSAGE_LEVEL_INFO:
      return "info";
    default:
      return "log";
  }
}

enum ModifierBit : unsigned int {
  kModifierShift = 0x0001,
  kModifierCtrl = 0x0002,
  kModifierAlt = 0x0004,
  kModifierWin = 0x0008,
  kModifierAltGr = 0x0400,
  kLockCaps = 0x0200,
  kLockNum = 0x0100,
};

wv_modifier_e ConvertModifiers(unsigned int modifiers) {
  unsigned int wv_modifiers = WV_MODIFIER_NONE;
  if (modifiers & kModifierShift) {
    wv_modifiers |= WV_MODIFIER_SHIFT;
  }
  if (modifiers & kModifierCtrl) {
    wv_modifiers |= WV_MODIFIER_CONTROL;
  }
  if (modifiers & kModifierAlt) {
    wv_modifiers |= WV_MODIFIER_ALT;
  }
  if (modifiers & kModifierWin) {
    wv_modifiers |= WV_MODIFIER_SUPER;
  }
  if (modifiers & kModifierAltGr) {
    wv_modifiers |= WV_MODIFIER_HYPER;
  }
  if (modifiers & kLockCaps) {
    wv_modifiers |= WV_MODIFIER_CAPS_LOCK;
  }
  if (modifiers & kLockNum) {
    wv_modifiers |= WV_MODIFIER_NUM_LOCK;
  }
  return static_cast<wv_modifier_e>(wv_modifiers);
}

std::mutex g_view_registry_mutex;
std::map<void*, WvWebViewBackend*> g_view_registry;

}  // namespace

WvWebViewBackend::WvWebViewBackend(Delegate* delegate) : delegate_(delegate) {}

bool WvWebViewBackend::GlobalInitialize(bool standalone) {
  std::vector<const char*> argv = {
      "--disable-pinch",
      "--js-flags=--expose-gc",
      "--single-process",
      "--no-zygote",
  };
  if (standalone) {
    argv.push_back("--enable-wv-standalone");
  }
  int result = ftpw_webview_flutter_wv_set_arguments(
      static_cast<int>(argv.size()), argv.data());
  if (result != 0) {
    LOG_ERROR("wv_set_arguments() returned %d.", result);
    return false;
  }
  result = ftpw_webview_flutter_wv_init();
  if (result <= 0) {
    LOG_ERROR("wv_init() returned %d.", result);
    return false;
  }
  return true;
}

void WvWebViewBackend::GlobalShutdown() {
  FlushPendingTeardowns();
  ftpw_webview_flutter_wv_shutdown();
}

bool WvWebViewBackend::Create(double width, double height, void* window,
                              bool /*engine_policy*/) {
  window_ = window;

  view_ = ftpw_webview_flutter_wv_view_create();
  if (!view_) {
    return false;
  }
  ftpw_webview_flutter_wv_view_focus_set(view_, 1);

  void* context = ftpw_webview_flutter_wv_view_context_get(view_);
  void* cookie_manager =
      ftpw_webview_flutter_wv_context_cookie_manager_get(context);
  if (cookie_manager) {
    ftpw_webview_flutter_wv_cookie_manager_accept_policy_set(cookie_manager);
  }
  ftpw_webview_flutter_wv_context_cache_model_set(context);

  void* settings = ftpw_webview_flutter_wv_view_settings_get(view_);
  ftpw_webview_flutter_wv_settings_ime_panel_enabled_set(settings, true);
  ftpw_webview_flutter_wv_settings_force_zoom_set(settings, true);
  ftpw_webview_flutter_wv_view_ime_window_set(view_, window_);
  ftpw_webview_flutter_wv_view_key_events_enabled_set(view_, true);
#ifdef WEBVIEW_TIZEN_TOUCH_EVENTS_ENABLED
  ftpw_webview_flutter_wv_view_touch_events_enabled_set(view_, true);
  ftpw_webview_flutter_wv_view_mouse_events_enabled_set(view_, false);
#else
  ftpw_webview_flutter_wv_view_touch_events_enabled_set(view_, false);
  ftpw_webview_flutter_wv_view_mouse_events_enabled_set(view_, true);
#endif

  ftpw_webview_flutter_wv_view_javascript_alert_callback_set(
      view_, &WvWebViewBackend::OnJavaScriptAlertDialog, this);
  ftpw_webview_flutter_wv_view_javascript_confirm_callback_set(
      view_, &WvWebViewBackend::OnJavaScriptConfirmDialog, this);
  ftpw_webview_flutter_wv_view_javascript_prompt_callback_set(
      view_, &WvWebViewBackend::OnJavaScriptPromptDialog, this);

#ifdef TV_PROFILE
  ftpw_webview_flutter_wv_view_set_support_video_hole(view_, window_, true,
                                                      false);
#endif

  ftpw_webview_flutter_wv_view_add_cb(view_, "offscreen,frame,rendered",
                                      &WvWebViewBackend::OnFrameRendered, this);
  ftpw_webview_flutter_wv_view_add_cb(view_, "load,started",
                                      &WvWebViewBackend::OnLoadStarted, this);
  ftpw_webview_flutter_wv_view_add_cb(view_, "load,finished",
                                      &WvWebViewBackend::OnLoadFinished, this);
  ftpw_webview_flutter_wv_view_add_cb(view_, "load,progress",
                                      &WvWebViewBackend::OnProgress, this);
  ftpw_webview_flutter_wv_view_add_cb(view_, "load,error",
                                      &WvWebViewBackend::OnLoadError, this);
  ftpw_webview_flutter_wv_view_add_cb(
      view_, "console,message", &WvWebViewBackend::OnConsoleMessage, this);
  ftpw_webview_flutter_wv_view_add_cb(view_, "policy,navigation,decide",
                                      &WvWebViewBackend::OnNavigationPolicy,
                                      this);
  ftpw_webview_flutter_wv_view_add_cb(view_, "policy,response,decide",
                                      &WvWebViewBackend::OnResponsePolicy,
                                      this);
  ftpw_webview_flutter_wv_view_add_cb(view_, "url,changed",
                                      &WvWebViewBackend::OnUrlChange, this);

  ftpw_webview_flutter_wv_view_resize(view_,
                                      static_cast<int>(std::round(width)),
                                      static_cast<int>(std::round(height)));

  {
    std::lock_guard<std::mutex> lock(g_view_registry_mutex);
    g_view_registry[view_] = this;
  }

  return true;
}

std::function<void()> WvWebViewBackend::PrepareTeardown(
    std::shared_ptr<BufferPool> pool) {
  void* instance = view_;
  view_ = nullptr;

  std::function<void()> destroy;
  if (instance) {
    ftpw_webview_flutter_wv_view_remove_full_cb(
        instance, "offscreen,frame,rendered",
        &WvWebViewBackend::OnFrameRendered, this);
    ftpw_webview_flutter_wv_view_remove_full_cb(
        instance, "load,started", &WvWebViewBackend::OnLoadStarted, this);
    ftpw_webview_flutter_wv_view_remove_full_cb(
        instance, "load,finished", &WvWebViewBackend::OnLoadFinished, this);
    ftpw_webview_flutter_wv_view_remove_full_cb(
        instance, "load,progress", &WvWebViewBackend::OnProgress, this);
    ftpw_webview_flutter_wv_view_remove_full_cb(
        instance, "load,error", &WvWebViewBackend::OnLoadError, this);
    ftpw_webview_flutter_wv_view_remove_full_cb(
        instance, "console,message", &WvWebViewBackend::OnConsoleMessage, this);
    ftpw_webview_flutter_wv_view_remove_full_cb(
        instance, "policy,navigation,decide",
        &WvWebViewBackend::OnNavigationPolicy, this);
    ftpw_webview_flutter_wv_view_remove_full_cb(
        instance, "policy,response,decide", &WvWebViewBackend::OnResponsePolicy,
        this);
    ftpw_webview_flutter_wv_view_remove_full_cb(
        instance, "url,changed", &WvWebViewBackend::OnUrlChange, this);

    ftpw_webview_flutter_wv_view_javascript_alert_callback_set(
        instance, nullptr, nullptr);
    ftpw_webview_flutter_wv_view_javascript_confirm_callback_set(
        instance, nullptr, nullptr);
    ftpw_webview_flutter_wv_view_javascript_prompt_callback_set(
        instance, nullptr, nullptr);

    {
      std::lock_guard<std::mutex> lock(g_view_registry_mutex);
      g_view_registry.erase(instance);
    }

    ftpw_webview_flutter_wv_view_stop(instance);
    ftpw_webview_flutter_wv_view_suspend(instance);

    destroy = [instance]() { ftpw_webview_flutter_wv_view_destroy(instance); };
  }

  return RegisterPendingTeardown(std::move(pool), std::move(destroy));
}

void WvWebViewBackend::Offset(double left, double top) {}

void WvWebViewBackend::Resize(double width, double height) {
  ftpw_webview_flutter_wv_view_resize(view_,
                                      static_cast<int>(std::round(width)),
                                      static_cast<int>(std::round(height)));
}

void WvWebViewBackend::Touch(int event_type, int button_type, double x,
                             double y, double dx, double dy) {
#ifdef WEBVIEW_TIZEN_TOUCH_EVENTS_ENABLED
  SendTouchEvent(event_type, x, y);
#else
  SendMouseEvent(event_type, button_type, x, y, dx, dy);
#endif
}

void WvWebViewBackend::SendTouchEvent(int event_type, double x, double y) {
  wv_touch_event_type_e touch_event_type = WV_TOUCH_EVENT_START;
  wv_touch_point_state_e state = WV_TOUCH_POINT_STATE_DOWN;
  if (event_type == 0) {
    touch_event_type = WV_TOUCH_EVENT_START;
    state = WV_TOUCH_POINT_STATE_DOWN;
  } else if (event_type == 1) {
    touch_event_type = WV_TOUCH_EVENT_MOVE;
    state = WV_TOUCH_POINT_STATE_MOVE;
  } else if (event_type == 2) {
    touch_event_type = WV_TOUCH_EVENT_END;
    state = WV_TOUCH_POINT_STATE_UP;
  } else {
    LOG_WARN("Unknown touch event type: %d", event_type);
  }

  wv_touch_point_s point;
  point.id = 0;
  point.x = static_cast<int>(x);
  point.y = static_cast<int>(y);
  point.state = state;

  GList* points = g_list_append(nullptr, &point);
  ftpw_webview_flutter_wv_view_feed_touch_event(view_, touch_event_type, points,
                                                WV_MODIFIER_NONE);
  g_list_free(points);
}

void WvWebViewBackend::SendMouseEvent(int event_type, int button_type, double x,
                                      double y, double dx, double dy) {
  wv_mouse_button_type_e mouse_button_type =
      static_cast<wv_mouse_button_type_e>(0);
  switch (button_type) {
    case 1:
      mouse_button_type = WV_MOUSE_BUTTON_LEFT;
      break;
    case 2:
      mouse_button_type = WV_MOUSE_BUTTON_RIGHT;
      break;
    case 4:
      mouse_button_type = WV_MOUSE_BUTTON_MIDDLE;
      break;
  }

  int px = static_cast<int>(x);
  int py = static_cast<int>(y);

  if (event_type == 0) {
    mouse_button_type_ = mouse_button_type;
    ftpw_webview_flutter_wv_view_feed_mouse_down(view_, mouse_button_type_, px,
                                                 py);
  } else if (event_type == 1) {
    if (dy != 0) {
      ftpw_webview_flutter_wv_view_feed_mouse_wheel(view_, true,
                                                    dy > 0 ? 1 : -1, px, py);
    }
  } else if (event_type == 2) {
    ftpw_webview_flutter_wv_view_feed_mouse_up(view_, mouse_button_type_, px,
                                               py);
    mouse_button_type_ = mouse_button_type;
  } else {
    LOG_WARN("Unknown mouse event type: %d", event_type);
  }
}

bool WvWebViewBackend::SendKey(const char* key, const char* string,
                               const char* compose, uint32_t modifiers,
                               uint32_t scan_code, bool is_down) {
  if (strcmp(key, "XF86Exit") == 0 && !is_down) {
    return false;
  }

  if (strcmp(key, "XF86Back") == 0 && !is_down) {
    if (ftpw_webview_flutter_wv_view_back_possible(view_)) {
      ftpw_webview_flutter_wv_view_back(view_);
      return true;
    }
    return false;
  }

  wv_key_event_s key_event = {};
  key_event.key_name = key;
  key_event.key = key;
  key_event.string = string;
  key_event.compose = compose;
  key_event.modifiers = ConvertModifiers(modifiers);
  key_event.key_code = scan_code;
  ftpw_webview_flutter_wv_view_send_key_event(view_, &key_event,
                                              is_down ? 1 : 0);
  return true;
}

void WvWebViewBackend::Resume() {
  if (view_) {
    ftpw_webview_flutter_wv_view_resume(view_);
  }
}

void WvWebViewBackend::Stop() {
  if (view_) {
    ftpw_webview_flutter_wv_view_stop(view_);
  }
}

void WvWebViewBackend::SetJavaScriptEnabled(bool enabled) {
  ftpw_webview_flutter_wv_settings_javascript_enabled_set(
      ftpw_webview_flutter_wv_view_settings_get(view_), enabled);
}

void WvWebViewBackend::SetHasNavigationDelegate(bool has_navigation_delegate) {
  has_navigation_delegate_ = has_navigation_delegate;
}

void WvWebViewBackend::LoadUrl(const std::string& url) {
  ftpw_webview_flutter_wv_view_url_set(view_, url.c_str());
}

bool WvWebViewBackend::LoadUrlRequest(
    const std::string& url, int32_t method,
    const std::map<std::string, std::string>& headers,
    const std::vector<uint8_t>& body) {
  wv_http_method_e wv_method = WV_HTTP_METHOD_GET;
  if (method == 1) {
    wv_method = WV_HTTP_METHOD_POST;
  }

  GHashTable* wv_headers =
      g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
  for (const auto& header : headers) {
    g_hash_table_insert(wv_headers, g_strdup(header.first.c_str()),
                        g_strdup(header.second.c_str()));
  }

  bool ret = ftpw_webview_flutter_wv_view_url_request_set(
      view_, url.c_str(), wv_method, wv_headers,
      reinterpret_cast<const char*>(body.data()));
  g_hash_table_destroy(wv_headers);
  return ret;
}

void WvWebViewBackend::LoadHtmlString(const std::string& html,
                                      const std::string& base_url) {
  ftpw_webview_flutter_wv_view_html_string_load(view_, html.c_str(),
                                                base_url.c_str(), nullptr);
}

bool WvWebViewBackend::CanGoBack() {
  return ftpw_webview_flutter_wv_view_back_possible(view_);
}

bool WvWebViewBackend::CanGoForward() {
  return ftpw_webview_flutter_wv_view_forward_possible(view_);
}

void WvWebViewBackend::GoBack() { ftpw_webview_flutter_wv_view_back(view_); }

void WvWebViewBackend::GoForward() {
  ftpw_webview_flutter_wv_view_forward(view_);
}

void WvWebViewBackend::Reload() { ftpw_webview_flutter_wv_view_reload(view_); }

std::string WvWebViewBackend::GetCurrentUrl() {
  const char* url = ftpw_webview_flutter_wv_view_url_get(view_);
  return url ? url : "";
}

void WvWebViewBackend::EvaluateJavaScript(
    const std::string& javascript,
    std::function<void(bool success, const char* result_value)> callback) {
  auto* callback_ptr =
      new std::function<void(bool, const char*)>(std::move(callback));
  if (!ftpw_webview_flutter_wv_view_script_execute(
          view_, javascript.c_str(), &WvWebViewBackend::OnEvaluateJavaScript,
          callback_ptr)) {
    LOG_WARN("wv_view_script_execute failed.");
    (*callback_ptr)(false, nullptr);
    delete callback_ptr;
  }
}

void WvWebViewBackend::RegisterJavaScriptChannel(const std::string& name) {
  ftpw_webview_flutter_wv_view_javascript_message_handler_add(
      view_, &WvWebViewBackend::OnJavaScriptMessage, name.c_str());
}

void WvWebViewBackend::ClearCache() {
  ftpw_webview_flutter_wv_context_cache_clear(
      ftpw_webview_flutter_wv_view_context_get(view_));
}

void WvWebViewBackend::ClearLocalStorage() {
  ftpw_webview_flutter_wv_context_web_storage_delete_all(
      ftpw_webview_flutter_wv_view_context_get(view_));
}

std::string WvWebViewBackend::GetTitle() {
  const char* title = ftpw_webview_flutter_wv_view_title_get(view_);
  return title ? title : "";
}

void WvWebViewBackend::ScrollTo(int32_t x, int32_t y) {
  ftpw_webview_flutter_wv_view_scroll_set(view_, x, y);
}

void WvWebViewBackend::ScrollBy(int32_t x, int32_t y) {
  ftpw_webview_flutter_wv_view_scroll_by(view_, x, y);
}

void WvWebViewBackend::GetScrollPosition(int32_t* x, int32_t* y) {
  ftpw_webview_flutter_wv_view_scroll_pos_get(view_, x, y);
}

void WvWebViewBackend::SetBackgroundColor(int r, int g, int b, int a) {
  ftpw_webview_flutter_wv_view_bg_color_set(view_, r, g, b, a);
}

void WvWebViewBackend::SetUserAgent(const std::string& user_agent) {
  ftpw_webview_flutter_wv_view_user_agent_set(view_, user_agent.c_str());
}

std::string WvWebViewBackend::GetUserAgent() {
  const char* user_agent = ftpw_webview_flutter_wv_view_user_agent_get(view_);
  return user_agent ? user_agent : "";
}

void WvWebViewBackend::EnableZoom(bool enabled) {
  ftpw_webview_flutter_wv_settings_force_zoom_set(
      ftpw_webview_flutter_wv_view_settings_get(view_), enabled);
}

void WvWebViewBackend::JavaScriptAlertReply() {
  ftpw_webview_flutter_wv_view_javascript_alert_reply(view_);
}

void WvWebViewBackend::JavaScriptConfirmReply(bool result) {
  ftpw_webview_flutter_wv_view_javascript_confirm_reply(view_, result);
}

void WvWebViewBackend::JavaScriptPromptReply(const std::string& result) {
  ftpw_webview_flutter_wv_view_javascript_prompt_reply(view_, result.c_str());
}

void WvWebViewBackend::SetScrollbarVisible(bool visible) {
  scrollbar_enabled_ = visible;
  ftpw_webview_flutter_wv_view_main_frame_scrollbar_visible_set(
      view_, scrollbar_enabled_);
}

bool WvWebViewBackend::ClearCookies() {
  void* cookie_manager = ftpw_webview_flutter_wv_context_cookie_manager_get(
      ftpw_webview_flutter_wv_view_context_get(view_));
  if (cookie_manager) {
    ftpw_webview_flutter_wv_cookie_manager_cookies_clear(cookie_manager);
    return true;
  }
  return false;
}

void WvWebViewBackend::OnFrameRendered(void* obj, void* event_info,
                                       void* user_data) {
  if (event_info) {
    static_cast<WvWebViewBackend*>(user_data)->delegate_->OnFrameRendered(
        event_info);
  }
}

void WvWebViewBackend::OnLoadStarted(void* obj, void* event_info,
                                     void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  ftpw_webview_flutter_wv_view_main_frame_scrollbar_visible_set(
      backend->view_, backend->scrollbar_enabled_);
  const char* url = ftpw_webview_flutter_wv_view_url_get(backend->view_);
  backend->delegate_->OnLoadStarted(url ? url : "");
}

void WvWebViewBackend::OnLoadFinished(void* obj, void* event_info,
                                      void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  const char* url = ftpw_webview_flutter_wv_view_url_get(backend->view_);
  backend->delegate_->OnLoadFinished(url ? url : "");
}

void WvWebViewBackend::OnProgress(void* obj, void* event_info,
                                  void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  int32_t progress =
      static_cast<int32_t>((*static_cast<double*>(event_info)) * 100);
  backend->delegate_->OnProgress(progress);
}

void WvWebViewBackend::OnLoadError(void* obj, void* event_info,
                                   void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  void* error = event_info;
  const char* description =
      ftpw_webview_flutter_wv_error_description_get(error);
  const char* url = ftpw_webview_flutter_wv_error_url_get(error);
  backend->delegate_->OnLoadError(ftpw_webview_flutter_wv_error_code_get(error),
                                  description ? description : "",
                                  url ? url : "");
}

void WvWebViewBackend::OnConsoleMessage(void* obj, void* event_info,
                                        void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  void* message = event_info;
  wv_console_message_level_e log_level =
      ftpw_webview_flutter_wv_console_message_level_get(message);
  const char* text = ftpw_webview_flutter_wv_console_message_text_get(message);
  backend->delegate_->OnConsoleMessage(ConvertLogLevelToString(log_level),
                                       text ? text : "");
}

void WvWebViewBackend::OnNavigationPolicy(void* obj, void* event_info,
                                          void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  void* policy_decision = event_info;
  ftpw_webview_flutter_wv_policy_decision_use(policy_decision);

  if (!backend->has_navigation_delegate_) {
    return;
  }
  ftpw_webview_flutter_wv_view_suspend(backend->view_);
  const char* url =
      ftpw_webview_flutter_wv_policy_decision_url_get(policy_decision);
  backend->delegate_->OnNavigationPolicyDecide(url ? url : "");
}

void WvWebViewBackend::OnResponsePolicy(void* obj, void* event_info,
                                        void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  void* policy_decision = event_info;
  int status_code =
      ftpw_webview_flutter_wv_policy_decision_response_status_code_get(
          policy_decision);
  const char* url =
      ftpw_webview_flutter_wv_policy_decision_url_get(policy_decision);
  ftpw_webview_flutter_wv_policy_decision_use(policy_decision);

  if (!backend->has_navigation_delegate_ || status_code < 400) {
    return;
  }
  backend->delegate_->OnResponsePolicyDecide(url ? url : "", status_code);
}

void WvWebViewBackend::OnUrlChange(void* obj, void* event_info,
                                   void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  const char* url = ftpw_webview_flutter_wv_view_url_get(backend->view_);
  backend->delegate_->OnUrlChanged(url ? url : "");
}

void WvWebViewBackend::OnEvaluateJavaScript(void* obj, const char* result_value,
                                            void* user_data) {
  auto* callback =
      static_cast<std::function<void(bool, const char*)>*>(user_data);
  (*callback)(true, result_value);
  delete callback;
}

void WvWebViewBackend::OnJavaScriptMessage(void* obj,
                                           wv_script_message_s message) {
  WvWebViewBackend* backend = nullptr;
  {
    std::lock_guard<std::mutex> lock(g_view_registry_mutex);
    auto it = g_view_registry.find(obj);
    if (it != g_view_registry.end()) {
      backend = it->second;
    }
  }
  if (backend && message.name && message.body) {
    backend->delegate_->OnJavaScriptMessage(message.name,
                                            static_cast<char*>(message.body));
  }
}

bool WvWebViewBackend::OnJavaScriptAlertDialog(void* view, const char* message,
                                               void* data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(data);
  const char* url = ftpw_webview_flutter_wv_view_url_get(backend->view_);
  backend->delegate_->OnJavaScriptAlertDialog(message ? message : "",
                                              url ? url : "");
  return true;
}

bool WvWebViewBackend::OnJavaScriptConfirmDialog(void* view,
                                                 const char* message,
                                                 void* data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(data);
  const char* url = ftpw_webview_flutter_wv_view_url_get(backend->view_);
  backend->delegate_->OnJavaScriptConfirmDialog(message ? message : "",
                                                url ? url : "");
  return true;
}

bool WvWebViewBackend::OnJavaScriptPromptDialog(void* view, const char* message,
                                                const char* default_text,
                                                void* data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(data);
  const char* url = ftpw_webview_flutter_wv_view_url_get(backend->view_);
  backend->delegate_->OnJavaScriptPromptDialog(
      message ? message : "", default_text ? default_text : "", url ? url : "");
  return true;
}
