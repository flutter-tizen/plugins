/**
* @file           streaming_message.h
* @interfacetype  module
* @privlevel      None-privilege
* @privilege      None
* @product        TV, AV, B2B
* @version        1.0
* @SDK_Support    N
*
* Copyright (c) 2018 Samsung Electronics Co., Ltd All Rights Reserved
* PROPRIETARY/CONFIDENTIAL 
* This software is the confidential and proprietary
* information of SAMSUNG ELECTRONICS ("Confidential Information"). You shall
* not disclose such Confidential Information and shall use it only in
* accordance with the terms of the license agreement you entered into with
* SAMSUNG ELECTRONICS. SAMSUNG make no representations or warranties about the
* suitability of the software, either express or implied, including but not
* limited to the implied warranties of merchantability, fitness for a
* particular purpose, or non-infringement. SAMSUNG shall not be liable for any
* damages suffered by licensee as a result of using, modifying or distributing
* this software or its derivatives.
*/

#ifndef __PLUSPLAYER_TYPES_STREAMING_MESSAGE_H__
#define __PLUSPLAYER_TYPES_STREAMING_MESSAGE_H__

namespace plusplayer {

enum class StreamingMessageType {
  kNone = 0,
  // kResolutionChanged,
  // kAdEnd,
  // kAdStart,
  // kRenderDone,
  kBitrateChange,
  // kFragmentInfo,
  kSparseTrackDetect,
  // kStreamingEvent,
  // kDrmChallengeData,
  kDrmInitData,
  // kHttpErrorCode,
  // kDrmRenewSessionData,
  kStreamEventType,
  kStreamEventData,
  kStreamSyncFlush,
  kStreamMrsUrlChanged,
  kDrmKeyRotation,
  kFragmentDownloadInfo,
  kDvrLiveLag,
  kSparseTrackData,
  kConnectionRetry,
  kConfigLowLatency,
  kCurlErrorDebugInfo,
  kParDarChange
};

struct MessageParam {
  std::string data;
  int size = 0;
  int code = 0;    // Error or warning code 
};

}  // namespace plusplayer

#endif  // __PLUSPLAYER_TYPES_STREAMING_MESSAGE_H__