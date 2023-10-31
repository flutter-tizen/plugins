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
#ifndef __PLUSPLAYER_APPINFO_H__
#define __PLUSPLAYER_APPINFO_H__

#include <string>

namespace plusplayer {

/**
 * @brief Player app information.
 */
struct PlayerAppInfo {
  std::string id;      /**< App id */
  std::string version; /**< App version */
  std::string type;    /**< App type. ex)"MSE", "HTML5", etc.. */
};

}  // namespace plusplayer

#endif  // __PLUSPLAYER_PLAYER_APPINFO_H__