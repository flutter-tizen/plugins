/**
 * @file
 * @brief          Property enum.
 * @interfacetype  Platform
 * @privlevel      None-privilege
 * @privilege      None
 * @product        TV, AV, B2B
 * @version        8.0
 * @SDK_Support    N
 * @remark         This is a group of C style property related enum.
 * @see            Property enum conversion.
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

#ifndef __PLUSPLAYER_PLUSPLAYER_CAPI_PROPERTY_H__
#define __PLUSPLAYER_PLUSPLAYER_CAPI_PROPERTY_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Enumerations for plusplayer property.
 */
typedef enum {
  PLUSPLAYER_PROPERTY_ADAPTIVE_INFO,        /**< String containing custom
                                                   attributes for adaptive streaming
                                                   playback. Effective only for
                                               Adaptive Streaming Protocols.*/
  PLUSPLAYER_PROPERTY_LISTEN_SPARSE_TRACK,  /**< Sparse track is a
                                                   lightweight timeline-synced
                                                   stream used for captions,
                                                   metadata, or events, delivered
                                                   alongside audio/video without
                                                   continuous samples. Value
                                                   contains String value of
                                                   Sparse name to listen.
                                                   Effective only for Smooth
                                                   Streaming content */
  PLUSPLAYER_PROPERTY_CONFIG_LOW_LATENCY,   /**< String with low latency
                                                   setting. Effective for HLS,
                                                   DASH streaming */
  PLUSPLAYER_PROPERTY_ATSC3_L1_SERVER_TIME, /**< String value of int64_t
                                                   server time from ATSC like
                                                   broadcast tuner(for US/KR
                                                   products). Effective for
                                                   ATSC3.0 ICS LIVE DASH
                                                   streaming */
  PLUSPLAYER_PROPERTY_AUDIO_DESCRIPTION,    /**< String ["ON","OFF"] to control
                                                   is download AD stream. Effective
                                                   only for DASH streaming. */
  PLUSPLAYER_PROPERTY_PRESELECTION_TAG,     /**< String value of int , for
                                                   object based audio advance
                                                   experience. Effective only for
                                                   DASH streaming. */
  PLUSPLAYER_PROPERTY_USE_MAIN_OUT_SHARE,   /**< String exist when need
                                                   advance audio HW resource
                                                   control. Effective only for
                                                   DASH streaming. */
  PLUSPLAYER_PROPERTY_URL_AUTH_TOKEN,       /**< String http token for streaming
                                          engine download. Effective only for DASH
                                          streaming.*/
  PLUSPLAYER_PROPERTY_USER_LOW_LATENCY,     /**< String ["ON","OFF"] to control
                                                   is force enable low latency live
                                                   logic. Effective only for DASH
                                                   streaming.*/
  PLUSPLAYER_PROPERTY_MAX_BANDWIDTH,        /**< String value to limit ABR.
                                                   Effective only for DASH streaming.*/
  PLUSPLAYER_PROPERTY_OPEN_HTTP_HEADER,     /**< String value ["TRUE", "FALSE"]
                                                  to control is force enable if can
                                                  get libcurl headers with
                                                  "PLUSPLAYER_PROPERTY_HTTP_HEADER"
                                                  property. Effective only for DASH
                                                  streaming.
                                                */
  PLUSPLAYER_PROPERTY_AVAILABLE_BITRATE,    /**< String listing the available
                                                   bit-rates for the
                                                   currently-playing stream.
                                               Effective only for HLS, DASH, Smooth
                                               Streaming. */
  PLUSPLAYER_PROPERTY_CURRENT_LATENCY,      /**< String value of uint64_t live
                                               latency (only for lowlatency logic
                                               on). Effective only for DASH
                                               streaming. */
  PLUSPLAYER_PROPERTY_IS_DVB_DASH,       /**< String ["1","0"] , to show is DASH
                                                DVB profile  (for EU products).
                                                Effective only for HBBTV DASH case. */
  PLUSPLAYER_PROPERTY_LIVE_PLAYER_START, /**< String value of int64_t to
                                                show when live rejoin timepoint
                                                is live_start of [live_start,
                                                live_end] range. Effective only
                                                for HBBTV DASH case.*/
  PLUSPLAYER_PROPERTY_START_DATE,        /**< String value of int64_t to show
                                                content of live  MPD starting time.
                                                Effective only for DASH streaming. */
  PLUSPLAYER_PROPERTY_MPEGH_METADATA,    /**< String value of MPEG-H metadata.
                                            Effective only for DASH streaming.*/
  PLUSPLAYER_PROPERTY_DASH_STREAM_INFO, /**< String value of dash MPD. Effective
                                           only for DASH streaming.*/
  PLUSPLAYER_PROPERTY_HTTP_HEADER,   /**< String value of Dash engine download
                                       header. Effective only for DASH
                                       streaming.*/
  PLUSPLAYER_PROPERTY_OPEN_MANIFEST, /**<String value ["TRUE", "FALSE"] to
                                       control is force enable if can get
                                       manifest content callback . Effective
                                       only for DASH streaming. */
  PLUSPLAYER_PROPERTY_UNWANTED_RESOLUTION, /**String value format: 1920X1080 To
                                              set app supported max resolution ,
                                              remove resolution larger than the
                                              current set value from the mpd
                                              track. Effective only for DASH
                                              streaming.  */
  PLUSPLAYER_PROPERTY_UNWANTED_FRAMERATE,  /** String value[0-120] To set app
                                              supported max framerate, remove
                                              framerate larger than the current
                                              set value from the mpd track.
                                              Effective only for DASH streaming.
                                            */
  PLUSPLAYER_PROPERTY_AUDIO_STREAM_INFO, /** Get manifest audio stream property
                                            information. Effective only for DASH
                                            streaming.*/
  PLUSPLAYER_PROPERTY_SUBTITLE_STREAM_INFO, /** Get manifest subtitle stream
                                               property information. Effective
                                               only for DASH streaming. */
  PLUSPLAYER_PROPERTY_VIDEO_STREAM_INFO, /** Get manifest video stream property
                                            information. Effective only for DASH
                                            streaming. */
  PLUSPLAYER_PROPERTY_UPDATE_SAME_LANGUAGE_CODE /** String value[1, 0] update
                                                   the language code in manifest
                                                   like lang="dut+i", where "i"
                                                   will be an integer when there
                                                   are more than one adaptation
                                                   set with same language code.
                                                   Effective only for DASH
                                                   streaming. */
} plusplayer_property_e;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // __PLUSPLAYER_ESPLUSPLAYER_CAPI_STATE_H__
