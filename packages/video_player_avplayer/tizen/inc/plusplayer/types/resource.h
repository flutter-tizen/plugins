/**
 * @file
 * @brief          the stream information for playback
 * @interfacetype  Module
 * @privlevel      None-privilege
 * @privilege      None
 * @product        TV, AV, B2B
 * @version        3.0
 * @SDK_Support    N
 *
 * Copyright (c) 2020 Samsung Electronics Co., Ltd All Rights Reserved
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
#ifndef __PLUSPLAYER_TYPES_RESOURCE_H__
#define __PLUSPLAYER_TYPES_RESOURCE_H__

namespace plusplayer {

/**
 * @brief  Enumerations for the resource type
 */
enum class RscType { kVideoRenderer };

/**
 * @brief   Enumerations for resource allocate policy
 */
enum class RscAllocPolicy {
  /**
   * @description   exclusive policy, default policy
   */
  kRscAllocExclusive,
  /**
   * @description   conditional policy
   */
  kRscAllocConditional,
};

/**
 * @brief   Enumerations for audio resource type 
 */
enum PlayerAudioResourceType {
  /**
   * @description   all audio resources(decoder/audio out) to main resources
   */
  kPlayerAudioResourceTypeMain,
  /**
   * @description   only audio decoder to sub resource
   */
  kPlayerAudioResourceTypeSubDecoder,
  /**
   * @description   only audio out to simple mix out resource
   */
  kPlayerAudioResourceTypeSimpleMixOut,
};

}  // namespace plusplayer

#endif  // __PLUSPLAYER_TYPES_RESOURCE_H__