/**
 * @file           attribute.h
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
#ifndef __PLUSPLAYER_ATTRIBUTE_H__
#define __PLUSPLAYER_ATTRIBUTE_H__

namespace plusplayer {

/**
 * @brief Enumeration for plusplayer attribute
 *        If there is new attribute, please write details in below documents.
 *        http://wiki.vd.sec.samsung.net/display/plusplayer/TrackRenderer+Attribute
 */
enum class Attribute {
  kVideoQueueMaxByte,           // std::uint64_t
  kAudioQueueMaxByte,           // std::uint64_t
  kVideoQueueCurrentLevelByte,  // std::uint64_t
  kAudioQueueCurrentLevelByte,  // std::uint64_t
  kVideoMinByteThreshold,       // std::uint32_t
  kAudioMinByteThreshold,       // std::uint32_t
  kVideoQueueMaxTime,           // std::uint64_t
  kAudioQueueMaxTime,           // std::uint64_t
  kVideoQueueCurrentLevelTime,  // std::uint64_t
  kAudioQueueCurrentLevelTime,  // std::uint64_t
  kVideoMinTimeThreshold,       // std::uint32_t
  kAudioMinTimeThreshold,       // std::uint32_t
  kMax,
};

}  // namespace plusplayer

#endif  // __PLUSPLAYER_ATTRIBUTE_H__