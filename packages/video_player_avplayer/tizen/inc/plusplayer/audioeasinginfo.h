/**
 * @file           audioeasinginfo.h
 * @interfacetype  module
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
#ifndef __PLUSPLAYER_AUDIOEASINGINFO_H__
#define __PLUSPLAYER_AUDIOEASINGINFO_H__

namespace plusplayer {

enum class AudioEasingType {
  kAudioEasingLinear = 0,
  kAudioEasingIncubic,
  kAudioEasingOutcubic,
  kAudioEasingNone
};

/**
 * @brief  audio easing information struct
 */
struct AudioEasingInfo {
  uint32_t target_volume; /**< Audio easing target volume (0 ~ 100)*/
  uint32_t duration;      /**< Audio easing duration, in millisecond */
  AudioEasingType type;   /**< Audio easing type */
};
}  // namespace plusplayer

#endif  // __PLUSPLAYER_AUDIOEASINGINFO_H__