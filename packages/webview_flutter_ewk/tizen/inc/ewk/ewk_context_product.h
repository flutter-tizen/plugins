/*
 * Copyright (C) 2013-2016 Samsung Electronics. All rights reserved.
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
 * @file    ewk_context_product.h
 * @brief   Describes the context API.
 *
 * @note ewk_context encapsulates all pages related to specific use of
 *       Chromium-efl
 *
 * Applications have the option of creating a context different than the default
 * one and use it for a group of pages. All pages in the same context share the
 * same preferences, visited link set, local storage, etc.
 *
 * A process model can be specified per context. The default one is the shared
 * model where the web-engine process is shared among the pages in the context.
 * The second model allows each page to use a separate web-engine process.
 * This latter model is currently not supported by Chromium-efl.
 *
 */

#ifndef ewk_context_product_h
#define ewk_context_product_h

#include <stdint.h>

#include "ewk_context_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

enum _Ewk_Application_Type {
  EWK_APPLICATION_TYPE_WEBBROWSER = 0,
  EWK_APPLICATION_TYPE_HBBTV = 1,
  EWK_APPLICATION_TYPE_TIZENWRT = 2,
  EWK_APPLICATION_TYPE_OTHER = 3
};

typedef enum _Ewk_Application_Type Ewk_Application_Type;

/**
 * @brief Enumeration for password popup option.
 * @since_tizen 2.3
 */
enum Ewk_Context_Password_Popup_Option {
  EWK_CONTEXT_PASSWORD_POPUP_SAVE,    /**< The option of response */
  EWK_CONTEXT_PASSWORD_POPUP_NOT_NOW, /**< The option of response */
  EWK_CONTEXT_PASSWORD_POPUP_NEVER    /**< The option of response */
};
/**
 * @brief Creates a type name for @a Ewk_Context_Password_Popup_Option.
 * @since_tizen 2.3
 */
typedef enum Ewk_Context_Password_Popup_Option
    Ewk_Context_Password_Popup_Option;

/**
 * Callback to check a file request is allowed for specific tizen app id
 *
 * @param tizen_app_id Tizen app id.
 * @param url The url for the file request.
 * @return @c EINA_TRUE if the url can access or @c EINA_FALSE otherwise.
 */
typedef Eina_Bool (*Ewk_Context_Check_Accessible_Path_Callback)(
    const char* tizen_app_id, const char* url, void* user_data);

/**
 * @brief Sets the given proxy to network backend of specific context.
 *        Proxy string and bypass_rule string follow rules of proxy_config.h
 *        Note that, it does not support username:password. If proxy string
 *        contains username:password, the proxy setting will not work.
 *        default auth setting see @c ewk_context_proxy_default_auth_set api
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object to set proxy
 * @param[in] proxy URI to set
 * @param[in] bypass rule to set
 */
EXPORT_API void ewk_context_proxy_set(Ewk_Context* context, const char* proxy,
                                      const char* bypass_rule);

/**
 * @brief Gets the proxy URI from the network backend of specific context.
 *
 * @details It returns an internal string and should not\n
 *          be modified. The string is guaranteed to be stringshared.
 *          until next call of @c ewk_context_proxy_set api
 *          or @a context is destroyed.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object to get proxy URI
 *
 * @return current proxy URI or @c NULL if it's not set
 *
 * @see ewk_context_proxy_set
 */
EXPORT_API const char* ewk_context_proxy_uri_get(Ewk_Context* context);

/**
 * @brief Sets the given proxy URI to network backend of specific context.
 *
 * @since_tizen 2.3
 *
 * @param[in] context object to set proxy URI
 * @param[in] proxy URI to set
 */
EXPORT_API void ewk_context_proxy_uri_set(Ewk_Context* context,
                                          const char* proxy);

/**
 * @brief Gets the proxy bypass rule from the network backend of specific
 *        context.
 *
 * @details It returns an internal string and should not
 *          be modified. The string is guaranteed to be stringshared,
 *          until next call of @c ewk_context_proxy_set api
 *          or @a context is destroyed.
 *
 * @since_tizen 3.0
 *
 * @param[in] context context object to get proxy bypass rule
 *
 * @return current proxy bypass rule or @c NULL if it's not set
 *
 * @see ewk_context_proxy_set
 */
EXPORT_API const char* ewk_context_proxy_bypass_rule_get(Ewk_Context* context);

/*
 * Set a new max size for URL in Tizen APP.
 *
 * Must be called before loading URL.
 *
 * @param context context object
 * @param max_chars  max chars for url
 */
EXPORT_API void ewk_context_url_maxchars_set(Ewk_Context* context,
                                             size_t max_chars);

/**
 * @brief Sets a proxy auth credential to network backend of specific context.
 *
 * @details Normally, proxy auth credential should be got from the callback
 *          set by ewk_view_authentication_callback_set, once the username in
 *          this API has been set with a non-null value, the authentication
 *          callback will never been invoked. Try to avoid using this API.
 *
 * @since_tizen 3.0
 *
 * @param[in] context context object to set proxy
 * @param[in] username username to set
 * @param[in] password password to set
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_proxy_default_auth_set(Ewk_Context* context,
                                                        const char* username,
                                                        const char* password);

/**
 * @brief Callback for ewk_context_local_file_system_origins_get
 *
 * @since_tizen 2.3
 *
 * @param[in] origins local file system origins
 * @param[in] user_data user_data will be passsed when
 *            ewk_context_local_file_system_origins_get is called
 */
typedef void (*Ewk_Local_File_System_Origins_Get_Callback)(Eina_List* origins,
                                                           void* user_data);

/**
 * @brief Callback for ewk_context_web_database_quota_for_origin_get.
 *
 * @since_tizen 2.3
 *
 * @param[in] quota web database quota
 * @param[in] user_data user_data will be passsed when
 *            ewk_context_web_database_quota_for_origin_get is called
 */
typedef void (*Ewk_Web_Database_Quota_Get_Callback)(uint64_t quota,
                                                    void* user_data);

/**
 * @brief Callback for ewk_context_web_storage_origins_get.
 *
 * @since_tizen 2.3
 *
 * @param[in] origins web storage origins
 * @param[in] user_data user_data will be passsed when
 *            ewk_context_web_storage_origins_get is called
 */
typedef void (*Ewk_Web_Storage_Origins_Get_Callback)(Eina_List* origins,
                                                     void* user_data);

/**
 * @brief Callback for ewk_context_web_database_usage_for_origin_get.
 *
 * @since_tizen 2.3
 *
 * @param[in] usage web database usage
 * @param[in] user_data user_data will be passsed when
 * ewk_context_web_database_usage_for_origin_get is called
 */
typedef void (*Ewk_Web_Database_Usage_Get_Callback)(uint64_t usage,
                                                    void* user_data);

/**
 * @brief Callback for ewk_context_web_storage_usage_for_origin_get.
 *
 * @since_tizen 2.3
 *
 * @param[in] usage usage of web storage
 * @param[in] user_data user_data will be passsed when
 *            ewk_context_web_storage_usage_for_origin_get is called
 */
typedef void (*Ewk_Web_Storage_Usage_Get_Callback)(uint64_t usage,
                                                   void* user_data);

/**
 * @brief Callback for didStartDownload
 *
 * @since_tizen 2.3
 *
 * @param[in] download_url url to download
 * @param[in] user_data user_data will be passsed when download is started
 */
typedef void (*Ewk_Context_Did_Start_Download_Callback)(
    const char* download_url, void* user_data);

/**
 * @brief Callback for passworSaveConfirmPopupCallbackCall
 *
 * @since_tizen 2.3
 *
 * @param[in] view current view
 * @param[in] user_data user_data will be passsed when password save confirm
 *            popup show
 */
typedef void (*Ewk_Context_Password_Confirm_Popup_Callback)(Evas_Object* view,
                                                            void* user_data);

/**
 * @brief Sets callback for show password confirm popup.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] callback callback for show password confirm popup
 * @param[in] user_data user data
 */
EXPORT_API void ewk_context_password_confirm_popup_callback_set(
    Ewk_Context* context, Ewk_Context_Password_Confirm_Popup_Callback callback,
    void* user_data);

/**
 * @brief Password confirm popup reply
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] result The option of response
 */
EXPORT_API void ewk_context_password_confirm_popup_reply(
    Ewk_Context* context, Ewk_Context_Password_Popup_Option result);

/**
 * @brief Requests for setting application cache quota for origin.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] origin serucity origin
 * @param[in] quota size of quota
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_application_cache_quota_for_origin_set(
    Ewk_Context* context, const Ewk_Security_Origin* origin, int64_t quota);

/**
 * @brief Requests for deleting all local file systems.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool
ewk_context_local_file_system_all_delete(Ewk_Context* context);

/**
 * @brief Requests for deleting local file system for origin.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] origin local file system origin
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_local_file_system_delete(
    Ewk_Context* context, Ewk_Security_Origin* origin);

/**
 * @brief Requests for getting local file system origins.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] callback callback to get local file system origins
 * @param[in] user_data user_data will be passed when callback is called\n
 *    -I.e., user data will be kept until callback is called
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 *
 * @see ewk_context_origins_free
 */
EXPORT_API Eina_Bool ewk_context_local_file_system_origins_get(
    const Ewk_Context* context,
    Ewk_Local_File_System_Origins_Get_Callback callback, void* user_data);

/**
 * @brief Requests for deleting web databases for origin.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] origin database origin
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_web_database_delete(
    Ewk_Context* context, Ewk_Security_Origin* origin);

/**
 * @brief Requests for setting web database quota for origin.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] origin database origin
 * @param[in] quota size of quota
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_web_database_quota_for_origin_set(
    Ewk_Context* context, Ewk_Security_Origin* origin, uint64_t quota);

/**
 * @brief Deletes origin that is stored in web storage db.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] origin origin of db
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_web_storage_origin_delete(
    Ewk_Context* context, Ewk_Security_Origin* origin);

/**
 * @brief Gets list of origins that is stored in web storage db.
 *
 * @details This function allocates memory for context structure made from
 *          callback and user_data.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] callback callback to get web storage origins
 * @param[in] user_data user_data will be passed when callback is called\n
 *    -I.e., user data will be kept until callback is called
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure
 *
 * @see ewk_context_origins_free()
 */
EXPORT_API Eina_Bool ewk_context_web_storage_origins_get(
    Ewk_Context* context, Ewk_Web_Storage_Origins_Get_Callback callback,
    void* user_data);

/**
 * @brief Gets usage of web storage for certain origin.
 *
 * @details This function allocates memory for context structure made from
 *          callback and user_data.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] origin security origin
 * @param[in] callback callback to get web storage usage
 * @param[in] user_data user_data will be passed when callback is called\n
 *    -I.e., user data will be kept until callback is called
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_web_storage_usage_for_origin_get(
    Ewk_Context* context, Ewk_Security_Origin* origin,
    Ewk_Web_Storage_Usage_Get_Callback callback, void* user_data);

/**
 * @brief Queries if the cache is enabled
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 *
 * @return @c EINA_TRUE is cache is enabled or @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_cache_disabled_get(const Ewk_Context* context);

/**
 * @brief start memory sampler.
 *
 * @since_tizen 2.3
 *
 * @details This function is for logging of memory usage. The log file will be
 *          created in /tmp path. Sample is gathered every second until the
 *          specified interval has passed, 0 for infinite.
 *
 * @param[in] context context object
 * @param[in] timer_interval time in seconds after which sampler should
 *            stop gathering the measurement
 */
EXPORT_API void ewk_context_memory_sampler_start(Ewk_Context* context,
                                                 double timer_interval);

/**
 * @brief stop memory sampler.
 *
 * @param[in] context context object
 */
EXPORT_API void ewk_context_memory_sampler_stop(Ewk_Context* context);

/**
 * @brief Callback for ewk_context_vibration_client_callbacks_set
 *
 * @since_tizen 2.3
 *
 * @details Type definition for a function that will be called back when vibrate
 *          request receiveed from the vibration controller.
 *
 * @param[in] vibration_time the number of vibration times
 * @param[in] user_data user_data will be passsed when
 *            ewk_context_vibration_client_callbacks_set is called
 */
typedef void (*Ewk_Vibration_Client_Vibrate_Cb)(uint64_t vibration_time,
                                                void* user_data);

/**
 * @brief Callback for ewk_context_vibration_client_callbacks_set
 *
 * @since_tizen 2.3
 *
 * @details Type definition for a function that will be called back when cancel
 *           vibration request receiveed from the vibration controller.
 *
 * @param[in] user_data user_data will be passsed when
 *            ewk_context_vibration_client_callbacks_set is called
 */
typedef void (*Ewk_Vibration_Client_Vibration_Cancel_Cb)(void* user_data);

/**
 * @brief Sets memory saving mode.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] mode or disable memory saving mode
 *
 */
EXPORT_API void ewk_context_memory_saving_mode_set(Ewk_Context* context,
                                                   Eina_Bool mode);

/**
 * @brief Struct for password data
 * @since_tizen 2.3
 */
struct Ewk_Password_Data {
  char* url;
  Eina_Bool useFingerprint;
};

/**
 * @brief Deletes password data from DB for given URL
 *
 * @details The API will delete the a password data from DB.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] url url saved form password
 *
 * @see ewk_context_form_password_data_list_free
 * @see ewk_context_form_password_data_list_get
 */
EXPORT_API void ewk_context_form_password_data_delete(Ewk_Context* context,
                                                      const char* url);

/**
 * @brief Callback for ewk_context_form_password_data_list_get
 *
 * @since_tizen 3.0
 *
 * @param[in] list list of Ewk_Password_Data
 * @param[in] user_data user_data will be passed when
 *            ewk_context_form_password_data_list_get is called
 */
typedef void (*Ewk_Context_Form_Password_Data_List_Get_Callback)(
    Eina_List* list, void* user_data);

/**
 * @brief Asynchronous request to get list of all password data
 *
 * @since_tizen 3.0
 *
 * @param[in] context context object
 * @param[in] callback callback to get list of password data
 * @param[in] user_data user data will be passed when callback is called
 *
 * @see ewk_context_form_password_data_delete
 * @see ewk_context_form_password_data_list_free
 */
EXPORT_API void ewk_context_form_password_data_list_get(
    Ewk_Context* context,
    Ewk_Context_Form_Password_Data_List_Get_Callback callback, void* user_data);

/**
 * @brief Deletes a given password data list
 *
 * @details The API will delete the a password data list only from the memory.\n
 *          To remove the password data for URL permenantly,
 *          use ewk_context_form_password_data_delete
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] list Eina_List with Ewk_Password_Data
 *
 * @see ewk_context_form_password_data_delete
 * @see ewk_context_form_password_data_list_get
 */
EXPORT_API void ewk_context_form_password_data_list_free(Ewk_Context* context,
                                                         Eina_List* list);

/**
 * @brief Requests setting of the favicon database path.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] path database path
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_icon_database_path_set(Ewk_Context* context,
                                                        const char* path);

/**
 * @brief Deletes all known icons from database.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 */
EXPORT_API void ewk_context_icon_database_delete_all(Ewk_Context* context);

/**
 * @brief Requests for getting web database quota for origin.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] callback callback to get web database quota
 * @param[in] user_data user_data will be passed when callback is called\n
 *    -I.e., user data will be kept until callback is called
 * @param[in] origin database origin
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_web_database_quota_for_origin_get(
    Ewk_Context* context, Ewk_Web_Database_Quota_Get_Callback callback,
    void* user_data, Ewk_Security_Origin* origin);

/**
 * @brief Requests for getting web application cache origins.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] callback callback to get web application cache origins
 * @param[in] user_data user_data will be passsed when callback is called
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 *
 * @see ewk_context_origins_free
 */
EXPORT_API Eina_Bool ewk_context_application_cache_origins_get(
    Ewk_Context* context,
    Ewk_Web_Application_Cache_Origins_Get_Callback callback, void* user_data);

/**
 * To declare application type
 *
 * @param context context object
 * @param applicationType The Ewk_Application_Type enum
 *
 */
EXPORT_API void ewk_context_application_type_set(
    Ewk_Context* ewkContext, const Ewk_Application_Type applicationType);

/**
 * @brief Returns the application type.
 *
 * @param[in] context The context object
 *
 * @return #Ewk_Application_Type
 */
EXPORT_API Ewk_Application_Type
ewk_context_application_type_get(Ewk_Context* ewkContext);

EXPORT_API void ewk_context_form_candidate_data_clear(Ewk_Context* ewkContext);

EXPORT_API void ewk_context_form_password_data_clear(Ewk_Context* ewkContext);

/**
 * Requests for setting application memory cache capacities.
 *
 * @param ewk_context context object
 * @param cache_min_dead_capacity set min dead memory cache value
 * @param cache_max_dead_capacity set max dead memory cache value
 * @param cache_total_capacity set total memory cache value
 *
 */
EXPORT_API void ewk_context_cache_model_memory_cache_capacities_set(
    Ewk_Context* ewk_context, uint32_t cache_min_dead_capacity,
    uint32_t cache_max_dead_capacity, uint32_t cache_total_capacity);

/**
 * Requests for getting application page cache capacity.
 *
 * @param ewk_context context object
 * @param cache_value the page memory cache value
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_cache_model_page_cache_capacity_get(
    Ewk_Context* ewk_context, uint32_t* cache_value);

/**
 * Requests for setting application page cache capacity.
 *
 * @param ewk_context context object
 * @param capacity set page memory cache capacity value
 */
EXPORT_API void ewk_context_cache_model_page_cache_capacity_set(
    Ewk_Context* ewk_context, uint32_t capacity);

/**
 * Requests for application memory cache capacity.
 *
 * @param ewk_context context object
 * @param capacity_value the memory cache capacity value
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_cache_model_memory_cache_capacity_get(
    Ewk_Context* ewk_context, uint32_t* capacity_value);

/**
 * Requests for application memory min dead capacity.
 *
 * @param ewk_context context object
 * @param capacity_value the memory min dead capacity value
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_cache_model_memory_min_dead_capacity_get(
    Ewk_Context* ewk_context, uint32_t* capacity_value);

/**
 * Requests for application memory max dead capacity.
 *
 * @param ewk_context context object
 * @param capacity_value the memory max dead capacity value
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_cache_model_memory_max_dead_capacity_get(
    Ewk_Context* ewk_context, uint32_t* capacity_value);

/**
 * @brief Sets default zoom factor
 *
 * Sets default zoom factor for all pages opened with this context. Default
 * zoom can be overridden with ewk_view_page_zoom_set on per page basis.
 *
 * @since_tizen 3.0
 *
 * @param[in] context context object
 * @param[in] zoom_factor default zoom factor
 */
EXPORT_API void ewk_context_default_zoom_factor_set(Ewk_Context* context,
                                                    double zoom_factor);

/**
 * @brief Gets default zoom factor
 *
 * Gets default zoom factor for all pages opened with this context.
 *
 * @since_tizen 3.0
 *
 * @param[in] context context object
 *
 * @return @c default zoom factor or negative value on error
 */
EXPORT_API double ewk_context_default_zoom_factor_get(Ewk_Context* context);

/**
 * @brief Sets callback for checking file request accessibility. When requested
 *        to load a file, need to invoke the callback to check whether the file
 *        path is accessible
 *
 * @param context context object
 * @param callback The callback of Ewk_Context_Check_Accessible_Path_Callback
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_check_accessible_path_callback_set(
    Ewk_Context* context, Ewk_Context_Check_Accessible_Path_Callback callback,
    void* user_data);

/**
 * @brief Registers url schemes as CORS enabled. It is applied
 *        for all the pages opened within the context.
 *        This API is supposed to be used by Web frameworks,
 *        not by Web browser application.
 *
 * @param context context object
 * @param schemes The URL schemes list which will be added to
 *                web security policy as valid schemes to pass CORS check.
 *
 */
EXPORT_API void ewk_context_register_url_schemes_as_cors_enabled(
    Ewk_Context* context, const Eina_List* schemes);

/**
 * @brief Register JS plugin mime types. It is applied
 *        for all the pages opened within the context.
 *        The API is intended to be used by web applications to
 *        override default behaviour of the object tag.
 *
 * @param context context object
 * @param mime_types The MIME types will be checked by the renderer frame loader
 *                   to skip creating default frame for the object tags
 *                   with the registered MIME type.
 */
EXPORT_API void ewk_context_register_jsplugin_mime_types(
    Ewk_Context* context, const Eina_List* mime_types);

/**
 * Sets callback for started download.
 *
 * @param context context object
 * @param callback callback for started download
 * @param user_data user data
 */
EXPORT_API void ewk_context_did_start_download_callback_set(
    Ewk_Context* context, Ewk_Context_Did_Start_Download_Callback callback,
    void* user_data);

/**
 * Sets vibration client callbacks to handle the tactile feedback in the form of
 * vibration in the client application when the content asks for vibration.
 *
 * To stop listening for vibration events, you may call this function with @c
 * @param vibrate The function to call when the vibrate request received from
 * the controller (may be @c NULL).
 * @param cancel The function to call when the cancel vibration request received
 *        from the controller (may be @c NULL).
 * @param data User data (may be @c NULL).
 */
EXPORT_API void ewk_context_vibration_client_callbacks_set(
    Ewk_Context* context, Ewk_Vibration_Client_Vibrate_Cb vibrate,
    Ewk_Vibration_Client_Vibration_Cancel_Cb cancel, void* data);

/**
 * @brief Set WebSDI.
 *        To use WebSDI for client authentication,
 *        'web-sdi' metadata need to be added in config.xml of each app.
 *        In that case, XWalk will call ewk_context_websdi_set API with true
 *        value.
 *
 * @param enable true means app wants to get a Client Certificate as WebSDI,
 *               otherwise, false
 */
EXPORT_API void ewk_context_websdi_set(Eina_Bool enable);

EXPORT_API Eina_Bool ewk_context_websdi_get();

/**
 * @brief Set disable nosniff.
 *        Some apps have the wrong "Content-Type" in response header and
 *        at the same time, set "X-Content-Type-Options:nosniff" which
 *        don't allow WebCore to sniff the right content type
 *
 *since_tizen 3.0
 *
 *
 * @param context context object.
 * @param enable true means app wants to disable nosniff,
 *               otherwise, false
 */
EXPORT_API void ewk_context_disable_nosniff_set(Ewk_Context* context,
                                                Eina_Bool enable);

/**
 * @brief Set EMP Certificate file.
 *        First, Try to check a empCAPath and load certificate files in a
 * empCAPath. If there is no file or it is not valid file, Try to load
 * certificate files in a defaultCAPath.
 *
 * @param context context object.
 * @param empCAPath Path to certificate files downloaded via EMP
 * @param defaultCAPath Path to certificate files have been used
 */
EXPORT_API void ewk_context_emp_certificate_file_set(
    Ewk_Context* context, const char* emp_ca_path, const char* default_ca_path);

/**
 * @brief Requests for getting web database usage for origin.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] callback callback to get web database usage
 * @param[in] user_data user_data will be passed when callback is called\n
 *    -I.e., user data will be kept until callback is called
 * @param[in] origin database origin
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_web_database_usage_for_origin_get(
    Ewk_Context* context, Ewk_Web_Database_Usage_Get_Callback callback,
    void* user_data, Ewk_Security_Origin* origin);

/**
 * @brief Updates use fingerprint value from DB for given URL
 *
 * @details The API will update use fingerprint value on DB for given URL.
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] url url saved form password
 * @param[in] useFingerprint fingerprint for given URL will be used or not
 *
 * @see ewk_context_form_password_data_list_free
 * @see ewk_context_form_password_data_delete
 * @see ewk_context_form_password_data_list_get
 */
EXPORT_API void ewk_context_form_password_data_update(Ewk_Context* context,
                                                      const char* url,
                                                      Eina_Bool useFingerprint);

/**
 * @brief Sets the list of preferred languages.
 *
 * @details This function sets the list of preferred langages.\n
 *          This list will be used to build the "Accept-Language" header that
 *          will be included in the network requests.\n
 *          The client can pass @c NULL for languages to clear what is set
 *          before.
 *
 * @since_tizen 2.3
 *
 * @remarks All contexts will be affected.
 *
 * @param[in] languages The list of preferred languages (char* as data),\n
 *                      otherwise @c NULL
 */
EXPORT_API void ewk_context_preferred_languages_set(Eina_List* languages);

/**
 * Sets PWA storage path @a context.
 *
 * @param context context object
 * @param pwa_storage_path PWA storage path
 *
 * @return @c EINA_TRUE if the pwa_storage_path set successfully, @c EINA_FALSE
 * otherwise
 */
EXPORT_API Eina_Bool ewk_context_pwa_storage_path_set(
    Ewk_Context* context, const char* pwa_storage_path);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // ewk_context_product_h
