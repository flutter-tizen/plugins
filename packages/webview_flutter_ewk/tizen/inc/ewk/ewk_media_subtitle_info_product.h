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

/**
 * @file    ewk_media_subtitle_info_product.h
 * @brief   .
 */

#ifndef ewk_media_subtitle_info_product_h
#define ewk_media_subtitle_info_product_h

#include <Eina.h>
#include <Evas.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Ewk_Media_Subtitle_Info Ewk_Media_Subtitle_Info;

/**
 * Get id of subtitle.
 *
 * @param meia subtitle info's structure
 *
 * @return @c subtitle id
 */
EXPORT_API int ewk_media_subtitle_info_id_get(Ewk_Media_Subtitle_Info *data);

/**
 * Get url of subtitle.
 *
 * @param meia subtitle info's structure
 *
 * @return @c subtitle url
 */
EXPORT_API const char *ewk_media_subtitle_info_url_get(
    Ewk_Media_Subtitle_Info *data);

/**
 * Get srcLang of subtitle.
 *
 * @param meia subtitle info's structure
 *
 * @return @c subtitle srcLang
 */
EXPORT_API const char *ewk_media_subtitle_info_lang_get(
    Ewk_Media_Subtitle_Info *data);

/**
 * Get label of subtitle.
 *
 * @param meia subtitle info's structure
 *
 * @return @c subtitle label
 */
EXPORT_API const char *ewk_media_subtitle_info_label_get(
    Ewk_Media_Subtitle_Info *data);

typedef struct _Ewk_Media_Subtitle_Data Ewk_Media_Subtitle_Data;

/**
 * Get id of subtitle.
 *
 * @param meia subtitle data's structure
 *
 * @return @c subtitle id
 */
EXPORT_API int ewk_media_subtitle_data_id_get(Ewk_Media_Subtitle_Data *data);

/**
 * Get timestamp of subtitle.
 *
 * @param meia subtitle data's structure
 *
 * @return @c subtitle timestamp
 */
EXPORT_API double ewk_media_subtitle_data_timestamp_get(
    Ewk_Media_Subtitle_Data *data);

/**
 * Get data size of subtitle.
 *
 * @param meia subtitle data's structure
 *
 * @return @c subtitle data size
 */
EXPORT_API unsigned ewk_media_subtitle_data_size_get(
    Ewk_Media_Subtitle_Data *data);

/**
 * Get data of subtitle.
 *
 * @param meia subtitle data's structure
 *
 * @return @c subtitle data
 */
EXPORT_API const void *ewk_media_subtitle_data_get(
    Ewk_Media_Subtitle_Data *data);

#if defined(OS_TIZEN_TV_PRODUCT)
Ewk_Media_Subtitle_Info *ewkMediaSubtitleInfoCreate(int id, const char *url,
                                                    const char *lang,
                                                    const char *label);

void ewkMediaSubtitleInfoDelete(Ewk_Media_Subtitle_Info *data);

Ewk_Media_Subtitle_Data *ewkMediaSubtitleDataCreate(int id, double timestamp,
                                                    const void *data,
                                                    unsigned size);

void ewkMediaSubtitleDataDelete(Ewk_Media_Subtitle_Data *data);
#endif

#ifdef __cplusplus
}
#endif
#endif  // ewk_media_subtitle_info_product_h
