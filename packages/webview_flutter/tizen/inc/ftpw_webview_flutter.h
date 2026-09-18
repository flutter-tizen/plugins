// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef FLUTTER_TIZEN_PLUGINS_WRAPPER_WEBVIEW_FLUTTER_H_
#define FLUTTER_TIZEN_PLUGINS_WRAPPER_WEBVIEW_FLUTTER_H_

#include <stdbool.h>

typedef enum {
  EWK_TOUCH_START,
  EWK_TOUCH_MOVE,
  EWK_TOUCH_END,
} Ewk_Touch_Event_Type;

typedef struct _Ewk_Touch_Point Ewk_Touch_Point;

typedef enum {
  EWK_MOUSE_BUTTON_LEFT = 1,
  EWK_MOUSE_BUTTON_MIDDLE = 2,
  EWK_MOUSE_BUTTON_RIGHT = 3
} Ewk_Mouse_Button_Type;

struct _Ewk_Touch_Point {
  int id;
  int x;
  int y;
  int state;
};

typedef unsigned char (*Ewk_View_JavaScript_Alert_Callback)(
    void *o, const char *alert_text, void *user_data);
typedef unsigned char (*Ewk_View_JavaScript_Confirm_Callback)(
    void *o, const char *message, void *user_data);
typedef unsigned char (*Ewk_View_JavaScript_Prompt_Callback)(
    void *o, const char *message, const char *default_value, void *user_data);

typedef enum {
  EWK_CONSOLE_MESSAGE_LEVEL_NULL,
  EWK_CONSOLE_MESSAGE_LEVEL_LOG,
  EWK_CONSOLE_MESSAGE_LEVEL_WARNING,
  EWK_CONSOLE_MESSAGE_LEVEL_ERROR,
  EWK_CONSOLE_MESSAGE_LEVEL_DEBUG,
  EWK_CONSOLE_MESSAGE_LEVEL_INFO,
} Ewk_Console_Message_Level;

typedef enum {
  WV_HTTP_METHOD_GET,
  WV_HTTP_METHOD_POST = 2,
} wv_http_method_e;

typedef enum {
  WV_TOUCH_EVENT_START,
  WV_TOUCH_EVENT_MOVE,
  WV_TOUCH_EVENT_END,
} wv_touch_event_type_e;

typedef enum {
  WV_TOUCH_POINT_STATE_DOWN,
  WV_TOUCH_POINT_STATE_UP,
  WV_TOUCH_POINT_STATE_MOVE,
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
  const char *key_name;
  const char *key;
  const char *string;
  const char *compose;
  unsigned int timestamp;
  wv_modifier_e modifiers;
  int event_flags;
  unsigned int key_code;
} wv_key_event_s;

typedef struct {
  const char *name;
  void *body;
} wv_script_message_s;

typedef void (*wv_view_cb)(void *obj, void *event_info, void *user_data);
typedef void (*wv_view_script_execute_cb)(void *view, const char *result_value,
                                          void *user_data);
typedef void (*wv_view_script_message_cb)(void *view,
                                          wv_script_message_s message);
typedef bool (*wv_view_javascript_alert_cb)(void *view, const char *alert_text,
                                            void *user_data);
typedef bool (*wv_view_javascript_confirm_cb)(void *view, const char *message,
                                              void *user_data);
typedef bool (*wv_view_javascript_prompt_cb)(void *view, const char *message,
                                             const char *default_value,
                                             void *user_data);

#ifdef __cplusplus
extern "C" {
#endif

unsigned char ftpw_webview_flutter_ewk_view_bg_color_set(void *obj, int r,
                                                         int g, int b, int a);
unsigned char ftpw_webview_flutter_ewk_view_touch_events_enabled_set(
    void *view, unsigned char enabled);
unsigned char ftpw_webview_flutter_ewk_view_feed_touch_event(
    void *obj, Ewk_Touch_Event_Type type, const void *points,
    const void *modifiers);
unsigned char ftpw_webview_flutter_ewk_view_mouse_events_enabled_set(
    void *view, unsigned char enabled);
unsigned char ftpw_webview_flutter_ewk_view_feed_mouse_down(
    void *obj, Ewk_Mouse_Button_Type button, int x, int y);
unsigned char ftpw_webview_flutter_ewk_view_feed_mouse_up(
    void *obj, Ewk_Mouse_Button_Type button, int x, int y);
unsigned char ftpw_webview_flutter_ewk_view_feed_mouse_wheel(
    void *obj, unsigned char y_direction, int step, int x, int y);
unsigned char ftpw_webview_flutter_ewk_view_send_key_event(
    void *obj, void *key_event, unsigned char is_press);
void ftpw_webview_flutter_ewk_view_offscreen_rendering_enabled_set(
    void *obj, unsigned char enabled);
void ftpw_webview_flutter_ewk_view_ime_window_set(void *obj, void *window);
unsigned char ftpw_webview_flutter_ewk_view_key_events_enabled_set(
    void *obj, unsigned char enabled);
unsigned char ftpw_webview_flutter_ewk_view_set_support_video_hole(
    void *obj, void *window, unsigned char enabled, unsigned char boo);
void ftpw_webview_flutter_ewk_view_javascript_alert_callback_set(
    void *o, Ewk_View_JavaScript_Alert_Callback callback, void *user_data);
void ftpw_webview_flutter_ewk_view_javascript_confirm_callback_set(
    void *o, Ewk_View_JavaScript_Confirm_Callback callback, void *user_data);
void ftpw_webview_flutter_ewk_view_javascript_prompt_callback_set(
    void *o, Ewk_View_JavaScript_Prompt_Callback callback, void *user_data);
void ftpw_webview_flutter_ewk_view_javascript_alert_reply(void *o);
void ftpw_webview_flutter_ewk_view_javascript_confirm_reply(
    void *o, unsigned char result);
void ftpw_webview_flutter_ewk_view_javascript_prompt_reply(void *o,
                                                           const char *result);
unsigned char ftpw_webview_flutter_ewk_view_main_frame_scrollbar_visible_set(
    void *obj, unsigned char enabled);
void ftpw_webview_flutter_ewk_set_arguments(int argc, char **argv);
int ftpw_webview_flutter_ewk_set_version_policy(int preference);
void ftpw_webview_flutter_ewk_settings_ime_panel_enabled_set(
    void *settings, unsigned char enabled);
void ftpw_webview_flutter_ewk_settings_force_zoom_set(void *settings,
                                                      unsigned char enable);
Ewk_Console_Message_Level ftpw_webview_flutter_ewk_console_message_level_get(
    const void *message);
const char *ftpw_webview_flutter_ewk_console_message_text_get(
    const void *message);
int ftpw_webview_flutter_wv_init(void);
int ftpw_webview_flutter_wv_shutdown(void);
int ftpw_webview_flutter_wv_set_arguments(int argc, const char **argv);
void *ftpw_webview_flutter_wv_view_create(void);
void ftpw_webview_flutter_wv_view_destroy(void *view);
void ftpw_webview_flutter_wv_view_resize(void *view, int w, int h);
bool ftpw_webview_flutter_wv_view_focus_set(void *view, int focused);
bool ftpw_webview_flutter_wv_view_url_set(void *view, const char *url);
const char *ftpw_webview_flutter_wv_view_url_get(void *view);
bool ftpw_webview_flutter_wv_view_url_request_set(void *view, const char *url,
                                                  wv_http_method_e method,
                                                  void *headers,
                                                  const char *body);
bool ftpw_webview_flutter_wv_view_html_string_load(void *view, const char *html,
                                                   const char *base_url,
                                                   const char *unreachable_url);
bool ftpw_webview_flutter_wv_view_back_possible(void *view);
bool ftpw_webview_flutter_wv_view_forward_possible(void *view);
bool ftpw_webview_flutter_wv_view_back(void *view);
bool ftpw_webview_flutter_wv_view_forward(void *view);
bool ftpw_webview_flutter_wv_view_reload(void *view);
bool ftpw_webview_flutter_wv_view_stop(void *view);
void ftpw_webview_flutter_wv_view_suspend(void *view);
void ftpw_webview_flutter_wv_view_resume(void *view);
const char *ftpw_webview_flutter_wv_view_title_get(void *view);
bool ftpw_webview_flutter_wv_view_user_agent_set(void *view,
                                                 const char *user_agent);
const char *ftpw_webview_flutter_wv_view_user_agent_get(void *view);
bool ftpw_webview_flutter_wv_view_scroll_set(void *view, int x, int y);
void ftpw_webview_flutter_wv_view_scroll_by(void *view, int dx, int dy);
bool ftpw_webview_flutter_wv_view_scroll_pos_get(void *view, int *x, int *y);
bool ftpw_webview_flutter_wv_view_script_execute(
    void *view, const char *script, wv_view_script_execute_cb callback,
    void *user_data);
bool ftpw_webview_flutter_wv_view_bg_color_set(void *view, int r, int g, int b,
                                               int a);
void *ftpw_webview_flutter_wv_view_context_get(void *view);
void *ftpw_webview_flutter_wv_view_settings_get(void *view);
int ftpw_webview_flutter_wv_view_feed_touch_event(
    void *view, wv_touch_event_type_e event_type, void *points,
    wv_modifier_e modifiers);
int ftpw_webview_flutter_wv_view_feed_mouse_down(void *view,
                                                 wv_mouse_button_type_e button,
                                                 int x, int y);
int ftpw_webview_flutter_wv_view_feed_mouse_up(void *view,
                                               wv_mouse_button_type_e button,
                                               int x, int y);
int ftpw_webview_flutter_wv_view_feed_mouse_wheel(void *view, bool y_direction,
                                                  int step, int x, int y);
int ftpw_webview_flutter_wv_view_send_key_event(void *view,
                                                const wv_key_event_s *key_event,
                                                int is_press);
int ftpw_webview_flutter_wv_view_touch_events_enabled_set(void *view,
                                                          int enabled);
bool ftpw_webview_flutter_wv_view_mouse_events_enabled_set(void *view,
                                                           bool enabled);
bool ftpw_webview_flutter_wv_view_key_events_enabled_set(void *view,
                                                         bool enabled);
void ftpw_webview_flutter_wv_view_ime_window_set(void *view, void *window);
int ftpw_webview_flutter_wv_view_add_cb(void *view, const char *event,
                                        wv_view_cb func, const void *user_data);
int ftpw_webview_flutter_wv_view_remove_full_cb(void *view, const char *event,
                                                wv_view_cb func,
                                                const void *user_data);
int ftpw_webview_flutter_wv_view_javascript_alert_callback_set(
    void *view, wv_view_javascript_alert_cb callback, void *user_data);
int ftpw_webview_flutter_wv_view_javascript_confirm_callback_set(
    void *view, wv_view_javascript_confirm_cb callback, void *user_data);
int ftpw_webview_flutter_wv_view_javascript_prompt_callback_set(
    void *view, wv_view_javascript_prompt_cb callback, void *user_data);
void ftpw_webview_flutter_wv_view_javascript_alert_reply(void *view);
void ftpw_webview_flutter_wv_view_javascript_confirm_reply(void *view,
                                                           bool result);
void ftpw_webview_flutter_wv_view_javascript_prompt_reply(void *view,
                                                          const char *result);
bool ftpw_webview_flutter_wv_view_javascript_message_handler_add(
    void *view, wv_view_script_message_cb callback, const char *name);
bool ftpw_webview_flutter_wv_view_set_support_video_hole(void *view,
                                                         void *window,
                                                         bool enable,
                                                         bool is_video_window);
bool ftpw_webview_flutter_wv_view_main_frame_scrollbar_visible_set(
    void *view, bool visible);
void *ftpw_webview_flutter_wv_context_cookie_manager_get(void *context);
int ftpw_webview_flutter_wv_context_cache_model_set(void *context);
int ftpw_webview_flutter_wv_context_web_storage_delete_all(void *context);
int ftpw_webview_flutter_wv_context_cache_clear(void *context);
int ftpw_webview_flutter_wv_cookie_manager_accept_policy_set(void *manager);
int ftpw_webview_flutter_wv_cookie_manager_cookies_clear(void *manager);
bool ftpw_webview_flutter_wv_settings_javascript_enabled_set(void *settings,
                                                             bool enable);
void ftpw_webview_flutter_wv_settings_ime_panel_enabled_set(void *settings,
                                                            bool enable);
bool ftpw_webview_flutter_wv_settings_force_zoom_set(void *settings,
                                                     bool enable);
int ftpw_webview_flutter_wv_error_code_get(void *error);
const char *ftpw_webview_flutter_wv_error_description_get(void *error);
const char *ftpw_webview_flutter_wv_error_url_get(void *error);
int ftpw_webview_flutter_wv_policy_decision_use(void *policy_decision);
const char *ftpw_webview_flutter_wv_policy_decision_url_get(
    void *policy_decision);
int ftpw_webview_flutter_wv_policy_decision_response_status_code_get(
    void *policy_decision);
wv_console_message_level_e ftpw_webview_flutter_wv_console_message_level_get(
    void *message);
const char *ftpw_webview_flutter_wv_console_message_text_get(void *message);

#ifdef __cplusplus
}
#endif

#endif
