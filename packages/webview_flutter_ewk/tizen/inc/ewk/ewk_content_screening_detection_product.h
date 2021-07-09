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

#ifndef ewk_content_screening_detection_product_h
#define ewk_content_screening_detection_product_h

#include <Eina.h>
#include <tizen.h>

#include "ewk_error.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Ewk_Content_Screening_Detection Ewk_Content_Screening_Detection;

/**
 * Set the variable to allow the release confirm about malware error.
 *
 * @param content_screening_detection malware information data
 *
 * @param confirmed decided permission value from user
 */
EXPORT_API void ewk_content_screening_detection_confirmed_set(
    Ewk_Content_Screening_Detection* content_screening_detection,
    Eina_Bool confirmed);

/**
 * Suspend the operation for content screening detection.
 *
 * This suspends the operation for content screening detection when the signal
 * is emitted. This is very usefull to decide the policy from the additional UI
 * operation like the popup.
 *
 * @param content_screening_detection malware information data
 *
 * @return @c EINA_TRUE on success or @c EINA_FALSE on failure
 */
EXPORT_API void ewk_content_screening_detection_suspend(
    Ewk_Content_Screening_Detection* content_screening_detection);

/**
 * Get the variable errro structure to check the error cause about malware
 * error.
 *
 * @param content_screening_detection malware information data
 *
 * @return @c error
 */
EXPORT_API Ewk_Error* ewk_content_screening_detection_error_get(
    Ewk_Content_Screening_Detection* content_screening_detection);

/**
 * @brief Get the level to check the error cause about malware error.
 *
 * @since_tizen 2.3
 *
 * @param[in] content_screening_detection malware information data
 *
 * @return level
 */
EXPORT_API int ewk_content_screening_detection_level_get(
    Ewk_Content_Screening_Detection* content_screening_detection);

/**
 * @brief Get the name to check the error cause about malware error.
 *
 * @since_tizen 2.3
 *
 * @param[in] content_screening_detection malware information data
 *
 * @return name
 */
EXPORT_API const char* ewk_content_screening_detection_name_get(
    Ewk_Content_Screening_Detection* content_screening_detection);

/**
 * @brief Get the url to check the error cause about malware error.
 *
 * @since_tizen 2.3
 *
 * @param[in] content_screening_detection malware information data
 *
 * @return url
 */
EXPORT_API const char* ewk_content_screening_detection_url_get(
    Ewk_Content_Screening_Detection* content_screening_detection);

#ifdef __cplusplus
}
#endif
#endif  // ewk_content_screening_detection_product_h
