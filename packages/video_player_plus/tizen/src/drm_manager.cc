// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "drm_manager.h"

#include "drm_licence.h"
#include "log.h"

static plusplayer::drm::Type TransferDrmType(int drmtype) {
  LOG_ERROR("drmType:%d", drmtype);

  if (drmtype == DRM_TYPE_PLAYREADAY) {
    return plusplayer::drm::Type::kPlayready;
  } else if (drmtype == DRM_TYPE_WIDEVINECDM) {
    return plusplayer::drm::Type::kWidevineCdm;
  } else {
    LOG_ERROR("Unknown PrepareCondition");
    return plusplayer::drm::Type::kNone;
  }
}

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
                       PlusplayerRef plusplayer)
    : drm_type_(drmType), license_url_(licenseUrl), plusplayer_(plusplayer) {}

DrmManager::~DrmManager() {}

bool DrmManager::CreateDrmSession(void) {
  DMGRSetDRMLocalMode(drm_manager_handle_);
  std::string drm_sub_type = GetDrmSubType(drm_type_);
  drm_session_ = DMGRCreateDRMSession(drm_manager_handle_, DM_TYPE_EME,
                                      drm_sub_type.c_str());
  if (drm_session_ == nullptr) {
    LOG_ERROR("Fail to create drm session");
    return false;
  }
  SetDataParam_t configure_param;
  configure_param.param1 = reinterpret_cast<void *>(OnDrmManagerError);
  configure_param.param2 = drm_session_;
  int ret = DMGRSetData(drm_manager_handle_, drm_session_,
                        const_cast<char *>("error_event_callback"),
                        reinterpret_cast<void *>(&configure_param));
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("Fail to set error event callback");
    return false;
  }
  return true;
}

bool DrmManager::SetChallengeCondition() {
  SetDataParam_t pSetDataParam;
  pSetDataParam.param1 = reinterpret_cast<void *>(OnChallengeData);
  pSetDataParam.param2 = reinterpret_cast<void *>(this);
  int ret =
      DMGRSetData(drm_manager_handle_, drm_session_, "eme_request_key_callback",
                  reinterpret_cast<void *>(&pSetDataParam));
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("Fail to set eme request key callback");
    return false;
  }
  return true;
}

bool DrmManager::SetPlayerDrm() {
  PlusplayerWrapperProxy &instance = PlusplayerWrapperProxy::GetInstance();
  plusplayer::drm::Property property;
  property.handle = 0;
  int ret = DMGRGetData(drm_manager_handle_, drm_session_, "drm_handle",
                        reinterpret_cast<void **>(&property.handle));
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("Fail to get drm handle");
    return false;
  }

  property.type = TransferDrmType(drm_type_);
  SetDataParam_t user_data;
  user_data.param1 = static_cast<void *>(this);
  user_data.param2 = drm_session_;
  property.license_acquired_cb =
      reinterpret_cast<plusplayer::drm::LicenseAcquiredCb>(
          DMGRSecurityInitCompleteCB);
  property.license_acquired_userdata =
      reinterpret_cast<plusplayer::drm::UserData>(&user_data);
  property.external_decryption = false;
  instance.SetDrm(plusplayer_, property);
  return true;
}

bool DrmManager::InitializeDrmSession(const std::string &url) {
  drm_manager_handle_ = OpenDrmManager();

  if (drm_manager_handle_ == nullptr) {
    LOG_ERROR("Fail to open drm manager");
    return false;
  }

  if (!CreateDrmSession()) {
    LOG_ERROR("Fail to create drm session");
    return false;
  }

  int ret = DMGRSetData(
      drm_manager_handle_, drm_session_, (const char *)"set_playready_manifest",
      const_cast<void *>(reinterpret_cast<const void *>(url.c_str())));

  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("Fail to set playready manifest callback");
    return false;
  }

  if (!SetChallengeCondition()) {
    LOG_ERROR("Fail to set challenge condition");
    return false;
  }

  std::string josn_string = "";
  ret = DMGRSetData(
      drm_manager_handle_, drm_session_, "json_string",
      const_cast<void *>(reinterpret_cast<const void *>(josn_string.c_str())));
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("Fail to set json string callback");
    return false;
  }

  ret = DMGRSetData(drm_manager_handle_, drm_session_, "Initialize", nullptr);
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("initialize failed");
    return false;
  }
  if (!SetPlayerDrm()) {
    LOG_ERROR("Fail to set player drm");
    return false;
  }
  return true;
}

void DrmManager::ReleaseDrmSession() {
  if (source_id_ > 0) {
    g_source_remove(source_id_);
  }
  source_id_ = 0;

  if (drm_session_ != nullptr) {
    int ret = DM_ERROR_NONE;

    ret = DMGRSetData(drm_manager_handle_, drm_session_, "Finalize", nullptr);
    if (ret != DM_ERROR_NONE) {
      LOG_INFO("Fail to Finalize ");
    }

    ret = DMGRReleaseDRMSession(drm_manager_handle_, drm_session_);
    if (ret != DM_ERROR_NONE) {
      LOG_INFO("Fail to release drm session");
    }
    drm_session_ = nullptr;
  }

  if (drm_manager_handle_) {
    CloseDrmManager(drm_manager_handle_);
    drm_manager_handle_ = nullptr;
  }
}

int DrmManager::OnChallengeData(void *session_id, int msgType, void *msg,
                                int msgLen, void *userData) {
  LOG_INFO("OnChallengeData, MsgType: %d", msgType);
  DrmManager *drm_manager = static_cast<DrmManager *>(userData);

  char license_url[128] = {0};
  strcpy(license_url, drm_manager->license_url_.c_str());

  LOG_INFO("[VideoPlayer] license_url %s", license_url);
  drm_manager->ppb_response_ = nullptr;
  unsigned long responseLen = 0;
  LOG_INFO("The challenge data length is %d", msgLen);

  std::string challengeData(msgLen, 0);
  memcpy(&challengeData[0], reinterpret_cast<char *>(msg), msgLen);
  // Get the license from the DRM Server
  DRM_RESULT drm_result = CBmsDrmLicenseHelper::DoTransaction_TZ(
      license_url, (const void *)&challengeData[0],
      (unsigned long)challengeData.length(), &drm_manager->ppb_response_,
      &responseLen, (CBmsDrmLicenseHelper::EDrmType)drm_manager->drm_type_,
      nullptr, nullptr);

  LOG_INFO("DRM Result:0x%lx", drm_result);
  LOG_INFO("Response:%s", drm_manager->ppb_response_);
  LOG_INFO("ResponseSize:%ld", responseLen);
  if (DRM_SUCCESS != drm_result || nullptr == drm_manager->ppb_response_ ||
      0 == responseLen) {
    LOG_ERROR("License Acquisition Failed.");
    return DM_ERROR_MANIFEST_PARSE_ERROR;
  }

  drm_manager->license_param_.param1 = session_id;
  drm_manager->license_param_.param2 = drm_manager->ppb_response_;
  drm_manager->license_param_.param3 = reinterpret_cast<void *>(responseLen);

  drm_manager->source_id_ = g_idle_add(InstallEMEKey, drm_manager);
  LOG_INFO("source_id_: %d", drm_manager->source_id_);

  return DM_ERROR_NONE;
}

gboolean DrmManager::InstallEMEKey(void *pData) {
  LOG_INFO("InstallEMEKey idler callback...");
  DrmManager *drm_manager = static_cast<DrmManager *>(pData);
  int ret = DM_ERROR_NONE;
  if (drm_manager == nullptr) {
    LOG_INFO("drm manager is null");
    return true;
  }

  LOG_INFO("Start Install license key!");
  // Make sure there is data in licenseParam.
  if (drm_manager->ppb_response_ == nullptr) {
    LOG_ERROR("ppb_response_ is null");
    return false;
  }
  LOG_INFO("DMGRSetData for install_eme_key");
  ret = DMGRSetData(drm_manager->drm_manager_handle_,
                    (DRMSessionHandle_t)drm_manager->drm_session_,
                    "install_eme_key",
                    reinterpret_cast<void *>(&drm_manager->license_param_));
  if (ret != DM_ERROR_NONE) {
    LOG_INFO("Fail to install eme key");
    return false;
  }

  free(drm_manager->ppb_response_);
  drm_manager->ppb_response_ = nullptr;
  return false;
}

void DrmManager::OnDrmManagerError(long errCode, char *errMsg, void *userData) {
  LOG_ERROR("OnDrmManagerError:[%ld][%s]", errCode, errMsg);
}

void DrmManager::OnPlayerAdaptiveStreamingControl(
    const plusplayer::MessageParam &msg) {
  LOG_INFO("msg size:%d", msg.size);
  char *pssh = new char[msg.size];
  if (nullptr == pssh) {
    LOG_ERROR("Memory Allocation Failed");
    return;
  }

  if (true == msg.data.empty() || 0 == msg.size) {
    LOG_ERROR("Empty data.");
    return;
  }
  memcpy(pssh, msg.data.c_str(), msg.size);
  SetDataParam_t psshDataParam;
  psshDataParam.param1 = reinterpret_cast<void *>(pssh);
  psshDataParam.param2 = reinterpret_cast<void *>(msg.size);
  int ret = DMGRSetData(
      drm_manager_handle_, drm_session_,
      const_cast<char *>(reinterpret_cast<const char *>("update_pssh_data")),
      reinterpret_cast<void *>(&psshDataParam));
  if (ret != DM_ERROR_NONE) {
    LOG_ERROR("setdata failed for renew callback");
    delete[] pssh;
    return;
  }
  delete[] pssh;
}

void DrmManager::OnDrmInit(int *drmhandle, unsigned int len,
                           unsigned char *psshdata,
                           plusplayer::TrackType type) {
  SetDataParam_t setDataParam;
  setDataParam.param2 = drm_session_;
  if (DMGRSecurityInitCompleteCB(drm_manager_handle_, drmhandle, len, psshdata,
                                 reinterpret_cast<void *>(&setDataParam))) {
    LOG_INFO("DMGRSecurityInitCompleteCB sucessfully!");
    PlusplayerWrapperProxy &instance = PlusplayerWrapperProxy::GetInstance();
    instance.DrmLicenseAcquiredDone(plusplayer_, type);
  }
}
