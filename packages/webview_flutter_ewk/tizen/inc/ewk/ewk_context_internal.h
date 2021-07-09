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
 * @file    ewk_context_internal.h
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
 */

#ifndef ewk_context_internal_h
#define ewk_context_internal_h

#include "ewk_application_cache_manager_internal.h"
#include "ewk_context.h"
#include "ewk_favicon_database_internal.h"
#include "ewk_notification_internal.h"
#include "ewk_security_origin_internal.h"
#include "ewk_storage_manager_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Extensible API enum is not supported in chromium.
typedef void* Ewk_Extensible_API;

typedef struct Ewk_Context_Exceeded_Quota Ewk_Context_Exceeded_Quota;

typedef void (*Ewk_Context_Notification_Show_Callback)(Ewk_Context*,
                                                       Ewk_Notification*,
                                                       void*);

typedef void (*Ewk_Context_Notification_Cancel_Callback)(Ewk_Context*, uint64_t,
                                                         void*);

/**
 * @internal
 * @brief Enumeration for compression proxy image quality.
 * @since_tizen 2.3
 */
enum _Ewk_Compression_Proxy_Image_Quality {
  EWK_COMPRESSION_PROXY_IMAGE_QUALITY_LOW = 0, /**< @internal Low */
  EWK_COMPRESSION_PROXY_IMAGE_QUALITY_MEDIUM,  /**< @internal Medium */
  EWK_COMPRESSION_PROXY_IMAGE_QUALITY_HIGH     /**< @internal High */
};

/**
 * @internal
 * @brief Creates a type name for @a #Ewk_Compression_Proxy_Image_Quality.
 * @since_tizen 2.3
 */
typedef enum _Ewk_Compression_Proxy_Image_Quality
    Ewk_Compression_Proxy_Image_Quality;

/**
 * Deletes Ewk_Context.
 *
 * @param context Ewk_Context to delete
 */
EXPORT_API void ewk_context_delete(Ewk_Context* context);

/**
 * Notify low memory to free unused memory.
 *
 * @param o context object to notify low memory.
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE otherwise.
 */
EXPORT_API Eina_Bool ewk_context_notify_low_memory(Ewk_Context* ewkContext);

/**
 * @typedef Ewk_Local_File_System_Origins_Get_Callback
 * Ewk_Local_File_System_Origins_Get_Callback
 * @brief Type definition for use with
 * ewk_context_local_file_system_origins_get()
 */
typedef void (*Ewk_Local_File_System_Origins_Get_Callback)(Eina_List* origins,
                                                           void* user_data);

/**
 * Callback for ewk_context_application_cache_origins_get
 *
 * @param origins web application cache origins
 * @param user_data user_data will be passsed when
 * ewk_context_application_cache_origins_get is called
 */
typedef void (*Ewk_Web_Application_Cache_Origins_Get_Callback)(
    Eina_List* origins, void* user_data);

/**
 * Callback for ewk_context_application_cache_quota_get.
 *
 * @param quota web application cache quota
 * @param user_data user_data will be passsed when
 * ewk_context_application_cache_quota_get is called
 */
typedef void (*Ewk_Web_Application_Cache_Quota_Get_Callback)(int64_t quota,
                                                             void* user_data);

/**
 * Callback for ewk_context_application_cache_usage_for_origin_get.
 *
 * @param usage web application cache usage for origin
 * @param user_data user_data will be passsed when
 * ewk_context_application_cache_usage_for_origin_get is called
 */
typedef void (*Ewk_Web_Application_Cache_Usage_For_Origin_Get_Callback)(
    int64_t usage, void* user_data);

/**
 * Callback for ewk_context_application_cache_path_get.
 *
 * @param path web application cache directory
 * @param user_data user_data will be passsed when
 * ewk_context_application_cache_path_get is called
 */
// typedef void (*Ewk_Web_Application_Cache_Path_Get_Callback)(const char* path,
// void* user_data);

/**
 * Callback for ewk_context_web_database_origins_get.
 *
 * @param origins web database origins
 * @param user_data user_data will be passsed when
 * ewk_context_web_database_origins_get is called
 */
typedef void (*Ewk_Web_Database_Origins_Get_Callback)(Eina_List* origins,
                                                      void* user_data);

/**
 * Callback for ewk_context_web_database_quota_for_origin_get.
 *
 * @param quota web database quota
 * @param user_data user_data will be passsed when
 * ewk_context_web_database_quota_for_origin_get is called
 */
typedef void (*Ewk_Web_Database_Quota_Get_Callback)(uint64_t quota,
                                                    void* user_data);

/**
 * Callback for ewk_context_web_database_path_get.
 *
 * @param path web database directory
 * @param user_data user_data will be passsed when
 * ewk_context_web_database_path_get is called
 */
// typedef void (*Ewk_Web_Database_Path_Get_Callback)(const char* path, void*
// user_data);

/**
 * Callback for didStartDownload
 *
 * @param download_url url to download
 * @param user_data user_data will be passsed when download is started
 */
typedef void (*Ewk_Context_Did_Start_Download_Callback)(
    const char* download_url, void* user_data);

/**
 * Callback for overriding default mime type
 *
 * @param url url for which the mime type can be overridden
 * @param mime current mime type assumed by the web engine
 * @param new_mime string with a new mime type for content pointer by url.
 * Should be allocated dynamically by malloc or calloc. If callback returns
 * EINA_TRUE, the browser will take ownership of the allocated memory and free
 * it when it's no longer needed using the free() function
 * @return true in case mime should be overridden by the contents of new_mime,
 * false otherwise. If false will be returned, the browser won't free the
 * new_mime
 * @param user_data user_data will be passsed when
 * ewk_context_mime_override_callback_set is called
 */
typedef Eina_Bool (*Ewk_Context_Override_Mime_For_Url_Callback)(
    const char* url, const char* mime, char** new_mime, void* user_data);

/*
 * Callback for changed profiles.
 *
 * @param data user data will be passed when autofill profile is changed
 */
typedef void (*Ewk_Context_Form_Autofill_Profile_Changed_Callback)(void* data);

/*
 * Callback for changed credit_cards.
 *
 * @param data user data will be passed when credit card is changed
 */
typedef void (*Ewk_Context_Form_Autofill_CreditCard_Changed_Callback)(
    void* data);

/**
 * Sets callback for credit_cards change notification.
 *
 * @param callback pointer to callback function
 * @param user_data user data returned on callback
 *
 */
EXPORT_API void ewk_context_form_autofill_credit_card_changed_callback_set(
    Ewk_Context_Form_Autofill_CreditCard_Changed_Callback callback,
    void* user_data);

/**
 * Requests for freeing origins.
 *
 * @param origins list of origins
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_origins_free(Eina_List* origins);

/**
 * Requests for deleting web application cache for origin.
 *
 * @param context context object
 * @param origin application cache origin
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_application_cache_delete(
    Ewk_Context* context, Ewk_Security_Origin* origin);

/**
 * Requests for getting application cache usage for origin.
 *
 * @param context context object
 * @param origin security origin
 * @param result_callback callback to get web database usage
 * @param user_data user_data will be passed when result_callback is called
 *    -I.e., user data will be kept until callback is called
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_application_cache_usage_for_origin_get(
    Ewk_Context* context, const Ewk_Security_Origin* origin,
    Ewk_Web_Application_Cache_Usage_For_Origin_Get_Callback callback,
    void* user_data);

/**
 * Requests for deleting web databases for origin.
 *
 * @param context context object
 * @param origin database origin
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_web_database_delete(
    Ewk_Context* context, Ewk_Security_Origin* origin);

/**
 * Requests for getting web database origins.
 *
 * @param context context object
 * @param result_callback callback to get web database origins
 * @param user_data user_data will be passed when result_callback is called
 *    -I.e., user data will be kept until callback is called
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 *
 * @see ewk_context_origins_free
 */
EXPORT_API Eina_Bool ewk_context_web_database_origins_get(
    Ewk_Context* context, Ewk_Web_Database_Origins_Get_Callback callback,
    void* user_data);

/**
 * Requests for setting soup data path(soup data include cookie and cache).
 *
 * @param context context object
 * @param path soup data path to set
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_soup_data_directory_set(Ewk_Context* context,
                                                         const char* path);

/**
 * Toggles the cache enable and disable
 *
 * Function works asynchronously.
 * By default the cache is disabled resulting in not storing network data on
 * disk.
 *
 * @param context context object
 * @param enable or disable cache
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_cache_disabled_set(Ewk_Context* ewkContext,
                                                    Eina_Bool cacheDisabled);

/**
 * Adds CA certificates to persistent NSS certificate database
 *
 * Function accepts a path to a CA certificate file, a path to a directory
 * containing CA certificate files, or a colon-seprarated list of those.
 *
 * Certificate files should have *.crt extension.
 *
 * Directories are traversed recursively.
 *
 * @param context context object
 * @param certificate_file path to a CA certificate file(s), see above for
 * details
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_certificate_file_set(
    Ewk_Context* context, const char* certificate_path);

/**
 * @internal
 * @brief Enable or disable proxy
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] enabled enable or disable proxy
 *
 */
EXPORT_API void ewk_context_compression_proxy_enabled_set(Ewk_Context* context,
                                                          Eina_Bool enabled);

/**
 * @internal
 * @brief Returns currently proxy is enabled or not
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 *
 * @return @c EINA_TRUE on enabled or @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool
ewk_context_compression_proxy_enabled_get(Ewk_Context* context);

/**
 * @internal
 * @brief Set image quality of proxy compression
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[in] quality image quality
 *
 */
EXPORT_API void ewk_context_compression_proxy_image_quality_set(
    Ewk_Context* context, Ewk_Compression_Proxy_Image_Quality quality);

/**
 * @internal
 * @brief Returns current image quality
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 *
 * @return @c Ewk_Compression_Proxy_Image_Quality
 */
EAPI Ewk_Compression_Proxy_Image_Quality
ewk_context_compression_proxy_image_quality_get(Ewk_Context* context);

/**
 * @internal
 * @brief Returns original and compressed data size
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 * @param[out] original_size uncompressed data size
 * @param[out] compressed_size compressed size
 *
 */
EXPORT_API void ewk_context_compression_proxy_data_size_get(
    Ewk_Context* context, unsigned* original_size, unsigned* compressed_size);

/**
 * @internal
 * @brief Reset original and compressed data size
 *
 * @since_tizen 2.3
 *
 * @param[in] context context object
 *
 */
EXPORT_API void ewk_context_compression_proxy_data_size_reset(
    Ewk_Context* context);

/**
 * Gets CA certifcate file path
 *
 * It returns an internal string and should not be modified.
 *
 * @param context context object
 *
 * @return @c certificate_file is path which is set during
 * ewk_context_certificate_file_set or @c NULL otherwise
 */
EXPORT_API const char* ewk_context_certificate_file_get(
    const Ewk_Context* context);

/**
 * Requests to clear cache
 *
 * @param context context object
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_cache_clear(Ewk_Context* ewkContext);

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
 * Sets callback for overriding mime type
 *
 * @param context context object
 * @param callback callback to be invoked whenver the mime type can be
 * overridden
 * @param user_data user data
 */
EXPORT_API void ewk_context_mime_override_callback_set(
    Ewk_Context* context, Ewk_Context_Override_Mime_For_Url_Callback callback,
    void* user_data);

/**
 * @typedef Ewk_Vibration_Client_Vibrate_Cb Ewk_Vibration_Client_Vibrate_Cb
 * @brief Type definition for a function that will be called back when vibrate
 * request receiveed from the vibration controller.
 */
typedef void (*Ewk_Vibration_Client_Vibrate_Cb)(uint64_t vibration_time,
                                                void* user_data);

/**
 * @typedef Ewk_Vibration_Client_Vibration_Cancel_Cb
 * Ewk_Vibration_Client_Vibration_Cancel_Cb
 * @brief Type definition for a function that will be called back when cancel
 * vibration request receiveed from the vibration controller.
 */
typedef void (*Ewk_Vibration_Client_Vibration_Cancel_Cb)(void* user_data);

/**
 * Increases the reference count of the given object.
 *
 * @param context context object to increase the reference count
 *
 * @return Ewk_Context object on success or @c NULL on failure
 */
EXPORT_API Ewk_Context* ewk_context_ref(Ewk_Context* context);

/**
 * Decreases the reference count of the given object, possibly freeing it.
 *
 * When the reference count it's reached 0, the Ewk_Context is freed.
 *
 * @param context context object to decrease the reference count
 */
EXPORT_API void ewk_context_unref(Ewk_Context* context);
/**
 * Gets default Ewk_Context instance.
 *
 * The returned Ewk_Context object @b should not be unref'ed if application
 * does not call ewk_context_ref() for that.
 *
 * @return Ewk_Context object.
 */
EXPORT_API Ewk_Context* ewk_context_default_get(void);

/**
 * Creates a new Ewk_Context.
 *
 * The returned Ewk_Context object @b should be unref'ed after use.
 *
 * @return Ewk_Context object on success or @c NULL on failure
 *
 * @see ewk_context_unref
 * @see ewk_context_new_with_injected_bundle_path
 */
EXPORT_API Ewk_Context* ewk_context_new(void);

/**
 * Creates a new Ewk_Context.
 *
 * The returned Ewk_Context object @b should be unref'ed after use.
 *
 * @param path path of injected bundle library
 *
 * @return Ewk_Context object on success or @c NULL on failure
 *
 * @see ewk_context_unref
 * @see ewk_context_new
 */
EXPORT_API Ewk_Context* ewk_context_new_with_injected_bundle_path(
    const char* path);

/**
 * @brief Creates a new Ewk_Context in incognito mode.
 *
 * @param[in] path Path of injected bundle library
 *
 * @return The Ewk_Context object on success or otherwise @c NULL on failure
 *
 * @see ewk_context_new
 * @see ewk_context_new_with_injected_bundle_path
 */
EXPORT_API Ewk_Context*
ewk_context_new_with_injected_bundle_path_in_incognito_mode(const char* path);

/**
 * Sets additional plugin directory.
 *
 * @param context context object
 * @param path the directory to be set
 *
 * @return @c EINA_TRUE if the directory was set, @c EINA_FALSE otherwise
 */
EINA_DEPRECATED EXPORT_API Eina_Bool
ewk_context_additional_plugin_path_set(Ewk_Context* context, const char* path);

/**
 * Sets vibration client callbacks to handle the tactile feedback in the form of
 * vibration in the client application when the content asks for vibration.
 *
 * To stop listening for vibration events, you may call this function with @c
 * NULL for the callbacks.
 *
 * @param context context object to set vibration client callbacks.
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
 * Sets callback for profiles change notification.
 *
 * @param callback pointer to callback function
 * @param user_data user data returned on callback
 *
 */
EXPORT_API void ewk_context_form_autofill_profile_changed_callback_set(
    Ewk_Context_Form_Autofill_Profile_Changed_Callback callback,
    void* user_data);

/**
 * Gets the existing profile for given index
 *
 * The obtained profile must be deleted by ewk_autofill_profile_delete.
 *
 * @param context context object
 * @param id profile id
 *
 * @return @c Ewk_Autofill_Profile if profile exists, @c NULL otherwise
 * @see ewk_autofill_profile_delete
 */
EXPORT_API Ewk_Autofill_Profile* ewk_context_form_autofill_profile_get(
    Ewk_Context* context, unsigned id);

/**
 * Get tizen extensible api enable state
 *
 * @param context context object
 * @param extensible_api extensible API name of every kind
 *
 * @return @c EINA_TRUE if the extensibleAPI set as true or @c EINA_FALSE
 * otherwise
 */
EXPORT_API Eina_Bool ewk_context_tizen_extensible_api_string_get(
    Ewk_Context* context, const char* extensible_api);

/**
 * Toggles tizen extensible api enable and disable
 *
 * @param context context object
 * @param extensible_api extensible API name of every kind
 * @param enable enable or disable tizen extensible api
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_tizen_extensible_api_string_set(
    Ewk_Context* context, const char* extensible_api, Eina_Bool enable);

/**
 * Toggles tizen extensible api enable and disable
 *
 * @param context context object
 * @param extensibleAPI extensible API of every kind
 * @param enable enable or disable tizen extensible api
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE otherwise
 */
EINA_DEPRECATED EXPORT_API Eina_Bool ewk_context_tizen_extensible_api_set(
    Ewk_Context* context, Ewk_Extensible_API extensible_api, Eina_Bool enable);

/**
 * Get tizen extensible api enable state
 *
 * @param context context object
 * @param extensibleAPI extensible API of every kind
 *  *
 * @return @c EINA_TRUE if the extensibleAPI set as true or @c EINA_FALSE
 * otherwise
 */
EINA_DEPRECATED EXPORT_API Eina_Bool ewk_context_tizen_extensible_api_get(
    Ewk_Context* ewkContext, Ewk_Extensible_API extensibleAPI);

/**
 * Reset storage path such as web storage, web database, application cache and
 * so on
 *
 * @param context context object
 *
 */
EXPORT_API void ewk_context_storage_path_reset(Ewk_Context* ewkContext);

/**
 * Sets the given id for the pixmap
 *
 * @param context context object
 * @param pixmap id
 *
 * @return @c EINA_TRUE if the pixmap set successfully, @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_pixmap_set(Ewk_Context* context, int pixmap);

/**
 * Start the inspector server
 *
 * @param context context object
 * @param port number
 *
 * @return @c return the port number
 */
EXPORT_API unsigned int ewk_context_inspector_server_start(Ewk_Context* context,
                                                           unsigned int port);

/**
 * Stop the inspector server
 *
 * @param context context object
 *
 * @return @c EINA_TRUE if the inspector server stop set successfully, @c
 * EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_inspector_server_stop(Ewk_Context* context);

///------- belows are extension of chromium-ewk ---------------------------

/**
 * Gets the id for the pixmap
 *
 * @param context context object
 *
 * @return @c id for the pixmap. On error default return is 0.
 */
EXPORT_API int ewk_context_pixmap_get(Ewk_Context* context);

EINA_DEPRECATED EXPORT_API void ewk_send_widget_info(Ewk_Context* context,
                                                     const char* tizen_id,
                                                     double scale,
                                                     const char* theme,
                                                     const char* encodedBundle);

/**
 * Sets tizen application id for @a context.
 *
 * Must be called before loading content in order to call
 * DynamicPluginStartSession of injected bundle.
 *
 * @param context context object
 * @param tizen_app_id tizen application id
 */
EXPORT_API void ewk_context_tizen_app_id_set(Ewk_Context* context,
                                             const char* tizen_app_id);

/**
 * Sets tizen application version for @a context.
 *
 * Must be called after use ewk_context_tizen_app_id_set
 * or ewk_send_widget_info beacuse need drop the process
 * privillage and set dynamic plugin.
 *
 * @param context context object
 * @param tizen_app_version tizen application version
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_tizen_app_version_set(
    Ewk_Context* context, const char* tizen_app_version);

/**
 * Gets the application cache manager instance for this @a context.
 *
 * @param context context object to query.
 *
 * @return Ewk_Cookie_Manager object instance or @c NULL in case of failure.
 */
EXPORT_API Ewk_Application_Cache_Manager*
ewk_context_application_cache_manager_get(const Ewk_Context* context);

/**
 * Gets the favicon database instance for this @a context.
 *
 * @param context context object to query.
 *
 * @return Ewk_Favicon_Database object instance or @c NULL in case of failure.
 */
EXPORT_API Ewk_Favicon_Database* ewk_context_favicon_database_get(
    const Ewk_Context* context);

/**
 * Sets the favicon database directory for this @a context.
 *
 * Sets the directory path to be used to store the favicons database
 * for @a context on disk. Passing @c NULL as @a directory_path will
 * result in using the default directory for the platform.
 *
 * Calling this method also means enabling the favicons database for
 * its use from the applications, it is therefore expected to be
 * called only once. Further calls for the same instance of
 * @a context will not have any effect.
 *
 * @param context context object to update
 * @param directory_path database directory path to set
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_favicon_database_directory_set(
    Ewk_Context* context, const char* directory_path);

/**
 * Gets the storage manager instance for this @a context.
 *
 * @param context context object to query.
 *
 * @return Ewk_Storage_Manager object instance or @c NULL in case of failure.
 */
EXPORT_API Ewk_Storage_Manager* ewk_context_storage_manager_get(
    const Ewk_Context* context);

/**
 * Requests for deleting all web databases.
 *
 * @param context context object
 *
 * @return @c EINA_TRUE on successful request or @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool ewk_context_web_database_delete_all(Ewk_Context* context);

/**
 * Sets app_control.
 *
 * @param context context object
 * @param app_control app_control handle.
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise
 *
 */
EXPORT_API Eina_Bool ewk_context_app_control_set(const Ewk_Context* context,
                                                 void* app_control);

EXPORT_API Eina_Bool ewk_context_notification_callbacks_set(
    Ewk_Context* context, Ewk_Context_Notification_Show_Callback show_callback,
    Ewk_Context_Notification_Cancel_Callback cancel_callback, void* user_data);

EXPORT_API Eina_Bool
ewk_context_notification_callbacks_reset(Ewk_Context* context);

/**
 * Set time offset
 *
 * @param context context object
 * @param timeOffset The value will be added to system time as offset, it will
 * be used for adjusting JS Date, certificate, cookie, animation etc.
 *
 */
EXPORT_API void ewk_context_time_offset_set(Ewk_Context* context,
                                            double time_offset);

/**
 * Set timezone offset
 *
 * @param context context object
 * @param timezoneOffset The value will be used to set tz ENV.
 * @param daylightSavingTime The value is for daylight saving time use, and will
 * be used to set tz ENV.
 *
 */
EXPORT_API void ewk_context_timezone_offset_set(Ewk_Context* context,
                                                double time_zone_offset,
                                                double daylight_saving_time);

/**
 * Sets max refresh rate for @a context.
 *
 * @param context context object
 * @param max_refresh_rate screen FPS will not exceed max_refresh_rate. It can
 * be from @c 1 to @c 60
 */
EXPORT_API void ewk_context_max_refresh_rate_set(Ewk_Context* context,
                                                 int max_refresh_rate);

enum _Ewk_Audio_Latency_Mode {
  EWK_AUDIO_LATENCY_MODE_LOW = 0,   /**< Low audio latency mode */
  EWK_AUDIO_LATENCY_MODE_MID,       /**< Middle audio latency mode */
  EWK_AUDIO_LATENCY_MODE_HIGH,      /**< High audio latency mode */
  EWK_AUDIO_LATENCY_MODE_ERROR = -1 /** Error */
};

typedef enum _Ewk_Audio_Latency_Mode Ewk_Audio_Latency_Mode;

/**
 * @brief Set the mode of audio latency.
 *
 * @since_tizen 3.0
 *
 * @param[in] context context object
 * @param[in] latency_mode Ewk_Audio_Latency_Mode
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_audio_latency_mode_set(
    Ewk_Context* context, Ewk_Audio_Latency_Mode latency_mode);

/**
 * @brief Get the mode of audio latency.
 *
 * @since_tizen 3.0
 *
 * @param[in] context context object
 *
 * @return @c Ewk_Audio_Latency_Mode if successful, @c
 * EWK_AUDIO_LATENCY_MODE_ERROR otherwise
 */
EXPORT_API Ewk_Audio_Latency_Mode
ewk_context_audio_latency_mode_get(Ewk_Context* context);

/**
 * @typedef Ewk_Push_Message_Cb
 * @brief Type definition for a function that will be called back when send
 * web_push_message request receiveed from the web_push controller.
 */
typedef void (*Ewk_Push_Message_Cb)(const char* sender_id,
                                    const char* push_data, void* user_data);

/**
 * Sets web push callbacks to handle the web push message
 *
 * To stop web push events, you may call this function with @c
 * NULL for the callbacks.
 *
 * @param context context object to set vibration client callbacks.
 * @param push call function when the send the web push message from the
 *        controller (may be @c NULL).
 * @param user_data User data (may be @c NULL).
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_push_message_callback_set(
    Ewk_Context* context, Ewk_Push_Message_Cb callback, void* user_data);

/**
 * Sends the web push message to control web push events.
 *
 * @param context context object to set vibration client callbacks.
 * @param push_data User data (may be @c NULL).
 *
 * @return @c EINA_TRUE if successful, @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_context_send_push_message(Ewk_Context* context,
                                                   const char* push_data);

/**
 * Result callback for @a ewk_context_service_worker_register api.
 *
 * @param context context object
 * @param scope_url scope url for which service worker registration was called
 * @param script_url service worker script url
 * @param result @c EINA_TRUE on success or @c EINA_FALSE on failure
 * @param user_data user data passed to @a ewk_context_service_worker_register
 * api
 */
typedef void (*Ewk_Context_Service_Worker_Registration_Result_Callback)(
    Ewk_Context* context, const char* scope_url, const char* script_url,
    Eina_Bool result, void* user_data);

/**
 * Register service worker for scope. It allows to register third party service
 * workers also.
 *
 * @param context context object
 * @param scope_url scope url for which service worker should be registered
 * @param script_url service worker script url
 * @param result_callback result callback
 * @param user_data user data to be passed to @a result_callback
 */
EXPORT_API void ewk_context_service_worker_register(
    Ewk_Context* context, const char* scope_url, const char* script_url,
    Ewk_Context_Service_Worker_Registration_Result_Callback result_callback,
    void* user_data);

/**
 * Result callback for @a ewk_context_service_worker_unregister api.
 *
 * @param context context object
 * @param scope_url scope url for which service worker unregistration was called
 * @param result @c EINA_TRUE on success or @c EINA_FALSE on failure
 * @param user_data user data passed to @a ewk_context_service_worker_unregister
 * api
 */
typedef void (*Ewk_Context_Service_Worker_Unregistration_Result_Callback)(
    Ewk_Context* context, const char* scope_url, Eina_Bool result,
    void* user_data);

/**
 * Unregister service worker for scope.
 *
 * @param context context object
 * @param scope_url scope url for which service worker should be registered
 * @param result_callback result callback
 * @param user_data user data to be passed to @a result_callback
 */
EXPORT_API void ewk_context_service_worker_unregister(
    Ewk_Context* context, const char* scope_url,
    Ewk_Context_Service_Worker_Unregistration_Result_Callback result_callback,
    void* user_data);

/**
 * Sets whether to allow app control scheme(appcontrol://) or not.
 *
 * @since_tizen 5.0
 *
 * @param[in] context context object to enable/disable app control scheme
 * @param[in] enabled a state to set
 *
 */
EXPORT_API void ewk_context_enable_app_control(Ewk_Context* context,
                                               Eina_Bool enabled);

#ifdef __cplusplus
}
#endif

#endif  // ewk_context_internal_h
