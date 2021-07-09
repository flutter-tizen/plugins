/*
 * Copyright (C) 2013-2016 Samsung Electronics.
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
 * @file    ewk_context.h
 * @brief   This file describes the Ewk Context API.
 *
 * @remarks ewk_context encapsulates all pages related to the specific use of
 *          Chromium-efl
 *
 * Applications have the option of creating a context different from the default
 * one and using it for a group of pages. All pages in the same context share
 * the same preferences, visited link set, local storage, and so on.
 *
 * A process model can be specified per context. The default one is the shared
 * model where the web-engine process is shared among the pages in the context.
 * The second model allows each page to use a separate web-engine process.
 * This latter model is currently not supported by Chromium-efl.
 *
 */

#ifndef ewk_context_h
#define ewk_context_h

#include <Eina.h>
#include <Evas.h>
#include <tizen.h>

#include "ewk_autofill_profile.h"
#include "ewk_cookie_manager.h"
#include "ewk_intercept_request.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup WEBVIEW
 * @{
 */

/**
 * @brief The structure type that creates a type name for #Ewk_Context.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef struct Ewk_Context Ewk_Context;

/**
 * @brief   Contains option for cache model
 *
 * @since_tizen 2.4
 */
enum _Ewk_Cache_Model {
  EWK_CACHE_MODEL_DOCUMENT_VIEWER,   /**< Use the smallest cache capacity. */
  EWK_CACHE_MODEL_DOCUMENT_BROWSER,  /**< Use bigger cache capacity than
                                        EWK_CACHE_MODEL_DOCUMENT_VIEWER. */
  EWK_CACHE_MODEL_PRIMARY_WEBBROWSER /**< Use the biggest cache capacity. */
};

/**
 * @brief The structure type that creates a type name for Ewk_Cache_Model.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 3.0 @endif
 */
typedef enum _Ewk_Cache_Model Ewk_Cache_Model;

/**
 * @brief Gets the cookie manager instance for this @a context.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] context The context object to query
 *
 * @return The Ewk_Cookie_Manager object instance,\n
 *         otherwise @c NULL in case of failure
 */
EXPORT_API Ewk_Cookie_Manager* ewk_context_cookie_manager_get(
    const Ewk_Context* context);

/**
 * @if MOBILE
 * @brief Requests for deleting all web application caches.
 *
 * @since_tizen 2.4
 *
 * @param[in] context Context object
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA FALSE
 * @endif
 */
EXPORT_API Eina_Bool
ewk_context_application_cache_delete_all(Ewk_Context* context);

/**
 * @brief Requests to set the cache model.
 *
 * The default cache option is #EWK_CACHE_MODEL_DOCUMENT_VIEWER.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] context The context object
 * @param[in] model The cache model
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE
 */
EXPORT_API Eina_Bool ewk_context_cache_model_set(Ewk_Context* context,
                                                 Ewk_Cache_Model model);

/**
 * @brief Returns the cache model type.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] context The context object
 *
 * @return #Ewk_Cache_Model
 */
EXPORT_API Ewk_Cache_Model
ewk_context_cache_model_get(const Ewk_Context* context);

/**
 * @if MOBILE
 * @brief Saves the created profile into permanent storage
 *
 * The profile used to save must be created by ewk_autofill_profile_new.
 * Data can be added to the created profile by ewk_autofill_profile_data_set.
 *
 * @since_tizen 2.4
 *
 * @param[in] context Context object
 * @param[in] profile Ewk_Autofill_Profile
 *
 * @return @c EINA_TRUE if the profile data is saved successfully, otherwise
 *         @c EINA_FALSE
 *
 * @see ewk_autofill_profile_new
 * @endif
 */
EXPORT_API Eina_Bool ewk_context_form_autofill_profile_add(
    Ewk_Context* context, Ewk_Autofill_Profile* profile);

/**
 * @if MOBILE
 * @brief Gets the existing profile for given index
 *
 * @since_tizen 2.4
 *
 * @param[in] context Context object
 * @param[in] id Profile
 *
 * @return @c Ewk_Autofill_Profile if profile exists, otherwise @c NULL
 *
 * @see ewk_autofill_profile_delete
 * @endif
 */
EXPORT_API Ewk_Autofill_Profile* ewk_context_form_autofill_profile_get(
    Ewk_Context* context, unsigned id);

/**
 * @if MOBILE
 * @brief Gets a list of all existing profiles
 *
 * The obtained profile must be deleted by ewk_autofill_profile_delete.
 *
 * @since_tizen 2.4
 *
 * @param[in] context Context object
 *
 * @return @c Eina_List of Ewk_Autofill_Profile, otherwise @c NULL
 *
 * @see ewk_autofill_profile_delete
 * @endif
 */
EXPORT_API Eina_List* ewk_context_form_autofill_profile_get_all(
    Ewk_Context* context);

/**
 * @if MOBILE
 * @brief Sets the given profile for the given id
 *
 * Data can be added to the created profile by ewk_autofill_profile_data_set.
 *
 * @since_tizen 2.4
 *
 * @param[in] context Context object
 * @param[in] id Profile
 * @param[in] profile Ewk_Autofill_Profile
 *
 * @return @c EINA_TRUE if the profile data is set successfully, otherwise
 *         @c EINA_FALSE
 *
 * @see ewk_autofill_profile_new
 * @see ewk_context_form_autofill_profile_add
 * @endif
 */
EXPORT_API Eina_Bool ewk_context_form_autofill_profile_set(
    Ewk_Context* context, unsigned id, Ewk_Autofill_Profile* profile);

/**
 * @if MOBILE
 * @brief Removes Autofill Form profile completely
 *
 * @since_tizen 2.4
 *
 * @param[in] context Context object
 * @param[in] id Profile
 *
 * @return @c EINA_TRUE if the profile data is removed successfully, otherwise
           @c EINA_FALSE
 *
 * @see ewk_context_form_autofill_profile_get_all
 * @endif
 */
EXPORT_API Eina_Bool
ewk_context_form_autofill_profile_remove(Ewk_Context* context, unsigned id);

/**
 * @if MOBILE
 * @brief Deletes all candidate form data from DB
 *
 * @since_tizen 2.4
 *
 * @param[in] context Context object
 * @endif
 */
EXPORT_API void ewk_context_form_candidate_data_delete_all(
    Ewk_Context* context);

/**
 * @if MOBILE
 * @brief Deletes whole password data from DB
 *
 * @since_tizen 2.4
 *
 * @param context Context object
 * @endif
 */
EXPORT_API void ewk_context_form_password_data_delete_all(Ewk_Context* context);

/**
 * @brief Clears HTTP caches in the local storage and
 *        all resources cached in memory\n
 *        such as images, CSS, JavaScript, XSL, and fonts for @a context.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] context The context object to clear all resource caches
 */
EXPORT_API void ewk_context_resource_cache_clear(Ewk_Context* context);

/**
 * @if MOBILE
 * @brief Requests to get image representing the given URL.
 *
 * @since_tizen 2.4
 *
 * @param[in] context Context object
 * @param[in] uri Which url to query icon, must not be @c 0
 * @param[in] canvas Evas instance where to add resulting object,
 *            must not be @c 0
 *
 * @return @c Evas_Object instance, otherwise @c 0
 * @endif
 */
EXPORT_API Evas_Object* ewk_context_icon_database_icon_object_add(
    Ewk_Context* context, const char* uri, Evas* canvas);

/**
 * @if MOBILE
 * @brief Requests for deleting all web indexed databases.
 *
 * @since_tizen 2.4
 *
 * @param[in] context Context object
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 * @endif
 */
EXPORT_API Eina_Bool
ewk_context_web_indexed_database_delete_all(Ewk_Context* context);

/**
 * @if MOBILE
 * @brief Deletes web storage.
 *
 * @details This function does not ensure that all data will be removed.
 *          Should be used to extend free physical memory.
 *
 * @since_tizen 2.4
 *
 * @param[in] context Context object
 *
 * @return @c EINA_TRUE on success, otherwise @c EINA_FALSE
 * @endif
 */
EXPORT_API Eina_Bool ewk_context_web_storage_delete_all(Ewk_Context* context);

/**
 * @brief   Callback for intercept request feature.
 *
 * @details Ewk_Context_Intercept_Request_Callback callback allows host
 *          application to intercept a resource request and write custom
 *          response.
 *
 * @warning Callback is not called on UI thread, so user should be cautious
 *          when accessing their data also used on UI thread. No ewk api other
 *          than ewk_intercept_request_* api should be used in the callback.
 *
 * @remarks Inside the callback user can use the following EWK API calls on
 *          a given Ewk_Intercept_Request instance to decide if request should
 *          be intercepted or handled normally:
 *          - ewk_intercept_request_url_get
 *          - ewk_intercept_request_http_method_get
 *          - ewk_intercept_request_headers_get
 *
 *          To let engine handle the request normally, use
 *          ewk_intercept_request_ignore *inside* the callback. Using it later
 *          results in undefined behavior.
 *
 *          To respond to a request with a custom response, use
 *          ewk_intercept_request_response_set or
 *          ewk_intercept_request_response_body_set inside or outside the
 *          callback. Using it outside of the callback is advised for better
 *          responsiveness. If your responses take too much time to prepare, it
 *          would block handling other requests. You can also use
 *          ewk_intercept_request_response_write outside the callback to write
 *          response in chunks.
 *
 *          Handling a request is done after using either
 *          ewk_intercept_request_ignore, ewk_intercept_request_response_set,
 *          ewk_intercept_request_response_body_set, and in some cases after
 *          ewk_intercept_request_response_write_chunk.
 *
 *          Further EWK API calls on this Ewk_Intercept_Request instance result
 *          in undefined behavior.
 *
 *          Failing to call one of these functions results in leaking
 *          Ewk_Intercept_Request object.
 *
 * @since_tizen 3.0
 *
 * @param[in] ewk_context Ewk_Context for which callback was set
 * @param[in] intercept_request Intercept request object for reading request
 *            info and writing response data
 * @param[in] user_data User data passed to
 *            ewk_context_intercept_request_callback_set
 */
typedef void (*Ewk_Context_Intercept_Request_Callback)(
    Ewk_Context* ewk_context, Ewk_Intercept_Request* intercept_request,
    void* user_data);

/**
 * @brief   Sets Ewk_Context_Intercept_Request_Callback
 *
 * @details Sets Ewk_Context_Intercept_Request_Callback to give a chance to
 *          intercept resource request before sending it and return a custom
 *          response.
 *
 * @remarks Pass NULL pointer as @a callback to reset current callback.
 *
 * @since_tizen 3.0
 *
 * @param[in] ewk_context Ewk_Context object to intercept requests
 * @param[in] callback New callback, NULL resets current callback
 * @param[in] user_data User data passed to @a callback
 *
 * @see Ewk_Context_Intercept_Request_Callback
 */
EXPORT_API void ewk_context_intercept_request_callback_set(
    Ewk_Context* ewk_context, Ewk_Context_Intercept_Request_Callback callback,
    void* user_data);

/**
 * @brief   Sets Ewk_Context_Intercept_Request_Cancel_Callback
 *
 * @details Sets Ewk_Context_Intercept_Request_Cancel_Callback to give a chance
 * to notify intercept resource request to client when url request is cancelled.
 *
 * @remarks Pass NULL pointer as @a callback to reset current callback.
 *
 *
 * @param[in] ewk_context Ewk_Context object to intercept requests
 * @param[in] callback New callback, NULL resets current callback
 * @param[in] user_data User data passed to @a callback
 *
 * @see Ewk_Context_Intercept_Request_Callback
 */

typedef Ewk_Context_Intercept_Request_Callback
    Ewk_Context_Intercept_Request_Cancel_Callback;

EXPORT_API void ewk_context_intercept_request_cancel_callback_set(
    Ewk_Context* ewk_context,
    Ewk_Context_Intercept_Request_Cancel_Callback callback, void* user_data);

/**
 * @brief Gets the enabled state of background music.
 *
 * @since_tizen 4.0
 *
 * @param[in] context The context object
 *
 * @return @c EINA_TRUE if background music is enabled, @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_background_music_get(Ewk_Context* context);

/**
 * @brief Sets the enabled state of background music.
 *
 * @details This function allows the application to continue playing instead of
 * pausing when it moves to the background.
 *
 * @since_tizen 4.0
 *
 * @param[in] context The context object
 * @param[in] enable Enable or disable background music
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_background_music_set(Ewk_Context* context,
                                                      Eina_Bool enable);

/**
 * @brief Gets the enabled state of blocking multimedia on call.
 *
 * @since_tizen 4.0
 *
 * @param[in] context The context object
 *
 * @return @c EINA_TRUE if blocking multimedia on call is enabled, @c EINA_FALSE
 * otherwise
 */
EXPORT_API Eina_Bool
ewk_context_block_multimedia_on_call_get(Ewk_Context* context);

/**
 * @brief Sets the enabled state of blocking multimedia on call.
 *
 * @details The application can set the flag to allow or disallow media playback
 * during active call.
 *
 * @since_tizen 4.0
 *
 * @param[in] context The context object
 * @param[in] enable Enable or disable blocking multimedia on call
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_block_multimedia_on_call_set(
    Ewk_Context* context, Eina_Bool enable);

/**
 * @brief Gets the enabled state of rotation lock.
 *
 * @since_tizen 4.0
 *
 * @param[in] context The context object
 *
 * @return @c EINA_TRUE if rotation lock is enabled, @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_rotation_lock_get(Ewk_Context* context);

/**
 * @brief Sets the enabled state of rotation lock.
 *
 * @details The application can use this function to lock the auto screen
 * rotation feature.
 *
 * @since_tizen 4.0
 *
 * @param[in] context The context object
 * @param[in] enable Enable or disable rotation lock
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_rotation_lock_set(Ewk_Context* context,
                                                   Eina_Bool enable);

/**
 * @brief Gets the enabled state of sound overlap.
 *
 * @since_tizen 4.0
 *
 * @param[in] context The context object
 *
 * @return @c EINA_TRUE if sound overlap is enabled, @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_sound_overlap_get(Ewk_Context* context);

/**
 * @brief Sets the enabled state of sound overlap.
 *
 * @details The application can set the flag to allow or disallow the sound
 * overlap with other apps.
 *
 * @since_tizen 4.0
 *
 * @param[in] context The context object
 * @param[in] enable Enable or disable sound overlap
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_sound_overlap_set(Ewk_Context* context,
                                                   Eina_Bool enable);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // ewk_context_h
