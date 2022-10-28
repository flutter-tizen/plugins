// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_DRM_MANAGER_H_
#define FLUTTER_PLUGIN_DRM_MANAGER_H_

#include <dlfcn.h>
#include <glib.h>

#include <memory>
#include <string>

#include "drm_manager_service_proxy.h"
#include "player.h"

class DrmManager {
 public:
  DrmManager(int drmType, const std::string &licenseUrl, player_h player_);
  ~DrmManager();
  bool InitializeDrmSession(const std::string &url);
  void ReleaseDrmSession();

 private:
  bool CreateDrmSession(void);
  bool SetChallengeCondition();
  bool SetPlayerDrm(const std::string &url);
  static void OnDrmManagerError(long errCode, char *errMsg, void *userData);
  static int OnChallengeData(void *session_id, int msgType, void *msg,
                             int msgLen, void *userData);
  static int UpdatePsshDataCB(drm_init_data_type type, void *data, int length,
                              void *user_data);

  unsigned char *ppb_response_ = nullptr;
  SetDataParam_t m_param;
  DRMSessionHandle_t drm_session_ = nullptr;
  void *drm_manager_handle_ = nullptr;
  void *media_player_handle_ = nullptr;
  int drm_type_ = DRM_TYPE_NONE;
  std::string license_url_;
  player_h player_;
};

#endif  // FLUTTER_PLUGIN_DRM_MANAGER_H_
