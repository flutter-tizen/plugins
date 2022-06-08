/*
 * Copyright (C) 2011-2016 Samsung Electronics. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY SAMSUNG ELECTRONICS. AND ITS CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SAMSUNG ELECTRONICS. OR ITS
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    ewk_view_internal.h
 * @brief   Chromium main smart object.
 *
 * This object provides view related APIs of Chromium to EFL object.
 */

#ifndef ewk_view_internal_h
#define ewk_view_internal_h

#include "ewk_app_installation_request_internal.h"
#include "ewk_auth_challenge_internal.h"
#include "ewk_context_internal.h"
#include "ewk_enums_internal.h"
#include "ewk_frame_internal.h"
#include "ewk_geolocation_internal.h"
#include "ewk_history_internal.h"
#include "ewk_hit_test_internal.h"
#include "ewk_page_group_internal.h"
#include "ewk_quota_permission_request_internal.h"
#include "ewk_touch_internal.h"
#include "ewk_user_media_internal.h"
#include "ewk_view.h"
#include "ewk_window_features_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Enum values containing text directionality values.
typedef enum {
  EWK_TEXT_DIRECTION_RIGHT_TO_LEFT,
  EWK_TEXT_DIRECTION_LEFT_TO_RIGHT
} Ewk_Text_Direction;

enum Ewk_Password_Popup_Option {
  EWK_PASSWORD_POPUP_SAVE,
  EWK_PASSWORD_POPUP_NOT_NOW,
  EWK_PASSWORD_POPUP_NEVER,
  EWK_PASSWORD_POPUP_OK = EWK_PASSWORD_POPUP_SAVE,
  EWK_PASSWORD_POPUP_CANCEL = EWK_PASSWORD_POPUP_NOT_NOW
};
typedef enum Ewk_Password_Popup_Option Ewk_Password_Popup_Option;

typedef struct Ewk_View_Smart_Data Ewk_View_Smart_Data;
typedef struct Ewk_View_Smart_Class Ewk_View_Smart_Class;

enum Ewk_Select_Menu_Item_Type {
  OPTION,
  CHECKABLE_OPTION,
  GROUP,
  SEPARATOR,
  SUBMENU,  // This is currently only used by Pepper, not by WebKit.
  TYPE_LAST = SUBMENU
};
typedef enum Ewk_Select_Menu_Item_Type Ewk_Select_Menu_Item_Type;

struct Ewk_Select_Menu_Item_Info {
  const char* label;
  const char* icon;
  const char* tool_tip;
  Ewk_Select_Menu_Item_Type type;
  unsigned action;
  Eina_Bool rtl;
  Eina_Bool has_directional_override;
  Eina_Bool enabled;
  Eina_Bool checked;
  Eina_List* submenu;
};
typedef struct Ewk_Select_Menu_Item_Info Ewk_Select_Menu_Item_Info;

struct Ewk_Select_Menu {
  Evas_Object* evas_object;
  int selected_index;
  Eina_List* items;
  Eina_Bool is_multiple_selection;
  Eina_Rectangle bounds;
  double item_font_size;
};
typedef struct Ewk_Select_Menu Ewk_Select_Menu;

// #if PLATFORM(TIZEN)
/// Creates a type name for _Ewk_Event_Gesture.
typedef struct Ewk_Event_Gesture Ewk_Event_Gesture;

/// Represents a gesture event.
struct Ewk_Event_Gesture {
  Ewk_Gesture_Type type;     /**< type of the gesture event */
  Evas_Coord_Point position; /**< position of the gesture event */
  Evas_Point velocity; /**< velocity of the gesture event. The unit is pixel per
                          second. */
  double scale;        /**< scale of the gesture event */
  int count;           /**< count of the gesture */
  unsigned int timestamp; /**< timestamp of the gesture */
};

// #if ENABLE(TIZEN_FOCUS_UI)
enum Ewk_Unfocus_Direction {
  EWK_UNFOCUS_DIRECTION_NONE = 0,
  EWK_UNFOCUS_DIRECTION_FORWARD,
  EWK_UNFOCUS_DIRECTION_BACKWARD,
  EWK_UNFOCUS_DIRECTION_UP,
  EWK_UNFOCUS_DIRECTION_DOWN,
  EWK_UNFOCUS_DIRECTION_LEFT,
  EWK_UNFOCUS_DIRECTION_RIGHT,
};
typedef enum Ewk_Unfocus_Direction Ewk_Unfocus_Direction;
// #endif

// #if ENABLE(TIZEN_INPUT_TAG_EXTENSION)
/**
 * \enum    Ewk_Input_Type
 * @brief   Provides type of focused input element
 */
enum Ewk_Input_Type {
  EWK_INPUT_TYPE_TEXT,
  EWK_INPUT_TYPE_TELEPHONE,
  EWK_INPUT_TYPE_NUMBER,
  EWK_INPUT_TYPE_EMAIL,
  EWK_INPUT_TYPE_URL,
  EWK_INPUT_TYPE_PASSWORD,
  EWK_INPUT_TYPE_COLOR,
  EWK_INPUT_TYPE_DATE,
  EWK_INPUT_TYPE_DATETIME,
  EWK_INPUT_TYPE_DATETIMELOCAL,
  EWK_INPUT_TYPE_MONTH,
  EWK_INPUT_TYPE_TIME,
  EWK_INPUT_TYPE_WEEK
};
typedef enum Ewk_Input_Type Ewk_Input_Type;
// #endif // ENABLE(TIZEN_INPUT_TAG_EXTENSION)

// #if ENABLE(TIZEN_WEBKIT2_TEXT_SELECTION)
/**
 * \enum    Ewk_Selection_Handle_Type
 * @brief   Provides type of selection handle
 */
enum Ewk_Selection_Handle_Type {
  EWK_SELECTION_HANDLE_TYPE_LEFT,
  EWK_SELECTION_HANDLE_TYPE_RIGHT,
  EWK_SELECTION_HANDLE_TYPE_LARGE
};
typedef enum Ewk_Selection_Handle_Type Ewk_Selection_Handle_Type;
// #endif // ENABLE(TIZEN_WEBKIT2_TEXT_SELECTION)
// #endif // #if PLATFORM(TIZEN)

enum Ewk_View_Mode {
  EWK_VIEW_MODE_WINDOWED = 0,
  EWK_VIEW_MODE_FLOATING,
  EWK_VIEW_MODE_FULLSCREEN,
  EWK_VIEW_MODE_MAXIMIZED,
  EWK_VIEW_MODE_MINIMIZED
};
typedef enum Ewk_View_Mode Ewk_View_Mode;

enum Ewk_Top_Control_State {
  EWK_TOP_CONTROL_SHOWN = 1,
  EWK_TOP_CONTROL_HIDDEN = 2,
  EWK_TOP_CONTROL_BOTH = 3
};
typedef enum Ewk_Top_Control_State Ewk_Top_Control_State;

enum Ewk_Mouse_Button_Type {
  EWK_Mouse_Button_Left = 1,
  EWK_Mouse_Button_Middle = 2,
  EWK_Mouse_Button_Right = 3
};
typedef enum Ewk_Mouse_Button_Type Ewk_Mouse_Button_Type;

/// Ewk view's class, to be overridden by sub-classes.
struct Ewk_View_Smart_Class {
  Evas_Smart_Class sc; /**< all but 'data' is free to be changed. */
  unsigned long version;

  Evas_Object* (*window_create)(
      Ewk_View_Smart_Data* sd,
      const Ewk_Window_Features*
          window_features); /**< creates a new window, requested by webkit */
  void (*window_close)(Ewk_View_Smart_Data* sd); /**< closes a window */

  Eina_Bool (*context_menu_show)(Ewk_View_Smart_Data* sd, Evas_Coord x,
                                 Evas_Coord y, Ewk_Context_Menu* menu);
  Eina_Bool (*context_menu_hide)(Ewk_View_Smart_Data* sd);

  Eina_Bool (*popup_menu_show)(Ewk_View_Smart_Data* sd, Eina_Rectangle rect,
                               Ewk_Text_Direction text_direction,
                               double page_scale_factor, Eina_List* items,
                               int selected_index);
  Eina_Bool (*popup_menu_hide)(Ewk_View_Smart_Data* sd);
  Eina_Bool (*popup_menu_update)(Ewk_View_Smart_Data* sd, Eina_Rectangle rect,
                                 Ewk_Text_Direction text_direction,
                                 Eina_List* items, int selected_index);

  Eina_Bool (*text_selection_down)(Ewk_View_Smart_Data* sd, int x, int y);
  Eina_Bool (*text_selection_up)(Ewk_View_Smart_Data* sd, int x, int y);

  Eina_Bool (*input_picker_show)(Ewk_View_Smart_Data* sd,
                                 Ewk_Input_Type inputType,
                                 const char* inputValue);

  Eina_Bool (*orientation_lock)(Ewk_View_Smart_Data* sd, int orientations);
  void (*orientation_unlock)(Ewk_View_Smart_Data* sd);

  // event handling:
  //  - returns true if handled
  //  - if overridden, have to call parent method if desired
  Eina_Bool (*focus_in)(Ewk_View_Smart_Data* sd);
  Eina_Bool (*focus_out)(Ewk_View_Smart_Data* sd);
  Eina_Bool (*fullscreen_enter)(Ewk_View_Smart_Data* sd,
                                Ewk_Security_Origin* origin);
  Eina_Bool (*fullscreen_exit)(Ewk_View_Smart_Data* sd);
  Eina_Bool (*mouse_wheel)(Ewk_View_Smart_Data* sd,
                           const Evas_Event_Mouse_Wheel* ev);
  Eina_Bool (*mouse_down)(Ewk_View_Smart_Data* sd,
                          const Evas_Event_Mouse_Down* ev);
  Eina_Bool (*mouse_up)(Ewk_View_Smart_Data* sd, const Evas_Event_Mouse_Up* ev);
  Eina_Bool (*mouse_move)(Ewk_View_Smart_Data* sd,
                          const Evas_Event_Mouse_Move* ev);
  Eina_Bool (*key_down)(Ewk_View_Smart_Data* sd, const Evas_Event_Key_Down* ev);
  Eina_Bool (*key_up)(Ewk_View_Smart_Data* sd, const Evas_Event_Key_Up* ev);

  // color picker:
  //   - Shows and hides color picker.
  Eina_Bool (*input_picker_color_request)(Ewk_View_Smart_Data* sd, int r, int g,
                                          int b, int a);
  Eina_Bool (*input_picker_color_dismiss)(Ewk_View_Smart_Data* sd);

  // storage:
  //   - Web database.
  unsigned long long (*exceeded_database_quota)(
      Ewk_View_Smart_Data* sd, const char* databaseName,
      const char* displayName, unsigned long long currentQuota,
      unsigned long long currentOriginUsage,
      unsigned long long currentDatabaseUsage,
      unsigned long long expectedUsage);

  Eina_Bool (*formdata_candidate_show)(Ewk_View_Smart_Data* sd, int x, int y,
                                       int w, int h);
  Eina_Bool (*formdata_candidate_hide)(Ewk_View_Smart_Data* sd);
  Eina_Bool (*formdata_candidate_update_data)(Ewk_View_Smart_Data* sd,
                                              Eina_List* dataList);
  Eina_Bool (*formdata_candidate_is_showing)(Ewk_View_Smart_Data* sd);

  Eina_Bool (*gesture_start)(Ewk_View_Smart_Data* sd,
                             const Ewk_Event_Gesture* ev);
  Eina_Bool (*gesture_end)(Ewk_View_Smart_Data* sd,
                           const Ewk_Event_Gesture* ev);
  Eina_Bool (*gesture_move)(Ewk_View_Smart_Data* sd,
                            const Ewk_Event_Gesture* ev);

  void (*selection_handle_down)(Ewk_View_Smart_Data* sd,
                                Ewk_Selection_Handle_Type handleType, int x,
                                int y);
  void (*selection_handle_move)(Ewk_View_Smart_Data* sd,
                                Ewk_Selection_Handle_Type handleType, int x,
                                int y);
  void (*selection_handle_up)(Ewk_View_Smart_Data* sd,
                              Ewk_Selection_Handle_Type handleType, int x,
                              int y);

  Eina_Bool (*window_geometry_set)(Ewk_View_Smart_Data* sd, Evas_Coord x,
                                   Evas_Coord y, Evas_Coord width,
                                   Evas_Coord height);
  Eina_Bool (*window_geometry_get)(Ewk_View_Smart_Data* sd, Evas_Coord* x,
                                   Evas_Coord* y, Evas_Coord* width,
                                   Evas_Coord* height);
};

// #if PLATFORM(TIZEN)
/**
 * Callback for ewk_view_web_app_capable_get
 *
 * @param capable web application capable
 * @param user_data user_data will be passsed when ewk_view_web_app_capable_get
 * is called
 */
typedef void (*Ewk_Web_App_Capable_Get_Callback)(Eina_Bool capable,
                                                 void* user_data);

/**
 * Callback for ewk_view_web_app_icon_get
 *
 * @param icon_url web application icon
 * @param user_data user_data will be passsed when ewk_view_web_app_icon_get is
 * called
 */
typedef void (*Ewk_Web_App_Icon_URL_Get_Callback)(const char* icon_url,
                                                  void* user_data);

/**
 * Callback for ewk_view_screenshot_contents_get_async
 *
 * @param image captured screenshot
 * @param user_data user_data will be passsed when
 * ewk_view_screenshot_contents_get_async is called
 */
typedef void (*Ewk_Web_App_Screenshot_Captured_Callback)(Evas_Object* image,
                                                         void* user_data);

/**
 * Callback for ewk_view_web_app_icon_urls_get.
 *
 * @param icon_urls list of Ewk_Web_App_Icon_Data for web app
 * @param user_data user_data will be passsed when
 * ewk_view_web_app_icon_urls_get is called
 */
typedef void (*Ewk_Web_App_Icon_URLs_Get_Callback)(Eina_List* icon_urls,
                                                   void* user_data);
// #endif

/**
 * Callback for ewk_view_notification_permission_callback_set
 *
 * @param o view object to request the notification permission
 * @param request Ewk_Notification_Permission_Request object to get the
 * information about notification permission request.
 * @param user_data user data
 *
 * @return returned value is not used
 */
typedef Eina_Bool (*Ewk_View_Notification_Permission_Callback)(
    Evas_Object* o, Ewk_Notification_Permission_Request* request,
    void* user_data);

/**
 * Defines a callback for scale change.
 *
 * @param o view object to register on scale change
 * @param scale_factor the scale applied to view
 * @param user_data a pointer to data specified by
 * ewk_view_scale_changed_callback_set
 */
typedef void (*Ewk_View_Scale_Changed_Callback)(Evas_Object* o,
                                                double scale_factor,
                                                void* user_data);

/**
 * Defines a callback for show or hide the notification to user.
 *
 * @param o view object to register on scale change
 * @param show bool flag to indicate whether to show the mic notification.
 *     show is true when bluetooth mic is opened or voice key is pressed
 * shortly. show is false when voice key is hold for more than 500ms
 * @param user_data a pointer to data specified by
 * ewk_view_smartrc_show_mic_notification_callback_set
 */
typedef void (*Ewk_View_SmartRC_Mic_Notification_Callback)(Evas_Object* o,
                                                           Eina_Bool show,
                                                           void* user_data);

/**
 * The version you have to put into the version field
 * in the @a Ewk_View_Smart_Class structure.
 */
#define EWK_VIEW_SMART_CLASS_VERSION 1UL

/**
 * Initializer for whole Ewk_View_Smart_Class structure.
 *
 * @param smart_class_init initializer to use for the "base" field
 * (Evas_Smart_Class).
 *
 * @see EWK_VIEW_SMART_CLASS_INIT_NULL
 * @see EWK_VIEW_SMART_CLASS_INIT_VERSION
 * @see EWK_VIEW_SMART_CLASS_INIT_NAME_VERSION
 */
#define EWK_VIEW_SMART_CLASS_INIT(smart_class_init) \
  { smart_class_init, EWK_VIEW_SMART_CLASS_VERSION }

/**
 * Initializer to zero a whole Ewk_View_Smart_Class structure.
 *
 * @see EWK_VIEW_SMART_CLASS_INIT_VERSION
 * @see EWK_VIEW_SMART_CLASS_INIT_NAME_VERSION
 * @see EWK_VIEW_SMART_CLASS_INIT
 */
#define EWK_VIEW_SMART_CLASS_INIT_NULL \
  EWK_VIEW_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NULL)

/**
 * Initializer to zero a whole Ewk_View_Smart_Class structure and set
 * name and version.
 *
 * Similar to EWK_VIEW_SMART_CLASS_INIT_NULL, but will set version field of
 * Evas_Smart_Class (base field) to latest EVAS_SMART_CLASS_VERSION and name
 * to the specific value.
 *
 * It will keep a reference to name field as a "const char *", that is,
 * name must be available while the structure is used (hint: static or global!)
 * and will not be modified.
 *
 * @see EWK_VIEW_SMART_CLASS_INIT_NULL
 * @see EWK_VIEW_SMART_CLASS_INIT_VERSION
 * @see EWK_VIEW_SMART_CLASS_INIT
 */
#define EWK_VIEW_SMART_CLASS_INIT_NAME_VERSION(name) \
  EWK_VIEW_SMART_CLASS_INIT(EVAS_SMART_CLASS_INIT_NAME_VERSION(name))

typedef struct EwkViewImpl EwkViewImpl;
/**
 * @brief Contains an internal View data.
 *
 * It is to be considered private by users, but may be extended or
 * changed by sub-classes (that's why it's in public header file).
 */
struct Ewk_View_Smart_Data {
  Evas_Object_Smart_Clipped_Data base;
  const Ewk_View_Smart_Class* api; /**< reference to casted class instance */
  Evas_Object* self;               /**< reference to owner object */
  EwkViewImpl* priv;               /**< should never be accessed, c++ stuff */
  struct {
    Evas_Coord x, y, w, h; /**< last used viewport */
  } view;
  struct { /**< what changed since last smart_calculate */
    Eina_Bool any : 1;

    // WebKit use these but we don't. We should remove these if we are sure
    // we do it right.
    Eina_Bool size : 1;
    Eina_Bool position : 1;
  } changed;
};

/**
 * Sets the smart class APIs, enabling view to be inherited.
 *
 * @param api class definition to set, all members with the
 *        exception of @a Evas_Smart_Class->data may be overridden, must
 *        @b not be @c NULL
 *
 * @note @a Evas_Smart_Class->data is used to implement type checking and
 *       is not supposed to be changed/overridden. If you need extra
 *       data for your smart class to work, just extend
 *       Ewk_View_Smart_Class instead.
 *       The Evas_Object which inherits the ewk_view should use
 *       ewk_view_smart_add() to create Evas_Object instead of
 *       evas_object_smart_add() because it performs additional initialization
 *       for the ewk_view.
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure (probably
 *         version mismatch)
 *
 * @see ewk_view_smart_add()
 */
EXPORT_API Eina_Bool ewk_view_smart_class_set(Ewk_View_Smart_Class* api);

/**
 * Creates a new EFL WebKit view object with Evas_Smart and Ewk_Context.
 *
 * @note The Evas_Object which inherits the ewk_view should create its
 *       Evas_Object using this API instead of evas_object_smart_add()
 *       because the default initialization for ewk_view is done in this API.
 *
 * @param e canvas object where to create the view object
 * @param smart Evas_Smart object. Its type should be EWK_VIEW_TYPE_STR
 * @param context Ewk_Context object which is used for initializing
 * @param pageGroup Ewk_Page_Group object which is used for initializing
 *
 * @return view object on success or @c NULL on failure
 */
EXPORT_API Evas_Object* ewk_view_smart_add(Evas* e, Evas_Smart* smart,
                                           Ewk_Context* context,
                                           Ewk_Page_Group* pageGroup);

/**
 * Creates a new EFL WebKit view object based on specific Ewk_Context.
 *
 * @note If used to created a new ewk_view object in response to a
 *       "create,window" smart signal (emitted by the web engine), this
 *       function must pass as context parameter the same context instance
 *       as of the originating ewk_view object's context, e.g.:
 *
 *       - Scenario 1: parent view created with default context:
 *
 *         parent_view = ewk_view_add(evas); // uses the default context.
 *                                 or
 *         parent_view = ewk_view_add_with_context(evas,
 * ewk_context_default_get()); then
 *         // Valid cases:
 *         child_view = ewk_view_add_with_context(evas,
 * ewk_view_context_get(parent_view)); or child_view =
 * ewk_view_add_with_context(evas, ewk_context_default_get());
 *
 *         // Invalid case:
 *         view_view = ewk_view_add_with_context(evas, ewk_context_new());
 *
 *       - Scenario 2: parent created with a new context:
 *
 *         parent_view = ewk_view_add_with_context(evas, ewk_context_new());
 *                                 then
 *         // Valid case:
 *         child_view = ewk_view_add_with_context(evas,
 * ewk_view_context_get(parent_view));
 *
 *         // Invalid cases:
 *         child_view = ewk_view_add_with_context(evas, ewk_context_new());
 *                                 or
 *         child_view = ewk_view_add_with_context(evas,
 * ewk_context_default_get());
 *
 * @param e canvas object where to create the view object
 * @param context Ewk_Context object to declare process model
 *
 * @return view object on success or @c NULL on failure
 */
EXPORT_API Evas_Object* ewk_view_add_with_context(Evas* e,
                                                  Ewk_Context* context);

/**
 * @brief Gets the widget of this view.
 *
 * @since_tizen 3.0
 *
 * @param[in] o The view object to get the widget
 *
 * @return widget on success or @c NULL on failure
 */
EXPORT_API Evas_Object* ewk_view_widget_get(Evas_Object* view);

enum Ewk_Page_Visibility_State {
  EWK_PAGE_VISIBILITY_STATE_VISIBLE,
  EWK_PAGE_VISIBILITY_STATE_HIDDEN,
  EWK_PAGE_VISIBILITY_STATE_PRERENDER
};
typedef enum Ewk_Page_Visibility_State Ewk_Page_Visibility_State;

/**
 * Callback for ewk_view_script_execute
 *
 * @param o the view object
 * @param result_value value returned by script
 * @param user_data user data
 */
typedef void (*Ewk_View_Script_Execute_Callback)(Evas_Object* o,
                                                 const char* result_value,
                                                 void* user_data);

/**
 * Callback for ewk_view_plain_text_get
 *
 * @param o the view object
 * @param plain_text the contents of the given frame converted to plain text
 * @param user_data user data
 */
typedef void (*Ewk_View_Plain_Text_Get_Callback)(Evas_Object* o,
                                                 const char* plain_text,
                                                 void* user_data);

/**
 * Creates a type name for the callback function used to get the page contents.
 *
 * @param o view object
 * @param data mhtml data of the page contents
 * @param user_data user data will be passed when ewk_view_mhtml_data_get is
 * called
 */
typedef void (*Ewk_View_MHTML_Data_Get_Callback)(Evas_Object* o,
                                                 const char* data,
                                                 void* user_data);

/**
 * @brief Gets whether horizontal panning is holding.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to get whether horizontal panning is holding
 *
 * @return @c EINA_TRUE if horizontal panning is holding
 *         @c EINA_FALSE if not or on failure
 */
EXPORT_API Eina_Bool ewk_view_horizontal_panning_hold_get(Evas_Object* o);

/**
 * @brief Sets to hold horizontal panning.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to set to hold horizontal panning
 * @param[in] hold @c EINA_TRUE to hold horizontal panning
 *        @c EINA_FALSE not to hold
 */
EXPORT_API void ewk_view_horizontal_panning_hold_set(Evas_Object* o,
                                                     Eina_Bool hold);

/**
 * @brief Gets whether horizontal panning is holding.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to get whether horizontal panning is holding
 *
 * @return @c EINA_TRUE if horizontal panning is holding
 *         @c EINA_FALSE if not or on failure
 */
EXPORT_API Eina_Bool ewk_view_horizontal_panning_hold_get(Evas_Object* o);

/**
 * @brief Sets to hold horizontal panning.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to set to hold horizontal panning
 * @param[in] hold @c EINA_TRUE to hold horizontal panning
 *        @c EINA_FALSE not to hold
 */
EXPORT_API void ewk_view_horizontal_panning_hold_set(Evas_Object* o,
                                                     Eina_Bool hold);

/**
 * @brief Gets whether vertical panning is holding.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to get whether vertical panning is holding
 *
 * @return @c EINA_TRUE if vertical panning is holding
 *         @c EINA_FALSE if not or on failure
 */
EXPORT_API Eina_Bool ewk_view_vertical_panning_hold_get(Evas_Object* o);

/**
 * Block/Release the vertical pan
 *
 * @param o view object on which pan is to be blocked/release
 * @param hold status of pan
 */
EXPORT_API void ewk_view_vertical_panning_hold_set(Evas_Object* o,
                                                   Eina_Bool hold);

/**
 * Gets the minimum and maximum value of the scale range or -1 on failure
 *
 * @param o view object to get the minimum and maximum value of the scale range
 * @param min_scale Pointer to an double in which to store the minimum scale
 * factor of the object.
 * @param max_scale Pointer to an double in which to store the maximum scale
 * factor of the object.
 *
 * @note Use @c NULL pointers on the scale components you're not
 * interested in: they'll be ignored by the function.
 */
EXPORT_API void ewk_view_scale_range_get(Evas_Object* o, double* min_scale,
                                         double* max_scale);

typedef Eina_Bool (*Ewk_View_Password_Confirm_Popup_Callback)(
    Evas_Object* o, const char* message, void* user_data);
EXPORT_API void ewk_view_password_confirm_popup_callback_set(
    Evas_Object* o, Ewk_View_Password_Confirm_Popup_Callback callback,
    void* user_data);
EXPORT_API void ewk_view_password_confirm_popup_reply(
    Evas_Object* o, Ewk_Password_Popup_Option result);

typedef Eina_Bool (*Ewk_View_JavaScript_Alert_Callback)(Evas_Object* o,
                                                        const char* alert_text,
                                                        void* user_data);
EXPORT_API void ewk_view_javascript_alert_callback_set(
    Evas_Object* o, Ewk_View_JavaScript_Alert_Callback callback,
    void* user_data);
EXPORT_API void ewk_view_javascript_alert_reply(Evas_Object* o);

typedef Eina_Bool (*Ewk_View_JavaScript_Confirm_Callback)(Evas_Object* o,
                                                          const char* message,
                                                          void* user_data);
EXPORT_API void ewk_view_javascript_confirm_callback_set(
    Evas_Object* o, Ewk_View_JavaScript_Confirm_Callback callback,
    void* user_data);

/**
 * Callback for ewk_view_javascript_prompt_callback_set
 *
 * @param o the view object
 * @param message the text to be displayed on the prompt popup
 * @param default_value default text to be entered in the prompt dialog
 * @param user_data user data
 */
typedef Eina_Bool (*Ewk_View_JavaScript_Prompt_Callback)(
    Evas_Object* o, const char* message, const char* default_value,
    void* user_data);

/**
 * Display javascript prompt popup
 *
 * @param o view object
 * @param callback callback function to be called when the prompt popup is to be
 * opened
 * @param user_data user data
 *
 */
EXPORT_API void ewk_view_javascript_prompt_callback_set(
    Evas_Object* o, Ewk_View_JavaScript_Prompt_Callback callback,
    void* user_data);

//#if ENABLE(TIZEN_APPLICATION_CACHE)
typedef Eina_Bool (*Ewk_View_Applicacion_Cache_Permission_Callback)(
    Evas_Object* o, Ewk_Security_Origin* origin, void* user_data);
//#endif

typedef void (*Ewk_View_Exceeded_Indexed_Database_Quota_Callback)(
    Evas_Object* o, Ewk_Security_Origin* origin, long long currentQuota,
    void* user_data);
EXPORT_API void ewk_view_exceeded_indexed_database_quota_callback_set(
    Evas_Object* o, Ewk_View_Exceeded_Indexed_Database_Quota_Callback callback,
    void* user_data);
EXPORT_API void ewk_view_exceeded_indexed_database_quota_reply(Evas_Object* o,
                                                               Eina_Bool allow);

typedef Eina_Bool (*Ewk_View_Exceeded_Database_Quota_Callback)(
    Evas_Object* o, Ewk_Security_Origin* origin, const char* database_name,
    unsigned long long expectedQuota, void* user_data);
EXPORT_API void ewk_view_exceeded_database_quota_callback_set(
    Evas_Object* o, Ewk_View_Exceeded_Database_Quota_Callback callback,
    void* user_data);
EXPORT_API void ewk_view_exceeded_database_quota_reply(Evas_Object* o,
                                                       Eina_Bool allow);

typedef Eina_Bool (*Ewk_View_Exceeded_Local_File_System_Quota_Callback)(
    Evas_Object* o, Ewk_Security_Origin* origin, long long currentQuota,
    void* user_data);
EXPORT_API void ewk_view_exceeded_local_file_system_quota_callback_set(
    Evas_Object* o, Ewk_View_Exceeded_Local_File_System_Quota_Callback callback,
    void* user_data);
EXPORT_API void ewk_view_exceeded_local_file_system_quota_reply(
    Evas_Object* o, Eina_Bool allow);
//#if ENABLE(TIZEN_FOCUS_UI)
typedef Eina_Bool (*Ewk_View_Unfocus_Allow_Callback)(
    Evas_Object* o, Ewk_Unfocus_Direction direction, void* user_data);
//#endif

/**
 * Callback for geolocation permission request feature.
 *
 * @param ewk_view view object where geolocation permission was requested
 * @param request geolocation permission request object
 * @param user_data user data passed to
 *        ewk_view_geolocation_permission_callback_set
 *
 * @return Unused
 */
typedef Eina_Bool (*Ewk_View_Geolocation_Permission_Callback)(
    Evas_Object* ewk_view, Ewk_Geolocation_Permission_Request* request,
    void* user_data);

/**
 * Sets callback which will be called upon geolocation permission request. This
 * function can be used also to unset this callback. Do that by passing NULL as
 * callback param.
 *
 * @param ewk_view view object to set the callback to
 * @param callback callback function called upon geolocation permission request
 * @param user_data user_data passsed to set callback when called
 *
 * @note When callback is set by this function, it will be called insted of
 *       "geolocation,permission,request" smart callback.
 */
EXPORT_API void ewk_view_geolocation_permission_callback_set(
    Evas_Object* ewk_view, Ewk_View_Geolocation_Permission_Callback callback,
    void* user_data);

typedef Eina_Bool (*Ewk_View_User_Media_Permission_Callback)(
    Evas_Object* ewk_view,
    Ewk_User_Media_Permission_Request* user_media_permission_request,
    void* user_data);
EXPORT_API void ewk_view_user_media_permission_callback_set(
    Evas_Object* ewk_view, Ewk_View_User_Media_Permission_Callback callback,
    void* user_data);

/**
 * Callback for ewk_view_authentication_callback_set
 *
 * @param o the view object
 * @param auth_challenge Ewk_Auth_Challenge object to get the information about
 * authentication
 * @param user_data user data
 */
typedef void (*Ewk_View_Authentication_Callback)(
    Evas_Object* o, Ewk_Auth_Challenge* auth_challenge, void* user_data);

/**
 * Sets the callback authentication.
 *
 * @param o the view object
 * @param callback callback function to be called when the authentication is
 * called
 * @param user_data user data
 *
 * @note When callback is set by this function,
 *       class of Ewk_Auth_Challenge is passed by callback function.
 *
 * @see Ewk_View_Authentication_Callback
 */
EXPORT_API void ewk_view_authentication_callback_set(
    Evas_Object* o, Ewk_View_Authentication_Callback callback, void* user_data);

EXPORT_API Eina_Bool ewk_view_mode_set(Evas_Object* ewkView,
                                       Ewk_View_Mode view_mode);

/**
 * Requests the specified plain text string into the view object
 *
 * @note The mime type of document will be "text/plain".
 *
 * @return @c EINA_TRUE on successful request, @c EINA_FALSE on errors
 */
EXPORT_API Eina_Bool ewk_view_plain_text_set(Evas_Object* o,
                                             const char* plain_text);

/**
 * Requests for setting page visibility state.
 *
 * @param o view object to set the page visibility
 * @param page_visibility_state visible state of the page to set
 * @param initial_state @c EINA_TRUE if this function is called at page
 * initialization time,
 *                     @c EINA_FALSE otherwise
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_page_visibility_state_set(
    Evas_Object* o, Ewk_Page_Visibility_State page_visibility_state,
    Eina_Bool initial_state);

/**
 * Request to set the user agent with application name.
 *
 * @param o view object to set the user agent with application name
 *
 * @param application_name string to set the user agent
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_application_name_for_user_agent_set(
    Evas_Object* o, const char* application_name);

/**
 * Returns application name string.
 *
 * @param o view object to get the application name
 *
 * @return @c application name. The returned string @b should be freed by
 *         eina_stringshare_del() after use.
 */
EXPORT_API const char* ewk_view_application_name_for_user_agent_get(
    const Evas_Object* o);

/*
 * add custom header
 *
 * @param o view object to add custom header
 *
 * @param name custom header name to add the custom header
 *
 * @param value custom header value to add the custom header
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_custom_header_add(const Evas_Object* o,
                                                const char* name,
                                                const char* value);
/**
 * remove custom header
 *
 * @param o view object to remove custom header
 *
 * @param name custom header name to remove the custom header
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_custom_header_remove(const Evas_Object* o,
                                                   const char* name);
/**
 * clears all custom headers
 *
 * @param o view object to clear custom headers
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_custom_header_clear(const Evas_Object* o);

/**
 * Returns the evas image object of the specified viewArea of page
 *
 * The returned evas image object @b should be freed after use.
 *
 * @param o view object to get specified rectangle of cairo surface.
 * @param viewArea rectangle of cairo surface.
 * @param scaleFactor scale factor of cairo surface.
 * @param canvas canvas for creating evas image.
 *
 * @return newly allocated evas image object on sucess or @c 0 on failure.
 */
EXPORT_API Evas_Object* ewk_view_screenshot_contents_get(
    const Evas_Object* o, Eina_Rectangle viewArea, float scaleFactor,
    Evas* canvas);

/**
 * Makes request of evas image object of the specified viewArea of page
 * asynchronously
 *
 * The returned evas image object through async callback @b should be freed
 * after use.
 *
 * @param o view object to get specified rectangle of cairo surface.
 * @param viewArea rectangle of cairo surface.
 * @param scaleFactor scale factor of cairo surface.
 * @param canvas canvas for creating evas image.
 * @param callback result callback to get captured screenshot.
 * @param user_data will be passed when result_callback is called
 *    -I.e., user data will be kept until callback is called.
 *
 * @return @c EINA_TRUE on successful request, @c EINA_FALSE on errors.
 */
EXPORT_API Eina_Bool ewk_view_screenshot_contents_get_async(
    const Evas_Object* o, Eina_Rectangle viewArea, float scaleFactor,
    Evas* canvas, Ewk_Web_App_Screenshot_Captured_Callback callback,
    void* user_data);

/**
 * Start a server for inspecting web pages
 * This server will be used by Remote Web Browser to transfer messages over
 * network
 *
 * @param [o] view object to debug
 * @param [in] port It is a port number for the server. A free port on system
 * will be allocated if port is 0
 *
 * @return @c assigned port number on success or @c 0 on failure
 */
EXPORT_API unsigned int ewk_view_inspector_server_start(Evas_Object* o,
                                                        unsigned int port);

/**
 * Stop a server for inspecting web pages
 *
 * @param [o] view object to debug
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_view_inspector_server_stop(Evas_Object* o);

/**
 * Gets the possible scroll size of the given view.
 *
 * Possible scroll size is contents size minus the viewport size.
 *
 * @param o view object to get scroll size
 * @param w the pointer to store the horizontal size that is possible to scroll,
 *        may be @c 0
 * @param h the pointer to store the vertical size that is possible to scroll,
 *        may be @c 0
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise and
 *         values are zeroed
 */
EXPORT_API Eina_Bool ewk_view_scroll_size_get(const Evas_Object* o, int* w,
                                              int* h);

/**
 * Executes editor command.
 *
 * @param o view object to execute command
 * @param command editor command to execute
 * @param value the value to be passed into command
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_command_execute(Evas_Object* o,
                                              const char* command,
                                              const char* value);

/**
 * Retrieve the contents in plain text.
 *
 * @param o view object whose contents to retrieve.
 * @param callback result callback
 * @param user_data user data
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_view_plain_text_get(
    Evas_Object* o, Ewk_View_Plain_Text_Get_Callback callback, void* user_data);

/**
 * Get page contents as MHTML data
 *
 * @param o view object to get the page contents
 * @param callback callback function to be called when the operation is finished
 * @param user_data user data to be passed to the callback function
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_view_mhtml_data_get(
    Evas_Object* o, Ewk_View_MHTML_Data_Get_Callback callback, void* user_data);

typedef void (*Ewk_View_Hit_Test_Request_Callback)(Evas_Object* o, int x, int y,
                                                   int hit_test_mode,
                                                   Ewk_Hit_Test*,
                                                   void* user_data);

/**
 * Requests new hit test for given view object and point.
 *
 * @param o              view object to do hit test on
 * @param x              the horizontal position to query
 * @param y              the vertical position to query
 * @param hit_test_mode  the Ewk_Hit_Test_Mode enum value to query
 * @param callback       callback to be executed when hit test request was
 * finished
 *
 * @return EINA_TRUE if hit test request was queued, otherwise EINA_FALSE
 */
EXPORT_API Eina_Bool
ewk_view_hit_test_request(Evas_Object* o, int x, int y, int hit_test_mode,
                          Ewk_View_Hit_Test_Request_Callback, void* user_data);

/**
 * Deprecated.
 * Notify that notification is closed.
 *
 * @param notification_list list of Ewk_Notification pointer
 *        notification_list is freed in this function.
 *
 * @return this function will always return EINA_FALSE since it is deprecated
 *
 * @deprecated
 * @see ewk_notification_closed
 */
EINA_DEPRECATED EXPORT_API Eina_Bool
ewk_view_notification_closed(Evas_Object* o, Eina_List* notification_list);

/**
 * @deprecated Deprecated since Tizen 5.0. Manual encoding selection is removed
 * from upstream Sets the encoding and reloads the page.
 *
 * @param ewkView view to set the encoding
 * @param encoding the new encoding to set or @c 0 to restore the default one
 */
EINA_DEPRECATED EXPORT_API void ewk_view_encoding_custom_set(
    Evas_Object* ewkView, const char* encoding);

// #endif // ENABLE(TIZEN_INPUT_TAG_EXTENSION)

/**
 * Sets whether the ewk_view supports the mouse events or not.
 *
 * The ewk_view will support the mouse events if EINA_TRUE or not support the
 * mouse events otherwise. The default value is EINA_TRUE.
 *
 * @param o view object to enable/disable the mouse events
 * @param enabled a state to set
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_mouse_events_enabled_set(Evas_Object* o,
                                                       Eina_Bool enabled);

/**
 * Queries if the ewk_view supports the mouse events.
 *
 * @param o view object to query if the mouse events are enabled
 *
 * @return @c EINA_TRUE if the mouse events are enabled or @c EINA_FALSE
 * otherwise
 */
EXPORT_API Eina_Bool ewk_view_mouse_events_enabled_get(const Evas_Object* o);

typedef Eina_Bool (*Ewk_Orientation_Lock_Cb)(Evas_Object* o,
                                             Eina_Bool need_lock,
                                             int orientation, void* user_data);

/**
 * Deprecated
 * Sets callback of orientation lock function
 *
 * func will be called when screen lock is called or unlock is called.
 * When screen.lockOrientation is called, need_lock will be true and orientation
 * will be the flags which should be locked.
 * For example, when contents called 'screen.lockOrientation("portrait"),
 * orientation will be EWK_SCREEN_ORIENTATION_PORTRAIT_PRIMARY |
 * EWK_SCREEN_ORIENTATION_PORTRAIT_SECONDARY When screen.unlockOrientation is
 * called, need_lock will be false.
 *
 * @param o view object to set the callback of orientation
 * @param func callback function to be called when screen orientation is locked
 * or unlocked.
 * @param use_data user_data will be passsed when ewk_view_web_app_icon_get is
 * called
 *
 * @return current URI on success or @c 0 on failure
 */
EINA_DEPRECATED EXPORT_API void ewk_view_orientation_lock_callback_set(
    Evas_Object* o, Ewk_Orientation_Lock_Cb func, void* user_data);

/**
 * Sets the callback on the scale factor change.
 *
 * The given callback function will be called when the engine has successfully
 * changed the scale factor. It happens, for example, after ewk_view_scale_set
 * call but not only. The engine itself can change the scale while rendering
 * the content, for example, when ewk_settings_auto_fitting is turned on.
 *
 * @param o view object to set the callback
 * @param callback the callback funtion which will be called on scale change
 * @param user_data pointer to the data which will be passed while calling the
 * callback
 */
EXPORT_API void ewk_view_scale_changed_callback_set(
    Evas_Object* o, Ewk_View_Scale_Changed_Callback callback, void* user_data);

/**
 * Clears the highlight of searched text.
 *
 * @param o view object to find text
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on errors
 */
EXPORT_API Eina_Bool ewk_view_text_find_highlight_clear(Evas_Object* o);

/**
 * Sets whether the ewk_view supports the touch events or not.
 *
 * The ewk_view will support the touch events if @c EINA_TRUE or not support the
 * touch events otherwise. The default value is @c EINA_FALSE.
 *
 * @param o view object to enable/disable the touch events
 * @param enabled a state to set
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_touch_events_enabled_set(Evas_Object* o,
                                                       Eina_Bool enabled);

/**
 * Queries if the ewk_view supports the touch events.
 *
 * @param o view object to query if the touch events are enabled
 *
 * @return @c EINA_TRUE if the touch events are enabled or @c EINA_FALSE
 * otherwise
 */
EXPORT_API Eina_Bool ewk_view_touch_events_enabled_get(const Evas_Object* o);

/// Enum values containing Content Security Policy header types.
enum _Ewk_CSP_Header_Type {
  EWK_REPORT_ONLY,
  EWK_ENFORCE_POLICY,
  EWK_DEFAULT_POLICY
};
typedef enum _Ewk_CSP_Header_Type Ewk_CSP_Header_Type;

/**
 * Set received Content Security Policy data from web app
 *
 * @param o view object
 * @param policy Content Security Policy data
 * @param type Content Security Policy header type
 *
 */
EXPORT_API void ewk_view_content_security_policy_set(Evas_Object* o,
                                                     const char* policy,
                                                     Ewk_CSP_Header_Type type);

/**
 * When font-family is "Tizen", use system's Settings font as default
 * font-family
 *
 * @param o view object
 *
 */
EXPORT_API void ewk_view_use_settings_font(Evas_Object* o);

/*
 * Get cookies associated with an URL.
 *
 * @param o view object in which URL is opened.
 * @param url the url for which cookies needs to be obtained.
 *
 * @return @c character array containing cookies, @c NULL if no cookies are
 * found.
 *
 * The return character array has to be owned by the application and freed when
 * not required.
 */
EXPORT_API char* ewk_view_get_cookies_for_url(Evas_Object* o, const char* url);

/**
 * Sets whether to draw transparent background or not.
 *
 * @param o view object to enable/disable transparent background
 * @param enabled a state to set
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool
ewk_view_draws_transparent_background_set(Evas_Object* o, Eina_Bool enabled);

/**
 * @brief Queries if transparent background is enabled.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to get whether transparent background is enabled or
 * not
 *
 * @return @c EINA_TRUE on enable or @c EINA_FALSE on disable
 */
EXPORT_API Eina_Bool ewk_view_draws_transparent_background_get(Evas_Object* o);

/**
 * @brief Sets the background color and transparency of the view.
 *
 * @note Should be used after ewk_view_url_set().
 *
 * @param[in] o view object to change the background color
 * @param[in] r red color component [0..255]
 * @param[in] g green color component [0..255]
 * @param[in] b blue color component [0..255]
 * @param[in] a transparency [0..255]
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_bg_color_set(Evas_Object* o, int r, int g, int b,
                                           int a);

/**
 * set a font for browser application
 *
 * @param o view object
 *
 */
EXPORT_API void ewk_view_browser_font_set(Evas_Object* o);

/**
 * Load the error page which web page is not found.
 *
 * @param ewkView view object whose session needs to be stored.
 * @param ErrorUrl that could not be found.
 *
 * @return void
 */
EXPORT_API void ewk_view_not_found_error_page_load(Evas_Object* ewkView,
                                                   const char* ErrorUrl);

/**
 * Enable or disable supporting of the split scrolling for overflow scroll.
 *
 * @param ewkView view object to set the support of the split scrolling for
 * overflow scroll
 * @param enable @c EINA_TRUE to support split scrolling, @c EINA_FALSE not to
 * support
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_split_scroll_overflow_enabled_set(
    Evas_Object* ewkView, const Eina_Bool enabled);

/**
 * @brief Gets the staus of split scrolling supporting for overflow scroll.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to get the status of split scrolling supporting
 *
 * @return the status of split scrolling supporting
 */
EXPORT_API Eina_Bool
ewk_view_split_scroll_overflow_enabled_get(const Evas_Object* o);

/**
 * Deprecated.
 * Enable/disable focus ring.
 *
 * @note Focus ring is enabled by default but disabled for wrt on TV profile
 *
 * @param ewkView view object
 * @param enabled @c EINA_TRUE to enable the focus ring, @c EINA_FALSE to
 * disable
 *
 */
EINA_DEPRECATED EXPORT_API void ewk_view_draw_focus_ring_enable_set(
    Evas_Object* ewkView, Eina_Bool enable);

/**
 * Queries the current zoom factor of the page.
 *
 * @param o view object to get the zoom factor
 *
 * @return current zoom factor in use on success or @c -1.0 on failure
 */
EXPORT_API double ewk_view_page_zoom_get(const Evas_Object* o);

/**
 * Sets zoom factor of the current page.
 *
 * @note ewk_view_page_zoom_set internally might use older page than current
 *       one, if you called it immediately after ewk_view_url_set. To be safe
 *       from such race, use it from inside 'url,changed' callback, which can
 *       be registered on ewk_view.
 *
 * @param o view object to set the zoom level
 * @param zoom_factor a new level to set
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_view_page_zoom_set(Evas_Object* o, double zoom_factor);

/**
 * Creates a new EFL WebKit view object with Evas_Smart and Ewk_Context.
 *
 * @note The Evas_Object which inherits the ewk_view should create its
 *       Evas_Object using this API instead of evas_object_smart_add()
 *       because the default initialization for ewk_view is done in this API.
 *
 * @param e canvas object where to create the view object
 * @param smart Evas_Smart object. Its type should be EWK_VIEW_TYPE_STR
 * @param context Ewk_Context object which is used for initializing
 * @param pageGroup Ewk_Page_Group object which is used for initializing
 *
 * @return view object on success or @c NULL on failure
 */
EXPORT_API Evas_Object* ewk_view_smart_add(Evas* e, Evas_Smart* smart,
                                           Ewk_Context* context,
                                           Ewk_Page_Group* pageGroup);

/**
 * Callback for quota permission request feature.
 *
 * @param ewkView view object where quota permission was requested
 * @param request quota permission request object
 * @param user_data user_data passed to
 * ewk_view_quota_permission_request_callback_set
 */
typedef void (*Ewk_Quota_Permission_Request_Callback)(
    Evas_Object* ewkView, const Ewk_Quota_Permission_Request* request,
    void* user_data);

/**
 * Sets callback quota permission request.
 *
 * func will be called when page requests increased storage quota.
 *
 * @param ewkView view object to set the callback
 * @param func callback function to be called when quota permission is requested
 * @param use_data user_data will be passsed to callback function
 *
 * @return void
 */
EXPORT_API void ewk_view_quota_permission_request_callback_set(
    Evas_Object* ewkView, Ewk_Quota_Permission_Request_Callback callback,
    void* user_data);

/**
 * Set reply to quota permission request.
 *
 * Set @c EINA_TRUE if user accepts new quota size for origin
 * or @c EINA_FALSE if user doesn't accept new quota for origin.
 *
 * @param request view object to set the support of the split scrolling for
 * overflow scroll
 * @param enable @c EINA_TRUE allow quota size, @c EINA_FALSE
 *
 * @return void
 */
EXPORT_API void ewk_view_quota_permission_request_reply(
    const Ewk_Quota_Permission_Request* request, const Eina_Bool allow);

/**
 * Cancels quota permission request.
 *
 * @param request view object to set the support of the split scrolling for
 * overflow scroll
 *
 * @return void
 */
EXPORT_API void ewk_view_quota_permission_request_cancel(
    const Ewk_Quota_Permission_Request* request);

/*
 * Requests web login using password database.
 *
 * @param o view object
 *
 * @return void
 */
EXPORT_API Eina_Bool ewk_view_web_login_request(Evas_Object* ewkView);

/**
 * Sets the notification permission callback.
 *
 * @param o view object to request the notification permission
 * @param callback Ewk_View_Notification_Permission_Callback function to
 * notification permission
 * @param user_data user data
 */
EXPORT_API void ewk_view_notification_permission_callback_set(
    Evas_Object* o, Ewk_View_Notification_Permission_Callback callback,
    void* user_data);

/**
 * @brief Callback invoked when theme color is changed
 *
 * @details Ewk_View_Did_Change_Theme_Color_Callback callback allows host\n
 *          application to receive information regarding theme color change.\n
 *          Callback will be called in following cases:\n
 *          - no "theme-color" meta tag => "theme-color" meta tag\n
 *          - "theme-color" meta tag => different "theme-color" meta tag\n
 *          - "theme-color" meta tag => no "theme-color" meta tag\n
 *          In case of no "theme-color" meta tag @a r, @a g, @a b and @a a\n
 *          equal 0.
 *
 * @since_tizen 3.0
 *
 * @param[in] o View object for which callback was set
 * @param[in] r Red color component ranged from 0 to 255
 * @param[in] g Green color component ranged from 0 to 255
 * @param[in] b Blue color component ranged from 0 to 255
 * @param[in] a Opacity value ranged from 0 to 255
 * @param[in] user_data User data passed to\n
 *                      ewk_view_did_change_theme_color_callback_set
 *
 * @see ewk_view_did_change_theme_color_callback_set
 */
typedef void (*Ewk_View_Did_Change_Theme_Color_Callback)(Evas_Object* o, int r,
                                                         int g, int b, int a,
                                                         void* user_data);

/**
 * @brief Sets Ewk_View_Did_Change_Theme_Color_Callback
 *
 * @since_tizen 3.0
 *
 * @param[in] o View object to receive theme color change information
 * @param[in] callback New callback, NULL resets current callback
 * @param[in] user_data User data that will be passed to @a callback
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE
 *
 * @see Ewk_View_Did_Change_Theme_Color_Callback
 */
EXPORT_API Eina_Bool ewk_view_did_change_theme_color_callback_set(
    Evas_Object* o, Ewk_View_Did_Change_Theme_Color_Callback callback,
    void* user_data);

/**
 * @brief Callback invoked when save page is done
 *
 * @since_tizen 3.0
 *
 * @param[in] o View object for which callback was set
 * @param[in] file_path File path of saved page on success or @c NULL on failure
 * @param[in] user_data User data passed to ewk_view_save_page
 *
 * @see ewk_view_save_page_as_mhtml
 */
typedef void (*Ewk_View_Save_Page_Callback)(Evas_Object* o,
                                            const char* file_path,
                                            void* user_data);

/**
 * @brief Save current page as MHTML format
 *
 * @since_tizen 3.0
 *
 * @param[in] o View object which has page to save
 * @param[in] path File path where saved page should be placed\n
 *                 It can be full path of saved page or path of
 *                 existing directory\n
 *                 If path is existing directory, file name will be chosen\n
 *                 according to the title of the current page\n
 *                 (e.g. /path-to-directory/Google.mhtml)\n
 *                 If path has no extension, default extension(mhtml)\n
 *                 will be appended
 * @param[in] callback Callback to be called when the operation is finished
 * @param[in] user_data User data that will be passed to @a callback
 *
 * @return @c EINA_TRUE on successful request,\n
 *         otherwise @c EINA_FALSE
 *
 * @see Ewk_View_Save_Page_Callback
 */
EXPORT_API Eina_Bool ewk_view_save_page_as_mhtml(
    Evas_Object* o, const char* path, Ewk_View_Save_Page_Callback callback,
    void* user_data);

/*
 * @brief Set Reader mode enable
 *
 * @since_tizen 3.0
 *
 * @param[in] ewk_view view object to enable/disable the Reader mode
 * @param[in] enable a state to set
 *
 * @note Reader Mode support or not will be known after the ewk_view object\n
 *       received "reader,mode" callback from evas_object smart callback.
 */
EXPORT_API void ewk_view_reader_mode_set(Evas_Object* ewk_view,
                                         Eina_Bool enable);

/**
 * @brief Sets the height of top controls.
 *
 * @since_tizen 3.0
 *
 * @param[in] ewk_view view object to set height of top control
 * @param[in] top_height the height of the top controls in pixels
 * @param[in] bottom_height the height of the bottom controls in pixels
 *
 * @return @c EINA_TRUE on success @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_view_top_controls_height_set(Evas_Object* ewk_view,
                                                      size_t top_height,
                                                      size_t bottom_height);

/**
 * @brief Sets the state of top controls.
 *
 * @since_tizen 3.0
 *
 * @note EWK_TOP_CONTROL_BOTH for current to preserve the current position.
 *
 * @param[in] ewk_view view object to set state of top control
 * @param[in] constraint constrain the top controls to being shown or hidden
 * @param[in] current set current state
 * @param[in] animate whether or not to animate to the proper state
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_top_controls_state_set(
    Evas_Object* ewk_view, Ewk_Top_Control_State constraint,
    Ewk_Top_Control_State current, Eina_Bool animation);

/**
 * @brief Sets the visibility of main frame scrollbar.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object
 * @param[in] visible visibility of main frame scrollbar
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool
ewk_view_main_frame_scrollbar_visible_set(Evas_Object* o, Eina_Bool visible);

/**
 * @brief Set Default XHR LongPolling Timeout.
 *
 * @details XML Http Request by default does not have a timeout value. The
 *          timeout can be set using "timeout" field during creation of XHR
 *          object. This API allows for setting the timeout globally for
 *          whole View. Request timeout value will be calculated as minimum
 *          of argument of the API and value set in XHR field. If XHR field
 *          value is 0, value from API will be used.
 *
 * @since_tizen 3.0
 *
 * @param[in] o view object
 * @param[in] timeout XHR LongPolling timeout value (in seconds)
 */
EXPORT_API void ewk_view_session_timeout_set(Evas_Object* o,
                                             unsigned long timeout);

/**
 * @brief Enforces web page to close
 *
 * @since_tizen 3.0
 *
 * @param[in] o view
 *
 * @return @c EINA_TRUE on success @c EINA_FALSE otherwise
 *
 */
EXPORT_API Eina_Bool ewk_view_page_close(Evas_Object* o);

/**
 * @brief Requests the manifest data from a given URL.
 *
 * @since_tizen 3.0
 *
 * @param[in] o view object to request the manifest information
 * @param[in] callback Ewk_View_Request_Manifest_Callback function
 * @param[in] manifest_url a given URL
 * @param[in] host_url a host url used to resolve relative URLs
 *            located in manifest file
 *
 * @see Ewk_View_Request_Manifest_Callback
 */
EXPORT_API void ewk_view_request_manifest_from_url(
    Evas_Object* o, Ewk_View_Request_Manifest_Callback callback,
    void* user_data, const char* host_url, const char* manifest_url);

/**
 * Callback for @a ewk_view_app_installation_request_callback_set api.
 *
 * @since_tizen 4.0
 *
 * @param[in] o view object
 * @param[in] request app installation request
 * @param[in] user_data user data passed to
 *                      @a ewk_view_app_installation_request_callback_set api
 */
typedef void (*Ewk_App_Installation_Request_Callback)(
    Evas_Object* o, Ewk_App_Installation_Request* request, void* user_data);

/**
 * Set callback handling app installation request. It is called in reaction to
 * window.navigator.installApp() api being called.
 *
 * @since_tizen 4.0
 *
 * @param[in] o view object
 * @param[in] callback callback to be set; Can be null
 * @param[in] user_data user data to be passed to @a callback; Can be null
 */
EXPORT_API void ewk_view_app_installation_request_callback_set(
    Evas_Object* o, Ewk_App_Installation_Request_Callback callback,
    void* user_data);

/**
 * Sets the state to apply Blur Effect for widgets
 *
 * @since_tizen 3.0
 *
 * This function let WebKit knows that the widget is tizen 2.x widget.
 * WebKit will query  this information to decide to apply Blur effect or not.
 * @param[in] o view object to set the visibility state.
 * @param[in] state @c EINA_TRUE if tizen 2.x widget
 *              @c EINA_FALSE otherwise.
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure.
 */
EXPORT_API void ewk_view_mirrored_blur_set(Evas_Object* o, Eina_Bool state);

/**
 * Sets whether the ewk_view renders to offscreen buffer or not.
 *
 * @since_tizen 5.0
 *
 * @param[in] o view object
 * @param[in] enabled a state to set
 */
EXPORT_API void ewk_view_offscreen_rendering_enabled_set(Evas_Object* o,
                                                         Eina_Bool enabled);

/**
 * Sets the window object which is used for IME.
 *
 * @since_tizen 5.5
 *
 * @param[in] o view object
 * @param[in] window the top-level window object
 */
EXPORT_API void ewk_view_ime_window_set(Evas_Object* o, void* window);

/**
 * Sends mouse down event.
 *
 * @since_tizen 6.0
 *
 * @param[in] o view object
 * @param[in] button button type
 * @param[in] x horizontal position of mouse event
 * @param[in] y vertical position of mouse event
 */
EXPORT_API void ewk_view_feed_mouse_down(Evas_Object* o,
                                         Ewk_Mouse_Button_Type button, int x,
                                         int y);

/**
 * Sends mouse up event.
 *
 * @since_tizen 6.0
 *
 * @param[in] o view object
 * @param[in] button button type
 * @param[in] x horizontal position of mouse event
 * @param[in] y vertical position of mouse event
 */
EXPORT_API void ewk_view_feed_mouse_up(Evas_Object* o,
                                       Ewk_Mouse_Button_Type button, int x,
                                       int y);

/**
 * Sends mouse move event.
 *
 * @since_tizen 6.0
 *
 * @param[in] o view object
 * @param[in] x horizontal position of mouse event
 * @param[in] y vertical position of mouse event
 */
EXPORT_API void ewk_view_feed_mouse_move(Evas_Object* o, int x, int y);

/**
 * Sends mouse wheel event.
 *
 * @since_tizen 6.0
 *
 * @param[in] o view object
 * @param[in] y_direction wheel mouse direction
 * @param[in] step how much mouse wheel was scrolled up or down
 * @param[in] x horizontal position of mouse event
 * @param[in] y vertical position of mouse event
 */
EXPORT_API void ewk_view_feed_mouse_wheel(Evas_Object* o, Eina_Bool y_direction,
                                          int step, int x, int y);

/**
 * Sends mouse out event.
 *
 * @since_tizen 6.0
 *
 * @param[in] o view object
 */
EXPORT_API void ewk_view_feed_mouse_out(Evas_Object* o);

#ifdef __cplusplus
}
#endif
#endif  // ewk_view_internal_h
