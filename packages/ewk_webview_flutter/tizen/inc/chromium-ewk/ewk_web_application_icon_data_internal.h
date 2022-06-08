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
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SAMSUNG ELECTRONICS. OR ITS
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef ewk_web_application_icon_data_internal_h
#define ewk_web_application_icon_data_internal_h

#include <Eina.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Creates a type name for _Ewk_Web_App_Icon_Data.
typedef struct _Ewk_Web_App_Icon_Data Ewk_Web_App_Icon_Data;

/**
 * Requests for getting icon size string of Ewk_Web_App_Icon_Data.
 *
 * @param icon_data Ewk_Web_App_Icon_Data object to get icon size
 *
 * @return icon size string of requested icon data
 */
EXPORT_API const char *ewk_web_application_icon_data_size_get(
    Ewk_Web_App_Icon_Data *data);

/**
 * Requests for getting icon url string of Ewk_Web_App_Icon_Data.
 *
 * @param icon_data Ewk_Web_App_Icon_Data object to get icon url
 *
 * @return icon url string of requested icon data
 */
EXPORT_API const char *ewk_web_application_icon_data_url_get(
    Ewk_Web_App_Icon_Data *data);

#ifdef __cplusplus
}
#endif
#endif  // ewk_web_application_icon_data_internal_h
