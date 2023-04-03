// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VIDEO_PLAYER_VIDEOHOLE_PLUGIN_DRM_MANAGER_H_
#define VIDEO_PLAYER_VIDEOHOLE_PLUGIN_DRM_MANAGER_H_

#include <dlfcn.h>
#include <glib.h>

#include <memory>
#include <string>

#include "drm_manager_service_proxy.h"
#include "player.h"

typedef intptr_t (*FuncLicenseCB)(uint8_t *challenge_data, size_t challenge_len,
                                  int64_t player_id);

class DrmManager {
 public:
  DrmManager(int drmType, const std::string &licenseUrl, player_h player,
             int64_t player_id);
  ~DrmManager();
  bool InitializeDrmSession(const std::string &url);
  void ReleaseDrmSession();
  void SetLicenseData(void *response_data, size_t response_len);
  void GetChallengeData(FuncLicenseCB callback);

 private:
  bool CreateDrmSession(void);
  bool SetChallengeCondition();
  bool SetPlayerDrm(const std::string &url);
  static void OnDrmManagerError(long errCode, char *errMsg, void *userData);
  static int OnChallengeData(void *session_id, int msgType, void *msg,
                             int msgLen, void *userData);
  static int UpdatePsshDataCB(drm_init_data_type type, void *data, int length,
                              void *user_data);

  SetDataParam_t security_param_;
  DRMSessionHandle_t drm_session_ = nullptr;
  void *drm_manager_handle_ = nullptr;
  void *media_player_handle_ = nullptr;
  int drm_type_ = DRM_TYPE_NONE;
  std::string license_url_;
  player_h player_;
  FuncLicenseCB get_challenge_cb_ = nullptr;
  unsigned char *ppb_response_ = nullptr;
  unsigned long pb_response_len_ = 0;
  int64_t player_id_ = -1;
};

#endif  // VIDEO_PLAYER_VIDEOHOLE_PLUGIN_DRM_MANAGER_H_
