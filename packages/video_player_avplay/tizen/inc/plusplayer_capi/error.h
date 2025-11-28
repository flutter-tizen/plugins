/**
 * @file
 * @brief          Error related enums
 * @interfacetype  Platform
 * @privlevel      None-privilege
 * @privilege      None
 * @product        TV, AV, B2B
 * @version        8.0
 * @SDK_Support    N
 * @remark         This is a group of C style error related enum.
 * @see            All error enum values will be converted to this managed error
 *                 types.
 *
 * Copyright (c) 2025 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __PLUSPLAYER_PLUSPLAYER_CAPI_ERROR_H__
#define __PLUSPLAYER_PLUSPLAYER_CAPI_ERROR_H__

#include "tizen.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PLUSPLAYER_ERROR_CLASS_CAPI (TIZEN_ERROR_PLAYER | 0x20)

/* This is for custom defined plusplayer error. */
#define PLUSPLAYER_CUSTOM_ERROR_CLASS_CAPI (TIZEN_ERROR_PLAYER | 0x1000)

/**
 * @brief  Enumerations for the error type
 */
typedef enum {
  PLUSPLAYER_ERROR_TYPE_NONE = TIZEN_ERROR_NONE, /**< Successful */
  PLUSPLAYER_ERROR_TYPE_OUT_OF_MEMORY =
      TIZEN_ERROR_OUT_OF_MEMORY, /**< Out of memory */
  PLUSPLAYER_ERROR_TYPE_INVALID_PARAMETER =
      TIZEN_ERROR_INVALID_PARAMETER, /**< Invalid parameter */
  PLUSPLAYER_ERROR_TYPE_NO_SUCH_FILE =
      TIZEN_ERROR_NO_SUCH_FILE, /**< No such file or directory */
  PLUSPLAYER_ERROR_TYPE_INVALID_OPERATION =
      TIZEN_ERROR_INVALID_OPERATION, /**< Invalid operation */
  PLUSPLAYER_ERROR_TYPE_FILE_NO_SPACE_ON_DEVICE =
      TIZEN_ERROR_FILE_NO_SPACE_ON_DEVICE, /**< No space left on the device */
  PLUSPLAYER_ERROR_TYPE_FEATURE_NOT_SUPPORTED_ON_DEVICE =
      TIZEN_ERROR_NOT_SUPPORTED, /**< Not supported */
  PLUSPLAYER_ERROR_TYPE_SEEK_FAILED =
      PLUSPLAYER_ERROR_CLASS_CAPI | 0x01, /**< Seek operation failure */
  PLUSPLAYER_ERROR_TYPE_INVALID_STATE =
      PLUSPLAYER_ERROR_CLASS_CAPI | 0x02, /**< Invalid state */
  PLUSPLAYER_ERROR_TYPE_NOT_SUPPORTED_FILE =
      PLUSPLAYER_ERROR_CLASS_CAPI | 0x03, /**< File format not supported */
  PLUSPLAYER_ERROR_TYPE_INVALID_URI =
      PLUSPLAYER_ERROR_CLASS_CAPI | 0x04, /**< Invalid URI */
  PLUSPLAYER_ERROR_TYPE_SOUND_POLICY =
      PLUSPLAYER_ERROR_CLASS_CAPI | 0x05, /**< Sound policy error */
  PLUSPLAYER_ERROR_TYPE_CONNECTION_FAILED =
      PLUSPLAYER_ERROR_CLASS_CAPI | 0x06, /**< Streaming connection failed */
  PLUSPLAYER_ERROR_TYPE_VIDEO_CAPTURE_FAILED =
      PLUSPLAYER_ERROR_CLASS_CAPI | 0x07, /**< Video capture failed */
  PLUSPLAYER_ERROR_TYPE_DRM_EXPIRED =
      PLUSPLAYER_ERROR_CLASS_CAPI | 0x08, /**< Expired license */
  PLUSPLAYER_ERROR_TYPE_DRM_NO_LICENSE =
      PLUSPLAYER_ERROR_CLASS_CAPI | 0x09, /**< No license */
  PLUSPLAYER_ERROR_TYPE_DRM_FUTURE_USE =
      PLUSPLAYER_ERROR_CLASS_CAPI | 0x0a, /**< License for future use */
  PLUSPLAYER_ERROR_TYPE_NOT_PERMITTED =
      PLUSPLAYER_ERROR_CLASS_CAPI | 0x0b, /**< Format not permitted */
  PLUSPLAYER_ERROR_TYPE_RESOURCE_LIMIT =
      PLUSPLAYER_ERROR_CLASS_CAPI | 0x0c, /**< Resource limit */
  PLUSPLAYER_ERROR_TYPE_PERMISSION_DENIED =
      TIZEN_ERROR_PERMISSION_DENIED, /**< Permission denied */
  PLUSPLAYER_ERROR_TYPE_SERVICE_DISCONNECTED =
      PLUSPLAYER_ERROR_CLASS_CAPI | 0x0d, /**< Socket connection lost*/
  PLUSPLAYER_ERROR_TYPE_NO_BUFFER_SPACE =
      TIZEN_ERROR_BUFFER_SPACE, /**< No buffer space available */
  PLUSPLAYER_ERROR_TYPE_NOT_SUPPORTED_AUDIO_CODEC =
      PLUSPLAYER_ERROR_CLASS_CAPI | 0x0e, /**< Not supported audio codec but
                                             video can be played  */
  PLUSPLAYER_ERROR_TYPE_NOT_SUPPORTED_VIDEO_CODEC =
      PLUSPLAYER_ERROR_CLASS_CAPI | 0x0f, /**< Not supported video codec but
                                             video can be played */
  PLUSPLAYER_ERROR_TYPE_NOT_SUPPORTED_SUBTITLE =
      PLUSPLAYER_ERROR_CLASS_CAPI | 0x10, /**< Not supported subtitle format */
  PLUSPLAYER_ERROR_TYPE_DRM_DECRYPTION_FAILED =
      PLUSPLAYER_CUSTOM_ERROR_CLASS_CAPI |
      0x05, /**< playready drm error info */
  PLUSPLAYER_ERROR_TYPE_NOT_SUPPORTED_FORMAT =
      PLUSPLAYER_CUSTOM_ERROR_CLASS_CAPI | 0x08, /**< format not supported */
  PLUSPLAYER_ERROR_TYPE_STREAMING_PLAYER =
      PLUSPLAYER_CUSTOM_ERROR_CLASS_CAPI | 0x09,  // confirm it later
  PLUSPLAYER_ERROR_TYPE_DTCPFSK = PLUSPLAYER_CUSTOM_ERROR_CLASS_CAPI | 0x0a,
  PLUSPLAYER_ERROR_TYPE_PRELOADING_TIMEOUT =
      PLUSPLAYER_CUSTOM_ERROR_CLASS_CAPI |
      0x0b, /**< can't finish preloading in time*/
  PLUSPLAYER_ERROR_TYPE_NETWORK_ERROR =
      PLUSPLAYER_CUSTOM_ERROR_CLASS_CAPI | 0x0c, /**< for network error*/
  PLUSPLAYER_ERROR_TYPE_NOT_CHANNEL_SURFING_ERROR =
      PLUSPLAYER_CUSTOM_ERROR_CLASS_CAPI |
      0x0d, /**< for channel surfing error*/
  PLUSPLAYER_ERROR_TYPE_UNKNOWN
} plusplayer_error_type_e;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // __PLUSPLAYER_PLUSPLAYER_CAPI_ERROR_H__
