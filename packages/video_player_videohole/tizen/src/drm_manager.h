// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_DRM_MANAGER_H_
#define FLUTTER_PLUGIN_DRM_MANAGER_H_

#include <player.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "drm_manager_service_proxy.h"

typedef std::function<std::vector<uint8_t>(
    const std::vector<uint8_t> &challenge)>
    ChallengeCallback;

class DrmManager {
 public:
  explicit DrmManager(int drm_type, const std::string &license_server_url,
                      player_h player);
  ~DrmManager();

  bool InitializeDrmSession(const std::string &url);
  void ReleaseDrmSession();

  void SetChallengeCallback(ChallengeCallback callback) {
    challenge_callback_ = callback;
  }

 private:
  bool CreateDrmSession();
  bool SetPlayerDrm(const std::string &url);
  bool SetChallengeCondition();

  static int OnChallengeData(void *session_id, int message_type, void *message,
                             int message_length, void *user_data);
  static int UpdatePsshDataCB(drm_init_data_type type, void *data, int length,
                              void *user_data);
  static void OnDrmManagerError(long error_code, char *error_message,
                                void *user_data);

  SetDataParam_t security_param_;
  DRMSessionHandle_t drm_session_ = nullptr;
  void *drm_manager_handle_ = nullptr;
  void *media_player_handle_ = nullptr;

  int drm_type_;
  std::string license_server_url_;
  player_h player_;

  ChallengeCallback challenge_callback_;
};

#endif  // FLUTTER_PLUGIN_DRM_MANAGER_H_
