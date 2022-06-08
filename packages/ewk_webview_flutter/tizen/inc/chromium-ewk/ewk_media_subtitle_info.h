// Copyright 2016 Samsung Electronics. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * @file    ewk_media_subtitle_info.h
 * @brief   .
 */

#ifndef ewk_media_subtitle_info_h
#define ewk_media_subtitle_info_h

#include <Eina.h>
#include <Evas.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Ewk_Media_Subtitle_Info Ewk_Media_Subtitle_Info;
typedef struct _Ewk_First_Timestamp_Info Ewk_First_Timestamp_Info;
typedef struct _Ewk_PES_Info Ewk_PES_Info;

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

#ifdef __cplusplus
}
#endif
#endif  // ewk_media_subtitle_info_h
