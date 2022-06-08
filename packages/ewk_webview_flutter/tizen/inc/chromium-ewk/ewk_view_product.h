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
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
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
 * @file    ewk_view_product.h
 * @brief   Chromium main smart object.
 *
 * This object provides view related APIs of Chromium to EFL object.
 */

#ifndef ewk_view_product_h
#define ewk_view_product_h

#include "ewk_context_product.h"
#include "ewk_media_playback_info_product.h"
#include "ewk_value_product.h"
#include "ewk_view_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A callback to check whether allowed to run mixed content or not
 *
 * @param ewkView view object
 * @param user_data user_data will be passed when callback is called
 * @return true: allow to run mixed content. false: not allow to run mixed
 * content
 */
typedef Eina_Bool (*Ewk_View_Run_Mixed_Content_Confirm_Callback)(
    Evas_Object* ewkView, void* user_data);

/**
 * @brief Creates a new EFL Chromium view object.
 *
 * @since_tizen 2.3
 *
 * @param[in] e canvas object where to create the view object
 * @param[in] data a pointer to data to restore session data
 * @param[in] length length of session data to restore session data
 *
 * @return view object on success or @c NULL on failure
 */
EXPORT_API Evas_Object* ewk_view_add_with_session_data(Evas* e,
                                                       const char* data,
                                                       unsigned length);

/**
 * @brief Gets the reference object for frame that represents the main frame.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to get main frame
 *
 * @return frame reference of frame object on success, or NULL on failure
 */
EXPORT_API Ewk_Frame_Ref ewk_view_main_frame_get(Evas_Object* o);

/**
 * @brief Reply of javascript alert popup
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object
 */
EXPORT_API void ewk_view_javascript_alert_reply(Evas_Object* o);

/**
 * @brief Reply of javascript confirm popup
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object
 * @param[in] result result of javascript confirm popup
 */
EXPORT_API void ewk_view_javascript_confirm_reply(Evas_Object* o,
                                                  Eina_Bool result);

/**
 * @brief Reply of javascript prompt popup
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object
 * @param[in] result entered characters of javascript prompt popup
 */
EXPORT_API void ewk_view_javascript_prompt_reply(Evas_Object* o,
                                                 const char* result);

/**
 * @brief Callback for before unload popup
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object
 * @param[in] message the contents of before unload popup
 * @param[in] user_data user data
 */
typedef Eina_Bool (*Ewk_View_Before_Unload_Confirm_Panel_Callback)(
    Evas_Object* o, const char* message, void* user_data);

/**
 * @brief Sets callback of before unload popup
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to set the callback
 * @param[in] callback callback function for before unload popoup
 * @param[in] user_data user data
 */
EXPORT_API void ewk_view_before_unload_confirm_panel_callback_set(
    Evas_Object* o, Ewk_View_Before_Unload_Confirm_Panel_Callback callback,
    void* user_data);

/**
 * @brief Reply of before unload popup
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object
 * @param[in] result result of before unload popup
 */
EXPORT_API void ewk_view_before_unload_confirm_panel_reply(Evas_Object* o,
                                                           Eina_Bool result);

/**
 * @brief Sets callback of getting application cache permission.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to set the callback of application cache permission
 * @param[in] callback function to be called when application cache need to
 *            get permission
 * @param[in] user_data user data
 */
EXPORT_API void ewk_view_application_cache_permission_callback_set(
    Evas_Object* o, Ewk_View_Applicacion_Cache_Permission_Callback callback,
    void* user_data);

/**
 * @brief Application cache permission confirm popup reply
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to reply permission confirm popup
 * @param[in] allow of response
 */
EXPORT_API void ewk_view_application_cache_permission_reply(Evas_Object* o,
                                                            Eina_Bool allow);

/**
 * @brief Set to callback to controll unfocus operation from the arrow of
 *        h/w keyboard.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object
 * @param[in] callback callback to controll unfocus operation from the arrow of
 *            h/w keyboard
 * @param[in] user_data user data
 */
EXPORT_API void ewk_view_unfocus_allow_callback_set(
    Evas_Object* o, Ewk_View_Unfocus_Allow_Callback callback, void* user_data);

/**
 * @brief Set to callback to show or hide the notification of bluetooth mic to
 * user.
 *
 * @since_tizen 5.0
 *
 * @param[in] o view object
 * @param[in] callback to show or hide the notification
 * @param[in] user_data user_data will be passed when result_callback is
 *            called\n -I.e., user data will be kept until callback is called
 */
EXPORT_API void ewk_view_smartrc_show_mic_notification_callback_set(
    Evas_Object* o, Ewk_View_SmartRC_Mic_Notification_Callback callback,
    void* user_data);

/**
 * @brief Requests loading the given contents.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to load document
 * @param[in] html what to load
 * @param[in] base_uri base uri to use for relative resources, may be @c 0,\n
 *        if provided @b must be an absolute uri
 *
 * @return @c EINA_TRUE on successful request, @c EINA_FALSE on errors
 */
EXPORT_API Eina_Bool ewk_view_html_contents_set(Evas_Object* o,
                                                const char* html,
                                                const char* base_uri);

/**
 * @brief Callback for ewk_view_cache_image_get
 *
 * @since_tizen 3.0
 *
 * @param[in] o view object
 * @param[in] image_url url of the image in the cache
 * @param[in] image cache image @b should be freed after use
 * @param[in] user_data user data
 */
typedef void (*Ewk_View_Cache_Image_Get_Callback)(Evas_Object* o,
                                                  const char* image_url,
                                                  Evas_Object* image,
                                                  void* user_data);

/**
 * @brief Asynchronous request for get the cache image specified in url.
 *
 * @since_tizen 3.0
 *
 * @param[in] o view object
 * @param[in] image_url url of the image in the cache
 * @param[in] canvas canvas for creating evas image
 * @param[in] callback result callback to get cache image
 * @param[in] user_data user_data will be passed when @a callback is called
 *
 * @return @c EINA_TRUE on successful request, @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_cache_image_get(
    const Evas_Object* o, const char* image_url, Evas* canvas,
    Ewk_View_Cache_Image_Get_Callback callback, void* user_data);

/**
 * @brief Requests for getting web application capable.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object
 * @param[in] callback result callback to get web database quota
 * @param[in] user_data user_data will be passed when result_callback is
 *            called\n -I.e., user data will be kept until callback is called
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_web_application_capable_get(
    Evas_Object* o, Ewk_Web_App_Capable_Get_Callback callback, void* user_data);

/**
 * @brief Requests for getting web application icon string.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object
 * @param[in] callback result callback to get web database quota
 * @param[in] user_data user_data will be passed when result_callback is
 *            called\n -I.e., user data will be kept until callback is called
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_web_application_icon_url_get(
    Evas_Object* o, Ewk_Web_App_Icon_URL_Get_Callback callback,
    void* user_data);

/**
 * @brief Requests for getting web application icon list of
 *        Ewk_Web_App_Icon_Data.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object
 * @param[in] callback result callback to get web application icon urls
 * @param[in] user_data user_data will be passed when result_callback is
 *            called\n -I.e., user data will be kept until callback is called
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_web_application_icon_urls_get(
    Evas_Object* o, Ewk_Web_App_Icon_URLs_Get_Callback callback,
    void* user_data);

/**
 * @brief Get the whole history(whole back & forward list) associated with this
 *        view.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to get the history(whole back & forward list)
 *
 * @return a newly allocated history of @b newly allocated item\n
 *         instance. This memory of each item must be released with\n
 *         ewk_history_free() after use
 *
 * @see ewk_history_free()
 */
EXPORT_API Ewk_History* ewk_view_history_get(Evas_Object* o);

/**
 * @brief Gets the selection ranges
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to get theselection ranges
 * @param[out] left_rect the start lect(left rect) of the selection ranges
 * @param[out] right_rect the end lect(right rect) of the selection ranges
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_text_selection_range_get(
    Evas_Object* o, Eina_Rectangle* left_rect, Eina_Rectangle* right_rect);

/**
 * @brief Sets the focused input element value
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to send the value
 * @param[in] value the string value to be set
 */
EXPORT_API void ewk_view_focused_input_element_value_set(Evas_Object* o,
                                                         const char* value);

/**
 * @brief Gets the focused input element's value
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to get the value
 *
 * @return focused input element's value on success or NULL on failure
 */
EXPORT_API const char* ewk_view_focused_input_element_value_get(Evas_Object* o);

/**
 * @brief Selects index of current popup menu.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object contains popup menu
 * @param[in] index index of item to select
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure (probably\n
 *         popup menu is not selected or index is out of range)
 */
EXPORT_API Eina_Bool ewk_view_popup_menu_select(Evas_Object* o,
                                                unsigned int index);

/**
 * @brief Selects Multiple indexes  of current popup menu.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object contains popup menu.
 * @param[in] changed_list  list of item selected and deselected
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure (probably\n
 *         popup menu is not selected or index is out of range)
 */
EXPORT_API Eina_Bool
ewk_view_popup_menu_multiple_select(Evas_Object* o, Eina_Inarray* changed_list);

/*
 * @brief Sets the user chosen color. To be used when implementing a color
 *        picker.
 *
 * @details The function should only be called when a color has been requested
 *          by the document.\n If called when this is not the case or when the
 *          input picker has been dismissed, this\n function will fail and
 *          return EINA_FALSE.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object contains color picker
 * @param[in] r red channel value to be set
 * @param[in] g green channel value to be set
 * @param[in] b blue channel value to be set
 * @param[in] a alpha channel value to be set
 *
 * @return @c EINA_TRUE on success @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_view_color_picker_color_set(Evas_Object* o, int r,
                                                     int g, int b, int a);

/**
 * @brief Feeds the touch event to the view.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to feed touch event
 * @param[in] type the type of touch event
 * @param[in] points a list of points (Ewk_Touch_Point) to process
 * @param[in] modifiers an Evas_Modifier handle to the list of modifier keys\n
 *        registered in the Evas. Users can get the Evas_Modifier from the
 *        Evas\n using evas_key_modifier_get() and can set each modifier key
 *        using\n evas_key_modifier_on() and evas_key_modifier_off()
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_feed_touch_event(Evas_Object* o,
                                               Ewk_Touch_Event_Type type,
                                               const Eina_List* points,
                                               const Evas_Modifier* modifiers);

/**
 * Creates a type name for the callback function used to get the background
 * color.
 *
 * @param o view object
 * @param r red color component
 * @param g green color component
 * @param b blue color component
 * @param a transparency
 * @param user_data user data will be passed when ewk_view_bg_color_get is
 * called
 */
typedef void (*Ewk_View_Background_Color_Get_Callback)(Evas_Object* o, int r,
                                                       int g, int b, int a,
                                                       void* user_data);

/**
 * Gets the background color and transparency of the view.
 *
 * @param o view object to get the background color from
 * @param callback callback function
 * @param user_data user data will be passed when the callback is called
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 *
 *  On success the background color of the view object o is retrieved
 *  in the callback function
 */
EXPORT_API Eina_Bool ewk_view_bg_color_get(
    Evas_Object* o, Ewk_View_Background_Color_Get_Callback callback,
    void* user_data);

/**
 * Callback for ewk_view_main_frame_scrollbar_visible_get
 *
 * @param o view object
 * @param visibility visibility of main frame scrollbar
 * @param user_data user data passed to
 * ewk_view_main_frame_scrollbar_visible_get
 */
typedef void (*Ewk_View_Main_Frame_Scrollbar_Visible_Get_Callback)(
    Evas_Object* o, Eina_Bool visible, void* user_data);

/**
 * @brief Gets the visibility of main frame scrollbar.
 *
 * @since_tizen 3.0
 *
 * @param[in] o view object
 * @param callback callback function
 * @param user_data user data will be passed when the callback is caller
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 *
 *  On success the visibility of the scrollbar of the view object o is retrieved
 *  in the callback function
 */
EXPORT_API Eina_Bool ewk_view_main_frame_scrollbar_visible_get(
    Evas_Object* view,
    Ewk_View_Main_Frame_Scrollbar_Visible_Get_Callback callback,
    void* user_data);

/**
 * @brief Gets the session data to be saved in a persistent store on
 *        browser exit
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object whose session needs to be stored.
 * @param[in] data out parameter session data
 * @param[in] length out parameter length of session data
 */
EXPORT_API void ewk_view_session_data_get(Evas_Object* o, const char** data,
                                          unsigned* length);

/**
 * @brief Reloads the current page's document without cache.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to reload current document
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_view_reload_bypass_cache(Evas_Object* o);

/**
 * @brief Creates a new hit test for the given veiw object and point.
 *
 * @since_tizen 2.3
 *
 * @remarks The returned object should be freed by ewk_hit_test_free().
 *
 * @param[in] o view object to do hit test on
 * @param[in] x the horizontal position to query
 * @param[in] y the vertical position to query
 * @param[in] hit_test_mode the #Ewk_Hit_Test_Mode enum value to query
 *
 * @return a newly allocated hit test on success, @c 0 otherwise
 */
EXPORT_API Ewk_Hit_Test* ewk_view_hit_test_new(Evas_Object* o, int x, int y,
                                               int hit_test_mode);

/**
 * Create PDF file of page contents
 *
 * @param o view object to get page contents.
 * @param width the suface width of PDF file.
 * @param height the suface height of PDF file.
 * @param fileName the file name for creating PDF file.
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
/* This return value is status of the request not the status of actual
 * operation. There should be some callback to get the actual status or reason
 * of failure.
 */
EXPORT_API Eina_Bool ewk_view_contents_pdf_get(Evas_Object* o, int width,
                                               int height,
                                               const char* fileName);

/**
 * Requests for setting callback function
 *
 * @param ewkView view object
 * @param user_data user_data will be passed when callback is called
 * @param callback callback function
 */
EXPORT_API void ewk_view_run_mixed_content_confirm_callback_set(
    Evas_Object* ewkView, Ewk_View_Run_Mixed_Content_Confirm_Callback callback,
    void* user_data);

/**
 * Returns the current favicon of view object.
 *
 * @param item view object to get current icon URL
 *
 * @return current favicon on success or @c NULL if unavailable or on failure.
 * The returned Evas_Object needs to be freed after use.
 */
EXPORT_API Evas_Object* ewk_view_favicon_get(const Evas_Object* ewkView);

/**
 * To resume new url network loading
 *
 * @param item view object to resume new url loading
 *
 */
EXPORT_API void ewk_view_resume_network_loading(Evas_Object* ewkView);

EXPORT_API void ewk_view_poweroff_suspend(Evas_Object* item);

/**
 * To suspend all url loading
 *
 * @param item view object to suspend url loading
 *
 */
EXPORT_API void ewk_view_suspend_network_loading(Evas_Object* ewkView);

/**
 * This function should be use for browser edge scroll.
 * It can also be used when the mouse pointer is out of webview.
 * Scrolls webpage of view by dx and dy.
 *
 * @param item view object to scroll
 * @param dx horizontal offset to scroll
 * @param dy vertical offset to scroll
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 */
EXPORT_API Eina_Bool ewk_view_edge_scroll_by(Evas_Object* item, int dx, int dy);

/**
 * Allow a browser to set its own cursor by setting a flag
 * which prevents setting a default web page cursor.
 *
 * @param ewkView view object
 * @param enable EINA_TRUE - prevent update of cursor by engine
 *               EINA_FALSE - allow for update of cursor by engine
 */
EXPORT_API void ewk_view_set_cursor_by_client(Evas_Object* ewkView,
                                              Eina_Bool enable);

/**
 * Reply of running mixed content or not
 *
 * @param ewkView view object
 * @param result reply
 */
EXPORT_API void ewk_view_run_mixed_content_confirm_reply(Evas_Object* ewkView,
                                                         Eina_Bool result);

/**
 * Sets the cover-area (soon rect) multiplier.
 *
 * @param ewkView view object
 * @param cover_area_multiplier the multiplier of cover-area.
 */
EXPORT_API void ewk_view_tile_cover_area_multiplier_set(
    Evas_Object* ewkView, float cover_area_multiplier);

/**
 * Set to enabled/disabled clear tiles on hide.
 *
 * @param ewkView view object
 * @param enabled/disabled a state to set
 *
 */
EXPORT_API void ewk_view_clear_tiles_on_hide_enabled_set(Evas_Object* ewkView,
                                                         Eina_Bool enable);

/**
 * @brief Callback for ewk_view_is_video_playing
 *
 * @param[in] o the view object
 * @param[in] is_playing video is playing or not
 * @param[in] user_data user_data will be passsed when ewk_view_is_video_playing
 * is called
 */
typedef void (*Ewk_Is_Video_Playing_Callback)(Evas_Object* o,
                                              Eina_Bool is_playing,
                                              void* user_data);

/**
 * @brief Asynchronous request for check if there is a video playing in the
 * given view
 *
 * @param[in] o The view object
 * @param[in] callback result callback to get web application capable
 * @param[in] user_data user_data will be passed when result_callback is called
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_is_video_playing(
    Evas_Object* o, Ewk_Is_Video_Playing_Callback callback, void* user_data);

/**
 * Callback for ewk_view_stop_video
 *
 * @param o view object
 * @param is_stopped video is stopped or not
 * @param user_data user_data will be passsed when ewk_view_stop_video is called
 */
typedef void (*Ewk_Stop_Video_Callback)(Evas_Object* o, Eina_Bool is_stopped,
                                        void* user_data);

/**
 * Asynchronous request for stopping any playing video in the given view
 *
 * @param[in] o The view object
 * @param[in] callback result callback to get web application capable
 * @param[in] user_data user_data will be passed when result_callback is called
 *
 * @return @c EINA_TRUE if any video was stopped or @c EINA_FALSE is there was
 * no active video
 */
EXPORT_API Eina_Bool ewk_view_stop_video(Evas_Object* o,
                                         Ewk_Stop_Video_Callback callback,
                                         void* user_data);

/**
 * @brief Sets the support of video hole and video window, Use H/W overlay for
 * performance of video output
 *
 * @since_tizen 3.0
 *
 * @param[in] o the view object
 * @param[in] o the top-level window object
 * @param[in] enable EINA_TRUE to set on support the video hole,
 *            EINA_FALSE otherwise
 * @param[in] enable EINA_TRUE to set on the video window of video hole,
 *            EINA_FALSE to set on the video windowless of video hole
 *
 * @return return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_set_support_video_hole(Evas_Object* ewkView,
                                                     void* window,
                                                     Eina_Bool enable,
                                                     Eina_Bool isVideoWindow);

/**
 * @brief Sets the support of canvas hole, Use H/W overlay for video quality of
 * WebGL 360 degree. Also, The WebBrowser provisionally want to show plane 360
 * video through canvas hole.
 *
 * @since_tizen 3.0
 *
 * @note Should be used after ewk_view_url_set().
 *
 * @param[in] o the view object
 * @param[in] url string (ex. "youtube.com")
 *
 * @return return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_set_support_canvas_hole(Evas_Object* ewkView,
                                                      const char* url);

/**
 * Callback for the generic sync call.
 * It requests for performing operation/call giving its name. Arguments
 * and return value is operation/call specific.
 *
 * @param[in] name requested call name
 * @param[in] arguments call argumets, format is defined by opertion itself
 * @param[in] user_data user_data will be passed when result_callback is called
 *
 * @return return value from the call, format is defind by operation itself
 */
typedef Ewk_Value (*Generic_Sync_Call_Callback)(const char* name,
                                                Ewk_Value arguments,
                                                void* user_data);

/**
 * Sets the function pointer for the generic sync call
 *
 * @param ewk_view view object to set the function pointer in
 * @param cb pointer to the function
 * @param user_data pointer to user data to be passed to the function when
 *        it's being called
 */
EXPORT_API void ewk_view_widget_pepper_extension_callback_set(
    Evas_Object* ewk_view, Generic_Sync_Call_Callback cb, void* user_data);

/**
 * Sets the pepper widget extension info
 *
 * @param ewk_view view object to set the info in
 * @param widget_pepper_ext_info the Ewk_Value containing the information
 */
EXPORT_API void ewk_view_widget_pepper_extension_info_set(
    Evas_Object* ewk_view, Ewk_Value widget_pepper_ext_info);

/**
 * @brief Sets the support of 4K video, Customize the device pixel ratio for
 * video plane.
 *
 * @since_tizen 3.0
 *
 * @note Should be used after ewk_view_url_set().
 *
 * @param[in] o the view object
 * @param[in] o enabled a state to set
 *
 * @return return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool
ewk_view_set_custom_device_pixel_ratio(Evas_Object* ewkView, Eina_Bool enabled);

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
 * Set the translated url to media player.
 *
 * @param ewkView view object
 * @param url string
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_media_translated_url_set(Evas_Object* ewkView,
                                                  const char* url);

/**
 * Set app is preload type or not.
 *
 * @param ewkView view object
 * @param is_preload if app is preload type
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_app_preload_set(Evas_Object* ewkView,
                                              Eina_Bool is_preload);

/**
 * Set app enable marlin or not.
 *
 * @param ewkView view object
 * @param is_enable   if app enable marlin drm
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_marlin_enable_set(Evas_Object* ewkView,
                                                Eina_Bool is_enable);

/**
 * Sets whitelisted DRM key systems. Passed key systems will be available
 * through EME. Other systems even if available in the platform will be
 * unavailable through EME
 *
 * @param ewkView View object
 * @param list Key system names
 * @param list_size Key system count
 */
EXPORT_API Eina_Bool ewk_view_key_system_whitelist_set(Evas_Object* ewkView,
                                                       const char** list,
                                                       unsigned list_size);

/**
 * Sets the active DRM system identifier as provided by the HbbTV application.
 *
 * @param ewkView View object
 * @param drm_system_id Identifier of requested DRM system
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_active_drm_set(Evas_Object* view,
                                             const char* drm_system_id);

/**
 * Inform webengine about decoder used by broadcast for dual decoding.
 *
 * @param view View object
 * @param decoder Identifier of used decoder
 */
EXPORT_API void ewk_view_broadcast_decoder_set(Evas_Object* view,
                                               Ewk_Hardware_Decoders decoder);

/**
 * Set the selected text track language to media player.
 *
 * @param ewkView view object
 * @param lang_list comma separated three_digit_language code. (For example
 * "eng,deu")
 *
 */
EXPORT_API void ewk_media_set_subtitle_lang(Evas_Object* ewkView,
                                            const char* lang_list);

/**
 * Set parental rating result to media player.
 *
 * @param ewkView view object
 * @param url   media url
 * @param is_pass  authentication result true/false
 *
 */
EXPORT_API void ewk_media_set_parental_rating_result(Evas_Object* ewkView,
                                                     const char* url,
                                                     Eina_Bool is_pass);

/**
 * Set the if use high bit rate to media player.
 *
 * @param ewkView view object
 * @param is_high  if app use high bit rate
 *
 */
EXPORT_API void ewk_media_start_with_high_bit_rate(Evas_Object* ewkView,
                                                   Eina_Bool is_high_bitrate);

/**
 * @brief Sends key event.
 *
 * @since_tizen 2.4
 *
 * @param[in] o The view object
 * @param[in] key_event Evas_Event_Key_Down struct or Evas_Event_Key_Up struct
 * @param[in] isPress EINA_TRUE: keydown, EINA_FALSE: keyup
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_send_key_event(Evas_Object* o, void* key_event,
                                             Eina_Bool is_press);

/**
 * @brief Sets whether the ewk_view supports the key events or not.
 *
 * @since_tizen 2.4
 *
 * @note Should be used after ewk_view_url_set().
 *
 * @remarks The ewk_view will support the key events if EINA_TRUE or not support
 * the key events otherwise. The default value is EINA_TRUE.
 *
 * @param[in] o The view object
 * @param[in] enabled a state to set
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_key_events_enabled_set(Evas_Object* o,
                                                     Eina_Bool enabled);

enum Ewk_Scrollbar_Orientation {
  EWK_HORIZONTAL_SCROLLBAR = 0,
  EWK_VERTICAL_SCROLLBAR
};

typedef enum Ewk_Scrollbar_Orientation Ewk_Scrollbar_Orientation;

struct Ewk_Scrollbar_Data {
  Ewk_Scrollbar_Orientation orientation; /**< scrollbar orientation */
  Eina_Bool focused;                     /**< isFocused */
};

typedef struct Ewk_Scrollbar_Data Ewk_Scrollbar_Data;

/**
 * @brief Adds an item to back forward list
 *
 * @since_tizen 2.4
 *
 * @param[in] o The view object
 * @param[in] item The back-forward list item instance
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_add_item_to_back_forward_list(
    Evas_Object* o, const Ewk_Back_Forward_List_Item* item);

/**
 * @brief Load the specified @a html string as the content of the view
 * overriding current history entry. Can be used to ignore the malicious page
 * while navigation backward/forward.
 *
 * @since_tizen 3.0
 *
 * @param[in] view object to load the HTML into
 * @param[in] html HTML data to load
 * @param[in] base_url Base URL used for relative paths to external objects
 *            (optional)
 * @param[in] unreachable_url URL that could not be reached (optional)
 *
 * @return @c EINA_TRUE if it the HTML was successfully loaded,
 *         @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_view_html_string_override_current_entry_load(
    Evas_Object* view, const char* html, const char* base_uri,
    const char* unreachable_url);

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
 * Clears the highlight of searched text.
 *
 * @param o view object to find text
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on errors
 */
EXPORT_API Eina_Bool ewk_view_text_find_highlight_clear(Evas_Object* o);

/**
 * Counts the given string in the document.
 *
 * This does not highlight the matched string and just count the matched
 * string.\n
 *
 * As the search is carried out through the whole document,\n
 * only the following #Ewk_Find_Options are valid.\n
 *  - EWK_FIND_OPTIONS_NONE\n
 *  - EWK_FIND_OPTIONS_CASE_INSENSITIVE\n
 *  - EWK_FIND_OPTIONS_AT_WORD_START\n
 *  - EWK_FIND_OPTIONS_TREAT_MEDIAL_CAPITAL_AS_WORD_START\n
 *
 * The "text,found" callback will be called with the number of matched string.
 *
 * @since_tizen 2.3
 *
 * @param o view object to find text
 * @param text text to find
 * @param options options to find
 * @param max_match_count maximum match count to find, unlimited if 0
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on errors
 */
EXPORT_API Eina_Bool ewk_view_text_matches_count(Evas_Object* o,
                                                 const char* text,
                                                 Ewk_Find_Options options,
                                                 unsigned max_match_count);

/**
 * Gets the current text zoom level.
 *
 * @param o view object to get the zoom level
 *
 * @return current zoom level in use on success or @c -1.0 on failure
 */
EXPORT_API double ewk_view_text_zoom_get(const Evas_Object* o);

/**
 * Sets the current text zoom level.
 *
 * @param o view object to set the zoom level
 * @param textZoomFactor a new level to set
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_view_text_zoom_set(Evas_Object* o,
                                            double text_zoom_factor);

/**
 * @brief Draw the evas image object for the VoiceManager
 *
 * @since_tizen 3.0
 *
 * @param[in] view the view object
 * @param[in] image evas image obejct for drawing
 * @param[in] rect rectangle of image object for drawing
 */
EXPORT_API void ewk_view_voicemanager_label_draw(Evas_Object* view,
                                                 Evas_Object* image,
                                                 Eina_Rectangle rect);

/**
 * @brief Hide and remove all labels for the VoiceManager
 *
 * @details All labels are cleared on mouse down.
 *
 * @since_tizen 3.0
 *
 * @param[in] view the view object
 */
EXPORT_API void ewk_view_voicemanager_labels_clear(Evas_Object* view);

/**
 * This api is used for Canal+ App and HBB TV Application.
 * They need to access various url required a client authentication while the
 * apps is running. So when XWalk call this API with host and related cert path,
 * We store these information to map. After that, When we get the "Certificate
 * Request" packet from server, We find matched cert path to host in the map.
 *
 * @param ewkView view object to add host and cert path to the map
 * @param host host that required client authentication
 * @param cert_path the file path stored certificate
 *
 */
EXPORT_API void ewk_view_add_dynamic_certificate_path(
    const Evas_Object* ewkView, const char* host, const char* cert_path);

/**
 * @brief Request to set the atk usage set by web app(config.xml).
 *
 * Some TV apps use WebSpeech instead of use ATK for regulation U.S.FCC
 *
 * @since_tizen 3.0 @if TV   @endif
 *
 * @param[in] o View object to set the atk use.
 * @param[in] enable EINA_TRUE to set on the atk use.
 *            EINA_FALSE makes atk not to use, but app use WebSpeech instead of
 * ATK.
 */
EXPORT_API void ewk_view_atk_deactivation_by_app(Evas_Object* view,
                                                 Eina_Bool enable);

typedef enum {
  EWK_TTS_MODE_DEFAULT = 0, /**< Default mode for normal application */
  EWK_TTS_MODE_NOTIFICATION =
      1, /**< Notification mode(it has same behavior with EWK_TTS_MODE_DEFAULT.
            not supported in vd) */
  EWK_TTS_MODE_SCREEN_READER =
      2 /**< Accessibiliity mode(TTS works only for accessibility mode) */
} ewk_tts_mode;

/**
 * @brief Sets tts mode
 * up to tizen 4.0(in VD), default tts mode is EWK_TTS_MODE_SCREEN_READER.
 * so TTS api disabled when accessibility mode turn off.
 * this api provided to use tts api in none accessibility mode
 * (tts mode decided in chromium's init time. so it should be called in init
 * time) tts mode affect to below web apis speech_syntesis
 *
 * @since_tizen 4.0 @if TV   @endif
 *
 * @param[in] o View object to set.
 * @param[in] ewk_tts_mode.
 *
 */
EXPORT_API Eina_Bool ewk_view_tts_mode_set(Evas_Object* view,
                                           ewk_tts_mode tts_mode);

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
EXPORT_API char* ewk_view_cookies_get(Evas_Object* o, const char* url);

/**
 * @internal
 * @brief Callback for ewk_view_notification_show_callback_set
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to request the notification show
 * @param[in] notification Ewk_Notification object to get the information about
 * notification show request
 * @param[in] user_data user data
 */
typedef Eina_Bool (*Ewk_View_Notification_Show_Callback)(
    Evas_Object* o, Ewk_Notification* notification, void* user_data);

/**
 * @internal
 * @brief Sets the notification show callback.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to request the notification show
 * @param[in] show_callback Ewk_View_Notification_Show_Callback function to
 * notification show
 * @param[in] user_data user data
 */
EXPORT_API void ewk_view_notification_show_callback_set(
    Evas_Object* o, Ewk_View_Notification_Show_Callback show_callback,
    void* user_data);

/**
 * @internal
 * @brief Callback for ewk_view_notification_cancel_callback_set
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to request the notification cancel
 * @param[in] notification_id Ewk_Notification object to get the information
 * about notification cancel request
 * @param[in] user_data user data
 */
typedef Eina_Bool (*Ewk_View_Notification_Cancel_Callback)(
    Evas_Object* o, uint64_t notification_id, void* user_data);

/**
 * @internal
 * @brief Sets the notification cancel callback.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to request the notification show
 * @param[in] cancel_callback Ewk_View_Notification_Cancel_Callback function to
 * notification cancel
 * @param[in] user_data user data
 */
EXPORT_API void ewk_view_notification_cancel_callback_set(
    Evas_Object* o, Ewk_View_Notification_Cancel_Callback cancel_callback,
    void* user_data);

/**
 * @brief Gets the current custom character encoding name.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view object to get the current encoding
 *
 * @return @c eina_strinshare containing the current encoding, or\n
 *         @c NULL if it's not set
 */
EXPORT_API const char* ewk_view_custom_encoding_get(const Evas_Object* o);

/**
 * @brief Sets the custom character encoding and reloads the page.
 *
 * @since_tizen 2.3
 *
 * @param[in] o view to set the encoding
 * @param[in] encoding the new encoding to set or @c NULL to restore the default
 * one
 *
 * @return @c EINA_TRUE on success @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_view_custom_encoding_set(Evas_Object* o,
                                                  const char* encoding);

/**
 * @brief Forces web page to relayout
 *
 * @since_tizen 2.3
 *
 * @param [in] o view
 */
EXPORT_API void ewk_view_force_layout(const Evas_Object* o);

/**
 * Gets the video's timestamp.
 *
 * @param o view object to get the video's timestamp
 *
 * @return timestamp value
 */
EXPORT_API double ewk_view_media_current_time_get(const Evas_Object* o);

/**
 * @brief Enforces web page to close
 *
 * @since_tizen 3.0
 *
 * @param[in] o view
 *
 * @return @c EINA_TRUE on success @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_view_page_close(Evas_Object* o);

/**
 * Clear all tile resources.
 *
 * @param ewkView view object
 */
EXPORT_API void ewk_view_clear_all_tiles_resources(Evas_Object* ewkView);

/**
 * Request canvas to be shown in full-screen.
 *
 * @param ewkView view object
 */
EXPORT_API void ewk_view_request_canvas_fullscreen(Evas_Object* ewkView);

/**
 * play 360 video in the view
 *
 * @param ewkView view object
 */
EXPORT_API void ewk_view_360video_play(Evas_Object* ewkView);

/**
 * pause 360 video in the view
 *
 * @param ewkView view object
 */
EXPORT_API void ewk_view_360video_pause(Evas_Object* ewkView);

/**
 * Callback for ewk_view_360video_duration
 *
 * @param o view object
 * @param duration 360 video's duration
 * @param user_data user_data will be passsed when ewk_view_360video_duration is
 * called
 */
typedef void (*Ewk_360_Video_Duration_Callback)(Evas_Object* o, double duration,
                                                void* user_data);

/**
 * get duration of the 360 video in the view
 *
 * @param ewkView view object
 *
 * @return duration of the video
 */
EXPORT_API void ewk_view_360video_duration(
    Evas_Object* ewkView, Ewk_360_Video_Duration_Callback callback,
    void* user_data);

/**
 * Callback for ewk_view_360video_current_time
 *
 * @param o view object
 * @param current_time 360 video's current time
 * @param user_data user_data will be passsed when
 * ewk_view_360video_current_time is called
 */
typedef void (*Ewk_360_Video_CurrentTime_Callback)(Evas_Object* o,
                                                   double current_time,
                                                   void* user_data);

/**
 * get current time of the 360 video in the view
 *
 * @param ewkView view object
 *
 * @return current time of the video
 */
EXPORT_API void ewk_view_360video_current_time(
    Evas_Object* ewkView, Ewk_360_Video_CurrentTime_Callback callback,
    void* user_data);

/**
 * set current time of the 360 video in the view
 *
 * @param ewkView view object
 *
 * @param current_time set current time
 */
EXPORT_API void ewk_view_360video_set_current_time(Evas_Object* ewkView,
                                                   double current_time);

/**
 * @brief Request to set the atk usage set by web app(config.xml).
 *
 * Some TV apps use WebSpeech instead of use ATK for regulation U.S.FCC
 *
 * @since_tizen 3.0 @if TV   @endif
 *
 * @param[in] o View object to set the atk use.
 * @param[in] enable EINA_TRUE to set on the atk use.
 *            EINA_FALSE makes atk not to use, but app use WebSpeech instead of
 * ATK.
 */
EXPORT_API void ewk_view_atk_deactivation_by_app(Evas_Object* view,
                                                 Eina_Bool enable);

/**
 * Requests execution of the given script in the main frame and subframes of the
 * page.
 *
 * The result value for the execution can be retrieved from the asynchronous
 * callback.
 *
 * @param o The view to execute script
 * @param script JavaScript to execute
 * @param callback The function to call when the execution is completed, may be
 * @c NULL
 * @param user_data User data, may be @c NULL
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_script_execute_all_frames(
    Evas_Object* o, const char* script, Ewk_View_Script_Execute_Cb callback,
    void* user_data);

/**
 * Floating video's window ON/OFF
 *
 * @param o view object
 * @param bool status (true/false)
 *
 */
EXPORT_API void ewk_view_floating_window_state_changed(const Evas_Object* o,
                                                       Eina_Bool status);

/**
 * Auto login by samsung pass
 *
 * @param view  view object
 * @param user_name user name to login
 * @param password  user password to login
 *
 */
EXPORT_API void ewk_view_auto_login(Evas_Object* view, const char* user_name,
                                    const char* password);

/**
 * @brief Selects index of select popup menu.
 *
 * @since_tizen 6.0
 *
 * @param[in] view view object contains popup menu
 * @param[in] index index of item to select
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure (probably\n
 *         popup menu is not selected or index is out of range)
 */
EXPORT_API Eina_Bool ewk_view_select_menu_select(Evas_Object* view, int index);

/**
 * @brief Selects Multiple indexes of select popup menu.
 *
 * @since_tizen 6.0
 *
 * @param[in] view view object contains popup menu.
 * @param[in] changed_list  list of item selected and deselected
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure (probably\n
 *         popup menu is not selected or index is out of range)
 */
EXPORT_API Eina_Bool
ewk_view_select_menu_multiple_select(Evas_Object* view, Eina_List* select_list);

/**
 * @brief Hides the select menu.
 *
 * @since_tizen 6.0
 *
 * @param[in] view the view object
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_select_menu_hide(Evas_Object* view);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  // ewk_view_product_h
