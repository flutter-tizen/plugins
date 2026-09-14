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
std::map<wv_view_h, WvWebViewBackend*> g_view_registry;

}  // namespace

WvWebViewBackend::WvWebViewBackend(Delegate* delegate) : delegate_(delegate) {}

bool WvWebViewBackend::GlobalInitialize(bool standalone) {
  auto& wv = WvInternalApiBinding::GetInstance();
  std::vector<const char*> argv = {
      "--disable-pinch",
      "--js-flags=--expose-gc",
      "--single-process",
      "--no-zygote",
  };
  if (standalone) {
    argv.push_back("--enable-wv-standalone");
  }
  int result = wv.main.SetArguments(static_cast<int>(argv.size()), argv.data());
  if (result != 0) {
    LOG_ERROR("wv_set_arguments() returned %d.", result);
    return false;
  }
  result = wv.main.Init();
  if (result <= 0) {
    LOG_ERROR("wv_init() returned %d.", result);
    return false;
  }
  return true;
}

void WvWebViewBackend::GlobalShutdown() {
  FlushPendingTeardowns();
  WvInternalApiBinding::GetInstance().main.Shutdown();
}

bool WvWebViewBackend::Create(double width, double height, void* window,
                              bool /*engine_policy*/) {
  window_ = window;

  auto& wv = WvInternalApiBinding::GetInstance();

  view_ = wv.view.Create();
  if (!view_) {
    return false;
  }
  wv.view.FocusSet(view_, 1);

  wv_context_h context = wv.view.ContextGet(view_);
  wv_cookie_manager_h cookie_manager = wv.context.CookieManagerGet(context);
  if (cookie_manager) {
    wv.cookie_manager.AcceptPolicySet(cookie_manager,
                                      WV_COOKIE_ACCEPT_POLICY_NO_THIRD_PARTY);
  }
  wv.context.CacheModelSet(context, WV_CACHE_MODEL_PRIMARY_WEBBROWSER);

  wv_settings_h settings = wv.view.SettingsGet(view_);
  wv.settings.ImePanelEnabledSet(settings, true);
  wv.settings.ForceZoomSet(settings, true);
  wv.view.ImeWindowSet(view_, window_);
  wv.view.KeyEventsEnabledSet(view_, true);
#ifdef WEBVIEW_TIZEN_TOUCH_EVENTS_ENABLED
  wv.view.TouchEventsEnabledSet(view_, true);
  wv.view.MouseEventsEnabledSet(view_, false);
#else
  wv.view.TouchEventsEnabledSet(view_, false);
  wv.view.MouseEventsEnabledSet(view_, true);
#endif

  wv.view.OnJavaScriptAlert(view_, &WvWebViewBackend::OnJavaScriptAlertDialog,
                            this);
  wv.view.OnJavaScriptConfirm(
      view_, &WvWebViewBackend::OnJavaScriptConfirmDialog, this);
  wv.view.OnJavaScriptPrompt(view_, &WvWebViewBackend::OnJavaScriptPromptDialog,
                             this);

#ifdef TV_PROFILE
  wv.view.SetSupportVideoHole(view_, window_, true, false);
#endif

  wv.view.AddCallback(view_, "offscreen,frame,rendered",
                      &WvWebViewBackend::OnFrameRendered, this);
  wv.view.AddCallback(view_, "load,started", &WvWebViewBackend::OnLoadStarted,
                      this);
  wv.view.AddCallback(view_, "load,finished", &WvWebViewBackend::OnLoadFinished,
                      this);
  wv.view.AddCallback(view_, "load,progress", &WvWebViewBackend::OnProgress,
                      this);
  wv.view.AddCallback(view_, "load,error", &WvWebViewBackend::OnLoadError,
                      this);
  wv.view.AddCallback(view_, "console,message",
                      &WvWebViewBackend::OnConsoleMessage, this);
  wv.view.AddCallback(view_, "policy,navigation,decide",
                      &WvWebViewBackend::OnNavigationPolicy, this);
  wv.view.AddCallback(view_, "policy,response,decide",
                      &WvWebViewBackend::OnResponsePolicy, this);
  wv.view.AddCallback(view_, "url,changed", &WvWebViewBackend::OnUrlChange,
                      this);

  wv.view.Resize(view_, static_cast<int>(std::round(width)),
                 static_cast<int>(std::round(height)));

  {
    std::lock_guard<std::mutex> lock(g_view_registry_mutex);
    g_view_registry[view_] = this;
  }

  return true;
}

std::function<void()> WvWebViewBackend::PrepareTeardown(
    std::shared_ptr<BufferPool> pool) {
  wv_view_h instance = view_;
  view_ = nullptr;

  std::function<void()> destroy;
  if (instance) {
    auto& wv = WvInternalApiBinding::GetInstance();
    wv.view.RemoveFullCallback(instance, "offscreen,frame,rendered",
                               &WvWebViewBackend::OnFrameRendered, this);
    wv.view.RemoveFullCallback(instance, "load,started",
                               &WvWebViewBackend::OnLoadStarted, this);
    wv.view.RemoveFullCallback(instance, "load,finished",
                               &WvWebViewBackend::OnLoadFinished, this);
    wv.view.RemoveFullCallback(instance, "load,progress",
                               &WvWebViewBackend::OnProgress, this);
    wv.view.RemoveFullCallback(instance, "load,error",
                               &WvWebViewBackend::OnLoadError, this);
    wv.view.RemoveFullCallback(instance, "console,message",
                               &WvWebViewBackend::OnConsoleMessage, this);
    wv.view.RemoveFullCallback(instance, "policy,navigation,decide",
                               &WvWebViewBackend::OnNavigationPolicy, this);
    wv.view.RemoveFullCallback(instance, "policy,response,decide",
                               &WvWebViewBackend::OnResponsePolicy, this);
    wv.view.RemoveFullCallback(instance, "url,changed",
                               &WvWebViewBackend::OnUrlChange, this);

    wv.view.OnJavaScriptAlert(instance, nullptr, nullptr);
    wv.view.OnJavaScriptConfirm(instance, nullptr, nullptr);
    wv.view.OnJavaScriptPrompt(instance, nullptr, nullptr);

    {
      std::lock_guard<std::mutex> lock(g_view_registry_mutex);
      g_view_registry.erase(instance);
    }

    wv.view.Stop(instance);
    wv.view.Suspend(instance);

    destroy = [instance]() {
      WvInternalApiBinding::GetInstance().view.Destroy(instance);
    };
  }

  return RegisterPendingTeardown(std::move(pool), std::move(destroy));
}

void WvWebViewBackend::Offset(double left, double top) {}

void WvWebViewBackend::Resize(double width, double height) {
  WvInternalApiBinding::GetInstance().view.Resize(
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
  point.x = static_cast<int>(x);
  point.y = static_cast<int>(y);
  point.state = state;

  GList* points = g_list_append(nullptr, &point);
  WvInternalApiBinding::GetInstance().view.FeedTouchEvent(
      view_, touch_event_type, points, WV_MODIFIER_NONE);
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

  auto& wv = WvInternalApiBinding::GetInstance();
  if (event_type == 0) {
    mouse_button_type_ = mouse_button_type;
    wv.view.FeedMouseDown(view_, mouse_button_type_, px, py);
  } else if (event_type == 1) {
    if (dy != 0) {
      wv.view.FeedMouseWheel(view_, true, dy > 0 ? 1 : -1, px, py);
    }
  } else if (event_type == 2) {
    wv.view.FeedMouseUp(view_, mouse_button_type_, px, py);
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

  auto& wv = WvInternalApiBinding::GetInstance();
  if (strcmp(key, "XF86Back") == 0 && !is_down) {
    if (wv.view.BackPossible(view_)) {
      wv.view.Back(view_);
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
  wv.view.SendKeyEvent(view_, &key_event, is_down ? 1 : 0);
  return true;
}

void WvWebViewBackend::Resume() {
  if (view_) {
    WvInternalApiBinding::GetInstance().view.Resume(view_);
  }
}

void WvWebViewBackend::Stop() {
  if (view_) {
    WvInternalApiBinding::GetInstance().view.Stop(view_);
  }
}

void WvWebViewBackend::SetJavaScriptEnabled(bool enabled) {
  auto& wv = WvInternalApiBinding::GetInstance();
  wv.settings.JavaScriptEnabledSet(wv.view.SettingsGet(view_), enabled);
}

void WvWebViewBackend::SetHasNavigationDelegate(bool has_navigation_delegate) {
  has_navigation_delegate_ = has_navigation_delegate;
}

void WvWebViewBackend::LoadUrl(const std::string& url) {
  WvInternalApiBinding::GetInstance().view.UrlSet(view_, url.c_str());
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

  bool ret = WvInternalApiBinding::GetInstance().view.UrlRequestSet(
      view_, url.c_str(), wv_method, wv_headers,
      reinterpret_cast<const char*>(body.data()));
  g_hash_table_destroy(wv_headers);
  return ret;
}

void WvWebViewBackend::LoadHtmlString(const std::string& html,
                                      const std::string& base_url) {
  WvInternalApiBinding::GetInstance().view.HtmlStringLoad(
      view_, html.c_str(), base_url.c_str(), nullptr);
}

bool WvWebViewBackend::CanGoBack() {
  return WvInternalApiBinding::GetInstance().view.BackPossible(view_);
}

bool WvWebViewBackend::CanGoForward() {
  return WvInternalApiBinding::GetInstance().view.ForwardPossible(view_);
}

void WvWebViewBackend::GoBack() {
  WvInternalApiBinding::GetInstance().view.Back(view_);
}

void WvWebViewBackend::GoForward() {
  WvInternalApiBinding::GetInstance().view.Forward(view_);
}

void WvWebViewBackend::Reload() {
  WvInternalApiBinding::GetInstance().view.Reload(view_);
}

std::string WvWebViewBackend::GetCurrentUrl() {
  const char* url = WvInternalApiBinding::GetInstance().view.UrlGet(view_);
  return url ? url : "";
}

void WvWebViewBackend::EvaluateJavaScript(
    const std::string& javascript,
    std::function<void(bool success, const char* result_value)> callback) {
  auto* callback_ptr =
      new std::function<void(bool, const char*)>(std::move(callback));
  if (!WvInternalApiBinding::GetInstance().view.ScriptExecute(
          view_, javascript.c_str(), &WvWebViewBackend::OnEvaluateJavaScript,
          callback_ptr)) {
    LOG_WARN("wv_view_script_execute failed.");
    (*callback_ptr)(false, nullptr);
    delete callback_ptr;
  }
}

void WvWebViewBackend::RegisterJavaScriptChannel(const std::string& name) {
  auto& wv = WvInternalApiBinding::GetInstance();
  wv.view.JavaScriptMessageHandlerAdd(
      view_, &WvWebViewBackend::OnJavaScriptMessage, name.c_str());
}

void WvWebViewBackend::ClearCache() {
  auto& wv = WvInternalApiBinding::GetInstance();
  wv.context.CacheClear(wv.view.ContextGet(view_));
}

void WvWebViewBackend::ClearLocalStorage() {
  auto& wv = WvInternalApiBinding::GetInstance();
  wv.context.WebStorageDeleteAll(wv.view.ContextGet(view_));
}

std::string WvWebViewBackend::GetTitle() {
  const char* title = WvInternalApiBinding::GetInstance().view.TitleGet(view_);
  return title ? title : "";
}

void WvWebViewBackend::ScrollTo(int32_t x, int32_t y) {
  WvInternalApiBinding::GetInstance().view.ScrollSet(view_, x, y);
}

void WvWebViewBackend::ScrollBy(int32_t x, int32_t y) {
  WvInternalApiBinding::GetInstance().view.ScrollBy(view_, x, y);
}

void WvWebViewBackend::GetScrollPosition(int32_t* x, int32_t* y) {
  WvInternalApiBinding::GetInstance().view.ScrollPosGet(view_, x, y);
}

void WvWebViewBackend::SetBackgroundColor(int r, int g, int b, int a) {
  WvInternalApiBinding::GetInstance().view.BgColorSet(view_, r, g, b, a);
}

void WvWebViewBackend::SetUserAgent(const std::string& user_agent) {
  WvInternalApiBinding::GetInstance().view.UserAgentSet(view_,
                                                        user_agent.c_str());
}

std::string WvWebViewBackend::GetUserAgent() {
  const char* user_agent =
      WvInternalApiBinding::GetInstance().view.UserAgentGet(view_);
  return user_agent ? user_agent : "";
}

void WvWebViewBackend::EnableZoom(bool enabled) {
  auto& wv = WvInternalApiBinding::GetInstance();
  wv.settings.ForceZoomSet(wv.view.SettingsGet(view_), enabled);
}

void WvWebViewBackend::JavaScriptAlertReply() {
  WvInternalApiBinding::GetInstance().view.JavaScriptAlertReply(view_);
}

void WvWebViewBackend::JavaScriptConfirmReply(bool result) {
  WvInternalApiBinding::GetInstance().view.JavaScriptConfirmReply(view_,
                                                                  result);
}

void WvWebViewBackend::JavaScriptPromptReply(const std::string& result) {
  WvInternalApiBinding::GetInstance().view.JavaScriptPromptReply(
      view_, result.c_str());
}

void WvWebViewBackend::SetScrollbarVisible(bool visible) {
  scrollbar_enabled_ = visible;
  WvInternalApiBinding::GetInstance().view.MainFrameScrollbarVisibleSet(
      view_, scrollbar_enabled_);
}

bool WvWebViewBackend::ClearCookies() {
  if (!view_) {
    return false;
  }
  auto& wv = WvInternalApiBinding::GetInstance();
  wv_cookie_manager_h cookie_manager =
      wv.context.CookieManagerGet(wv.view.ContextGet(view_));
  if (cookie_manager) {
    wv.cookie_manager.CookiesClear(cookie_manager);
    return true;
  }
  return false;
}

void WvWebViewBackend::OnFrameRendered(wv_view_h obj, void* event_info,
                                       void* user_data) {
  if (event_info) {
    static_cast<WvWebViewBackend*>(user_data)->delegate_->OnFrameRendered(
        event_info);
  }
}

void WvWebViewBackend::OnLoadStarted(wv_view_h obj, void* event_info,
                                     void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  auto& wv = WvInternalApiBinding::GetInstance();
  wv.view.MainFrameScrollbarVisibleSet(backend->view_,
                                       backend->scrollbar_enabled_);
  const char* url = wv.view.UrlGet(backend->view_);
  backend->delegate_->OnLoadStarted(url ? url : "");
}

void WvWebViewBackend::OnLoadFinished(wv_view_h obj, void* event_info,
                                      void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  const char* url =
      WvInternalApiBinding::GetInstance().view.UrlGet(backend->view_);
  backend->delegate_->OnLoadFinished(url ? url : "");
}

void WvWebViewBackend::OnProgress(wv_view_h obj, void* event_info,
                                  void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  int32_t progress =
      static_cast<int32_t>((*static_cast<double*>(event_info)) * 100);
  backend->delegate_->OnProgress(progress);
}

void WvWebViewBackend::OnLoadError(wv_view_h obj, void* event_info,
                                   void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  wv_error_h error = static_cast<wv_error_h>(event_info);
  auto& wv = WvInternalApiBinding::GetInstance();
  const char* description = wv.error.DescriptionGet(error);
  const char* url = wv.error.UrlGet(error);
  backend->delegate_->OnLoadError(
      wv.error.CodeGet(error), description ? description : "", url ? url : "");
}

void WvWebViewBackend::OnConsoleMessage(wv_view_h obj, void* event_info,
                                        void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  wv_console_message_h message = static_cast<wv_console_message_h>(event_info);
  auto& wv = WvInternalApiBinding::GetInstance();
  wv_console_message_level_e log_level = wv.console_message.LevelGet(message);
  const char* text = wv.console_message.TextGet(message);
  backend->delegate_->OnConsoleMessage(ConvertLogLevelToString(log_level),
                                       text ? text : "");
}

void WvWebViewBackend::OnNavigationPolicy(wv_view_h obj, void* event_info,
                                          void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  wv_policy_decision_h policy_decision =
      static_cast<wv_policy_decision_h>(event_info);
  auto& wv = WvInternalApiBinding::GetInstance();
  wv.policy_decision.Use(policy_decision);

  if (!backend->has_navigation_delegate_) {
    return;
  }
  wv.view.Suspend(backend->view_);
  const char* url = wv.policy_decision.UrlGet(policy_decision);
  backend->delegate_->OnNavigationPolicyDecide(url ? url : "");
}

void WvWebViewBackend::OnResponsePolicy(wv_view_h obj, void* event_info,
                                        void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  wv_policy_decision_h policy_decision =
      static_cast<wv_policy_decision_h>(event_info);
  auto& wv = WvInternalApiBinding::GetInstance();
  int status_code = wv.policy_decision.ResponseStatusCodeGet(policy_decision);
  const char* url = wv.policy_decision.UrlGet(policy_decision);
  wv.policy_decision.Use(policy_decision);

  if (!backend->has_navigation_delegate_ || status_code < 400) {
    return;
  }
  backend->delegate_->OnResponsePolicyDecide(url ? url : "", status_code);
}

void WvWebViewBackend::OnUrlChange(wv_view_h obj, void* event_info,
                                   void* user_data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(user_data);
  const char* url =
      WvInternalApiBinding::GetInstance().view.UrlGet(backend->view_);
  backend->delegate_->OnUrlChanged(url ? url : "");
}

void WvWebViewBackend::OnEvaluateJavaScript(wv_view_h obj,
                                            const char* result_value,
                                            void* user_data) {
  auto* callback =
      static_cast<std::function<void(bool, const char*)>*>(user_data);
  (*callback)(true, result_value);
  delete callback;
}

void WvWebViewBackend::OnJavaScriptMessage(wv_view_h obj,
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

bool WvWebViewBackend::OnJavaScriptAlertDialog(wv_view_h view,
                                               const char* message,
                                               void* data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(data);
  const char* url =
      WvInternalApiBinding::GetInstance().view.UrlGet(backend->view_);
  backend->delegate_->OnJavaScriptAlertDialog(message ? message : "",
                                              url ? url : "");
  return true;
}

bool WvWebViewBackend::OnJavaScriptConfirmDialog(wv_view_h view,
                                                 const char* message,
                                                 void* data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(data);
  const char* url =
      WvInternalApiBinding::GetInstance().view.UrlGet(backend->view_);
  backend->delegate_->OnJavaScriptConfirmDialog(message ? message : "",
                                                url ? url : "");
  return true;
}

bool WvWebViewBackend::OnJavaScriptPromptDialog(wv_view_h view,
                                                const char* message,
                                                const char* default_text,
                                                void* data) {
  WvWebViewBackend* backend = static_cast<WvWebViewBackend*>(data);
  const char* url =
      WvInternalApiBinding::GetInstance().view.UrlGet(backend->view_);
  backend->delegate_->OnJavaScriptPromptDialog(
      message ? message : "", default_text ? default_text : "", url ? url : "");
  return true;
}
