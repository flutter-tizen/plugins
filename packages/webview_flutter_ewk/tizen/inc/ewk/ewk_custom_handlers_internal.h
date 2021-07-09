/*
 * Copyright (C) 2012-2016 Samsung Electronics. All rights reserved.
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
 * @file    ewk_custom_handlers_internal.h
 * @brief   Custom scheme and content handlers.
 *          (http://www.w3.org/TR/html5/timers.html#custom-handlers)
 */

#ifndef ewk_custom_handlers_internal_h
#define ewk_custom_handlers_internal_h

#include <Evas.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Ewk_Custom_Handlers_Data Ewk_Custom_Handlers_Data;

/// Defines the handler states.
enum _Ewk_Custom_Handlers_State {
  /// Indicates that no attempt has been made to register the given handler.
  EWK_CUSTOM_HANDLERS_NEW,
  /// Indicates that the given handler has been registered or that the site is
  /// blocked from registering the handler.
  EWK_CUSTOM_HANDLERS_REGISTERED,
  /// Indicates that the given handler has been offered but was rejected.
  EWK_CUSTOM_HANDLERS_DECLINED
};

/// Creates a type name for @a _Ewk_Custom_Handlers_State.
typedef enum _Ewk_Custom_Handlers_State Ewk_Custom_Handlers_State;

/**
 * Get target(scheme or mime type) of custom handlers.
 *
 * @param data custom handlers's structure.
 *
 * @return @c target (scheme or mime type).
 */
EINA_DEPRECATED EXPORT_API Eina_Stringshare*
ewk_custom_handlers_data_target_get(const Ewk_Custom_Handlers_Data* data);

/**
 * Get base url of custom handlers.
 *
 * @param data custom handlers's structure.
 *
 * @return @c base url.
 */
EINA_DEPRECATED EXPORT_API Eina_Stringshare*
ewk_custom_handlers_data_base_url_get(const Ewk_Custom_Handlers_Data* data);

/**
 * Get url of custom handlers.
 *
 * @param data custom handlers's structure.
 *
 * @return @c url.
 */
EINA_DEPRECATED EXPORT_API Eina_Stringshare* ewk_custom_handlers_data_url_get(
    const Ewk_Custom_Handlers_Data* data);

/**
 * Get title of custom handlers.
 *
 * @param data custom handlers's structure.
 *
 * @return @c title.
 */
EINA_DEPRECATED EXPORT_API Eina_Stringshare* ewk_custom_handlers_data_title_get(
    const Ewk_Custom_Handlers_Data* data);

/**
 * Set result of isProtocolRegistered API.
 *
 * @param data custom handlers's structure
 * @param result(Ewk_Custom_Handlers_State) of isProtocolRegistered and
 * isContentRegistered API
 */
EINA_DEPRECATED EXPORT_API void ewk_custom_handlers_data_result_set(
    Ewk_Custom_Handlers_Data* data, Ewk_Custom_Handlers_State result);
#ifdef __cplusplus
}
#endif
#endif  // ewk_custom_handlers_internal_h
