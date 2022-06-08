/*
 * Copyright (C) 2012 Intel Corporation.
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
 * @file    ewk_error.h
 * @brief   This file describes the Ewk Web Error API.
 */

#ifndef ewk_error_h
#define ewk_error_h

#include <Eina.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup WEBVIEW
 * @{
 */

/**
 * @if MOBILE
 * @brief The structure type that creates a type name for #Ewk_Error.
 * @since_tizen 2.3
 * @endif
 */
typedef struct _Ewk_Error Ewk_Error;

/**
 * @if MOBILE
 * \enum   Ewk_Error_Code
 * @brief  Enumeration that provides an option to error codes.
 * @since_tizen 2.3
 * @endif
 */
typedef enum {
  EWK_ERROR_CODE_UNKNOWN,  /**< Unknown */
  EWK_ERROR_CODE_CANCELED, /**< User canceled */
  EWK_ERROR_CODE_CANT_SUPPORT_MIMETYPE,
  /**< Can't show page for this MIME Type */
  EWK_ERROR_CODE_FAILED_FILE_IO,       /**< Error */
  EWK_ERROR_CODE_CANT_CONNECT,         /**< Cannot connect to Network */
  EWK_ERROR_CODE_CANT_LOOKUP_HOST,     /**< Fail to look up host from DNS */
  EWK_ERROR_CODE_FAILED_TLS_HANDSHAKE, /**< Fail to SSL/TLS handshake */
  EWK_ERROR_CODE_INVALID_CERTIFICATE,  /**< Received certificate is invalid */
  EWK_ERROR_CODE_REQUEST_TIMEOUT,      /**< Connection timeout */
  EWK_ERROR_CODE_TOO_MANY_REDIRECTS,   /**< Too many redirects */
  EWK_ERROR_CODE_TOO_MANY_REQUESTS,  /**< Too many requests during this load */
  EWK_ERROR_CODE_BAD_URL,            /**< Malformed url */
  EWK_ERROR_CODE_UNSUPPORTED_SCHEME, /**< Unsupported scheme */
  EWK_ERROR_CODE_AUTHENTICATION,  /**< User authentication failed on server */
  EWK_ERROR_CODE_INTERNAL_SERVER, /**< Web server has internal server error */
  EWK_ERROR_CODE_CANNOTSHOWMIMETYPE = 100,
  EWK_ERROR_CODE_CANNOTSHOWURL,
  EWK_ERROR_CODE_FRAMELOADINTERRUPTEDBYPOLICYCHANGE,
  EWK_ERROR_CODE_CANNOTUSERESTRICTEDPORT,
  EWK_ERROR_CODE_CANNOTFINDPLUGIN = 200,
  EWK_ERROR_CODE_CANNOTLOADPLUGIN,
  EWK_ERROR_CODE_JAVAUNAVAILABLE,
  EWK_ERROR_CODE_PLUGINCANCELLEDCONNECTION,
  EWK_ERROR_CODE_PLUGINWILLHANDLELOAD,
} Ewk_Error_Code;

/**
 * @if MOBILE
 * @brief Query failing URL for this error.
 *
 * @details URL that failed loading.
 *
 * @since_tizen 2.3
 *
 * @param[in] error The error object to query
 *
 * @return The URL pointer, that may be @c NULL. This pointer is\n
 *         guaranteed to be eina_stringshare, so whenever possible\n
 *         save yourself some cpu cycles and use\n
 *         eina_stringshare_ref() instead of eina_stringshare_add() or\n
 *         strdup()
 * @endif
 */
EXPORT_API const char* ewk_error_url_get(const Ewk_Error* error);

/**
 * @if MOBILE
 * @brief Query the error code.
 *
 * @since_tizen 2.3
 *
 * @param[in] error The error object to query
 *
 * @return The error code #Ewk_Error_Code
 * @endif
 */
EXPORT_API int ewk_error_code_get(const Ewk_Error* error);

/**
 * @if MOBILE
 * @brief Query description for this error.
 *
 * @since_tizen 2.3
 *
 * @param[in] error The error object to query
 *
 * @return The description pointer, that may be @c NULL. This pointer is\n
 *         guaranteed to be eina_stringshare, so whenever possible\n
 *         save yourself some cpu cycles and use\n
 *         eina_stringshare_ref() instead of eina_stringshare_add() or\n
 *         strdup()
 * @endif
 */
EXPORT_API const char* ewk_error_description_get(const Ewk_Error* error);

/**
 * @if MOBILE
 * @brief Query if error should be treated as a cancellation.
 *
 * @since_tizen 2.3
 *
 * @param[in] error The error object to query
 *
 * @return @c EINA_TRUE if this error should be treated as a cancellation\n
 *         otherwise @c EINA_FALSE
 * @endif
 */
EXPORT_API Eina_Bool ewk_error_cancellation_get(const Ewk_Error* error);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // ewk_error_h
