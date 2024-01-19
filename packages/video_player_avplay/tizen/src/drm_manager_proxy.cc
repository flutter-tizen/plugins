// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "drm_manager_proxy.h"

#include <dlfcn.h>

#include "log.h"

DrmManagerProxy::DrmManagerProxy()
    : drm_manager_handle_(::dlopen("libdrmmanager.so.0", RTLD_LAZY)) {
  if (drm_manager_handle_ == nullptr) {
    LOG_ERROR("Fail to open drm manager.");
    return;
  }

  dmgr_set_data_ = reinterpret_cast<FuncDMGRSetData>(
      dlsym(drm_manager_handle_, "DMGRSetData"));
  if (dmgr_set_data_ == nullptr) {
    LOG_ERROR("Fail to find DMGRSetData.");
  }

  dmgr_get_data_ = reinterpret_cast<FuncDMGRGetData>(
      dlsym(drm_manager_handle_, "DMGRGetData"));
  if (dmgr_get_data_ == nullptr) {
    LOG_ERROR("Fail to find DMGRGetData.");
  }

  dmgr_set_drm_local_mode_ = reinterpret_cast<FuncDMGRSetDRMLocalMode>(
      dlsym(drm_manager_handle_, "DMGRSetDRMLocalMode"));
  if (dmgr_set_drm_local_mode_ == nullptr) {
    LOG_ERROR("Fail to find DMGRSetDRMLocalMode.");
  }

  dmgr_create_drm_session_ = reinterpret_cast<FuncDMGRCreateDRMSession>(
      dlsym(drm_manager_handle_, "DMGRCreateDRMSession"));
  if (dmgr_create_drm_session_ == nullptr) {
    LOG_ERROR("Fail to find DMGRCreateDRMSession.");
  }

  dmgr_security_init_complete_cb_ =
      reinterpret_cast<FuncDMGRSecurityInitCompleteCB>(
          dlsym(drm_manager_handle_, "DMGRSecurityInitCompleteCB"));
  if (dmgr_security_init_complete_cb_ == nullptr) {
    LOG_ERROR("Fail to find DMGRSecurityInitCompleteCB.");
  }

  dmgr_release_drm_session_ = reinterpret_cast<FuncDMGRReleaseDRMSession>(
      dlsym(drm_manager_handle_, "DMGRReleaseDRMSession"));
  if (dmgr_release_drm_session_ == nullptr) {
    LOG_ERROR("Fail to find DMGRReleaseDRMSession.");
  }
}

int DrmManagerProxy::DMGRSetData(DRMSessionHandle_t drm_session,
                                 const char* data_type, void* input_data) {
  if (dmgr_set_data_ == nullptr) {
    return DM_ERROR_DL;
  }
  return dmgr_set_data_(drm_session, data_type, input_data);
}

int DrmManagerProxy::DMGRGetData(DRMSessionHandle_t drm_session,
                                 const char* data_type, void* output_data) {
  if (dmgr_get_data_ == nullptr) {
    return DM_ERROR_DL;
  }
  return dmgr_get_data_(drm_session, data_type, output_data);
}

void DrmManagerProxy::DMGRSetDRMLocalMode() {
  if (dmgr_set_drm_local_mode_) {
    dmgr_set_drm_local_mode_();
  }
}

DRMSessionHandle_t DrmManagerProxy::DMGRCreateDRMSession(
    dm_type_e type, const char* drm_sub_type) {
  if (dmgr_create_drm_session_ == nullptr) {
    return nullptr;
  }
  return dmgr_create_drm_session_(type, drm_sub_type);
}

bool DrmManagerProxy::DMGRSecurityInitCompleteCB(int* drm_handle,
                                                 unsigned int len,
                                                 unsigned char* pssh_data,
                                                 void* user_data) {
  if (dmgr_security_init_complete_cb_ == nullptr) {
    return false;
  }
  return dmgr_security_init_complete_cb_(drm_handle, len, pssh_data, user_data);
}

int DrmManagerProxy::DMGRReleaseDRMSession(DRMSessionHandle_t drm_session) {
  if (dmgr_release_drm_session_ == nullptr) {
    return DM_ERROR_DL;
  }
  return dmgr_release_drm_session_(drm_session);
}

DrmManagerProxy::~DrmManagerProxy() {
  if (drm_manager_handle_) {
    dlclose(drm_manager_handle_);
    drm_manager_handle_ = nullptr;
  }
}
