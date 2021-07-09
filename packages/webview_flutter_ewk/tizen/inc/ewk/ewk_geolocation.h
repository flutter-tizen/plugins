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
 * @file    ewk_geolocation.h
 * @brief   This file describes the Ewk Geolocation API.
 */

#ifndef ewk_geolocation_h
#define ewk_geolocation_h

#include <Eina.h>
#include <tizen.h>

#include "ewk_security_origin.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup WEBVIEW
 * @{
 */

/**
 * @brief The structure type that creates a type name for
 * #Ewk_Geolocation_Permission_Request.
 *
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 */
typedef struct _Ewk_Geolocation_Permission_Request
    Ewk_Geolocation_Permission_Request;

/**
 * @brief Requests for getting origin of geolocation permission request.
 *
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @param[in] request Ewk_Geolocation_Permission_Request object to get origin
 *
 * @return security origin of geolocation permission data
 */
EXPORT_API const Ewk_Security_Origin *
ewk_geolocation_permission_request_origin_get(
    const Ewk_Geolocation_Permission_Request *request);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // ewk_geolocation_h
