// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_DRM_MANAGER_H_
#define FLUTTER_PLUGIN_DRM_MANAGER_H_

#include <glib.h>
#include <string>
#include "drm_manager_service_proxy.h"
#include "plus_player_proxy.h"

class DrmManager {
 public:
  DrmManager(int drmType, const std::string &licenseUrl,
             PlusplayerRef plusplayer);
  ~DrmManager();
  bool InitializeDrmSession(const std::string &url);
  void ReleaseDrmSession();
  void OnPlayerAdaptiveStreamingControl(const plusplayer::MessageParam &msg);
  void OnDrmInit(int *drmhandle, unsigned int len, unsigned char *psshdata,
                 plusplayer::TrackType type);

 private:
  bool CreateDrmSession(void);
  bool SetChallengeCondition();
  bool SetPlayerDrm();
  static void OnDrmManagerError(long errCode, char *errMsg, void *userData);
  static int OnChallengeData(void *session_id, int msgType, void *msg,
                             int msgLen, void *userData);
  static gboolean InstallEMEKey(void *pData);
  unsigned char *ppb_response_{nullptr};
  SetDataParam_t license_param_{nullptr};
  DRMSessionHandle_t drm_session_{nullptr};
  unsigned int source_id_{0};
  void *drm_manager_handle_{nullptr};
  int drm_type_{DRM_TYPE_NONE};
  std::string license_url_;
  PlusplayerRef plusplayer_{nullptr};
};

#endif  // FLUTTER_PLUGIN_DRM_MANAGER_H_
