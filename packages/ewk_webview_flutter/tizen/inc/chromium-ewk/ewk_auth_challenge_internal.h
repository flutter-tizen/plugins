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
 * @file    ewk_auth_challenge_internal.h
 * @brief   Describes the authentication challenge API.
 */

#ifndef ewk_auth_challenge_internal_h
#define ewk_auth_challenge_internal_h

#include <Eina.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Creates a type name for _Ewk_Auth_Challenge */
typedef struct _Ewk_Auth_Challenge Ewk_Auth_Challenge;

/**
 * Gets the realm string of authentication challenge received from
 * "Ewk_View_Authentication_Callback" function.
 *
 * @param auth_challenge authentication challenge instance received from
 * "Ewk_View_Authentication_Callback" function.
 * @return the realm of authentication challenge on success, @c 0 otherwise
 *
 * @see ewk_view_authentication_callback_set
 */
EXPORT_API const char* ewk_auth_challenge_realm_get(
    Ewk_Auth_Challenge* auth_challenge);

/**
 * Gets the url string of authentication challenge received from
 * "Ewk_View_Authentication_Callback" function.
 *
 * @param auth_challenge authentication challenge request instance received from
 * "Ewk_View_Authentication_Callback" function.
 * @return the url of authentication challenge on success, @c 0 otherwise
 *
 * @see ewk_view_authentication_callback_set
 */
EXPORT_API const char* ewk_auth_challenge_url_get(
    Ewk_Auth_Challenge* auth_challenge);

/**
 * Suspend the operation for authentication challenge.
 *
 * @param auth_challenge authentication challenge instance received from
 * "Ewk_View_Authentication_Callback" function.
 *
 * @see ewk_view_authentication_callback_set
 */
EXPORT_API void ewk_auth_challenge_suspend(Ewk_Auth_Challenge* auth_challenge);

/**
 *  If user select ok, send credential for authentication challenge from user
 * input.
 *
 * @param auth_challenge authentication challenge instance received from
 * "Ewk_View_Authentication_Callback" function.
 * @param user user id from user input.
 * @param password user password from user input.
 *
 * @see ewk_view_authentication_callback_set
 */
EXPORT_API void ewk_auth_challenge_credential_use(
    Ewk_Auth_Challenge* auth_challenge, const char* user, const char* password);

/**
 *  If user select cancel, send cancellation notification for authentication
 * challenge.
 *
 * @param auth_challenge authentication challenge instance received from
 * "Ewk_View_Authentication_Callback" function.
 *
 * @see ewk_view_authentication_callback_set
 */
EXPORT_API void ewk_auth_challenge_credential_cancel(
    Ewk_Auth_Challenge* auth_challenge);

#ifdef __cplusplus
}
#endif

#endif  // ewk_auth_challenge_internal_h
