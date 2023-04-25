// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "drm_manager_service_proxy.h"

#include <dlfcn.h>

FuncDMGRSetData DMGRSetData = nullptr;
FuncDMGRGetData DMGRGetData = nullptr;
FuncDMGRCreateDRMSession DMGRCreateDRMSession = nullptr;
FuncDMGRSecurityInitCompleteCB DMGRSecurityInitCompleteCB = nullptr;
FuncDMGRReleaseDRMSession DMGRReleaseDRMSession = nullptr;
FuncPlayerSetDrmHandle player_set_drm_handle = nullptr;
FuncPlayerSetDrmInitCompleteCB player_set_drm_init_complete_cb = nullptr;
FuncPlayerSetDrmInitDataCB player_set_drm_init_data_cb = nullptr;

void* OpenDrmManager() { return dlopen("libdrmmanager.so.0", RTLD_LAZY); }

void* OpenMediaPlayer() {
  return dlopen("libcapi-media-player.so.0", RTLD_LAZY);
}

int InitDrmManager(void* handle) {
  if (!handle) {
    return DM_ERROR_INVALID_PARAM;
  }

  DMGRSetData = reinterpret_cast<FuncDMGRSetData>(dlsym(handle, "DMGRSetData"));
  if (!DMGRSetData) {
    return DM_ERROR_DL;
  }

  DMGRGetData = reinterpret_cast<FuncDMGRGetData>(dlsym(handle, "DMGRGetData"));
  if (!DMGRGetData) {
    return DM_ERROR_DL;
  }

  DMGRCreateDRMSession = reinterpret_cast<FuncDMGRCreateDRMSession>(
      dlsym(handle, "DMGRCreateDRMSession"));
  if (!DMGRCreateDRMSession) {
    return DM_ERROR_DL;
  }

  DMGRSecurityInitCompleteCB = reinterpret_cast<FuncDMGRSecurityInitCompleteCB>(
      dlsym(handle, "DMGRSecurityInitCompleteCB"));
  if (!DMGRSecurityInitCompleteCB) {
    return DM_ERROR_DL;
  }

  DMGRReleaseDRMSession = reinterpret_cast<FuncDMGRReleaseDRMSession>(
      dlsym(handle, "DMGRReleaseDRMSession"));
  if (!DMGRReleaseDRMSession) {
    return DM_ERROR_DL;
  }

  return DM_ERROR_NONE;
}

int InitMediaPlayer(void* handle) {
  player_set_drm_handle = reinterpret_cast<FuncPlayerSetDrmHandle>(
      dlsym(handle, "player_set_drm_handle"));
  if (!player_set_drm_handle) {
    return DM_ERROR_DL;
  }

  player_set_drm_init_complete_cb =
      reinterpret_cast<FuncPlayerSetDrmInitCompleteCB>(
          dlsym(handle, "player_set_drm_init_complete_cb"));
  if (!player_set_drm_init_complete_cb) {
    return DM_ERROR_DL;
  }

  player_set_drm_init_data_cb = reinterpret_cast<FuncPlayerSetDrmInitDataCB>(
      dlsym(handle, "player_set_drm_init_data_cb"));
  if (!player_set_drm_init_data_cb) {
    return DM_ERROR_DL;
  }

  return DM_ERROR_NONE;
}

void CloseDrmManager(void* handle) {
  if (handle) {
    dlclose(handle);
  }
}

void CloseMediaPlayer(void* handle) {
  if (handle) {
    dlclose(handle);
  }
}
