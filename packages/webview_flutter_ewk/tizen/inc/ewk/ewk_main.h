/*
 * Copyright (C) 2009-2010 ProFUSION embedded systems
 * Copyright (C) 2009-2016 Samsung Electronics
 * Copyright (C) 2012 Intel Corporation
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
 * @file    ewk_main.h
 * @brief   This file is the general initialization of Chromium-efl,
            not tied to any view object.
 */

#ifndef ewk_main_h
#define ewk_main_h

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
 * @brief Initializes Chromium's instance.
 *
 * - Initializes components needed by EFL,\n
 * - Increases a reference count of Chromium's instance.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @return A reference count of Chromium's instance on success,\n
 *         otherwise @c 0 on failure
 */
EXPORT_API int ewk_init(void);

/**
 * @brief Decreases a reference count of Chromium's instance,
 *        possibly destroying it.
 *
 * @details If the reference count reaches @c 0, Chromium's instance is
 *          destroyed.
 *
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
 * @return A reference count of Chromium's instance
 */
EXPORT_API int ewk_shutdown(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif  // ewk_main_h
