// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VIDEO_PLAYER_VIDEOHOLE_PLUGIN_DRM_MANAGER_H_
#define VIDEO_PLAYER_VIDEOHOLE_PLUGIN_DRM_MANAGER_H_

#include <dlfcn.h>
#include <glib.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "drm_manager_service_proxy.h"
#include "player.h"

typedef std::function<std::vector<uint8_t>(
    const std::vector<uint8_t> &challenge)>
    ChallengeCallback;

class DrmManager {
 public:
  DrmManager(int drmType, const std::string &licenseUrl, player_h player);
  ~DrmManager();
  bool InitializeDrmSession(const std::string &url);
  void ReleaseDrmSession();

  void SetChallengeCallback(ChallengeCallback callback) {
    challenge_callback_ = callback;
  }

 private:
  bool CreateDrmSession(void);
  bool SetChallengeCondition();
  bool SetPlayerDrm(const std::string &url);
  static void OnDrmManagerError(long errCode, char *errMsg, void *userData);
  static int OnChallengeData(void *session_id, int msg_type,
                             void *challenge_data, int challenge_length,
                             void *user_data);
  static int UpdatePsshDataCB(drm_init_data_type type, void *data, int length,
                              void *user_data);

  SetDataParam_t security_param_;
  DRMSessionHandle_t drm_session_ = nullptr;
  void *drm_manager_handle_ = nullptr;
  void *media_player_handle_ = nullptr;
  int drm_type_ = DRM_TYPE_NONE;
  std::string license_url_;
  player_h player_;
  ChallengeCallback challenge_callback_;
};

#endif  // VIDEO_PLAYER_VIDEOHOLE_PLUGIN_DRM_MANAGER_H_
