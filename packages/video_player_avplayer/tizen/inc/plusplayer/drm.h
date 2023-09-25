/**
* @file           drm.h
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

#ifndef __PLUSPLAYER_DRM_H__
#define __PLUSPLAYER_DRM_H__

namespace plusplayer {

namespace drm {

using LicenseAcquiredCb = void*;
using UserData = void*;
#ifdef DRM_MAPI_AARCH_64
using DrmHandle = unsigned long;
#else
using DrmHandle = int;
#endif

enum class Type {
  kNone = 0,
  kPlayready,
  kMarlin,
  kVerimatrix,
  kWidevineClassic,
  kSecuremedia,
  kSdrm,
  kWidevineCdm = 8,
  kMax
};

// post from hlsdemux for getright

struct Property {
  Type type = Type::kNone; // Drm type
  DrmHandle handle = 0; // Drm handle
  bool external_decryption = false; // External Decryption Mode
  LicenseAcquiredCb license_acquired_cb = nullptr; // The cb will be invoked when license was acquired.
  UserData license_acquired_userdata = nullptr; // The userdata will be sent by license_acquired_cb
};
  
}  // namespace drm

}  // namespace plusplayer

#endif  // __PLUSPLAYER_DRM_H__
