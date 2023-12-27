// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "drm_manager_proxy.h"

#include <dlfcn.h>

FuncDMGRSetData DMGRSetData = nullptr;
FuncDMGRGetData DMGRGetData = nullptr;
FuncDMGRSetDRMLocalMode DMGRSetDRMLocalMode = nullptr;
FuncDMGRCreateDRMSession DMGRCreateDRMSession = nullptr;
FuncDMGRSecurityInitCompleteCB DMGRSecurityInitCompleteCB = nullptr;
FuncDMGRReleaseDRMSession DMGRReleaseDRMSession = nullptr;

void* OpenDrmManagerProxy() { return dlopen("libdrmmanager.so.0", RTLD_LAZY); }

int InitDrmManagerProxy(void* handle) {
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

  DMGRSetDRMLocalMode = reinterpret_cast<FuncDMGRSetDRMLocalMode>(
      dlsym(handle, "DMGRSetDRMLocalMode"));
  if (!DMGRSetDRMLocalMode) {
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

void CloseDrmManagerProxy(void* handle) {
  if (handle) {
    dlclose(handle);
  }
}
