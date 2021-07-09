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
 * @file    ewk_cookie_manager_product.h
 * @brief   Describes the Ewk Cookie Manager product API.
 */

#ifndef ewk_cookie_manager_product_h
#define ewk_cookie_manager_product_h

#include "ewk_cookie_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @typedef Ewk_Cookie_Manager_Changes_Watch_Cb
 * Ewk_Cookie_Manager_Changes_Watch_Cb
 * @brief Callback type for use with ewk_cookie_manager_changes_watch()
 * @warning Callback is not called on UI thread, so user should be cautious
 *          when accessing their data also used on UI thread.
 */
typedef void (*Ewk_Cookie_Manager_Changes_Watch_Cb)(void *event_info);

/**
 * Watch for cookies' changes in @a manager.
 *
 * Pass @c NULL as value for @a callback to stop watching for changes.
 *
 * When the cookie is modified, it actually is deleted and added again so
 * the callback is called twice. User data provided to the API is passed
 * to callback function as a parameter. Callback function is not informed
 * about action type (add/delete) and does not get any identification
 * information about it (like page URL or cookie name).
 *
 * @param manager The cookie manager to watch.
 * @param callback function that will be called every time cookies are added,
 * removed or modified.
 * @param data User data (may be @c NULL).
 */
EXPORT_API void ewk_cookie_manager_changes_watch(
    Ewk_Cookie_Manager *manager, Ewk_Cookie_Manager_Changes_Watch_Cb callback,
    void *data);

#ifdef __cplusplus
}
#endif

#endif  // ewk_cookie_manager_product_h
