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
 * @file    ewk_policy_decision.h
 * @brief   This file describes the Ewk Policy Decision API.
 */

#ifndef ewk_policy_decision_h
#define ewk_policy_decision_h

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
 * \enum   _Ewk_Policy_Decision_Type
 * @brief  Enumeration that provides an option to policy decision types.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
enum _Ewk_Policy_Decision_Type {
  EWK_POLICY_DECISION_USE,      /**< Use */
  EWK_POLICY_DECISION_DOWNLOAD, /**< Download */
  EWK_POLICY_DECISION_IGNORE    /**< Ignore */
};

/**
 * @brief Enumeration that creates a type name for the Ewk_Policy_Decision_Type.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef enum _Ewk_Policy_Decision_Type Ewk_Policy_Decision_Type;

/**
 * @brief The structure type that creates a type name for Ewk_Policy_Decision.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef struct _Ewk_Policy_Decision Ewk_Policy_Decision;

/**
 * \enum   _Ewk_Policy_Navigation_Type
 * @brief  Enumeration that provides an option to policy navigation types.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
enum _Ewk_Policy_Navigation_Type {
  EWK_POLICY_NAVIGATION_TYPE_LINK_CLICKED = 0,     /**< Link clicked */
  EWK_POLICY_NAVIGATION_TYPE_FORM_SUBMITTED = 1,   /**< Form submitted */
  EWK_POLICY_NAVIGATION_TYPE_BACK_FORWARD = 2,     /**< Back forward */
  EWK_POLICY_NAVIGATION_TYPE_RELOAD = 3,           /**< Reload */
  EWK_POLICY_NAVIGATION_TYPE_FORM_RESUBMITTED = 4, /**< Form resubmitted */
  EWK_POLICY_NAVIGATION_TYPE_OTHER = 5             /**< Other */
};

/**
 * @brief Enumeration that creates a type name for #Ewk_Policy_Navigation_Type.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef enum _Ewk_Policy_Navigation_Type Ewk_Policy_Navigation_Type;

/**
 * @brief Returns a cookie from the Policy Decision object.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] policy_decision The policy decision object
 *
 * @return The cookie string on success,\n
 *         otherwise an empty string on failure
 */
EXPORT_API const char* ewk_policy_decision_cookie_get(
    Ewk_Policy_Decision* policy_decision);

/**
 * @brief Returns a URL from the Policy Decision object.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] policy_decision The policy decision object
 *
 * @return The URL string on success,\n
 *         otherwise an empty string on failure
 */
EXPORT_API const char* ewk_policy_decision_url_get(
    Ewk_Policy_Decision* policy_decision);

/**
 * @brief Returns a scheme from the Policy Decision object.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] policy_decision The policy decision object
 *
 * @return The scheme string on success,\n
 *         otherwise an empty string on failure
 */
EXPORT_API const char* ewk_policy_decision_scheme_get(
    Ewk_Policy_Decision* policy_decision);

/**
 * @brief Returns a host from the Policy Decision object.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] policy_decision The policy decision object
 *
 * @return The host string on success,\n
 *         otherwise an empty string on failure
 */
EXPORT_API const char* ewk_policy_decision_host_get(
    Ewk_Policy_Decision* policy_decision);

/**
 * @brief Returns an HTTP method from the Policy Decision object.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] policy_decision The policy decision object
 *
 * @return The HTTP method string on success,\n
 *         otherwise an empty string on failure
 */
EXPORT_API const char* ewk_policy_decision_http_method_get(
    Ewk_Policy_Decision* policy_decision);

/**
 * @brief Returns a MIME type for response data from the Policy Decision object.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] policy_decision policy decision object
 *
 * @return The MIME type string on success,\n
 *         otherwise an empty string on failure
 */
EXPORT_API const char* ewk_policy_decision_response_mime_get(
    Ewk_Policy_Decision* policy_decision);

/**
 * @brief Return HTTP headers for response data from the Policy Decision object.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] policy_decision The policy decision object
 *
 * @return The HTTP headers on success,\n
 *         otherwise @c NULL on failure
 */
EXPORT_API const Eina_Hash* ewk_policy_decision_response_headers_get(
    Ewk_Policy_Decision* policy_decision);

/**
 * @brief Returns an HTTP status code from the Policy Decision object.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] policy_decision The policy decision object
 *
 * @return The HTTP status code number
 */
EXPORT_API int ewk_policy_decision_response_status_code_get(
    Ewk_Policy_Decision* policy_decision);

/**
 * @brief Returns a policy type from the Policy Decision object.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] policy_decision The policy decision object
 *
 * @return The policy type
 */
EXPORT_API Ewk_Policy_Decision_Type
ewk_policy_decision_type_get(const Ewk_Policy_Decision* policy_decision);

/**
 * @brief Accepts the action which triggers this decision.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] policy_decision The policy decision object
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool
ewk_policy_decision_use(Ewk_Policy_Decision* policy_decision);

/**
 * @brief Ignores the action which triggers this decision.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] policy_decision The policy decision object
 *
 * @return @c EINA_TRUE on success,\n
 *         otherwise @c EINA_FALSE on failure
 */
EXPORT_API Eina_Bool
ewk_policy_decision_ignore(Ewk_Policy_Decision* policy_decision);

/**
 * @brief Returns a navigation type from the Policy Decision object.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] policy_decision The policy decision object
 *
 * @return The navigation type
 */
EXPORT_API Ewk_Policy_Navigation_Type
ewk_policy_decision_navigation_type_get(Ewk_Policy_Decision* policy_decision);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  // ewk_policy_decision_h
