// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "drm_manager.h"

#include "drm_license_helper.h"
#include "log.h"

static std::string GetDrmSubType(int drm_type) {
  switch (drm_type) {
    case DRM_TYPE_PLAYREADAY:
      return "com.microsoft.playready";
    case DRM_TYPE_WIDEVINECDM:
    default:
      return "com.widevine.alpha";
  }
}

DrmManager::DrmManager(int drm_type, const std::string &license_server_url,
                       player_h player)
    : drm_type_(drm_type),
      license_server_url_(license_server_url),
      player_(player) {}

DrmManager::~DrmManager() {}

bool DrmManager::InitializeDrmSession(const std::string &url) {
  drm_manager_handle_ = OpenDrmManager();
  if (!drm_manager_handle_) {
    LOG_ERROR("[DrmManager] Failed to dlopen libdrmmanager.");
    return false;
  }
  int ret = InitDrmManager(drm_manager_handle_);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] Failed to initialize DRM manager: %s",
              get_error_message(ret));
    return false;
  }

  media_player_handle_ = OpenMediaPlayer();
  if (!media_player_handle_) {
    LOG_ERROR("[DrmManager] Failed to dlopen libcapi-media-player.");
    return false;
  }
  ret = InitMediaPlayer(media_player_handle_);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] Failed to initialize Media Player: %s",
              get_error_message(ret));
    return false;
  }

  if (!CreateDrmSession()) {
    LOG_ERROR("[DrmManager] Failed to create a DRM session.");
    return false;
  }

  if (!SetPlayerDrm(url)) {
    LOG_ERROR("[DrmManager] Failed to set player DRM handle.");
    return false;
  }

  if (!SetChallengeCondition()) {
    LOG_ERROR("[DrmManager] Failed to set challenge condition.");
    return false;
  }

  ret = DMGRSetData(drm_session_, "Initialize", nullptr);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] Failed to initialize DRM session.");
    return false;
  }
  return true;
}

bool DrmManager::CreateDrmSession() {
  std::string sub_type = GetDrmSubType(drm_type_);
  LOG_INFO("[DrmManager] drm_sub_type: %s", sub_type.c_str());

  drm_session_ = DMGRCreateDRMSession(DM_TYPE_EME, sub_type.c_str());
  if (!drm_session_) {
    LOG_ERROR("[DrmManager] DMGRCreateDRMSession failed.");
    return false;
  }
  LOG_INFO("[DrmManager] drm_session: %p", drm_session_);

  return true;
}

bool DrmManager::SetPlayerDrm(const std::string &url) {
  int ret = DMGRSetData(drm_session_, "set_playready_manifest",
                        static_cast<void *>(const_cast<char *>(url.c_str())));
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] Setting set_playready_manifest failed: %s",
              get_error_message(ret));
    return false;
  }

  SetDataParam_t configure_param = {};
  configure_param.param1 = reinterpret_cast<void *>(OnDrmManagerError);
  configure_param.param2 = drm_session_;
  ret = DMGRSetData(drm_session_, "error_event_callback", &configure_param);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] Setting error_event_callback failed: %s",
              get_error_message(ret));
    return false;
  }

  int drm_handle = 0;
  ret = DMGRGetData(drm_session_, "drm_handle", &drm_handle);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] Getting drm_handle failed: %s",
              get_error_message(ret));
    return false;
  }
  LOG_INFO("[DrmManager] drm_handle: %d", drm_handle);

  ret = player_set_drm_handle(player_, PLAYER_DRM_TYPE_EME, drm_handle);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[DrmManager] player_set_drm_handle failed: %s",
              get_error_message(ret));
    return false;
  }

  // IMPORTANT: SetDataParam_t cannot be stack allocated because
  // DMGRSecurityInitCompleteCB is called multiple times during video playback
  // and the parameter should always be available.
  security_param_ = {};
  security_param_.param1 = player_;
  security_param_.param2 = drm_session_;
  ret = player_set_drm_init_complete_cb(player_, DMGRSecurityInitCompleteCB,
                                        &security_param_);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[DrmManager] player_set_drm_init_complete_cb failed: %s",
              get_error_message(ret));
    return false;
  }

  ret = player_set_drm_init_data_cb(player_, UpdatePsshDataCB, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[DrmManager] player_set_drm_init_data_cb failed: %s",
              get_error_message(ret));
    return false;
  }
  return true;
}

bool DrmManager::SetChallengeCondition() {
  SetDataParam_t challenge_data_param = {};
  challenge_data_param.param1 = reinterpret_cast<void *>(OnChallengeData);
  challenge_data_param.param2 = this;
  int ret = DMGRSetData(drm_session_, "eme_request_key_callback",
                        &challenge_data_param);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] Setting eme_request_key_callback failed: %s",
              get_error_message(ret));
    return false;
  }
  return true;
}

int DrmManager::OnChallengeData(void *session_id, int message_type,
                                void *message, int message_length,
                                void *user_data) {
  LOG_INFO("[DrmManager] session_id: %s", session_id);
  DrmManager *self = static_cast<DrmManager *>(user_data);

  LOG_INFO("[DrmManager] drm_type: %d", self->drm_type_);
  LOG_INFO("[DrmManager] license_server_url: %s",
           self->license_server_url_.c_str());
  LOG_INFO("[DrmManager] Challenge length: %d", message_length);

  std::vector<uint8_t> response;
  if (!self->license_server_url_.empty()) {
    // Get license via the license server.
    unsigned char *response_data = nullptr;
    unsigned long response_length = 0;
    DRM_RESULT ret = DrmLicenseHelper::DoTransactionTZ(
        self->license_server_url_.c_str(), message, message_length,
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
        static_cast<uint8_t *>(message),
        static_cast<uint8_t *>(message) + message_length);
    response = self->challenge_callback_(challenge);
  }
  LOG_INFO("[DrmManager] Response length: %d", response.size());

  SetDataParam_t license_param = {};
  license_param.param1 = session_id;
  license_param.param2 = response.data();
  license_param.param3 = reinterpret_cast<void *>(response.size());
  int ret = DMGRSetData(self->drm_session_, "install_eme_key", &license_param);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] Setting install_eme_key failed: %s",
              get_error_message(ret));
  }
  return 0;
}

int DrmManager::UpdatePsshDataCB(drm_init_data_type type, void *data,
                                 int length, void *user_data) {
  DrmManager *self = static_cast<DrmManager *>(user_data);
  LOG_INFO("[DrmManager] drm_session: %p", self->drm_session_);

  SetDataParam_t pssh_data_param = {};
  pssh_data_param.param1 = data;
  pssh_data_param.param2 = reinterpret_cast<void *>(length);
  int ret =
      DMGRSetData(self->drm_session_, "update_pssh_data", &pssh_data_param);
  if (DM_ERROR_NONE != ret) {
    LOG_ERROR("[DrmManager] Setting update_pssh_data failed: %s",
              get_error_message(ret));
    return 0;
  }
  return 1;
}

void DrmManager::ReleaseDrmSession() {
  if (drm_session_) {
    int ret = DMGRSetData(drm_session_, "Finalize", nullptr);
    if (ret != DM_ERROR_NONE) {
      LOG_ERROR("[DrmManager] Finalize failed: %s", get_error_message(ret));
    }
    ret = DMGRReleaseDRMSession(drm_session_);
    if (ret != DM_ERROR_NONE) {
      LOG_ERROR("[DrmManager] Releasing DRM session failed: %s",
                get_error_message(ret));
    }
    drm_session_ = nullptr;
  }

  // Close dlopen handles.
  CloseDrmManager(drm_manager_handle_);
  drm_manager_handle_ = nullptr;
  CloseMediaPlayer(media_player_handle_);
  media_player_handle_ = nullptr;
}

void DrmManager::OnDrmManagerError(long error_code, char *error_message,
                                   void *user_data) {
  LOG_ERROR("[DrmManager] DRM manager had error: [%ld][%s]", error_code,
            error_message);
}
