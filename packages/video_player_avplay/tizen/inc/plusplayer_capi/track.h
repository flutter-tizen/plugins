/**
 * @file
 * @brief          Track enum.
 * @interfacetype  Platform
 * @privlevel      None-privilege
 * @privilege      None
 * @product        TV, AV, B2B
 * @version        8.0
 * @SDK_Support    N
 * @remark         This is a group of C style Track related enums and
 * structures.
 * @see            Track enum conversion and structures.
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

#ifndef __PLUSPLAYER_PLUSPLAYER_CAPI_TRACK_H__
#define __PLUSPLAYER_PLUSPLAYER_CAPI_TRACK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
/**
 * @brief  Enumerations for plusplayer track type.
 */
typedef enum {
  PLUSPLAYER_TRACK_TYPE_AUDIO = 0, /**< Track type Audio */
  PLUSPLAYER_TRACK_TYPE_VIDEO,     /**< Track type Video */
  PLUSPLAYER_TRACK_TYPE_SUBTITLE   /**< Track type Subtitle */
} plusplayer_track_type_e;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // __PLUSPLAYER_PLUSPLAYER_CAPI_TRACK_H__
