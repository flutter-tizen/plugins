// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "drm_manager_service_proxy.h"

#include <dlfcn.h>

#include "log.h"

FuncDMGRSetData DMGRSetData = nullptr;
FuncDMGRGetData DMGRGetData = nullptr;
FuncDMGRCreateDRMSession DMGRCreateDRMSession = nullptr;
FuncDMGRSecurityInitCompleteCB DMGRSecurityInitCompleteCB = nullptr;
FuncDMGRReleaseDRMSession DMGRReleaseDRMSession = nullptr;
FuncPlayerSetDrmHandle player_set_drm_handle = nullptr;
FuncPlayerSetDrmInitCompleteCB player_set_drm_init_complete_cb = nullptr;
FuncPlayerSetDrmInitDataCB player_set_drm_init_data_cb = nullptr;

void* Dlsym(void* handle, const char* name) {
  if (!handle) {
    LOG_ERROR("[DrmManagerService] dlsym failed, handle is null");
    return nullptr;
  }
  return dlsym(handle, name);
}

void* OpenDrmManager() { return dlopen("libdrmmanager.so.0", RTLD_LAZY); }

void* OpenMediaPlayer() {
  return dlopen("libcapi-media-player.so.0", RTLD_LAZY);
}

int InitDrmManager(void* handle) {
  DMGRSetData = reinterpret_cast<FuncDMGRSetData>(Dlsym(handle, "DMGRSetData"));
  if (DMGRSetData == nullptr) {
    return DM_ERROR_UNKOWN;
  }

  DMGRGetData = reinterpret_cast<FuncDMGRGetData>(Dlsym(handle, "DMGRGetData"));
  if (DMGRGetData == nullptr) {
    return DM_ERROR_UNKOWN;
  }

  DMGRCreateDRMSession = reinterpret_cast<FuncDMGRCreateDRMSession>(
      Dlsym(handle, "DMGRCreateDRMSession"));
  if (DMGRCreateDRMSession == nullptr) {
    return DM_ERROR_UNKOWN;
  }

  DMGRSecurityInitCompleteCB = reinterpret_cast<FuncDMGRSecurityInitCompleteCB>(
      Dlsym(handle, "DMGRSecurityInitCompleteCB"));
  if (DMGRSecurityInitCompleteCB == nullptr) {
    return DM_ERROR_UNKOWN;
  }

  DMGRReleaseDRMSession = reinterpret_cast<FuncDMGRReleaseDRMSession>(
      Dlsym(handle, "DMGRReleaseDRMSession"));
  if (DMGRReleaseDRMSession == nullptr) {
    return DM_ERROR_UNKOWN;
  }

  return DM_ERROR_NONE;
}

int InitMediaPlayer(void* handle) {
  player_set_drm_handle = reinterpret_cast<FuncPlayerSetDrmHandle>(
      Dlsym(handle, "player_set_drm_handle"));
  if (player_set_drm_handle == nullptr) {
    return DM_ERROR_UNKOWN;
  }

  player_set_drm_init_complete_cb =
      reinterpret_cast<FuncPlayerSetDrmInitCompleteCB>(
          Dlsym(handle, "player_set_drm_init_complete_cb"));
  if (player_set_drm_init_complete_cb == nullptr) {
    return DM_ERROR_UNKOWN;
  }

  player_set_drm_init_data_cb = reinterpret_cast<FuncPlayerSetDrmInitDataCB>(
      dlsym(handle, "player_set_drm_init_data_cb"));
  if (player_set_drm_init_data_cb == nullptr) {
    return DM_ERROR_UNKOWN;
  }

  return DM_ERROR_NONE;
}

int CloseDrmManager(void* handle) {
  if (handle == nullptr) {
    LOG_ERROR("[DrmManagerService] handle is null");
    return -1;
  }
  return dlclose(handle);
}

int CloseMediaPlayer(void* handle) {
  if (handle == nullptr) {
    LOG_ERROR("[DrmManagerService] handle is null");
    return -1;
  }
  return dlclose(handle);
}
