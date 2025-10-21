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

#ifndef __PLUSPLAYER_PLUSPLAYER_STREAMING_MESSAGE_H__
#define __PLUSPLAYER_PLUSPLAYER_STREAMING_MESSAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

/**
 * @brief Enumeration of streaming message types
 */
typedef enum {
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_NONE = 0,      /** < None */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_BITRATECHANGE, /** < Bitrate change occurred
                                                    */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_SPARSETRACKDETECT, /** < Sparse track
                                                          detected */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_DRMINITDATA, /** < DRM initialization data
                                                    received */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_STREAMEVENTTYPE,     /** < Stream event type
                                                            notification */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_STREAMEVENTDATA,     /** < Stream event data
                                                            notification */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_STREAMSYNCFLUSH,     /** < Stream
                                                            synchronization flush */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_STREAMMRSURLCHANGED, /** < MRSS URL changed
                                                          */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_DRMKEYROTATION, /** < DRM key rotation event
                                                     */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_FRAGMENTDOWNLOADINFO, /** < Fragment
                                                             download info */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_DVRLIVELAG,       /** < DVR live lag info */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_SPARSETRACKDATA,  /** < Sparse track data
                                                         received */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_CONNECTIONRETRY,  /** < Connection retry
                                                         attempt */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_CONFIGLOWLATENCY, /** < Low latency config
                                                         update */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_CURLERRORDEBUGINFO, /** < CURL error debug
                                                           info */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_PARDARCHANGE,  /** < Parda change event */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_DASHMPDANCHOR, /** < DASH MPD anchor update
                                                    */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_DASHREMOVESTREAM, /** < DASH stream removal
                                                       */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_MEDIASYNCCSSCII,  /** < Media sync SS CII
                                                         data */
  PLUSPLAYER_STREAMING_MESSAGE_TYPE_DASHLIVETOVOD     /** < DASH live to VOD
                                                         transition */
} plusplayer_streaming_message_type_e;

/**
 * @brief  plusplayer_message_param_s structure type
 */
typedef struct {
  /**
   * @brief  Message Data
   */
  char *data;
  /**
   * @brief  size/length of message data
   */
  int size;
  /**
   * @brief  Message Code if any
   */
  int code;  // Error or warning code
} plusplayer_message_param_s;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // __PLUSPLAYER_PLUSPLAYER_STREAMING_MESSAGE_H__
