// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wv_webview_backend.h"

#include <glib.h>

#include <cmath>
#include <cstring>
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
  int result = ftpw_flutter_inappwebview_wv_set_arguments(
      static_cast<int>(argv.size()), argv.data());
  if (result != 0) {
    LOG_ERROR("wv_set_arguments() returned %d.", result);
    return false;
  }
  result = ftpw_flutter_inappwebview_wv_init();
  if (result <= 0) {
    LOG_ERROR("wv_init() returned %d.", result);
    return false;
  }
  return true;
}

void WvWebViewBackend::GlobalShutdown() {
  FlushPendingTeardowns();
  ftpw_flutter_inappwebview_wv_shutdown();
}

bool WvWebViewBackend::Create(double width, double height, void* window) {
  view_ = ftpw_flutter_inappwebview_wv_view_create();
  if (!view_) {
    return false;
  }
  ftpw_flutter_inappwebview_wv_view_focus_set(view_, 1);

  void* context = ftpw_flutter_inappwebview_wv_view_context_get(view_);
  if (context) {
    void* cookie_manager =
        ftpw_flutter_inappwebview_wv_context_cookie_manager_get(context);
    if (cookie_manager) {
      ftpw_flutter_inappwebview_wv_cookie_manager_accept_policy_set(
          cookie_manager, WV_COOKIE_ACCEPT_POLICY_NO_THIRD_PARTY);
    }
    ftpw_flutter_inappwebview_wv_context_cache_model_set(
        context, WV_CACHE_MODEL_PRIMARY_WEBBROWSER);
  } else {
    LOG_WARN("Unable to access the WV context; skipping cookie/cache setup.");
  }

  void* settings = ftpw_flutter_inappwebview_wv_view_settings_get(view_);
  ftpw_flutter_inappwebview_wv_settings_ime_panel_enabled_set(settings, true);
  ftpw_flutter_inappwebview_wv_settings_force_zoom_set(settings, true);
  ftpw_flutter_inappwebview_wv_view_ime_window_set(view_, window);
  ftpw_flutter_inappwebview_wv_view_key_events_enabled_set(view_, true);
#ifdef WEBVIEW_TIZEN_TOUCH_EVENTS_ENABLED
  ftpw_flutter_inappwebview_wv_view_touch_events_enabled_set(view_, true);
  ftpw_flutter_inappwebview_wv_view_mouse_events_enabled_set(view_, false);
#else
  ftpw_flutter_inappwebview_wv_view_touch_events_enabled_set(view_, false);
  ftpw_flutter_inappwebview_wv_view_mouse_events_enabled_set(view_, true);
#endif

  ftpw_flutter_inappwebview_wv_view_javascript_alert_callback_set(
      view_, &WvWebViewBackend::OnJavaScriptAlertDialog, this);
  ftpw_flutter_inappwebview_wv_view_javascript_confirm_callback_set(
      view_, &WvWebViewBackend::OnJavaScriptConfirmDialog, this);
  ftpw_flutter_inappwebview_wv_view_javascript_prompt_callback_set(
      view_, &WvWebViewBackend::OnJavaScriptPromptDialog, this);

#ifdef TV_PROFILE
  ftpw_flutter_inappwebview_wv_view_set_support_video_hole(view_, window, true,
                                                           false);
#endif

  ftpw_flutter_inappwebview_wv_view_add_cb(view_, "offscreen,frame,rendered",
                                           &WvWebViewBackend::OnFrameRendered,
                                           this);
  ftpw_flutter_inappwebview_wv_view_add_cb(
      view_, "load,started", &WvWebViewBackend::OnLoadStarted, this);
  ftpw_flutter_inappwebview_wv_view_add_cb(
      view_, "load,finished", &WvWebViewBackend::OnLoadFinished, this);
  ftpw_flutter_inappwebview_wv_view_add_cb(view_, "load,progress",
                                           &WvWebViewBackend::OnProgress, this);
  ftpw_flutter_inappwebview_wv_view_add_cb(
      view_, "load,error", &WvWebViewBackend::OnLoadError, this);
  ftpw_flutter_inappwebview_wv_view_add_cb(
      view_, "console,message", &WvWebViewBackend::OnConsoleMessage, this);
  ftpw_flutter_inappwebview_wv_view_add_cb(
      view_, "policy,navigation,decide", &WvWebViewBackend::OnNavigationPolicy,
      this);
  ftpw_flutter_inappwebview_wv_view_add_cb(
      view_, "url,changed", &WvWebViewBackend::OnUrlChange, this);
  ftpw_flutter_inappwebview_wv_view_add_cb(
      view_, "title,changed", &WvWebViewBackend::OnTitleChange, this);

  ftpw_flutter_inappwebview_wv_view_resize(
      view_, static_cast<int>(std::round(width)),
      static_cast<int>(std::round(height)));

  return true;
}

WvWebViewBackend::~WvWebViewBackend() {
  if (void* instance = DetachView()) {
    ftpw_flutter_inappwebview_wv_view_destroy(instance);
  }
}

void* WvWebViewBackend::DetachView() {
  void* instance = view_;
  view_ = nullptr;
  if (!instance) {
    return nullptr;
  }

  ftpw_flutter_inappwebview_wv_view_remove_full_cb(
      instance, "offscreen,frame,rendered", &WvWebViewBackend::OnFrameRendered,
      this);
  ftpw_flutter_inappwebview_wv_view_remove_full_cb(
      instance, "load,started", &WvWebViewBackend::OnLoadStarted, this);
  ftpw_flutter_inappwebview_wv_view_remove_full_cb(
      instance, "load,finished", &WvWebViewBackend::OnLoadFinished, this);
  ftpw_flutter_inappwebview_wv_view_remove_full_cb(
      instance, "load,progress", &WvWebViewBackend::OnProgress, this);
  ftpw_flutter_inappwebview_wv_view_remove_full_cb(
      instance, "load,error", &WvWebViewBackend::OnLoadError, this);
  ftpw_flutter_inappwebview_wv_view_remove_full_cb(
      instance, "console,message", &WvWebViewBackend::OnConsoleMessage, this);
  ftpw_flutter_inappwebview_wv_view_remove_full_cb(
      instance, "policy,navigation,decide",
      &WvWebViewBackend::OnNavigationPolicy, this);
  ftpw_flutter_inappwebview_wv_view_remove_full_cb(
      instance, "url,changed", &WvWebViewBackend::OnUrlChange, this);
  ftpw_flutter_inappwebview_wv_view_remove_full_cb(
      instance, "title,changed", &WvWebViewBackend::OnTitleChange, this);

  ftpw_flutter_inappwebview_wv_view_javascript_alert_callback_set(
      instance, nullptr, nullptr);
  ftpw_flutter_inappwebview_wv_view_javascript_confirm_callback_set(
      instance, nullptr, nullptr);
  ftpw_flutter_inappwebview_wv_view_javascript_prompt_callback_set(
      instance, nullptr, nullptr);

  // NOTE: A suspended view must be resumed before it can be stopped.
  ftpw_flutter_inappwebview_wv_view_resume(instance);
  ftpw_flutter_inappwebview_wv_view_stop(instance);

  return instance;
}

std::function<void()> WvWebViewBackend::PrepareTeardown(
    std::shared_ptr<BufferPool> pool) {
  void* instance = DetachView();

  std::function<void()> destroy;
  if (instance) {
    destroy = [instance]() {
      ftpw_flutter_inappwebview_wv_view_destroy(instance);
    };
  }

  return RegisterPendingTeardown(std::move(pool), std::move(destroy));
}

void WvWebViewBackend::Offset(double /* left */, double /* top */) {
  // NOTE: WV input coordinates are relative to the offscreen view.
}

void WvWebViewBackend::Resize(double width, double height) {
  ftpw_flutter_inappwebview_wv_view_resize(
      view_, static_cast<int>(std::round(width)),
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
  point.x = static_cast<int>(std::round(x));
  point.y = static_cast<int>(std::round(y));
  point.state = state;

  GList* points = g_list_append(nullptr, &point);
  ftpw_flutter_inappwebview_wv_view_feed_touch_event(view_, touch_event_type,
                                                     points, WV_MODIFIER_NONE);
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

  int px = static_cast<int>(std::round(x));
  int py = static_cast<int>(std::round(y));

  if (event_type == 0) {
    mouse_button_type_ = mouse_button_type;
    ftpw_flutter_inappwebview_wv_view_feed_mouse_down(view_, mouse_button_type_,
                                                      px, py);
  } else if (event_type == 1) {
    if (dy != 0) {
      ftpw_flutter_inappwebview_wv_view_feed_mouse_wheel(
          view_, true, dy > 0 ? 1 : -1, px, py);
    }
  } else if (event_type == 2) {
    ftpw_flutter_inappwebview_wv_view_feed_mouse_up(view_, mouse_button_type_,
                                                    px, py);
    mouse_button_type_ = mouse_button_type;
  } else {
    LOG_WARN("Unknown mouse event type: %d", event_type);
  }
}

bool WvWebViewBackend::SendKey(const char* key, const char* string,
                               const char* compose, uint32_t modifiers,
                               uint32_t scan_code, bool is_down) {
  if (key && strcmp(key, "XF86Exit") == 0 && !is_down) {
    return false;
  }

  if (key && strcmp(key, "XF86Back") == 0 && !is_down) {
    if (ftpw_flutter_inappwebview_wv_view_back_possible(view_)) {
      ftpw_flutter_inappwebview_wv_view_back(view_);
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
  ftpw_flutter_inappwebview_wv_view_send_key_event(view_, &key_event,
                                                   is_down ? 1 : 0);
  return true;
}

void WvWebViewBackend::Resume() {
  if (view_) {
    ftpw_flutter_inappwebview_wv_view_resume(view_);
  }
}

void WvWebViewBackend::Suspend() {
  if (view_) {
    ftpw_flutter_inappwebview_wv_view_suspend(view_);
  }
}

void WvWebViewBackend::Stop() {
  if (view_) {
    ftpw_flutter_inappwebview_wv_view_stop(view_);
  }
}

void WvWebViewBackend::SetJavaScriptEnabled(bool enabled) {
  ftpw_flutter_inappwebview_wv_settings_javascript_enabled_set(
      ftpw_flutter_inappwebview_wv_view_settings_get(view_), enabled);
}

bool WvWebViewBackend::LoadUrl(const std::string& url) {
  return ftpw_flutter_inappwebview_wv_view_url_set(view_, url.c_str());
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

  // NOTE: The WV API requires a NUL-terminated non-empty request body.
  std::string body_str(body.begin(), body.end());
  bool ret = ftpw_flutter_inappwebview_wv_view_url_request_set(
      view_, url.c_str(), wv_method, wv_headers,
      body.empty() ? nullptr : body_str.c_str());
  g_hash_table_destroy(wv_headers);
  return ret;
}

bool WvWebViewBackend::LoadHtmlString(const std::string& html,
                                      const std::string& base_url) {
  return ftpw_flutter_inappwebview_wv_view_html_string_load(
      view_, html.c_str(), base_url.c_str(), nullptr);
}

bool WvWebViewBackend::CanGoBack() {
  return ftpw_flutter_inappwebview_wv_view_back_possible(view_);
}

bool WvWebViewBackend::CanGoForward() {
  return ftpw_flutter_inappwebview_wv_view_forward_possible(view_);
}

bool WvWebViewBackend::GoBack() {
  return ftpw_flutter_inappwebview_wv_view_back(view_);
}

bool WvWebViewBackend::GoForward() {
  return ftpw_flutter_inappwebview_wv_view_forward(view_);
}

bool WvWebViewBackend::Reload() {
  return ftpw_flutter_inappwebview_wv_view_reload(view_);
}

std::string WvWebViewBackend::GetCurrentUrl() {
  const char* url = ftpw_flutter_inappwebview_wv_view_url_get(view_);
  return url ? url : "";
}

int32_t WvWebViewBackend::GetProgress() {
  if (!view_) {
    return 0;
  }
  double progress = ftpw_flutter_inappwebview_wv_view_load_progress_get(view_);
  if (progress < 0) {
    return 0;
  }
  return static_cast<int32_t>(progress * 100);
}

double WvWebViewBackend::GetScale() {
  if (!view_) {
    return 1.0;
  }
  double scale = ftpw_flutter_inappwebview_wv_view_scale_get(view_);
  return scale > 0 ? scale : 1.0;
}

void WvWebViewBackend::SetScale(double scale, int32_t x, int32_t y) {
  ftpw_flutter_inappwebview_wv_view_scale_set(view_, scale, x, y);
}

void WvWebViewBackend::EvaluateJavaScript(
    const std::string& javascript,
    std::function<void(bool success, const char* result_value)> callback) {
  auto* callback_ptr =
      new std::function<void(bool, const char*)>(std::move(callback));
  if (!ftpw_flutter_inappwebview_wv_view_script_execute(
          view_, javascript.c_str(), &WvWebViewBackend::OnEvaluateJavaScript,
          callback_ptr)) {
    LOG_WARN("wv_view_script_execute failed.");
    (*callback_ptr)(false, nullptr);
    delete callback_ptr;
  }
}

void WvWebViewBackend::ClearCache() {
  void* context = ftpw_flutter_inappwebview_wv_view_context_get(view_);
  if (context) {
    ftpw_flutter_inappwebview_wv_context_cache_clear(context);
  }
}

std::string WvWebViewBackend::GetTitle() {
  const char* title = ftpw_flutter_inappwebview_wv_view_title_get(view_);
  return title ? title : "";
}

void WvWebViewBackend::ScrollTo(int32_t x, int32_t y) {
  ftpw_flutter_inappwebview_wv_view_scroll_set(view_, x, y);
}

void WvWebViewBackend::GetScrollPosition(int32_t* x, int32_t* y) {
  ftpw_flutter_inappwebview_wv_view_scroll_pos_get(view_, x, y);
}

void WvWebViewBackend::SetBackgroundColor(int r, int g, int b, int a) {
  ftpw_flutter_inappwebview_wv_view_bg_color_set(view_, r, g, b, a);
}

void WvWebViewBackend::SetUserAgent(const std::string& user_agent) {
  ftpw_flutter_inappwebview_wv_view_user_agent_set(view_, user_agent.c_str());
}

std::string WvWebViewBackend::GetUserAgent() {
  const char* user_agent =
      ftpw_flutter_inappwebview_wv_view_user_agent_get(view_);
  return user_agent ? user_agent : "";
}

void WvWebViewBackend::EnableZoom(bool enabled) {
  ftpw_flutter_inappwebview_wv_settings_force_zoom_set(
      ftpw_flutter_inappwebview_wv_view_settings_get(view_), enabled);
}

void WvWebViewBackend::JavaScriptAlertReply() {
  ftpw_flutter_inappwebview_wv_view_javascript_alert_reply(view_);
}

void WvWebViewBackend::JavaScriptConfirmReply(bool result) {
  ftpw_flutter_inappwebview_wv_view_javascript_confirm_reply(view_, result);
}

void WvWebViewBackend::JavaScriptPromptReply(const char* result) {
  ftpw_flutter_inappwebview_wv_view_javascript_prompt_reply(view_, result);
}

bool WvWebViewBackend::ClearCookies() {
  if (!view_) {
    return false;
  }
  void* context = ftpw_flutter_inappwebview_wv_view_context_get(view_);
  void* cookie_manager =
      context ? ftpw_flutter_inappwebview_wv_context_cookie_manager_get(context)
              : nullptr;
  if (cookie_manager) {
    ftpw_flutter_inappwebview_wv_cookie_manager_cookies_clear(cookie_manager);
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
  ftpw_flutter_inappwebview_wv_view_main_frame_scrollbar_visible_set(
      backend->view_, true);
  const char* url = ftpw_flutter_inappwebview_wv_view_url_get(backend->view_);
  backend->delegate_->OnLoadStarted(url ? url : "");
}

void WvWebViewBackend::OnLoadFinished(void* obj, void* event_info,
                                      void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  const char* url = ftpw_flutter_inappwebview_wv_view_url_get(backend->view_);
  backend->delegate_->OnLoadFinished(url ? url : "");
}

void WvWebViewBackend::OnProgress(void* obj, void* event_info,
                                  void* user_data) {
  if (!event_info) {
    return;
  }
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  int32_t progress =
      static_cast<int32_t>((*static_cast<double*>(event_info)) * 100);
  backend->delegate_->OnProgress(progress);
}

void WvWebViewBackend::OnLoadError(void* obj, void* event_info,
                                   void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  void* error = static_cast<void*>(event_info);
  const char* description =
      ftpw_flutter_inappwebview_wv_error_description_get(error);
  const char* url = ftpw_flutter_inappwebview_wv_error_url_get(error);
  backend->delegate_->OnLoadError(
      ftpw_flutter_inappwebview_wv_error_code_get(error),
      description ? description : "", url ? url : "");
}

void WvWebViewBackend::OnConsoleMessage(void* obj, void* event_info,
                                        void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  void* message = static_cast<void*>(event_info);
  wv_console_message_level_e log_level =
      ftpw_flutter_inappwebview_wv_console_message_level_get(message);
  const char* text =
      ftpw_flutter_inappwebview_wv_console_message_text_get(message);
  backend->delegate_->OnConsoleMessage(ConvertLogLevelToString(log_level),
                                       text ? text : "");
}

void WvWebViewBackend::OnNavigationPolicy(void* obj, void* event_info,
                                          void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  void* policy_decision = static_cast<void*>(event_info);

  const char* current_url =
      ftpw_flutter_inappwebview_wv_view_url_get(backend->view_);
  const std::string url_before_navigation = current_url ? current_url : "";
  const char* url =
      ftpw_flutter_inappwebview_wv_policy_decision_url_get(policy_decision);
  const std::string requested_url = url ? url : "";
  ftpw_flutter_inappwebview_wv_policy_decision_use(policy_decision);

  backend->delegate_->OnNavigationPolicyDecide(requested_url,
                                               url_before_navigation);
}

void WvWebViewBackend::OnUrlChange(void* obj, void* event_info,
                                   void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  const char* url = ftpw_flutter_inappwebview_wv_view_url_get(backend->view_);
  backend->delegate_->OnUrlChanged(url ? url : "");
}

void WvWebViewBackend::OnTitleChange(void* obj, void* event_info,
                                     void* user_data) {
  const char* title = static_cast<const char*>(event_info);
  if (!title) {
    return;
  }
  static_cast<WvWebViewBackend*>(user_data)->delegate_->OnTitleChanged(title);
}

void WvWebViewBackend::OnEvaluateJavaScript(void* obj, const char* result_value,
                                            void* user_data) {
  auto* callback =
      static_cast<std::function<void(bool, const char*)>*>(user_data);
  (*callback)(true, result_value);
  delete callback;
}

bool WvWebViewBackend::OnJavaScriptAlertDialog(void* view, const char* message,
                                               void* data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(data);
  const char* url = ftpw_flutter_inappwebview_wv_view_url_get(backend->view_);
  backend->delegate_->OnJavaScriptAlertDialog(message ? message : "",
                                              url ? url : "");
  return true;
}

bool WvWebViewBackend::OnJavaScriptConfirmDialog(void* view,
                                                 const char* message,
                                                 void* data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(data);
  const char* url = ftpw_flutter_inappwebview_wv_view_url_get(backend->view_);
  backend->delegate_->OnJavaScriptConfirmDialog(message ? message : "",
                                                url ? url : "");
  return true;
}

bool WvWebViewBackend::OnJavaScriptPromptDialog(void* view, const char* message,
                                                const char* default_text,
                                                void* data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(data);
  const char* url = ftpw_flutter_inappwebview_wv_view_url_get(backend->view_);
  backend->delegate_->OnJavaScriptPromptDialog(
      message ? message : "", default_text ? default_text : "", url ? url : "");
  return true;
}
