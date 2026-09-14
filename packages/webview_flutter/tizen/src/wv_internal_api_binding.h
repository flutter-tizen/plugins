// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WV_INTERNAL_API_BINDING_H_
#define FLUTTER_PLUGIN_WV_INTERNAL_API_BINDING_H_

#include <glib.h>

#include <optional>

typedef struct wv_view_s* wv_view_h;
typedef struct wv_context_s* wv_context_h;
typedef struct wv_settings_s* wv_settings_h;
typedef struct wv_cookie_manager_s* wv_cookie_manager_h;
typedef struct wv_error_s* wv_error_h;
typedef struct wv_policy_decision_s* wv_policy_decision_h;
typedef struct wv_console_message_s* wv_console_message_h;

typedef enum {
  WV_HTTP_METHOD_GET,
  WV_HTTP_METHOD_HEAD,
  WV_HTTP_METHOD_POST,
  WV_HTTP_METHOD_PUT,
  WV_HTTP_METHOD_DELETE,
} wv_http_method_e;

typedef enum {
  WV_CACHE_MODEL_DOCUMENT_VIEWER,
  WV_CACHE_MODEL_DOCUMENT_BROWSER,
  WV_CACHE_MODEL_PRIMARY_WEBBROWSER
} wv_cache_model_e;

typedef enum {
  WV_TOUCH_EVENT_START,
  WV_TOUCH_EVENT_MOVE,
  WV_TOUCH_EVENT_END,
  WV_TOUCH_EVENT_CANCEL
} wv_touch_event_type_e;

typedef enum {
  WV_TOUCH_POINT_STATE_DOWN,
  WV_TOUCH_POINT_STATE_UP,
  WV_TOUCH_POINT_STATE_MOVE,
  WV_TOUCH_POINT_STATE_STILL,
  WV_TOUCH_POINT_STATE_CANCEL
} wv_touch_point_state_e;

typedef enum {
  WV_MODIFIER_NONE = 0,
  WV_MODIFIER_SHIFT = 1 << 0,
  WV_MODIFIER_CONTROL = 1 << 1,
  WV_MODIFIER_ALT = 1 << 2,
  WV_MODIFIER_SUPER = 1 << 3,
  WV_MODIFIER_HYPER = 1 << 4,
  WV_MODIFIER_CAPS_LOCK = 1 << 5,
  WV_MODIFIER_NUM_LOCK = 1 << 6
} wv_modifier_e;

typedef enum {
  WV_MOUSE_BUTTON_LEFT = 1,
  WV_MOUSE_BUTTON_MIDDLE = 2,
  WV_MOUSE_BUTTON_RIGHT = 3
} wv_mouse_button_type_e;

typedef enum {
  WV_COOKIE_ACCEPT_POLICY_ALWAYS,
  WV_COOKIE_ACCEPT_POLICY_NEVER,
  WV_COOKIE_ACCEPT_POLICY_NO_THIRD_PARTY
} wv_cookie_accept_policy_e;

typedef enum {
  WV_CONSOLE_MESSAGE_LEVEL_NULL,
  WV_CONSOLE_MESSAGE_LEVEL_LOG,
  WV_CONSOLE_MESSAGE_LEVEL_WARNING,
  WV_CONSOLE_MESSAGE_LEVEL_ERROR,
  WV_CONSOLE_MESSAGE_LEVEL_DEBUG,
  WV_CONSOLE_MESSAGE_LEVEL_INFO
} wv_console_message_level_e;

typedef struct {
  int id;
  int x;
  int y;
  wv_touch_point_state_e state;
} wv_touch_point_s;

typedef struct {
  const char* key_name;
  const char* key;
  const char* string;
  const char* compose;
  unsigned int timestamp;
  wv_modifier_e modifiers;
  int event_flags;
  unsigned int key_code;
} wv_key_event_s;

typedef struct {
  const char* name;
  void* body;
} wv_script_message_s;

typedef void (*wv_view_cb)(wv_view_h obj, void* event_info, void* user_data);
typedef void (*wv_view_script_execute_cb)(wv_view_h view,
                                          const char* result_value,
                                          void* user_data);
typedef void (*wv_view_script_message_cb)(wv_view_h view,
                                          wv_script_message_s message);
typedef bool (*wv_view_javascript_alert_cb)(wv_view_h view,
                                            const char* alert_text,
                                            void* user_data);
typedef bool (*wv_view_javascript_confirm_cb)(wv_view_h view,
                                              const char* message,
                                              void* user_data);
typedef bool (*wv_view_javascript_prompt_cb)(wv_view_h view,
                                             const char* message,
                                             const char* default_value,
                                             void* user_data);

typedef int (*WvInitFnPtr)(void);
typedef int (*WvShutdownFnPtr)(void);
typedef int (*WvSetArgumentsFnPtr)(int argc, const char** argv);

typedef struct {
  WvInitFnPtr Init = nullptr;
  WvShutdownFnPtr Shutdown = nullptr;
  WvSetArgumentsFnPtr SetArguments = nullptr;
} WvMainProcTable;

typedef wv_view_h (*WvViewCreateFnPtr)(void);
typedef void (*WvViewDestroyFnPtr)(wv_view_h view);
typedef void (*WvViewResizeFnPtr)(wv_view_h view, int w, int h);
typedef bool (*WvViewFocusSetFnPtr)(wv_view_h view, int focused);
typedef bool (*WvViewUrlSetFnPtr)(wv_view_h view, const char* url);
typedef const char* (*WvViewUrlGetFnPtr)(wv_view_h view);
typedef bool (*WvViewUrlRequestSetFnPtr)(wv_view_h view, const char* url,
                                         wv_http_method_e method,
                                         GHashTable* headers, const char* body);
typedef bool (*WvViewHtmlStringLoadFnPtr)(wv_view_h view, const char* html,
                                          const char* base_url,
                                          const char* unreachable_url);
typedef bool (*WvViewBackPossibleFnPtr)(wv_view_h view);
typedef bool (*WvViewForwardPossibleFnPtr)(wv_view_h view);
typedef bool (*WvViewBackFnPtr)(wv_view_h view);
typedef bool (*WvViewForwardFnPtr)(wv_view_h view);
typedef bool (*WvViewReloadFnPtr)(wv_view_h view);
typedef bool (*WvViewStopFnPtr)(wv_view_h view);
typedef void (*WvViewSuspendFnPtr)(wv_view_h view);
typedef void (*WvViewResumeFnPtr)(wv_view_h view);
typedef const char* (*WvViewTitleGetFnPtr)(wv_view_h view);
typedef bool (*WvViewUserAgentSetFnPtr)(wv_view_h view, const char* user_agent);
typedef const char* (*WvViewUserAgentGetFnPtr)(wv_view_h view);
typedef bool (*WvViewScrollSetFnPtr)(wv_view_h view, int x, int y);
typedef void (*WvViewScrollByFnPtr)(wv_view_h view, int dx, int dy);
typedef bool (*WvViewScrollPosGetFnPtr)(wv_view_h view, int* x, int* y);
typedef bool (*WvViewScriptExecuteFnPtr)(wv_view_h view, const char* script,
                                         wv_view_script_execute_cb callback,
                                         void* user_data);
typedef bool (*WvViewJavaScriptMessageHandlerAddFnPtr)(
    wv_view_h view, wv_view_script_message_cb callback, const char* name);
typedef bool (*WvViewBgColorSetFnPtr)(wv_view_h view, int r, int g, int b,
                                      int a);
typedef wv_context_h (*WvViewContextGetFnPtr)(wv_view_h view);
typedef wv_settings_h (*WvViewSettingsGetFnPtr)(wv_view_h view);
typedef int (*WvViewFeedTouchEventFnPtr)(wv_view_h view,
                                         wv_touch_event_type_e event_type,
                                         GList* points,
                                         wv_modifier_e modifiers);
typedef int (*WvViewFeedMouseDownFnPtr)(wv_view_h view,
                                        wv_mouse_button_type_e button, int x,
                                        int y);
typedef int (*WvViewFeedMouseUpFnPtr)(wv_view_h view,
                                      wv_mouse_button_type_e button, int x,
                                      int y);
typedef int (*WvViewFeedMouseMoveFnPtr)(wv_view_h view, int x, int y);
typedef int (*WvViewFeedMouseWheelFnPtr)(wv_view_h view, bool y_direction,
                                         int step, int x, int y);
typedef int (*WvViewSendKeyEventFnPtr)(wv_view_h view,
                                       const wv_key_event_s* key_event,
                                       int is_press);
typedef int (*WvViewTouchEventsEnabledSetFnPtr)(wv_view_h view, int enabled);
typedef bool (*WvViewMouseEventsEnabledSetFnPtr)(wv_view_h view, bool enabled);
typedef bool (*WvViewKeyEventsEnabledSetFnPtr)(wv_view_h view, bool enabled);
typedef void (*WvViewImeWindowSetFnPtr)(wv_view_h view, void* window);
typedef int (*WvViewAddCallbackFnPtr)(wv_view_h view, const char* event,
                                      wv_view_cb func, const void* user_data);
typedef int (*WvViewRemoveFullCallbackFnPtr)(wv_view_h view, const char* event,
                                             wv_view_cb func,
                                             const void* user_data);
typedef int (*WvViewJavaScriptAlertCallbackSetFnPtr)(
    wv_view_h view, wv_view_javascript_alert_cb callback, void* user_data);
typedef int (*WvViewJavaScriptConfirmCallbackSetFnPtr)(
    wv_view_h view, wv_view_javascript_confirm_cb callback, void* user_data);
typedef int (*WvViewJavaScriptPromptCallbackSetFnPtr)(
    wv_view_h view, wv_view_javascript_prompt_cb callback, void* user_data);
typedef void (*WvViewJavaScriptAlertReplyFnPtr)(wv_view_h view);
typedef void (*WvViewJavaScriptConfirmReplyFnPtr)(wv_view_h view, bool result);
typedef void (*WvViewJavaScriptPromptReplyFnPtr)(wv_view_h view,
                                                 const char* result);
typedef bool (*WvViewSetSupportVideoHoleFnPtr)(wv_view_h view, void* window,
                                               bool enable,
                                               bool is_video_window);
typedef bool (*WvViewMainFrameScrollbarVisibleSetFnPtr)(wv_view_h view,
                                                        bool visible);

typedef struct {
  WvViewCreateFnPtr Create = nullptr;
  WvViewDestroyFnPtr Destroy = nullptr;
  WvViewResizeFnPtr Resize = nullptr;
  WvViewFocusSetFnPtr FocusSet = nullptr;
  WvViewUrlSetFnPtr UrlSet = nullptr;
  WvViewUrlGetFnPtr UrlGet = nullptr;
  WvViewUrlRequestSetFnPtr UrlRequestSet = nullptr;
  WvViewHtmlStringLoadFnPtr HtmlStringLoad = nullptr;
  WvViewBackPossibleFnPtr BackPossible = nullptr;
  WvViewForwardPossibleFnPtr ForwardPossible = nullptr;
  WvViewBackFnPtr Back = nullptr;
  WvViewForwardFnPtr Forward = nullptr;
  WvViewReloadFnPtr Reload = nullptr;
  WvViewStopFnPtr Stop = nullptr;
  WvViewSuspendFnPtr Suspend = nullptr;
  WvViewResumeFnPtr Resume = nullptr;
  WvViewTitleGetFnPtr TitleGet = nullptr;
  WvViewUserAgentSetFnPtr UserAgentSet = nullptr;
  WvViewUserAgentGetFnPtr UserAgentGet = nullptr;
  WvViewScrollSetFnPtr ScrollSet = nullptr;
  WvViewScrollByFnPtr ScrollBy = nullptr;
  WvViewScrollPosGetFnPtr ScrollPosGet = nullptr;
  WvViewScriptExecuteFnPtr ScriptExecute = nullptr;
  WvViewBgColorSetFnPtr BgColorSet = nullptr;
  WvViewContextGetFnPtr ContextGet = nullptr;
  WvViewSettingsGetFnPtr SettingsGet = nullptr;
  WvViewFeedTouchEventFnPtr FeedTouchEvent = nullptr;
  WvViewFeedMouseDownFnPtr FeedMouseDown = nullptr;
  WvViewFeedMouseUpFnPtr FeedMouseUp = nullptr;
  WvViewFeedMouseMoveFnPtr FeedMouseMove = nullptr;
  WvViewFeedMouseWheelFnPtr FeedMouseWheel = nullptr;
  WvViewSendKeyEventFnPtr SendKeyEvent = nullptr;
  WvViewTouchEventsEnabledSetFnPtr TouchEventsEnabledSet = nullptr;
  WvViewMouseEventsEnabledSetFnPtr MouseEventsEnabledSet = nullptr;
  WvViewKeyEventsEnabledSetFnPtr KeyEventsEnabledSet = nullptr;
  WvViewImeWindowSetFnPtr ImeWindowSet = nullptr;
  WvViewAddCallbackFnPtr AddCallback = nullptr;
  WvViewRemoveFullCallbackFnPtr RemoveFullCallback = nullptr;
  WvViewJavaScriptAlertCallbackSetFnPtr OnJavaScriptAlert = nullptr;
  WvViewJavaScriptConfirmCallbackSetFnPtr OnJavaScriptConfirm = nullptr;
  WvViewJavaScriptPromptCallbackSetFnPtr OnJavaScriptPrompt = nullptr;
  WvViewJavaScriptAlertReplyFnPtr JavaScriptAlertReply = nullptr;
  WvViewJavaScriptConfirmReplyFnPtr JavaScriptConfirmReply = nullptr;
  WvViewJavaScriptPromptReplyFnPtr JavaScriptPromptReply = nullptr;
  WvViewJavaScriptMessageHandlerAddFnPtr JavaScriptMessageHandlerAdd = nullptr;
  WvViewSetSupportVideoHoleFnPtr SetSupportVideoHole = nullptr;
  WvViewMainFrameScrollbarVisibleSetFnPtr MainFrameScrollbarVisibleSet =
      nullptr;
} WvViewProcTable;

typedef wv_cookie_manager_h (*WvContextCookieManagerGetFnPtr)(
    wv_context_h context);
typedef int (*WvContextCacheModelSetFnPtr)(wv_context_h context,
                                           wv_cache_model_e model);
typedef int (*WvContextWebStorageDeleteAllFnPtr)(wv_context_h context);
typedef int (*WvContextCacheClearFnPtr)(wv_context_h context);

typedef struct {
  WvContextCookieManagerGetFnPtr CookieManagerGet = nullptr;
  WvContextCacheModelSetFnPtr CacheModelSet = nullptr;
  WvContextWebStorageDeleteAllFnPtr WebStorageDeleteAll = nullptr;
  WvContextCacheClearFnPtr CacheClear = nullptr;
} WvContextProcTable;

typedef int (*WvCookieManagerAcceptPolicySetFnPtr)(
    wv_cookie_manager_h manager, wv_cookie_accept_policy_e policy);
typedef int (*WvCookieManagerCookiesClearFnPtr)(wv_cookie_manager_h manager);

typedef struct {
  WvCookieManagerAcceptPolicySetFnPtr AcceptPolicySet = nullptr;
  WvCookieManagerCookiesClearFnPtr CookiesClear = nullptr;
} WvCookieManagerProcTable;

typedef bool (*WvSettingsJavaScriptEnabledSetFnPtr)(wv_settings_h settings,
                                                    bool enable);
typedef void (*WvSettingsImePanelEnabledSetFnPtr)(wv_settings_h settings,
                                                  bool enable);
typedef bool (*WvSettingsForceZoomSetFnPtr)(wv_settings_h settings,
                                            bool enable);

typedef struct {
  WvSettingsJavaScriptEnabledSetFnPtr JavaScriptEnabledSet = nullptr;
  WvSettingsImePanelEnabledSetFnPtr ImePanelEnabledSet = nullptr;
  WvSettingsForceZoomSetFnPtr ForceZoomSet = nullptr;
} WvSettingsProcTable;

typedef int (*WvErrorCodeGetFnPtr)(wv_error_h error);
typedef const char* (*WvErrorDescriptionGetFnPtr)(wv_error_h error);
typedef const char* (*WvErrorUrlGetFnPtr)(wv_error_h error);

typedef struct {
  WvErrorCodeGetFnPtr CodeGet = nullptr;
  WvErrorDescriptionGetFnPtr DescriptionGet = nullptr;
  WvErrorUrlGetFnPtr UrlGet = nullptr;
} WvErrorProcTable;

typedef int (*WvPolicyDecisionUseFnPtr)(wv_policy_decision_h policy_decision);
typedef const char* (*WvPolicyDecisionUrlGetFnPtr)(
    wv_policy_decision_h policy_decision);
typedef int (*WvPolicyDecisionResponseStatusCodeGetFnPtr)(
    wv_policy_decision_h policy_decision);

typedef struct {
  WvPolicyDecisionUseFnPtr Use = nullptr;
  WvPolicyDecisionUrlGetFnPtr UrlGet = nullptr;
  WvPolicyDecisionResponseStatusCodeGetFnPtr ResponseStatusCodeGet = nullptr;
} WvPolicyDecisionProcTable;

typedef wv_console_message_level_e (*WvConsoleMessageLevelGetFnPtr)(
    wv_console_message_h message);
typedef const char* (*WvConsoleMessageTextGetFnPtr)(
    wv_console_message_h message);

typedef struct {
  WvConsoleMessageLevelGetFnPtr LevelGet = nullptr;
  WvConsoleMessageTextGetFnPtr TextGet = nullptr;
} WvConsoleMessageProcTable;

class WvInternalApiBinding {
 public:
  static WvInternalApiBinding& GetInstance() {
    static WvInternalApiBinding instance = WvInternalApiBinding();
    return instance;
  }

  ~WvInternalApiBinding();

  WvInternalApiBinding(const WvInternalApiBinding&) = delete;
  WvInternalApiBinding& operator=(const WvInternalApiBinding&) = delete;

  bool Initialize();

  WvMainProcTable main;
  WvViewProcTable view;
  WvContextProcTable context;
  WvCookieManagerProcTable cookie_manager;
  WvSettingsProcTable settings;
  WvErrorProcTable error;
  WvPolicyDecisionProcTable policy_decision;
  WvConsoleMessageProcTable console_message;

 private:
  WvInternalApiBinding();

  void* handle_ = nullptr;
  std::optional<bool> initialize_result_;
};

#endif  // FLUTTER_PLUGIN_WV_INTERNAL_API_BINDING_H_
