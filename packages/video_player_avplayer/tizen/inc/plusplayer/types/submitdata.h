/**
 * @file           submitdata.h
 * @brief          the data type to submit
 * @interfacetype  Module
 * @privlevel      None-privilege
 * @privilege      None
 * @product        TV, AV, B2B
 * @version        0.0.1
 * @SDK_Support    N
 *
 * Copyright (c) 2019 Samsung Electronics Co., Ltd All Rights Reserved
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
#ifndef __PLUSPLAYER_TYPES_SUBMITDATA_H__
#define __PLUSPLAYER_TYPES_SUBMITDATA_H__

namespace plusplayer {

/**
 * @brief Enumerations for the type of espacket submitted
 */
enum class SubmitDataType {
  kCleanData,      // For clean data
  kEncryptedData,  // For encrypted data
  kTrustZoneData   // For trustzone data
};

}  // namespace plusplayer

#endif  // __PLUSPLAYER_TYPES_SUBMITDATA_H__