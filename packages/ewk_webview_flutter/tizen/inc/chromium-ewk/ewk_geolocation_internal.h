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

#ifndef ewk_geolocation_internal_h
#define ewk_geolocation_internal_h

#include "ewk_geolocation.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Request to allow / deny the geolocation permission request
 *
 * @param request permission request to allow or deny permission
 * @param allow allow or deny permission for geolocation
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE otherwise
 */
EXPORT_API Eina_Bool ewk_geolocation_permission_request_set(
    Ewk_Geolocation_Permission_Request* request, Eina_Bool allow);

/**
 * Suspend the operation for permission request.
 *
 * This suspends the operation for permission request.
 * This is very useful to decide the policy from the additional UI operation
 * like the popup.
 *
 * @param request Ewk_Geolocation_Permission_Request object to suspend
 *        permission request
 */
EXPORT_API void ewk_geolocation_permission_request_suspend(
    Ewk_Geolocation_Permission_Request* request);

/**
 * Reply the result about geolocation permission.
 *
 * @param request Ewk_Geolocation_Permission_Request object to get the
 *        information about geolocation permission request.
 * @param allow result about geolocation permission
 */
EXPORT_API Eina_Bool ewk_geolocation_permission_reply(
    Ewk_Geolocation_Permission_Request* request, Eina_Bool allow);

#ifdef __cplusplus
}
#endif

#endif  // ewk_geolocation_internal_h
