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

#ifndef EWK_EFL_INTEGRATION_PUBLIC_EWK_INTERCEPT_REQUEST_INTERNAL_H_
#define EWK_EFL_INTEGRATION_PUBLIC_EWK_INTERCEPT_REQUEST_INTERNAL_H_

#include <Eina.h>
#include <tizen.h>

#include "ewk_intercept_request.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns request url sheme from Intercept Request object.
 *
 * Returned string is owned by Intercept Request object, you have to make a
 * copy if you want to use it past owner lifetime.
 *
 * @param intercept_request intercept request instance received from
 *        Ewk_Context_Intercept_Request_Callback ewk_context callback
 *
 * @return @c url sheme string on success or NULL on failure
 */
EXPORT_API const char* ewk_intercept_request_scheme_get(
    Ewk_Intercept_Request* intercept_request);

/**
 * @brief Returns request's body from Intercept Request object.
 *
 * @remarks Returned bytes are owned by Intercept Request object, you have to
 *          make a copy if you want to use it past owner lifetime.
 *
 *          It is only allowed to use this function *inside* the
 *          Ewk_Context_Intercept_Request_Callback.
 *
 *          Use ewk_intercept_request_body_length_get to get length of returned
 *          bytes.
 *
 * @since_tizen 3.0
 *
 * @param[in] intercept_request intercept request instance received from
 *            Ewk_Context_Intercept_Request_Callback ewk_context callback
 *
 * @return @c request's body data on success or NULL on failure
 */
EXPORT_API const char* ewk_intercept_request_body_get(
    Ewk_Intercept_Request* intercept_request);

/**
 * @brief Returns request's body's length from Intercept Request object.
 *
 * @remarks It is only allowed to use this function *inside* the
 *          Ewk_Context_Intercept_Request_Callback.
 *
 * @since_tizen 3.0
 *
 * @param[in] intercept_request intercept request instance received from
 *            Ewk_Context_Intercept_Request_Callback ewk_context callback
 *
 * @return @c request's body length on success or -1 on failure
 */
EXPORT_API int64_t
ewk_intercept_request_body_length_get(Ewk_Intercept_Request* intercept_request);

#ifdef __cplusplus
}
#endif

#endif  // EWK_EFL_INTEGRATION_PUBLIC_EWK_INTERCEPT_REQUEST_INTERNAL_H_
