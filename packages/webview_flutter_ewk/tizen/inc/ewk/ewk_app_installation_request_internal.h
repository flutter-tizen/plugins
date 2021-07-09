// Copyright 2017 Samsung Electronics. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ewk_app_installation_request_internal_h
#define ewk_app_installation_request_internal_h

#include "ewk_manifest.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Struct for app installation request.
 *
 * @since_tizen 4.0
 */
typedef struct _Ewk_App_Installation_Request Ewk_App_Installation_Request;

/**
 * @brief Gets origin from which app installation request was called
 *
 * @since_tizen 4.0
 *
 * @param[in] request app installation request
 *
 * @return @c origin string. The string is only valid until related
 *         Ewk_App_Installation_Request object is valid.
 */
EXPORT_API const char* ewk_app_installation_request_origin_get(
    Ewk_App_Installation_Request* request);

/**
 * @brief Gets app url provided along with app installation request
 *
 * @since_tizen 4.0
 *
 * @param[in] request app installation request
 *
 * @return @c app url string. The string is only valid until related
 *         Ewk_App_Installation_Request object is valid.
 */
EXPORT_API const char* ewk_app_installation_request_app_url_get(
    Ewk_App_Installation_Request* request);

/**
 * @brief Gets manifest obtained from url provided along with app installation
 *        request
 *
 * @since_tizen 4.0
 *
 * @param[in] request app installation request
 *
 * @return @c manifest instance. The object is only valid until related
 *         Ewk_App_Installation_Request object is valid.
 */
EXPORT_API Ewk_View_Request_Manifest* ewk_app_installation_request_manifest_get(
    Ewk_App_Installation_Request* request);

/**
 * @brief Gets service worker url provided along with app installation request
 *
 * @since_tizen 4.0
 *
 * @param[in] request app installation request
 *
 * @return @c service worker url string. The string is only valid until related;
 * Can be nullptr Ewk_App_Installation_Request object is valid.
 */
EXPORT_API const char* ewk_app_installation_request_service_worker_url_get(
    Ewk_App_Installation_Request* request);

#ifdef __cplusplus
}
#endif

#endif  // ewk_app_installation_request_internal_h
