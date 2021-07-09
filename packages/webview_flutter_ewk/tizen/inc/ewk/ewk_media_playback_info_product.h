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
 * @file    ewk_media_playback_info_product.h
 * @brief   This file describes the ewk media playback info API.
 */

#ifndef ewk_media_playback_info_product_h
#define ewk_media_playback_info_product_h

#include <Eina.h>
#include <Evas.h>
#include <tizen.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Enum values used to inform webengine about decoder used by broadcast.
 */
typedef enum Ewk_Hardware_Decoders {
  EWK_HARDWARE_DECODERS_NONE,
  EWK_HARDWARE_DECODERS_MAIN,
  EWK_HARDWARE_DECODERS_SUB,
} Ewk_Hardware_Decoders;

typedef struct _Ewk_Media_Playback_Info Ewk_Media_Playback_Info;

/**
 * Get video id of media.
 *
 * @param meia playback info's structure
 *
 * @return @c video id
 */
EXPORT_API const int ewk_media_playback_info_video_id_get(
    Ewk_Media_Playback_Info* data);

/**
 * Get url of media.
 *
 * @param meia playback info's structure
 *
 * @return @c media url
 */
EXPORT_API const char* ewk_media_playback_info_media_url_get(
    Ewk_Media_Playback_Info* data);

/**
 * Get mime type of media.
 *
 * @param meia playback info's structure
 *
 * @return @c mime type
 */
EXPORT_API const char* ewk_media_playback_info_mime_type_get(
    Ewk_Media_Playback_Info* data);

/**
 * Get translated url of media.
 *
 * @param media playback info's structure
 *
 * @return @c translated url
 */
EXPORT_API const char* ewk_media_playback_info_translated_url_get(
    Ewk_Media_Playback_Info* data);

/**
 * Get drm info of media.
 *
 * @param media playback info's structure
 *
 * @return @c drm info
 */
EXPORT_API const char* ewk_media_playback_info_drm_info_get(
    Ewk_Media_Playback_Info* data);

/**
 * Get decoder info of media.
 *
 * @param data playback info's structure
 *
 * @return @c decoder name
 */
EXPORT_API Ewk_Hardware_Decoders
ewk_media_playback_info_decoder_get(Ewk_Media_Playback_Info* data);

/**
 * Set media resource acquired of media.
 *
 * @param media playback info's structure
 */
EXPORT_API void ewk_media_playback_info_media_resource_acquired_set(
    Ewk_Media_Playback_Info* data, Eina_Bool media_resource_acquired);

/**
 * Set translated url of media.
 *
 * @param media playback info's structure
 */
EXPORT_API void ewk_media_playback_info_translated_url_set(
    Ewk_Media_Playback_Info* data, const char* translated_url);

/**
 * Set drm info of media.
 *
 * @param media playback info's structure
 */
EXPORT_API void ewk_media_playback_info_drm_info_set(
    Ewk_Media_Playback_Info* data, const char* drm_info);

#if defined(OS_TIZEN_TV_PRODUCT)
Ewk_Media_Playback_Info* ewkMediaPlaybackInfoCreate(
    const int player_id, const char* url, const char* mime_type,
    Ewk_Hardware_Decoders decoder);
Eina_Bool ewk_media_playback_info_media_resource_acquired_get(
    Ewk_Media_Playback_Info* data);
void ewkMediaPlaybackInfoDelete(Ewk_Media_Playback_Info* data);
#endif

typedef struct _Ewk_DRM_Init_Data_Info {
  int init_type;
  void* data;
  int data_length;
} Ewk_DRM_Init_Data_Info;

typedef struct _Ewk_DRM_Init_Complete_Info {
  unsigned int pssh_data_length;
  unsigned char* pssh_data;
  int decryptor;
} Ewk_DRM_Init_Complete_Info;

typedef struct _Ewk_DRM_Error_Info {
  int error_code;
  const char* error_string;
} Ewk_DRM_Error_Info;

#ifdef __cplusplus
}
#endif
#endif  // ewk_media_playback_info_product_h
