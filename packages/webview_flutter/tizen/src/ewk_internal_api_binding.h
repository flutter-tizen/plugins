// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_EWK_INTERNAL_API_BINDING_H_
#define FLUTTER_PLUGIN_EWK_INTERNAL_API_BINDING_H_

#include <Evas.h>

typedef enum {
  EWK_TOUCH_START,
  EWK_TOUCH_MOVE,
  EWK_TOUCH_END,
  EWK_TOUCH_CANCEL
} Ewk_Touch_Event_Type;

typedef struct _Ewk_Touch_Point Ewk_Touch_Point;

typedef enum {
  EWK_Mouse_Button_Left = 1,
  EWK_Mouse_Button_Middle = 2,
  EWK_Mouse_Button_Right = 3
} Ewk_Mouse_Button_Type;

struct _Ewk_Touch_Point {
  int id;
  int x;
  int y;
  Evas_Touch_Point_State state;
};

typedef Eina_Bool (*Ewk_View_JavaScript_Alert_Callback)(Evas_Object* o,
                                                        const char* alert_text,
                                                        void* user_data);
typedef Eina_Bool (*Ewk_View_JavaScript_Confirm_Callback)(Evas_Object* o,
                                                          const char* message,
                                                          void* user_data);
typedef Eina_Bool (*Ewk_View_JavaScript_Prompt_Callback)(
    Evas_Object* o, const char* message, const char* default_value,
    void* user_data);

typedef Eina_Bool (*EwkViewBgColorSetFnPtr)(Evas_Object* obj, int r, int g,
                                            int b, int a);
typedef Eina_Bool (*EwkViewTouchEventsEnabledSetFnPtr)(Evas_Object* view,
                                                       Eina_Bool enabled);
typedef Eina_Bool (*EwkViewFeedTouchEventFnPtr)(Evas_Object* obj,
                                                Ewk_Touch_Event_Type type,
                                                const Eina_List* points,
                                                const Evas_Modifier* modifiers);
typedef Eina_Bool (*EwkViewMouseEventsEnabledSetFnPtr)(Evas_Object* view,
                                                       Eina_Bool enabled);
typedef Eina_Bool (*EwkViewFeedMouseDownFnPtr)(Evas_Object* obj,
                                               Ewk_Mouse_Button_Type button,
                                               int x, int y);
typedef Eina_Bool (*EwkViewFeedMouseUpFnPtr)(Evas_Object* obj,
                                             Ewk_Mouse_Button_Type button,
                                             int x, int y);
typedef Eina_Bool (*EwkViewFeedMouseMoveFnPtr)(Evas_Object* obj, int x, int y);
typedef Eina_Bool (*EwkViewFeedMouseWheelFnPtr)(Evas_Object* obj,
                                                Eina_Bool y_direction, int step,
                                                int x, int y);
typedef Eina_Bool (*EwkViewSendKeyEventFnPtr)(Evas_Object* obj, void* key_event,
                                              Eina_Bool is_press);
typedef void (*EwkViewOffscreenRenderingEnabledSetFnPtr)(Evas_Object* obj,
                                                         Eina_Bool enabled);
typedef void (*EwkViewImeWindowSetFnPtr)(Evas_Object* obj, void* window);
typedef Eina_Bool (*EwkViewKeyEventsEnabledSetFnPtr)(Evas_Object* obj,
                                                     Eina_Bool enabled);
typedef Eina_Bool (*EwkViewSupportVideoHoleSetFnPtr)(Evas_Object* obj,
                                                     void* window,
                                                     Eina_Bool enabled,
                                                     Eina_Bool boo);
typedef void (*EwkViewJavaScriptAlertCallbackSetFnPtr)(
    Evas_Object* o, Ewk_View_JavaScript_Alert_Callback callback,
    void* user_data);
typedef void (*EwkViewJavaScriptConfirmCallbackSetFnPtr)(
    Evas_Object* o, Ewk_View_JavaScript_Confirm_Callback callback,
    void* user_data);
typedef void (*EwkViewJavaScriptPromptCallbackSetFnPtr)(
    Evas_Object* o, Ewk_View_JavaScript_Prompt_Callback callback,
    void* user_data);
typedef void (*EwkViewJavaScriptAlertReplyFnPtr)(Evas_Object* o);
typedef void (*EwkViewJavaScriptConfirmReplyFnPtr)(Evas_Object* o,
                                                   Eina_Bool result);
typedef void (*EwkViewJavaScriptPromptReplyFnPtr)(Evas_Object* o,
                                                  const char* result);

typedef struct {
  EwkViewBgColorSetFnPtr SetBackgroundColor = nullptr;
  EwkViewTouchEventsEnabledSetFnPtr TouchEventsEnabledSet = nullptr;
  EwkViewFeedTouchEventFnPtr FeedTouchEvent = nullptr;
  EwkViewMouseEventsEnabledSetFnPtr MouseEventsEnabledSet = nullptr;
  EwkViewFeedMouseDownFnPtr FeedMouseDown = nullptr;
  EwkViewFeedMouseUpFnPtr FeedMouseUp = nullptr;
  EwkViewFeedMouseMoveFnPtr FeedMouseMove = nullptr;
  EwkViewFeedMouseWheelFnPtr FeedMouseWheel = nullptr;
  EwkViewSendKeyEventFnPtr SendKeyEvent = nullptr;
  EwkViewOffscreenRenderingEnabledSetFnPtr OffscreenRenderingEnabledSet =
      nullptr;
  EwkViewImeWindowSetFnPtr ImeWindowSet = nullptr;
  EwkViewKeyEventsEnabledSetFnPtr KeyEventsEnabledSet = nullptr;
  EwkViewSupportVideoHoleSetFnPtr SupportVideoHoleSet = nullptr;
  EwkViewJavaScriptAlertCallbackSetFnPtr OnJavaScriptAlert = nullptr;
  EwkViewJavaScriptConfirmCallbackSetFnPtr OnJavaScriptConfirm = nullptr;
  EwkViewJavaScriptPromptCallbackSetFnPtr OnJavaScriptPrompt = nullptr;
  EwkViewJavaScriptAlertReplyFnPtr JavaScriptAlertReply = nullptr;
  EwkViewJavaScriptConfirmReplyFnPtr JavaScriptConfirmReply = nullptr;
  EwkViewJavaScriptPromptReplyFnPtr JavaScriptPromptReply = nullptr;

} EwkViewProcTable;

typedef void (*EwkSetArgumentsFnPtr)(int argc, char** argv);
typedef int (*EwkSetVersionPolicyFnPtr)(int preference);

typedef struct {
  EwkSetArgumentsFnPtr SetArguments = nullptr;
  EwkSetVersionPolicyFnPtr SetVersionPolicy = nullptr;
} EwkMainProcTable;

typedef struct Ewk_Settings Ewk_Settings;
typedef void (*EwkSettingsImePanelEnabledSetFnPtr)(Ewk_Settings* settings,
                                                   Eina_Bool enabled);
typedef void (*EwkSettingsForceZoomSetFnPtr)(Ewk_Settings* settings,
                                             Eina_Bool enable);

typedef struct {
  EwkSettingsImePanelEnabledSetFnPtr ImePanelEnabledSet = nullptr;
  EwkSettingsForceZoomSetFnPtr ForceZoomSet = nullptr;
} EwkSettingsProcTable;

typedef struct _Ewk_Console_Message Ewk_Console_Message;

typedef enum {
  EWK_CONSOLE_MESSAGE_LEVEL_NULL,
  EWK_CONSOLE_MESSAGE_LEVEL_LOG,
  EWK_CONSOLE_MESSAGE_LEVEL_WARNING,
  EWK_CONSOLE_MESSAGE_LEVEL_ERROR,
  EWK_CONSOLE_MESSAGE_LEVEL_DEBUG,
  EWK_CONSOLE_MESSAGE_LEVEL_INFO,
} Ewk_Console_Message_Level;

typedef Ewk_Console_Message_Level (*EwkConsoleMessageLevelGetFnPtr)(
    const Ewk_Console_Message* message);
typedef Eina_Stringshare* (*EwkConsoleMessageTextGetFnPtr)(
    const Ewk_Console_Message* message);
typedef unsigned (*EwkConsoleMessageLineGetFnPtr)(
    const Ewk_Console_Message* message);
typedef Eina_Stringshare* (*EwkConsoleMessageSourceGetFnPtr)(
    const Ewk_Console_Message* message);

typedef struct {
  EwkConsoleMessageLevelGetFnPtr LevelGet = nullptr;
  EwkConsoleMessageTextGetFnPtr TextGet = nullptr;
  EwkConsoleMessageLineGetFnPtr LineGet = nullptr;
  EwkConsoleMessageSourceGetFnPtr SourceGet = nullptr;
} EwkConsoleMessageProcTable;

class EwkInternalApiBinding {
 public:
  static EwkInternalApiBinding& GetInstance() {
    static EwkInternalApiBinding instance = EwkInternalApiBinding();
    return instance;
  }

  ~EwkInternalApiBinding();

  EwkInternalApiBinding(const EwkInternalApiBinding&) = delete;
  EwkInternalApiBinding& operator=(const EwkInternalApiBinding&) = delete;

  bool Initialize();

  EwkViewProcTable view;
  EwkMainProcTable main;
  EwkSettingsProcTable settings;
  EwkConsoleMessageProcTable console_message;

 private:
  EwkInternalApiBinding();

  void* handle_ = nullptr;
};

#endif  // FLUTTER_PLUGIN_EWK_INTERNAL_API_BINDING_H_
