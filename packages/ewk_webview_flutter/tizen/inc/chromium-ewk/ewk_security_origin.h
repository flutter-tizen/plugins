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
 * @file    ewk_security_origin.h
 * @brief   This file describes the Ewk Security API.
 */

#ifndef ewk_security_origin_h
#define ewk_security_origin_h

#include <Eina.h>
#include <stdint.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup WEBVIEW
 * @{
 */

/**
 * @brief The structure type that creates a type name for #Ewk_Security_Origin.
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 */
typedef struct _Ewk_Security_Origin Ewk_Security_Origin;

/**
 * @brief Requests for getting host of security origin.
 *
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @param[in] origin Security origin
 *
 * @return host of security origin
 */
EXPORT_API Eina_Stringshare* ewk_security_origin_host_get(
    const Ewk_Security_Origin* origin);

/**
 * @brief Requests for getting host of security origin.
 *
 * @since_tizen @if MOBILE 2.4 @elseif WEARABLE 3.0 @endif
 *
 * @param[in] origin Security origin
 *
 * @return host of security origin
 */
EXPORT_API Eina_Stringshare* ewk_security_origin_protocol_get(
    const Ewk_Security_Origin* origin);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  // ewk_security_origin_h
