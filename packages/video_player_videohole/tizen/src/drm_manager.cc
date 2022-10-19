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
  int ret = 0;
  drm_manager_handle_ = OpenDrmManager();
  if (drm_manager_handle_ == nullptr) {
    LOG_ERROR("Fail to open drm manager");
    return false;
  }

  media_player_handle_ = OpenMediaPlayer();
  if (media_player_handle_ == nullptr) {
    LOG_ERROR("Fail to open libcapi-media-player.so.");
    return false;
  }

  if (!CreateDrmSession()) {
    LOG_ERROR("Fail to create drm session");
    return false;
  }

  if (!SetPlayerDrm(url)) {
    LOG_ERROR("Fail to set player drm");
    return false;
  }

  if (!SetChallengeCondition()) {
    LOG_ERROR("Fail to set challenge condition");
    return false;
  }

  ret = DMGRSetData(drm_manager_handle_, drm_session_, "Initialize", nullptr);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("initialize failed");
    return false;
  }
  return true;
}

bool DrmManager::CreateDrmSession(void) {
  LOG_INFO("dm_str: %s", GetDrmSubType(drm_type_).c_str());
  drm_session_ = DMGRCreateDRMSession(drm_manager_handle_, DM_TYPE_EME,
                                      GetDrmSubType(drm_type_).c_str());
  LOG_INFO("drm_session_ id: %p", drm_session_);
  return true;
}

bool DrmManager::SetPlayerDrm(const std::string &url) {
  int ret = 0;
  int drm_handle = 0;
  ret = DMGRSetData(drm_manager_handle_, drm_session_, "set_playready_manifest",
                    (void *)url.c_str());

  SetDataParam_t configure_param;
  configure_param.param1 = (void *)OnDrmManagerError;
  configure_param.param2 = (void *)drm_session_;
  ret = DMGRSetData(drm_manager_handle_, drm_session_,
                    (char *)"error_event_callback", (void *)&configure_param);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("setdata failed for renew callback:%s", get_error_message(ret));
    return false;
  }

  ret = DMGRGetData(drm_manager_handle_, drm_session_, "drm_handle",
                    (void **)&drm_handle);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("DMGRGetData drm_handle failed:%s", get_error_message(ret));
    return false;
  }
  LOG_DEBUG("DMGRGetData drm_handle succeed, drm handle: %d", drm_handle);

  if (player_set_drm_handle) {
    LOG_DEBUG("player_set_drm_handle drm_handle:%d", drm_handle);
    ret = player_set_drm_handle(media_player_handle_, (player_h)player_,
                                PLAYER_DRM_TYPE_EME, drm_handle);
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR("player_set_drm_handle failed:%s", get_error_message(ret));
      return false;
    }
  } else {
    LOG_ERROR("[VideoPlayer] Symbol not found:%s ", dlerror());
  }

  // IMPORTANT: Can't use local variable for SetDataParam_t, because
  // DMGRSecurityInitCompleteCB will response multiple times and exist all the
  // time during drm video play, if use malloc() way, also need to release the
  // memory after play end.
  m_param.param1 = (void *)player_;
  m_param.param2 = (void *)drm_session_;
  ret = player_set_drm_init_complete_cb(
      media_player_handle_, (player_h)player_,
      (security_init_complete_cb)DMGRSecurityInitCompleteCB(
          drm_manager_handle_),
      (void *)&m_param);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("player_set_drm_init_complete_cb failed:%s",
              get_error_message(ret));
    return false;
  }

  ret = player_set_drm_init_data_cb(media_player_handle_, (player_h)player_,
                                    (set_drm_init_data_cb)UpdatePsshDataCB,
                                    (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("player_set_drm_init_data_cb failed:%s", get_error_message(ret));
    return false;
  }
  return true;
}

bool DrmManager::SetChallengeCondition() {
  int ret = 0;
  SetDataParam_t *pChaSetDataParam = nullptr;
  pChaSetDataParam = (SetDataParam_t *)malloc(sizeof(SetDataParam_t));
  pChaSetDataParam->param1 = (void *)OnChallengeData;
  pChaSetDataParam->param2 = (void *)this;
  ret = DMGRSetData(drm_manager_handle_, drm_session_,
                    "eme_request_key_callback", (void *)pChaSetDataParam);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("challenge_data_callback failed:%s", get_error_message(ret));
  }
  if (pChaSetDataParam) {
    free(pChaSetDataParam);
  }
  return true;
}

int DrmManager::OnChallengeData(void *session_id, int msg_type, void *msg,
                                int msg_len, void *user_data) {
  int ret = 0;
  LOG_DEBUG("session_id is [%s]", session_id);
  DrmManager *pThis = static_cast<DrmManager *>(user_data);

  char license_url[128] = {0};
  strcpy(license_url, pThis->license_url_.c_str());
  LOG_INFO("[VideoPlayer] m_DrmType: %d", pThis->drm_type_);
  LOG_INFO("[VideoPlayer] license_url %s", license_url);
  pThis->ppb_response_ = nullptr;
  unsigned long pbResponse_len = 0;
  LOG_INFO("The challenge data length is %d", msg_len);
  std::string challengeData(msg_len, 0);
  memcpy(&challengeData[0], (char *)msg, msg_len);
  // Get the license from the DRM Server
  DRM_RESULT drm_result = CBmsDrmLicenseHelper::DoTransaction_TZ(
      license_url, (const void *)&challengeData[0],
      (unsigned long)challengeData.length(), &pThis->ppb_response_,
      &pbResponse_len, (CBmsDrmLicenseHelper::EDrmType)pThis->drm_type_,
      nullptr, nullptr);
  LOG_DEBUG("DRM Result:0x%lx", drm_result);
  LOG_DEBUG("Response:%s", pThis->ppb_response_);
  LOG_DEBUG("ResponseSize:%ld", pbResponse_len);

  SetDataParam_t *license_param =
      (SetDataParam_t *)malloc(sizeof(SetDataParam_t));
  license_param->param1 = session_id;
  license_param->param2 = pThis->ppb_response_;
  license_param->param3 = (void *)pbResponse_len;

  ret = DMGRSetData(pThis->drm_manager_handle_,
                    (DRMSessionHandle_t)pThis->drm_session_, "install_eme_key",
                    (void *)license_param);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("SetData for install_tvkey failed:%s", get_error_message(ret));
  }
  if (license_param) {
    free(license_param);
  }
  free(pThis->ppb_response_);
  pThis->ppb_response_ = nullptr;

  return 0;
}

int DrmManager::UpdatePsshDataCB(drm_init_data_type type, void *data,
                                 int length, void *user_data) {
  DrmManager *pThis = static_cast<DrmManager *>(user_data);
  if (pThis == nullptr) {
    LOG_ERROR("pThis == nullptr");
    return 0;
  }

  LOG_DEBUG("UpdatePsshDataCB, pDRMSession adds:%p", pThis->drm_session_);
  SetDataParam_t pssh_data;
  pssh_data.param1 = (void *)data;
  pssh_data.param2 = (void *)length;
  int ret = 0;

  ret = DMGRSetData(pThis->drm_manager_handle_, pThis->drm_session_,
                    "update_pssh_data", (void *)&pssh_data);
  if (DM_ERROR_NONE != ret) {
    LOG_ERROR("update_pssh_data failed:%s", get_error_message(ret));
    return 0;
  }
  return 1;
}

void DrmManager::ReleaseDrmSession() {
  if (drm_session_ != nullptr) {
    int ret = 0;

    ret = DMGRSetData(drm_manager_handle_, drm_session_, "Finalize", nullptr);
    if (ret != DM_ERROR_NONE) {
      LOG_ERROR("SetData Finalize failed:%s", get_error_message(ret));
    }
    ret = DMGRReleaseDRMSession(drm_manager_handle_, drm_session_);
    if (ret != DM_ERROR_NONE) {
      LOG_ERROR("ReleaseDRMSession failed:%s", get_error_message(ret));
    }
    drm_session_ = nullptr;
  }

  // close dlopen handle.
  CloseDrmManager(drm_manager_handle_);
  drm_manager_handle_ = nullptr;
  CloseMediaPlayer(media_player_handle_);
  media_player_handle_ = nullptr;
}

void DrmManager::OnDrmManagerError(long errCode, char *errMsg, void *userData) {
  LOG_ERROR("OnDrmManagerError:[%ld][%s]", errCode, errMsg);
}
