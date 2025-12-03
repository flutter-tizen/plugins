/**
 * @file
 * @brief          Drm related enums
 * @interfacetype  Platform
 * @privlevel      None-privilege
 * @privilege      None
 * @product        TV, AV, B2B
 * @version        8.0
 * @SDK_Support    N
 * @remark         This is a group of C style drm related data structures and
 *                 enums.
 * @see            Drm related event listeners, enum classes, etc.. are
 *                 converted to this.
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

#ifndef __PLUSPLAYER_PLUSPLAYER_CAPI_DRM_H__
#define __PLUSPLAYER_PLUSPLAYER_CAPI_DRM_H__

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of DRM types
 */
typedef enum {
  PLUSPLAYER_DRM_TYPE_NONE,        /**< No DRM applied */
  PLUSPLAYER_DRM_TYPE_PLAYREADY,   /**< Microsoft PlayReady DRM */
  PLUSPLAYER_DRM_TYPE_MARLIN,      /**< Marlin DRM */
  PLUSPLAYER_DRM_TYPE_VERIMATRIX,  /**< Verimatrix DRM */
  PLUSPLAYER_DRM_TYPE_WV_CLASSIC,  /**< Widevine Classic (Legacy) */
  PLUSPLAYER_DRM_TYPE_SECUREMEDIA, /**< SecureMedia DRM */
  PLUSPLAYER_DRM_TYPE_SDRM,        /**< Samsung DRM (SDRM) */
  PLUSPLAYER_DRM_TYPE_WIDEVINE_CDM /**< Widevine CDM (Client-Driven) */
} plusplayer_drm_type_e;

#ifdef DRM_MAPI_AARCH_64
typedef unsigned long Plusplayer_DrmHandle;
#else
typedef int Plusplayer_DrmHandle;
#endif

/**
 * @brief  Drm property structure
 */
typedef struct {
  /**
   * @description Drm type
   */
  plusplayer_drm_type_e type;

  /**
   * @description Drm session handle
   */
  Plusplayer_DrmHandle handle;

  /**
   * @description External Decryption Mode
   */
  bool external_decryption;

  /**
   * @description The cb will be invoked when license was acquired.
   */
  void *license_acquired_cb;

  /**
   * @description The userdata will be sent by license_acquired_cb
   */
  void *license_acquired_userdata;
} plusplayer_drm_property_s;

}  // extern "C"
#endif  // __PLUSPLAYER_PLUSPLAYER_CAPI_DRM_H__
