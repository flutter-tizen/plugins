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
 * @file    ewk_cookie_manager.h
 * @brief   This file describes the Ewk Cookie Manager API.
 */

#ifndef ewk_cookie_manager_h
#define ewk_cookie_manager_h

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
 * @brief The structure type that creates a type name for #Ewk_Cookie_Manager.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef struct Ewk_Cookie_Manager Ewk_Cookie_Manager;

/**
 * \enum    Ewk_Cookie_Accept_Policy
 *
 * @brief   Enumeration that contains accept policies for the cookies.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
enum Ewk_Cookie_Accept_Policy {
  EWK_COOKIE_ACCEPT_POLICY_ALWAYS, /**< Accepts every cookie sent from any page
                                    */
  EWK_COOKIE_ACCEPT_POLICY_NEVER,  /**< Rejects all cookies */
  EWK_COOKIE_ACCEPT_POLICY_NO_THIRD_PARTY /**< Accepts only cookies set by the
                                             main document loaded */
};

/**
 * @brief Enumeration that creates a type name for the Ewk_Cookie_Accept_Policy.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 */
typedef enum Ewk_Cookie_Accept_Policy Ewk_Cookie_Accept_Policy;

/**
 * @brief Sets @a policy as the cookie acceptance policy for @a manager.
 *
 * @details By default, only cookies set by the main document loaded are
 *          accepted.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] manager The cookie manager to update
 * @param[in] policy A #Ewk_Cookie_Accept_Policy
 */
EXPORT_API void ewk_cookie_manager_accept_policy_set(
    Ewk_Cookie_Manager* manager, Ewk_Cookie_Accept_Policy policy);

/**
 * @brief Called for use with ewk_cookie_manager_accept_policy_async_get().
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] policy A #Ewk_Cookie_Accept_Policy
 * @param[in] event_info The user data that will be passed when
 *            ewk_cookie_manager_accept_policy_async_get() is called
 */
typedef void (*Ewk_Cookie_Manager_Policy_Async_Get_Cb)(
    Ewk_Cookie_Accept_Policy policy, void* event_info);

/**
 * @brief Gets the cookie acceptance policy of @a manager asynchronously.
 *
 * @details By default, only cookies set by the main document loaded are
 *          accepted.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] manager The cookie manager to query
 * @param[in] callback The function to call when the policy is received
 * @param[in] data The user data (may be @c NULL)
 */
EXPORT_API void ewk_cookie_manager_accept_policy_async_get(
    const Ewk_Cookie_Manager* manager,
    Ewk_Cookie_Manager_Policy_Async_Get_Cb callback, void* data);

/**
 * @brief Deletes all the cookies of @a manager.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @param[in] manager The cookie manager to update
 */
EXPORT_API void ewk_cookie_manager_cookies_clear(Ewk_Cookie_Manager* manager);

/**
 * @brief Queries if the cookie manager allows cookies for file scheme URLs.
 *
 * @since_tizen 3.0
 *
 * @param[in] manager The cookie manager to query
 *
 * @return @c EINA_TRUE if cookies for file scheme are allowed or @c EINA_FALSE
 * otherwise
 */
EXPORT_API Eina_Bool
ewk_cookie_manager_file_scheme_cookies_allow_get(Ewk_Cookie_Manager* manager);

/**
 * @brief Sets whether cookie manager allows cookies for file scheme URLs.
 *
 * @since_tizen 3.0
 *
 * @param[in] manager The cookie manager to allow file scheme for cookies
 * @param[in] allow A state to set
 */
EXPORT_API void ewk_cookie_manager_file_scheme_cookies_allow_set(
    Ewk_Cookie_Manager* manager, Eina_Bool allow);

/**
 * \enum  Ewk_Cookie_Persistent_Storage
 * @brief Enumeration that creates a type name for the
 * #Ewk_Cookie_Persistent_Storage.
 * @since_tizen 3.0
 */
enum Ewk_Cookie_Persistent_Storage {
  EWK_COOKIE_PERSISTENT_STORAGE_TEXT,  /**< @deprecated Cookies are stored in a
                                          text file in the Mozilla "cookies.txt"
                                          format. (Deprecated since 6.0) */
  EWK_COOKIE_PERSISTENT_STORAGE_SQLITE /**<  Cookies are stored in a SQLite file
                                          in the current Mozilla format. */
};

/**
 * @brief The enum type that creates a type name for
 * Ewk_Cookie_Persistent_Storage.
 * @since_tizen 3.0
 */
typedef enum Ewk_Cookie_Persistent_Storage Ewk_Cookie_Persistent_Storage;

/**
 * @brief Sets the @a path where non-session cookies are stored persistently
 * using
 *        @a storage as the format to read/write the cookies.
 *
 * @details Cookies are initially read from @a path/Cookies to create an initial
 *          set of cookies. Then, non-session cookies will be written to @a
 * path/Cookies. By default, @a manager doesn't store the cookies persistently,
 * so you need to call this method to keep cookies saved across sessions. If @a
 * path does not exist it will be created.
 *
 * @remarks http://tizen.org/privilege/mediastorage is needed if input or output
 * path is relevant to media storage.\n
 *          http://tizen.org/privilege/externalstorage is needed if input or
 * output path is relevant to external storage.
 *
 * @since_tizen 3.0
 *
 * @param[in] manager The cookie manager to update
 * @param[in] path The path where to read/write Cookies
 * @param[in] storage The type of storage
 */
EXPORT_API void ewk_cookie_manager_persistent_storage_set(
    Ewk_Cookie_Manager* manager, const char* path,
    Ewk_Cookie_Persistent_Storage storage);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // ewk_cookie_manager_h
