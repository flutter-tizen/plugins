/*
 * Copyright (C) 2011-2016 Samsung Electronics.
 * Copyright (C) 2012 Intel Corporation.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/**
 * @file    ewk_view.h
 * @brief   This file describes the Chromium-efl main smart object.
 *
 * This object provides view related APIs of Chromium to EFL objects.
 */

#ifndef ewk_view_h
#define ewk_view_h

#include <Eina.h>
#include <Evas.h>
#include <string.h>
#include <tizen.h>

#include "ewk_back_forward_list.h"
#include "ewk_context.h"
#include "ewk_context_menu.h"
#include "ewk_manifest.h"
#include "ewk_settings.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup WEBVIEW
 * @{
 */

/**
 * @brief Request to set the current page's visibility.
 *
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @param[in] o View object to set the visibility.
 * @param[in] enable EINA_TRUE to set on the visibility of the page,
 *            EINA_FALSE otherwise.
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_visibility_set(Evas_Object* o, Eina_Bool enable);

/**
 * @brief Sends the orientation of the device.
 *
 * If orientation value is changed, orientationchanged event will occur.
 *
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @param[in] o View object to receive orientation event.
 * @param[in] orientation The new orientation of the device. (degree)
 *
 * orientation will be 0 degrees when the device is oriented to natural position
 *                  ,-90 degrees when it's left side is at the top
 *                  , 90 degrees when it's right side is at the top
 *                  ,180 degrees when it is upside down.
 */
EXPORT_API void ewk_view_orientation_send(Evas_Object* o, int orientation);

/**
 * @if MOBILE
 * @brief Returns the selection text.
 *
 * @details Returned string becomes invalidated upon next call to this api.
 *
 * @since_tizen 2.4
 *
 * @param[in] o View object to get selection text.
 *
 * @return @c selection text, otherwise @c NULL
 * @endif
 */
EXPORT_API const char* ewk_view_text_selection_text_get(Evas_Object* o);

/**
 * @if MOBILE
 * @brief Clears the current selection.
 *
 * @since_tizen 2.4
 *
 * @param[in] o View object with selection in progress
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 * @endif
 */
EXPORT_API Eina_Bool ewk_view_text_selection_clear(Evas_Object* o);

/**
 * @brief Creates a new EFL Chromium view object.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] e The canvas object where to create the view object
 *
 * @return The view object on success,\n
 *         otherwise @c NULL on failure
 */
EXPORT_API Evas_Object* ewk_view_add(Evas* e);

/**
 * @brief Creates a new EFL web view object in incognito mode.
 *
 * @since_tizen 3.0
 *
 * @param[in] e The canvas object where to create the view object
 *
 * @return The view object on success,\n
 *         otherwise @c NULL on failure
 */
EXPORT_API Evas_Object* ewk_view_add_in_incognito_mode(Evas* e);

/**
 * @brief Gets the #Ewk_Context of this view.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to get the #Ewk_Context
 *
 * @return The #Ewk_Context of this view,\n
 *         otherwise @c NULL on failure
 */
EXPORT_API Ewk_Context* ewk_view_context_get(const Evas_Object* o);

/**
 * @brief Asks the object to load the given URL.
 *
 * @remarks You can only be sure that url changed after ewk view\n
 *          smart callback 'url,changed' is called. This is important for\n
 *          EWK API functions which operate on pages.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to load @a url
 * @param[in] url The uniform resource identifier to load
 *
 * @return @c EINA_TRUE if @a o is valid, irrespective of load,\n
 *         otherwise @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_url_set(Evas_Object* o, const char* url);

/**
 * @brief Returns the current URL string of the view object.
 *
 * @details It returns an internal string that should not be modified.\n
 *          The string is guaranteed to be stringshared.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to get the current URL
 *
 * @return The current URL on success,\n
 *         otherwise @c NULL on failure
 */
EXPORT_API const char* ewk_view_url_get(const Evas_Object* o);

/**
 * @brief Returns the original URL string of the view object.
 *
 * @details It returns an internal string that should not be modified.\n
 *          The string is guaranteed to be stringshared.
 *
 * @since_tizen 4.0
 *
 * @param[in] o The view object to get the original URL
 *
 * @return The original URL on success,\n
 *         otherwise @c NULL on failure
 */
EXPORT_API const char* ewk_view_original_url_get(const Evas_Object* o);

/**
 * @brief Asks the main frame to reload the current document.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to reload the current document
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_reload(Evas_Object* o);

/**
 * @brief Asks the main frame to stop loading.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to stop loading
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_stop(Evas_Object* o);

/**
 * @brief Gets the #Ewk_Settings of this view.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to get the #Ewk_Settings
 *
 * @return The #Ewk_Settings of this view,\n
 *         otherwise @c NULL on failure
 */
EXPORT_API Ewk_Settings* ewk_view_settings_get(const Evas_Object* o);

/**
 * @brief Asks the main frame to navigate back in history.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to navigate back
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_back(Evas_Object* o);

/**
 * @brief Asks the main frame to navigate forward in history.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to navigate forward
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_forward(Evas_Object* o);

/**
 * @brief Checks whether it is possible to navigate backwards one item in
 *         history.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to query if backward navigation is possible
 *
 * @return @c EINA_TRUE if it is possible to navigate backwards in history,\n
 *         otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_back_possible(Evas_Object* o);

/**
 * @brief Checks whether it is possible to navigate forwards one item in
 *        history.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to query if forward navigation is possible
 *
 * @return @c EINA_TRUE if it is possible to navigate forwards in history,\n
 *         otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_forward_possible(Evas_Object* o);

/**
 * @brief Gets the back-forward list associated with this view.
 *
 * @details The returned instance is unique for this view and thus multiple
 *          calls\n
 *          to this function with the same view as a parameter returns the
 *          same handle.\n
 *          This handle is alive while the view is alive, thus one might want\n
 *          to listen for EVAS_CALLBACK_DEL on a given view (@a o) to know\n
 *          when to stop using the returned handle.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to get the back-forward navigation list
 *
 * @return The back-forward list instance handle associated with this view
 */
EXPORT_API Ewk_Back_Forward_List* ewk_view_back_forward_list_get(
    const Evas_Object* o);

/**
 * @brief Clears the back-forward list of a page.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to clear the back-forward list
 */
EXPORT_API void ewk_view_back_forward_list_clear(const Evas_Object* o);

/**
 * @brief Gets the current title of the main frame.
 *
 * @details It returns an internal string that should not be modified.\n
 *          The string is guaranteed to be stringshared.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to get the current title
 *
 * @return The current title on success,\n
 *         otherwise @c NULL on failure
 */
EXPORT_API const char* ewk_view_title_get(const Evas_Object* o);

/**
 * @brief Gets the current load progress of the page.
 *
 * @details The progress estimation from @c 0.0 to @c 1.0.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to get the current progress
 *
 * @return The load progress of the page, value from @c 0.0 to @c 1.0,\n
 *         otherwise @c -1.0 on failure
 */
EXPORT_API double ewk_view_load_progress_get(const Evas_Object* o);

/**
 * @brief Requests to set the user agent string.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to set the user agent string
 * @param[in] user_agent The user agent string to set,\n
 *                       otherwise @c NULL to restore the default one
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_user_agent_set(Evas_Object* o,
                                             const char* user_agent);

/**
 * @brief Returns the user agent string.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to get the user agent string
 *
 * @return The user agent string
 */
EXPORT_API const char* ewk_view_user_agent_get(const Evas_Object* o);

/**
 * @brief Gets the last known content's size.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to get the content's size
 * @param[in] width The width pointer to store the content's size width,
 *            may be @c 0
 * @param[in] height The height pointer to store the content's size height,
 *            may be @c 0
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE on failure and\n
 *         @a width and @a height are zeroed
 */
EXPORT_API Eina_Bool ewk_view_contents_size_get(const Evas_Object* o,
                                                Evas_Coord* width,
                                                Evas_Coord* height);

/**
 * @brief Callback for ewk_view_script_execute().
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object
 * @param[in] result_value The value returned by the script\n
 *            If executed script returns a value, it would be @a result_value,\n
 *            otherwise @c NULL if there is no value returned by the script
 * @param[in] user_data The user_data will be passed when
 *            ewk_view_script_execute() is called
 */
typedef void (*Ewk_View_Script_Execute_Cb)(Evas_Object* o,
                                           const char* result_value,
                                           void* user_data);

/**
 * @brief Requests the execution of the given script.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @remarks This allows to use @c NULL for the callback parameter\n
 *          So, if the result data from the script is not required,
 *          @c NULL might be used for the callback parameter\n
 *          Also, @a script should be valid statement according to JavaScript
 * language\n If @a script is empty, NULL or invalid statement, this function
 * returns @c EINA_FALSE
 *
 * @param[in] o The view object to execute the script
 * @param[in] script The JavaScript code string to execute
 * @param[in] callback The result callback
 * @param[in] user_data The user data
 *
 * @return @c EINA_TRUE  on success,\n
 *         otherwise @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool
ewk_view_script_execute(Evas_Object* o, const char* script,
                        Ewk_View_Script_Execute_Cb callback, void* user_data);

/**
 * @brief Scales the current page, centered at the given point.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to set the zoom level
 * @param[in] scale_factor A new level to set
 * @param[in] cx The x value of the center coordinate
 * @param[in] cy The y value of the center coordinate
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_scale_set(Evas_Object* o, double scale_factor,
                                        int cx, int cy);

/**
 * @brief Gets the current scale factor of the page.
 *
 * @details It returns the previous scale factor after ewk_view_scale_set() is
 *          called immediately\n
 *          until the scale factor of the page is really changed.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to get the scale factor of
 *
 * @return The current scale factor in use on success,\n
 *         otherwise @c -1.0 on failure
 */
EXPORT_API double ewk_view_scale_get(const Evas_Object* o);

/**
 * @brief Exits fullscreen when the back key is pressed.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to exit the fullscreen mode
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_fullscreen_exit(Evas_Object* o);

/**
 * @brief Suspends the operation associated with the view object.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to suspend
 */
EXPORT_API void ewk_view_suspend(Evas_Object* o);

/**
 * @brief Resumes the operation associated with the view object after calling
 *        ewk_view_suspend().
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to resume
 */
EXPORT_API void ewk_view_resume(Evas_Object* o);

/**
 * \enum Ewk_Http_Method
 * @brief Enumeration that provides HTTP method options.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
enum Ewk_Http_Method {
  EWK_HTTP_METHOD_GET,    /**< Get */
  EWK_HTTP_METHOD_HEAD,   /**< Head */
  EWK_HTTP_METHOD_POST,   /**< Post */
  EWK_HTTP_METHOD_PUT,    /**< Put */
  EWK_HTTP_METHOD_DELETE, /**< Delete */
};

/**
 * @brief Enumeration that creates a type name for the #Ewk_Http_Method.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef enum Ewk_Http_Method Ewk_Http_Method;

/**
 * @brief Requests loading of the given request data.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to load
 * @param[in] url The uniform resource identifier to load
 * @param[in] method The http method
 * @param[in] headers The http headers
 * @param[in] body The http body data
 *
 * @return @c EINA_TRUE on a successful request,\n
 *         otherwise @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_view_url_request_set(Evas_Object* o, const char* url,
                                              Ewk_Http_Method method,
                                              Eina_Hash* headers,
                                              const char* body);

/**
 * @if MOBILE
 * @brief Requests loading the given contents by MIME type into the view object.
 *
 * @since_tizen 2.3
 *
 * @param[in] o The view object to load
 * @param[in] contents The content to load
 * @param[in] contents_size The size of @a contents (in bytes)
 * @param[in] mime_type The type of @a contents,
 *            if @c 0 is given "text/html" is assumed
 * @param[in] encoding The encoding for @a contents,
 *            if @c 0 is given "UTF-8" is assumed
 * @param[in] base_uri The base URI to use for relative resources,
 *            may be @c 0,\n if provided @b must be an absolute URI
 *
 * @return @c EINA_TRUE on a successful request,\n
 *         otherwise @c EINA_FALSE on errors
 * @endif
 */
EXPORT_API Eina_Bool ewk_view_contents_set(Evas_Object* o, const char* contents,
                                           size_t contents_size,
                                           char* mime_type, char* encoding,
                                           char* base_uri);

/**
 * @brief Scrolls the webpage of view by dx and dy.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to scroll
 * @param[in] dx The horizontal offset to scroll
 * @param[in] dy The vertical offset to scroll
 */
EXPORT_API void ewk_view_scroll_by(Evas_Object* o, int dx, int dy);

/**
 * @brief Gets the current scroll position of the given view.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to get the current scroll position
 * @param[in] x The pointer to store the horizontal position, may be @c NULL
 * @param[in] y The pointer to store the vertical position, may be @c NULL
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_scroll_pos_get(Evas_Object* o, int* x, int* y);

/**
 * @brief Sets an absolute scroll of the given view.
 *
 * @details Both values are from zero to the contents size minus
 *          the viewport size.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to scroll
 * @param[in] x The horizontal position to scroll
 * @param[in] y The vertical position to scroll
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_scroll_set(Evas_Object* o, int x, int y);

/**
 * Enum values used to specify search options.
 * @brief  Enumeration that provides the option to find text.
 * @details It contains enum values used to specify search options.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
enum Ewk_Find_Options {
  EWK_FIND_OPTIONS_NONE, /**< No search flags, this means a case sensitive, no
                            wrap, forward only search */
  EWK_FIND_OPTIONS_CASE_INSENSITIVE = 1 << 0, /**< Case insensitive search */
  EWK_FIND_OPTIONS_AT_WORD_STARTS =
      1 << 1, /**< Search text only at the beginning of the words */
  EWK_FIND_OPTIONS_TREAT_MEDIAL_CAPITAL_AS_WORD_START =
      1 << 2, /**< Treat capital letters in the middle of words as word start */
  EWK_FIND_OPTIONS_BACKWARDS = 1 << 3, /**< Search backwards */
  EWK_FIND_OPTIONS_WRAP_AROUND =
      1 << 4, /**< If not present the search stops at the end of the document */
  EWK_FIND_OPTIONS_SHOW_OVERLAY = 1 << 5,        /**< Show overlay */
  EWK_FIND_OPTIONS_SHOW_FIND_INDICATOR = 1 << 6, /**< Show indicator */
  EWK_FIND_OPTIONS_SHOW_HIGHLIGHT = 1 << 7       /**< Show highlight */
};

/**
 * @brief Enumeration that creates a type name for the #Ewk_Find_Options.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef enum Ewk_Find_Options Ewk_Find_Options;

/**
 * @brief Searches and highlights the given string in the document.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o The view object to find text
 * @param[in] text The text to find
 * @param[in] options The options to find
 * @param[in] max_match_count The maximum match count to find, unlimited if @c 0
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise EINA_FALSE on errors
 */
EXPORT_API Eina_Bool ewk_view_text_find(Evas_Object* o, const char* text,
                                        Ewk_Find_Options options,
                                        unsigned max_match_count);

/**
 * @brief Loads the specified @a html string as the content of the view.
 *
 * @details External objects such as stylesheets or images referenced
 *          in the HTML\n document are located relative to @a baseUrl.\n
 *
 * @remarks There is an alternative API
 * ewk_view_html_string_override_current_entry_load() which is able to override
 * current entry which prevents from backing back to malicious page.
 *
 * If an @a unreachableUrl is passed it is used as the url for the loaded
 * content.\n This is typically used to display error pages for a failed load.
 *
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] o view object to load the HTML into
 * @param[in] html HTML data to load
 * @param[in] base_url Base URL used for relative paths to external objects
 *            (optional)
 * @param[in] unreachable_url URL that could not be reached (optional)
 *
 * @return @c EINA_TRUE if it the HTML was successfully loaded,
 *         @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_view_html_string_load(Evas_Object* o, const char* html,
                                               const char* base_url,
                                               const char* unreachable_url);

/**
 * @brief A ScriptMessage contains information that sent from JavaScript running
 * in a webpage.
 *
 * @since_tizen 3.0
 */
struct _Ewk_Script_Message {
  const char* name; /**< The name used to expose the object in JavaScript*/
  void* body;       /**< Message body */
};

/**
 * @brief A struct that creates a type name for the #Ewk_Script_Message.
 * @since_tizen 3.0
 */
typedef struct _Ewk_Script_Message Ewk_Script_Message;

/**
 * @brief Callback for ewk_view_javascript_message_handler_add().
 *
 * @since_tizen 3.0
 *
 * @param[in] o The view object
 * @param[in] message The ScriptMessage returned by the script. \n
 *    It will be passed when ewk_view_javascript_message_handler_add() is
 * called.
 */
typedef void (*Ewk_View_Script_Message_Cb)(Evas_Object* o,
                                           Ewk_Script_Message message);

/**
 * @brief Injects the supplied javascript message handler into webview.
 *
 * @since_tizen 3.0
 *
 * @remarks Note that injected objects will not appear in JavaScript
 *          until the page is next (re)loaded.
 *
 *          Certainly, specify privileges of using native API to users,
 *          if using native API affects system settings, stability or security.
 *          Privilege display name & description which will be noticed to user.
 *
 * @param[in] o The view object
 * @param[in] callback The result callback
 * @param[in] name The name used to expose the object in JavaScript
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_javascript_message_handler_add(
    Evas_Object* o, Ewk_View_Script_Message_Cb callback, const char* name);

/**
 * @brief Requests the execution of given name & result to the JavaScript
 * runtime.
 *
 * @since_tizen 3.0
 *
 * @param[in] o The view object
 * @param[in] name The name used to expose the object in JavaScript
 * @param[in] result The result to the JavaScript runtime
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_evaluate_javascript(Evas_Object* o,
                                                  const char* name,
                                                  const char* result);

/**
 * @brief Requests to set or unset a web view as the currently focused one.
 *
 * @since_tizen 3.0
 *
 * @param[in] o The view object.
 * @param[in] focused @c EINA_TRUE to set the focus on the web view,
 *            @c EINA_FALSE to remove the focus from the web view.
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_focus_set(const Evas_Object* o,
                                        Eina_Bool focused);

/**
 * @brief Checks whether a web view has the focus.
 *
 * @since_tizen 3.0
 *
 * @param[in] o The view object.
 *
 * @return @c EINA_TRUE if the web view has the focus, otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_view_focus_get(const Evas_Object* o);

/**
 * @brief Callback invoked when requested manifest inform is responded.
 *
 * @since_tizen 3.0
 *
 * @param[in] o View object for which callback was set
 * @param[in] manifest Received manifest object of current's page
 *            This object is valid after callback data is received.
 *            If the manifest file is empty, it is returned NULL.
 * @param[in] user_data User data passed to
 *            ewk_view_request_manifest
 *
 * @see ewk_view_request_manifest
 */
typedef void (*Ewk_View_Request_Manifest_Callback)(
    Evas_Object* o, Ewk_View_Request_Manifest* manifest, void* user_data);

/**
 * @brief Requests the manifest data of current's page.
 *
 * @since_tizen 3.0
 *
 * @param[in] o view object to request the manifest information
 * @param[in] callback Ewk_View_Request_Manifest_Callback function to request
 * manifest information
 * @param[in] user_data user data
 *
 * @see Ewk_View_Request_Manifest_Callback
 */
EXPORT_API void ewk_view_request_manifest(
    Evas_Object* o, Ewk_View_Request_Manifest_Callback callback,
    void* user_data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  // ewk_view_h
