/*
 * Copyright (C) 2016 Samsung Electronics. All rights reserved.
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
 * @file    ewk_cookie_manager_internal.h
 * @brief   Describes the Ewk Cookie Manager API.
 */

#ifndef ewk_cookie_manager_internal_h
#define ewk_cookie_manager_internal_h

#include "ewk_cookie_manager.h"
#include "ewk_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef Ewk_Cookie_Manager_Async_Hostnames_Get_Cb
 * Ewk_Cookie_Manager_Async_Hostnames_Get_Cb
 * @brief Callback type for use with
 * ewk_cookie_manager_async_hostnames_with_cookies_get
 *
 * @note The @a hostnames list items are guaranteed to be eina_stringshare.
 * Whenever possible save yourself some cpu cycles and use
 * eina_stringshare_ref() instead of eina_stringshare_add() or strdup().
 */
typedef void (*Ewk_Cookie_Manager_Async_Hostnames_Get_Cb)(Eina_List *hostnames,
                                                          Ewk_Error *error,
                                                          void *event_info);

/**
 * Asynchronously get the list of host names for which @a manager contains
 * cookies.
 *
 * @param manager The cookie manager to query.
 * @param callback The function to call when the host names have been received.
 * @param data User data (may be @c NULL).
 */
EXPORT_API void ewk_cookie_manager_async_hostnames_with_cookies_get(
    const Ewk_Cookie_Manager *manager,
    Ewk_Cookie_Manager_Async_Hostnames_Get_Cb callback, void *data);

/**
 * Remove all cookies of @a manager for the given @a hostname.
 *
 * @param manager The cookie manager to update.
 * @param hostname A host name.
 */
EXPORT_API void ewk_cookie_manager_hostname_cookies_clear(
    Ewk_Cookie_Manager *manager, const char *hostname);

#ifdef __cplusplus
}
#endif

#endif  // ewk_cookie_manager_internal_h
