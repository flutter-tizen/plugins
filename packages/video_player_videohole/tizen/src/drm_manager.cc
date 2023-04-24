// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "drm_manager.h"

#include "drm_licence.h"
#include "log.h"

/**
    DRM Type is DM_TYPE_EME,
    "com.microsoft.playready" => PLAYREADY
    "com.widevine.alpha" => Wideveine CDM
    "org.w3.clearkey" => Clear Key
    "org.w3.cdrmv1"  => ChinaDRM
*/
static std::string GetDrmSubType(int dm_type) {
  switch (dm_type) {
    case 1:
      return "com.microsoft.playready";
    case 2:
      return "com.widevine.alpha";
    default:
      return "com.widevine.alpha";
  }
}

DrmManager::DrmManager(int drmType, const std::string &licenseUrl,
                       player_h player)
    : drm_type_(drmType), license_url_(licenseUrl), player_(player) {}

DrmManager::~DrmManager() {}

bool DrmManager::InitializeDrmSession(const std::string &url) {
  drm_manager_handle_ = OpenDrmManager();
  if (drm_manager_handle_ == nullptr) {
    LOG_ERROR("[DrmManager] Fail to open drm manager");
    return false;
  }
  int init_drm = InitDrmManager(drm_manager_handle_);
  if (init_drm != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] Fail to init DrmManager");
    return false;
  }

  media_player_handle_ = OpenMediaPlayer();
  if (media_player_handle_ == nullptr) {
    LOG_ERROR("[DrmManager] Fail to open libcapi-media-player.so.");
    return false;
  }
  int init_player = InitMediaPlayer(media_player_handle_);
  if (init_player != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] Fail to init MediaPlayer");
    return false;
  }

  if (!CreateDrmSession()) {
    LOG_ERROR("[DrmManager] Fail to create drm session");
    return false;
  }

  if (!SetPlayerDrm(url)) {
    LOG_ERROR("[DrmManager] Fail to set player drm");
    return false;
  }

  if (!SetChallengeCondition()) {
    LOG_ERROR("[DrmManager] Fail to set challenge condition");
    return false;
  }

  int ret = DMGRSetData(drm_session_, "Initialize", nullptr);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] initialize drm session failed");
    return false;
  }
  return true;
}

bool DrmManager::CreateDrmSession(void) {
  LOG_INFO("[DrmManager] drm_type_str: %s", GetDrmSubType(drm_type_).c_str());
  drm_session_ =
      DMGRCreateDRMSession(DM_TYPE_EME, GetDrmSubType(drm_type_).c_str());
  LOG_INFO("[DrmManager] drm_session_ id: %p", drm_session_);
  return true;
}

bool DrmManager::SetPlayerDrm(const std::string &url) {
  int drm_handle = 0;
  int ret = DMGRSetData(
      drm_session_, "set_playready_manifest",
      const_cast<void *>(reinterpret_cast<const void *>(url.c_str())));

  SetDataParam_t configure_param;
  configure_param.param1 = reinterpret_cast<void *>(OnDrmManagerError);
  configure_param.param2 = reinterpret_cast<void *>(drm_session_);
  ret = DMGRSetData(drm_session_, "error_event_callback",
                    reinterpret_cast<void *>(&configure_param));
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] error_event_callback failed: %s",
              get_error_message(ret));
    return false;
  }

  ret = DMGRGetData(drm_session_, "drm_handle",
                    reinterpret_cast<void **>(&drm_handle));
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] drm_handle failed: %s", get_error_message(ret));
    return false;
  }
  LOG_DEBUG("[DrmManager] drm_handle succeed, drm handle: %d", drm_handle);

  ret = player_set_drm_handle(player_, PLAYER_DRM_TYPE_EME, drm_handle);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[DrmManager] player_set_drm_handle failed: %s",
              get_error_message(ret));
    return false;
  }

  // IMPORTANT: Can't use local variable for SetDataParam_t, because
  // DMGRSecurityInitCompleteCB will response multiple times and exist all the
  // time during drm video play, if use malloc() way, also need to release the
  // memory after play end.
  security_param_.param1 = reinterpret_cast<void *>(player_);
  security_param_.param2 = reinterpret_cast<void *>(drm_session_);
  ret = player_set_drm_init_complete_cb(
      player_, DMGRSecurityInitCompleteCB,
      reinterpret_cast<void *>(&security_param_));
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[DrmManager] player_set_drm_init_complete_cb failed: %s",
              get_error_message(ret));
    return false;
  }

  ret = player_set_drm_init_data_cb(player_, UpdatePsshDataCB,
                                    reinterpret_cast<void *>(this));
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[DrmManager] player_set_drm_init_data_cb failed: %s",
              get_error_message(ret));
    return false;
  }
  return true;
}

bool DrmManager::SetChallengeCondition() {
  SetDataParam_t *challenge_data_param = nullptr;
  challenge_data_param =
      static_cast<SetDataParam_t *>(malloc(sizeof(SetDataParam_t)));
  challenge_data_param->param1 = reinterpret_cast<void *>(OnChallengeData);
  challenge_data_param->param2 = reinterpret_cast<void *>(this);
  int ret = DMGRSetData(drm_session_, "eme_request_key_callback",
                        reinterpret_cast<void *>(challenge_data_param));
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] eme_request_key_callback failed: %s",
              get_error_message(ret));
    return false;
  }
  if (challenge_data_param) {
    free(challenge_data_param);
  }
  return true;
}

int DrmManager::OnChallengeData(void *session_id, int msg_type,
                                void *challenge_data, int challenge_length,
                                void *user_data) {
  LOG_INFO("[DrmManager] session_id: %s", session_id);
  DrmManager *self = static_cast<DrmManager *>(user_data);

  LOG_INFO("[DrmManager] drm_type: %d", self->drm_type_);
  LOG_INFO("[DrmManager] license_url: %s", self->license_url_.c_str());
  LOG_INFO("[DrmManager] Challenge length: %d", challenge_length);

  std::vector<uint8_t> response;
  if (!self->license_url_.empty()) {
    // Get license via the license server.
    unsigned char *response_data = nullptr;
    unsigned long response_length = 0;
    DRM_RESULT ret = DrmLicenseHelper::DoTransactionTZ(
        self->license_url_.c_str(), challenge_data, challenge_length,
        &response_data, &response_length,
        static_cast<DrmLicenseHelper::DrmType>(self->drm_type_), nullptr,
        nullptr);
    LOG_INFO("[DrmManager] Transaction result: 0x%lx", ret);
    response =
        std::vector<uint8_t>(response_data, response_data + response_length);
    free(response_data);
  } else {
    // Get license via the Dart callback.
    std::vector<uint8_t> challenge(
        static_cast<uint8_t *>(challenge_data),
        static_cast<uint8_t *>(challenge_data) + challenge_length);
    response = self->challenge_callback_(challenge);
  }
  LOG_INFO("[DrmManager] Response length: %d", response.size());

  SetDataParam_t license_param = {};
  license_param.param1 = session_id;
  license_param.param2 = response.data();
  license_param.param3 = reinterpret_cast<void *>(response.size());
  int ret = DMGRSetData(self->drm_session_, "install_eme_key", &license_param);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] install_eme_key failed: %s",
              get_error_message(ret));
  }

  return 0;
}

int DrmManager::UpdatePsshDataCB(drm_init_data_type type, void *data,
                                 int length, void *user_data) {
  DrmManager *drm_manager = static_cast<DrmManager *>(user_data);
  if (drm_manager == nullptr) {
    LOG_ERROR("[DrmManager] drm_manager == nullptr");
    return 0;
  }

  LOG_INFO("[DrmManager] drm_session_: %p", drm_manager->drm_session_);
  SetDataParam_t pssh_data_param;
  pssh_data_param.param1 = reinterpret_cast<void *>(data);
  pssh_data_param.param2 = reinterpret_cast<void *>(length);

  int ret = DMGRSetData(drm_manager->drm_session_, "update_pssh_data",
                        reinterpret_cast<void *>(&pssh_data_param));
  if (DM_ERROR_NONE != ret) {
    LOG_ERROR("[DrmManager] update_pssh_data failed: %s",
              get_error_message(ret));
    return 0;
  }
  return 1;
}

void DrmManager::ReleaseDrmSession() {
  if (drm_session_ != nullptr) {
    int ret = 0;
    ret = DMGRSetData(drm_session_, "Finalize", nullptr);
    if (ret != DM_ERROR_NONE) {
      LOG_ERROR("[DrmManager] Finalize failed: %s", get_error_message(ret));
    }
    ret = DMGRReleaseDRMSession(drm_session_);
    if (ret != DM_ERROR_NONE) {
      LOG_ERROR("[DrmManager] release drm session failed: %s",
                get_error_message(ret));
    }
    drm_session_ = nullptr;
  }

  // close dlopen handle.
  CloseDrmManager(drm_manager_handle_);
  drm_manager_handle_ = nullptr;
  CloseMediaPlayer(media_player_handle_);
  media_player_handle_ = nullptr;
}

void DrmManager::OnDrmManagerError(long error_code, char *error_msg,
                                   void *user_data) {
  LOG_ERROR("[DrmManager] Drm manager had error: [%ld][%s]", error_code,
            error_msg);
}
