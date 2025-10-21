/**
 * @file
 * @brief          State enum.
 * @interfacetype  Platform
 * @privlevel      None-privilege
 * @privilege      None
 * @product        TV, AV, B2B
 * @version        8.0
 * @SDK_Support    N
 * @remark         This is a group of C style state related enum.
 * @see            State enum conversion.
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

#ifndef __PLUSPLAYER_PLUSPLAYER_CAPI_STATE_H__
#define __PLUSPLAYER_PLUSPLAYER_CAPI_STATE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Enumerations for plusplayer state.
 */
typedef enum {
  PLUSPLAYER_STATE_NONE, /**<Player is created, but not opened*/
  PLUSPLAYER_STATE_IDLE, /**<Player is opened, but not prepared or player is
                            stopped*/
  PLUSPLAYER_STATE_TYPE_FINDER_READY,  /**<TypeFinder prepared*/
  PLUSPLAYER_STATE_TRACK_SOURCE_READY, /**<TrackSource prepared*/
  PLUSPLAYER_STATE_READY,              /**<Player is ready to play(start)*/
  PLUSPLAYER_STATE_PLAYING,            /**<Player is playing media*/
  PLUSPLAYER_STATE_PAUSED              /**<Player is playing media*/
} plusplayer_state_e;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // __PLUSPLAYER_ESPLUSPLAYER_CAPI_STATE_H__
