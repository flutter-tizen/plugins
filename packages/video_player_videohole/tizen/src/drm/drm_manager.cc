// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "drm_manager.h"

#include <flutter/method_result_functions.h>
#include <flutter/standard_method_codec.h>

#include "drm_license_helper.h"
#include "drm_license_request_channel.h"
#include "drm_license_request_native.h"
#include "drm_manager_proxy.h"
#include "log.h"

static std::string GetDrmSubType(int drm_type) {
  switch (drm_type) {
    case DrmManager::DRM_TYPE_PLAYREADAY:
      return "com.microsoft.playready";
    case DrmManager::DRM_TYPE_WIDEVINECDM:
    default:
      return "com.widevine.alpha";
  }
}

DrmManager::DrmManager() : drm_type_(DM_TYPE_NONE) {
  drm_manager_proxy_ = OpenDrmManagerProxy();
  if (drm_manager_proxy_) {
    int ret = InitDrmManagerProxy(drm_manager_proxy_);
    if (ret != DM_ERROR_NONE) {
      LOG_ERROR("[DrmManager] Fail to initialize DRM manager: %s",
                get_error_message(ret));
      CloseDrmManagerProxy(drm_manager_proxy_);
      drm_manager_proxy_ = nullptr;
    }
  } else {
    LOG_ERROR("[DrmManager] Fail to dlopen libdrmmanager.");
  }
}

DrmManager::~DrmManager() {
  ReleaseDrmSession();
  if (drm_manager_proxy_) {
    CloseDrmManagerProxy(drm_manager_proxy_);
    drm_manager_proxy_ = nullptr;
  }
}

bool DrmManager::CreateDrmSession(int drm_type, bool local_mode) {
  if (!drm_manager_proxy_) {
    LOG_ERROR("[DrmManager] Invalid handle of libdrmmanager.");
    return false;
  }

  if (local_mode) {
    DMGRSetDRMLocalMode();
  }

  drm_type_ = drm_type;
  std::string sub_type = GetDrmSubType(drm_type);
  LOG_INFO("[DrmManager] drm type is %s", sub_type.c_str());
  drm_session_ = DMGRCreateDRMSession(DM_TYPE_EME, sub_type.c_str());
  if (!drm_session_) {
    LOG_ERROR("[DrmManager] Fail to create drm session.");
    return false;
  }
  LOG_INFO("[DrmManager] Drm session is created, drm_session: %p",
           drm_session_);

  SetDataParam_t configure_param = {};
  configure_param.param1 = reinterpret_cast<void *>(OnDrmManagerError);
  configure_param.param2 = drm_session_;
  int ret = DMGRSetData(drm_session_, "error_event_callback", &configure_param);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR(
        "[DrmManager] Fail to set error_event_callback to drm session: %s",
        get_error_message(ret));
    ReleaseDrmSession();
    return false;
  }

  return true;
}

bool DrmManager::SetChallenge(const std::string &media_url,
                              flutter::BinaryMessenger *binary_messenger) {
  drm_license_request_ = std::make_shared<DrmLicenseRequestChannel>(
      binary_messenger, [this](const std::string &session_id,
                               const std::vector<uint8_t> &response_data) {
        InstallKey(session_id, response_data);
      });
  return DM_ERROR_NONE == SetChallenge(media_url);
}

bool DrmManager::SetChallenge(const std::string &media_url,
                              const std::string &license_server_url) {
  license_server_url_ = license_server_url;
  drm_license_request_ = std::make_shared<DrmLicenseRequestNative>(
      drm_type_, license_server_url,
      [this](const std::string &session_id,
             const std::vector<uint8_t> &response_data) {
        InstallKey(session_id, response_data);
      });
  return DM_ERROR_NONE == SetChallenge(media_url);
}

void DrmManager::ReleaseDrmSession() {
  if (drm_session_ == nullptr) {
    LOG_ERROR("[DrmManager] Already released.");
    return;
  }

  SetDataParam_t challenge_data_param = {};
  challenge_data_param.param1 = nullptr;
  challenge_data_param.param2 = nullptr;
  int ret = DMGRSetData(drm_session_, "eme_request_key_callback",
                        &challenge_data_param);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] Fail to unset eme_request_key_callback: %s",
              get_error_message(ret));
  }

  ret = DMGRSetData(drm_session_, "Finalize", nullptr);

  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] Fail to set finalize to drm session: %s",
              get_error_message(ret));
  }

  ret = DMGRReleaseDRMSession(drm_session_);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] Fail to release drm session: %s",
              get_error_message(ret));
  }
  drm_session_ = nullptr;
}

bool DrmManager::GetDrmHandle(int *handle) {
  if (drm_session_) {
    *handle = 0;
    int ret = DMGRGetData(drm_session_, "drm_handle", handle);
    if (ret != DM_ERROR_NONE) {
      LOG_ERROR("[DrmManager] Fail to get drm_handle from drm session: %s",
                get_error_message(ret));
      return false;
    }
    LOG_INFO("[DrmManager] Get drm handle: %d", *handle);
    return true;
  } else {
    LOG_ERROR("[DrmManager] Invalid drm session.");
    return false;
  }
}

int DrmManager::UpdatePsshData(const void *data, int length) {
  if (!drm_session_) {
    LOG_ERROR("[DrmManager] Invalid drm session.");
    return DM_ERROR_INVALID_SESSION;
  }

  SetDataParam_t pssh_data_param = {};
  pssh_data_param.param1 = const_cast<void *>(data);
  pssh_data_param.param2 = reinterpret_cast<void *>(length);
  int ret = DMGRSetData(drm_session_, "update_pssh_data", &pssh_data_param);
  if (DM_ERROR_NONE != ret) {
    LOG_ERROR("[DrmManager] Fail to set update_pssh_data to drm session: %s",
              get_error_message(ret));
  }
  return ret;
}

bool DrmManager::SecurityInitCompleteCB(int *drm_handle, unsigned int len,
                                        unsigned char *pssh_data,
                                        void *user_data) {
  // IMPORTANT: SetDataParam_t cannot be stack allocated because
  // DMGRSecurityInitCompleteCB is called multiple times during video playback
  // and the parameter should always be available.
  SetDataParam_t security_param = {};
  if (user_data) {
    security_param.param1 = user_data;
  }
  security_param.param2 = drm_session_;

  return DMGRSecurityInitCompleteCB(drm_handle, len, pssh_data,
                                    &security_param);
}

int DrmManager::SetChallenge(const std::string &media_url) {
  if (!drm_session_) {
    LOG_ERROR("[DrmManager] Invalid drm session.");
    return DM_ERROR_INVALID_SESSION;
  }

  SetDataParam_t challenge_data_param = {};
  challenge_data_param.param1 = reinterpret_cast<void *>(OnChallengeData);
  challenge_data_param.param2 = this;
  int ret = DMGRSetData(drm_session_, "eme_request_key_callback",
                        &challenge_data_param);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR(
        "[DrmManager] Fail to set eme_request_key_callback to drm session: "
        "%s",
        get_error_message(ret));
    return ret;
  }

  ret = DMGRSetData(drm_session_, "set_playready_manifest",
                    static_cast<void *>(const_cast<char *>(media_url.c_str())));
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR(
        "[DrmManager] Fail to set set_playready_manifest to drm session: %s",
        get_error_message(ret));
    return ret;
  }

  ret = DMGRSetData(drm_session_, "Initialize", nullptr);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] Fail to set initialize to drm session: %s",
              get_error_message(ret));
    return ret;
  }
  initialized_ = true;
  return ret;
}

int DrmManager::OnChallengeData(void *session_id, int message_type,
                                void *message, int message_length,
                                void *user_data) {
  LOG_INFO("[DrmManager] challenge data: %s, challenge length: %d", message,
           message_length);
  DrmManager *self = static_cast<DrmManager *>(user_data);
  LOG_INFO("[DrmManager] drm_type: %d, license server: %s", self->drm_type_,
           self->license_server_url_.c_str());
  self->drm_license_request_->RequestLicense(session_id, message_type, message,
                                             message_length);
  return DM_ERROR_NONE;
}

void DrmManager::OnDrmManagerError(long error_code, char *error_message,
                                   void *user_data) {
  LOG_ERROR("[DrmManager] DRM manager had an error: [%ld][%s]", error_code,
            error_message);
}

void DrmManager::InstallKey(const std::string &session_id,
                            const std::vector<uint8_t> &response) {
  LOG_INFO("[DrmManager] Start install license.");

  SetDataParam_t license_param = {};
  license_param.param1 =
      reinterpret_cast<void *>(const_cast<char *>(session_id.c_str()));
  license_param.param2 = const_cast<uint8_t *>(response.data());
  license_param.param3 = reinterpret_cast<void *>(response.size());
  int ret = DMGRSetData(drm_session_, "install_eme_key", &license_param);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] Fail to install eme key: %s",
              get_error_message(ret));
  }
}
