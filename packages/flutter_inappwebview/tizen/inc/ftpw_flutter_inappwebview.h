// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef FLUTTER_TIZEN_PLUGINS_WRAPPER_FLUTTER_INAPPWEBVIEW_H_
#define FLUTTER_TIZEN_PLUGINS_WRAPPER_FLUTTER_INAPPWEBVIEW_H_

typedef enum {
  EWK_TOUCH_START,
  EWK_TOUCH_MOVE,
  EWK_TOUCH_END
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

#ifdef __cplusplus
extern "C" {
#endif

unsigned char ftpw_flutter_inappwebview_ewk_view_bg_color_set(void *obj, int r,
                                                              int g, int b,
                                                              int a);
unsigned char ftpw_flutter_inappwebview_ewk_view_touch_events_enabled_set(
    void *view, unsigned char enabled);
unsigned char ftpw_flutter_inappwebview_ewk_view_feed_touch_event(
    void *obj, Ewk_Touch_Event_Type type, const void *points,
    const void *modifiers);
unsigned char ftpw_flutter_inappwebview_ewk_view_mouse_events_enabled_set(
    void *view, unsigned char enabled);
unsigned char ftpw_flutter_inappwebview_ewk_view_feed_mouse_down(
    void *obj, Ewk_Mouse_Button_Type button, int x, int y);
unsigned char ftpw_flutter_inappwebview_ewk_view_feed_mouse_up(
    void *obj, Ewk_Mouse_Button_Type button, int x, int y);
unsigned char ftpw_flutter_inappwebview_ewk_view_feed_mouse_wheel(
    void *obj, unsigned char y_direction, int step, int x, int y);
unsigned char ftpw_flutter_inappwebview_ewk_view_send_key_event(
    void *obj, void *key_event, unsigned char is_press);
void ftpw_flutter_inappwebview_ewk_view_offscreen_rendering_enabled_set(
    void *obj, unsigned char enabled);
void ftpw_flutter_inappwebview_ewk_view_ime_window_set(void *obj, void *window);
unsigned char ftpw_flutter_inappwebview_ewk_view_key_events_enabled_set(
    void *obj, unsigned char enabled);
unsigned char ftpw_flutter_inappwebview_ewk_view_set_support_video_hole(
    void *obj, void *window, unsigned char enabled, unsigned char is_supported);
void ftpw_flutter_inappwebview_ewk_view_javascript_alert_callback_set(
    void *o, Ewk_View_JavaScript_Alert_Callback callback, void *user_data);
void ftpw_flutter_inappwebview_ewk_view_javascript_confirm_callback_set(
    void *o, Ewk_View_JavaScript_Confirm_Callback callback, void *user_data);
void ftpw_flutter_inappwebview_ewk_view_javascript_prompt_callback_set(
    void *o, Ewk_View_JavaScript_Prompt_Callback callback, void *user_data);
void ftpw_flutter_inappwebview_ewk_view_javascript_alert_reply(void *o);
void ftpw_flutter_inappwebview_ewk_view_javascript_confirm_reply(
    void *o, unsigned char result);
void ftpw_flutter_inappwebview_ewk_view_javascript_prompt_reply(
    void *o, const char *result);
void ftpw_flutter_inappwebview_ewk_set_arguments(int argc, char **argv);
void ftpw_flutter_inappwebview_ewk_settings_ime_panel_enabled_set(
    void *settings, unsigned char enabled);
void ftpw_flutter_inappwebview_ewk_settings_force_zoom_set(
    void *settings, unsigned char enable);
Ewk_Console_Message_Level
ftpw_flutter_inappwebview_ewk_console_message_level_get(
    const void *message);
const char *ftpw_flutter_inappwebview_ewk_console_message_text_get(
    const void *message);

#ifdef __cplusplus
}
#endif

#endif
