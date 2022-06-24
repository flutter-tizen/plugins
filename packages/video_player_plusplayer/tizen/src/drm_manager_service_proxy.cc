// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "drm_manager_service_proxy.h"

#include <dlfcn.h>

#include "log.h"

typedef void (*FuncDMGRSetDRMLocalMode)();
typedef DRMSessionHandle_t (*FuncDMGRCreateDRMSession)(
    dm_type_e type, const char* drm_sub_type);
typedef int (*FuncDMGRSetData)(DRMSessionHandle_t drm_session,
                               const char* data_type, void* input_data);
typedef int (*FuncDMGRGetData)(DRMSessionHandle_t drm_session,
                               const char* data_type, void* output_data);
typedef bool (*FuncDMGRSecurityInitCompleteCB)(int* drm_handle,
                                               unsigned int len,
                                               unsigned char* pssh_data,
                                               void* user_data);
typedef int (*FuncDMGRReleaseDRMSession)(DRMSessionHandle_t drm_session);

void* Dlsym(void* handle, const char* name) {
  if (!handle) {
    LOG_ERROR("dlsym failed, handle is null");
    return nullptr;
  }
  return dlsym(handle, name);
}

void* OpenDrmManager() { return dlopen("libdrmmanager.so.0", RTLD_LAZY); }

int DMGRSetData(void* handle, DRMSessionHandle_t drm_session,
                const char* data_type, void* input_data) {
  FuncDMGRSetData DMGRSetData;
  *(void**)(&DMGRSetData) = Dlsym(handle, "DMGRSetData");
  if (DMGRSetData) {
    return DMGRSetData(drm_session, data_type, input_data);
  }
  return DM_ERROR_UNKOWN;
}

int DMGRGetData(void* handle, DRMSessionHandle_t drm_session,
                const char* data_type, void** output_data) {
  FuncDMGRGetData DMGRSetData;
  *(void**)(&DMGRSetData) = Dlsym(handle, "DMGRGetData");
  if (DMGRSetData) {
    return DMGRSetData(drm_session, data_type, output_data);
  }
  return DM_ERROR_UNKOWN;
}

void DMGRSetDRMLocalMode(void* handle) {
  FuncDMGRSetDRMLocalMode DMGRSetDRMLocalMode;
  *(void**)(&DMGRSetDRMLocalMode) = Dlsym(handle, "DMGRSetDRMLocalMode");
  if (DMGRSetDRMLocalMode) {
    return DMGRSetDRMLocalMode();
  }
}

DRMSessionHandle_t DMGRCreateDRMSession(void* handle, dm_type_e drm_type,
                                        const char* drm_sub_type) {
  FuncDMGRCreateDRMSession DMGRCreateDRMSession;
  *(void**)(&DMGRCreateDRMSession) = Dlsym(handle, "DMGRCreateDRMSession");
  if (DMGRCreateDRMSession) {
    return DMGRCreateDRMSession(drm_type, drm_sub_type);
  }
  return nullptr;
}

bool DMGRSecurityInitCompleteCB(void* handle, int* drm_handle, unsigned int len,
                                unsigned char* pssh_data, void* user_data) {
  FuncDMGRSecurityInitCompleteCB DMGRSecurityInitCompleteCB;
  *(void**)(&DMGRSecurityInitCompleteCB) =
      Dlsym(handle, "DMGRSecurityInitCompleteCB");
  if (DMGRSecurityInitCompleteCB) {
    return DMGRSecurityInitCompleteCB(drm_handle, len, pssh_data, user_data);
  }
  return false;
}

int DMGRReleaseDRMSession(void* handle, DRMSessionHandle_t drm_session) {
  FuncDMGRReleaseDRMSession DMGRReleaseDRMSession;
  *(void**)(&DMGRReleaseDRMSession) = Dlsym(handle, "DMGRReleaseDRMSession");
  if (DMGRReleaseDRMSession) {
    return DMGRReleaseDRMSession(drm_session);
  }
  return DM_ERROR_UNKOWN;
}

int CloseDrmManager(void* handle) {
  if (handle == nullptr) {
    LOG_ERROR("handle is null");
    return -1;
  }
  return dlclose(handle);
}