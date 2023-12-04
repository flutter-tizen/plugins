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

struct _Ewk_Touch_Point {
  int id;
  int x;
  int y;
  Evas_Touch_Point_State state;
};

typedef Eina_Bool (*EwkViewBgColorSetFnPtr)(Evas_Object* obj, int r, int g,
                                            int b, int a);
typedef Eina_Bool (*EwkViewFeedTouchEventFnPtr)(Evas_Object* obj,
                                                Ewk_Touch_Event_Type type,
                                                const Eina_List* points,
                                                const Evas_Modifier* modifiers);
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

typedef struct {
  EwkViewBgColorSetFnPtr SetBackgroundColor = nullptr;
  EwkViewFeedTouchEventFnPtr FeedTouchEvent = nullptr;
  EwkViewSendKeyEventFnPtr SendKeyEvent = nullptr;
  EwkViewOffscreenRenderingEnabledSetFnPtr OffscreenRenderingEnabledSet =
      nullptr;
  EwkViewImeWindowSetFnPtr ImeWindowSet = nullptr;
  EwkViewKeyEventsEnabledSetFnPtr KeyEventsEnabledSet = nullptr;
  EwkViewSupportVideoHoleSetFnPtr SupportVideoHoleSet = nullptr;
} EwkViewProcTable;

typedef void (*EwkSetArgumentsFnPtr)(int argc, char** argv);

typedef struct {
  EwkSetArgumentsFnPtr SetArguments = nullptr;
} EwkMainProcTable;

typedef struct Ewk_Settings Ewk_Settings;
typedef void (*EwkSettingsImePanelEnabledSetFnPtr)(Ewk_Settings* settings,
                                                   Eina_Bool enabled);

typedef struct {
  EwkSettingsImePanelEnabledSetFnPtr ImePanelEnabledSet = nullptr;
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
