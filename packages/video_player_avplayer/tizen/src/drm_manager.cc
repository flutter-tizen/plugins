// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "drm_manager.h"

#include "drm_license_helper.h"
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
      LOG_ERROR("Failed to initialize DRM manager: %s", get_error_message(ret));
      CloseDrmManagerProxy(drm_manager_proxy_);
      drm_manager_proxy_ = nullptr;
    }
  } else {
    LOG_ERROR("Failed to dlopen libdrmmanager.");
  }
}

DrmManager::~DrmManager() {
  ReleaseDrmSession();

  // Close dlopen handles.
  if (drm_manager_proxy_) {
    CloseDrmManagerProxy(drm_manager_proxy_);
    drm_manager_proxy_ = nullptr;
  }
}

bool DrmManager::CreateDrmSession(int drm_type, bool local_mode) {
  if (!drm_manager_proxy_) {
    LOG_ERROR("Invalid handle of libdrmmanager.");
    return false;
  }

  // plusplayer should use local mode
  if (local_mode) {
    DMGRSetDRMLocalMode();
  }

  drm_type_ = drm_type;
  std::string sub_type = GetDrmSubType(drm_type);
  LOG_INFO("drm type is %s", sub_type.c_str());
  drm_session_ = DMGRCreateDRMSession(DM_TYPE_EME, sub_type.c_str());
  if (!drm_session_) {
    LOG_ERROR("Failed to create drm session.");
    return false;
  }
  LOG_INFO("Drm session is created, drm_session: %p", drm_session_);

  SetDataParam_t configure_param = {};
  configure_param.param1 = reinterpret_cast<void *>(OnDrmManagerError);
  configure_param.param2 = drm_session_;
  int ret = DMGRSetData(drm_session_, "error_event_callback", &configure_param);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("Failed to set error_event_callback to drm session: %s",
              get_error_message(ret));
    ReleaseDrmSession();
    return false;
  }

  return true;
}

bool DrmManager::SetChallenge(const std::string &media_url,
                              const std::string &license_server_url) {
  license_server_url_ = license_server_url;
  return DM_ERROR_NONE == SetChallenge(media_url);
}

bool DrmManager::SetChallenge(const std::string &media_url,
                              ChallengeCallback callback) {
  challenge_callback_ = callback;
  return DM_ERROR_NONE == SetChallenge(media_url);
}

void DrmManager::ReleaseDrmSession() {
  if (source_id_ > 0) {
    g_source_remove(source_id_);
  }
  source_id_ = 0;

  if (drm_session_) {
    int ret = 0;
    if (initialized_) {
      ret = DMGRSetData(drm_session_, "Finalize", nullptr);
      if (ret == DM_ERROR_NONE) {
        initialized_ = false;
      } else {
        LOG_ERROR("Failed to set finalize to drm session: %s",
                  get_error_message(ret));
      }
    }
    ret = DMGRReleaseDRMSession(drm_session_);
    if (ret == DM_ERROR_NONE) {
      drm_session_ = nullptr;
    } else {
      LOG_ERROR("Failed to release drm session: %s", get_error_message(ret));
    }
  }
}

bool DrmManager::GetDrmHandle(int *handle) {
  if (drm_session_) {
    *handle = 0;
    int ret = DMGRGetData(drm_session_, "drm_handle", handle);
    if (ret != DM_ERROR_NONE) {
      LOG_ERROR("Failed to get drm_handle from drm session: %s",
                get_error_message(ret));
      return false;
    }
    LOG_INFO("Get drm handle: %d", *handle);
    return true;
  } else {
    LOG_ERROR("Invalid drm session");
    return false;
  }
}

int DrmManager::UpdatePsshData(const void *data, int length) {
  if (!drm_session_) {
    LOG_ERROR("Invalid drm session.");
    return DM_ERROR_INVALID_SESSION;
  }

  SetDataParam_t pssh_data_param = {};
  pssh_data_param.param1 = const_cast<void *>(data);
  pssh_data_param.param2 = reinterpret_cast<void *>(length);
  int ret = DMGRSetData(drm_session_, "update_pssh_data", &pssh_data_param);
  if (DM_ERROR_NONE != ret) {
    LOG_ERROR("Failed to set update_pssh_data to drm session: %s",
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
    LOG_ERROR("Invalid drm session.");
    return DM_ERROR_INVALID_SESSION;
  }

  SetDataParam_t challenge_data_param = {};
  challenge_data_param.param1 = reinterpret_cast<void *>(OnChallengeData);
  challenge_data_param.param2 = this;
  int ret = DMGRSetData(drm_session_, "eme_request_key_callback",
                        &challenge_data_param);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("Failed to set eme_request_key_callback to drm session: %s",
              get_error_message(ret));
    return ret;
  }

  ret = DMGRSetData(drm_session_, "set_playready_manifest",
                    static_cast<void *>(const_cast<char *>(media_url.c_str())));
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("Failed to set set_playready_manifest to drm session: %s",
              get_error_message(ret));
    return ret;
  }

  ret = DMGRSetData(drm_session_, "Initialize", nullptr);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("Failed to set initialize to drm session: %s",
              get_error_message(ret));
    return ret;
  }
  initialized_ = true;
  return ret;
}

int DrmManager::OnChallengeData(void *session_id, int message_type,
                                void *message, int message_length,
                                void *user_data) {
  LOG_INFO("challenge: %s, challenge length: %d", message, message_length);

  DrmManager *self = static_cast<DrmManager *>(user_data);
  LOG_INFO("drm_type: %d, license server: %s", self->drm_type_,
           self->license_server_url_.c_str());
  DataForLicenseProcess *data =
      new DataForLicenseProcess(session_id, message, message_length);
  data->user_data = self;
  self->source_id_ = g_idle_add(ProcessLicense, data);
  if (self->source_id_ <= 0) {
    LOG_ERROR("g_idle_add failed, request license");
    delete data;
    return DM_ERROR_INTERNAL_ERROR;
  }
  return DM_ERROR_NONE;
}

void DrmManager::OnDrmManagerError(long error_code, char *error_message,
                                   void *user_data) {
  LOG_ERROR("DRM manager had an error: [%ld][%s]", error_code, error_message);
}

gboolean DrmManager::ProcessLicense(void *user_data) {
  DataForLicenseProcess *data = static_cast<DataForLicenseProcess *>(user_data);
  DrmManager *self = static_cast<DrmManager *>(data->user_data);

  void *response = nullptr;
  unsigned long response_len = 0;
  if (!self->license_server_url_.empty()) {
    // Get license via the license server.
    unsigned char *response_data = nullptr;
    DRM_RESULT ret = DrmLicenseHelper::DoTransactionTZ(
        self->license_server_url_.c_str(), data->message.c_str(),
        data->message.size(), &response_data, &response_len,
        static_cast<DrmLicenseHelper::DrmType>(self->drm_type_), nullptr,
        nullptr);
    if (DRM_SUCCESS != ret || nullptr == response_data || 0 == response_len) {
      LOG_ERROR("Failed to get respone by license server url");
      delete data;
      return false;
    }
    response = static_cast<void *>(response_data);
  } else if (self->challenge_callback_) {
    // Get license via the Dart callback.
    self->challenge_callback_(data->message.c_str(), data->message.size(),
                              &response, &response_len);
    if (nullptr == response || 0 == response_len) {
      LOG_ERROR("Failed to get respone by callback");
      delete data;
      return false;
    }
  } else {
    LOG_ERROR("No way to request license");
    delete data;
    return false;
  }

  LOG_INFO("Response length: %ld", response_len);
  SetDataParam_t license_param = {};
  license_param.param1 = const_cast<void *>(
      reinterpret_cast<const void *>(data->session_id.c_str()));
  license_param.param2 = response;
  license_param.param3 = reinterpret_cast<void *>(response_len);
  int ret = DMGRSetData(self->drm_session_, "install_eme_key", &license_param);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("[DrmManager] Setting install_eme_key failed: %s",
              get_error_message(ret));
  }
  delete data;
  return false;
}
