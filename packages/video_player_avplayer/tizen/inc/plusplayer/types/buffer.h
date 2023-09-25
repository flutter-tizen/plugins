/**
* @file           buffer.h
* @brief          the buffer for playback
* @interfacetype  Module
* @privlevel      None-privilege
* @privilege      None
* @product        TV, AV, B2B
* @version        0.0.1
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
#ifndef __PLUSPLAYER_TYPES_BUFFER_H__
#define __PLUSPLAYER_TYPES_BUFFER_H__

#include <cstdint>

#include "tbm_type.h"

namespace plusplayer {

/**
 * @brief  Enumerations for the buffer status
 */
enum class BufferStatus { kUnderrun, kOverrun };

enum class DecodedVideoFrameBufferType {
  kNone,
  kCopy,
  kReference,
  kScale,
  kManualCopy,
};

enum class BufferOption {
  kBufferAudioMaxTimeSize,
  kBufferVideoMaxTimeSize,
  kBufferAudioMinTimeThreshold,
  kBufferVideoMinTimeThreshold,
  kBufferAudioMaxByteSize,
  kBufferVideoMaxByteSize,
  kBufferAudioMinByteThreshold,
  kBufferVideoMinByteThreshold,
  kBufferOptionMax
};

struct DecodedVideoPacket {
  uint64_t pts = 0;
  uint64_t duration = 0;
  tbm_surface_h surface_data = nullptr;  // tbm_surface
  void* scaler_index = nullptr;
};

/**
 * @brief Enumerations for the state of getting decoded packet
 */
enum class GetDecodedVideoFrameState {
  kErrorNone,
  kNoRemainingBufferError,
  kNoFilledBufferError,
  kUnknownError,
};

}  // namespace plusplayer

#endif  // __PLUSPLAYER_TYPES_BUFFER_H__
