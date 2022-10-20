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
typedef int (*FuncPlayerSetDrmHandle)(player_h player,
                                      player_drm_type_e drm_type,
                                      int drm_handle);
typedef int (*FuncPlayerSetDrmInitCompleteCB)(
    player_h player, security_init_complete_cb callback, void* user_data);
typedef int (*FuncPlayerSetDrmInitDataCB)(player_h player,
                                          set_drm_init_data_cb callback,
                                          void* user_data);

void* Dlsym(void* handle, const char* name) {
  if (!handle) {
    LOG_ERROR("dlsym failed, handle is null");
    return nullptr;
  }
  return dlsym(handle, name);
}

void* OpenDrmManager() { return dlopen("libdrmmanager.so.0", RTLD_LAZY); }
void* OpenMediaPlayer() {
  return dlopen("libcapi-media-player.so.0", RTLD_LAZY);
}

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
  FuncDMGRGetData DMGRGetData;
  *(void**)(&DMGRGetData) = Dlsym(handle, "DMGRGetData");
  if (DMGRGetData) {
    return DMGRGetData(drm_session, data_type, output_data);
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

FuncDMGRSecurityInitCompleteCB DMGRSecurityInitCompleteCB(void* handle) {
  FuncDMGRSecurityInitCompleteCB DMGRSecurityInitCompleteCB;
  *(void**)(&DMGRSecurityInitCompleteCB) =
      Dlsym(handle, "DMGRSecurityInitCompleteCB");
  if (DMGRSecurityInitCompleteCB) {
    return DMGRSecurityInitCompleteCB;
  }
  return nullptr;
}

int DMGRReleaseDRMSession(void* handle, DRMSessionHandle_t drm_session) {
  FuncDMGRReleaseDRMSession DMGRReleaseDRMSession;
  *(void**)(&DMGRReleaseDRMSession) = Dlsym(handle, "DMGRReleaseDRMSession");
  if (DMGRReleaseDRMSession) {
    return DMGRReleaseDRMSession(drm_session);
  }
  return DM_ERROR_UNKOWN;
}

int player_set_drm_handle(void* handle, player_h player,
                          player_drm_type_e drm_type, int drm_handle) {
  FuncPlayerSetDrmHandle player_set_drm_handle;
  *(void**)(&player_set_drm_handle) = Dlsym(handle, "player_set_drm_handle");
  if (player_set_drm_handle) {
    return player_set_drm_handle(player, drm_type, drm_handle);
  }
  return 0;
}

int player_set_drm_init_complete_cb(void* handle, player_h player,
                                    security_init_complete_cb callback,
                                    void* user_data) {
  FuncPlayerSetDrmInitCompleteCB player_set_drm_init_complete_cb;
  *(void**)(&player_set_drm_init_complete_cb) =
      Dlsym(handle, "player_set_drm_init_complete_cb");
  if (player_set_drm_init_complete_cb) {
    return player_set_drm_init_complete_cb(player, callback, user_data);
  }
  return 0;
}

int player_set_drm_init_data_cb(void* handle, player_h player,
                                set_drm_init_data_cb callback,
                                void* user_data) {
  FuncPlayerSetDrmInitDataCB player_set_drm_init_data_cb;
  *(void**)(&player_set_drm_init_data_cb) =
      dlsym(handle, "player_set_drm_init_data_cb");
  if (player_set_drm_init_data_cb) {
    return player_set_drm_init_data_cb(player, callback, user_data);
  }
  return 0;
}

int CloseDrmManager(void* handle) {
  if (handle == nullptr) {
    LOG_ERROR("handle is null");
    return -1;
  }
  return dlclose(handle);
}

int CloseMediaPlayer(void* handle) {
  if (handle == nullptr) {
    LOG_ERROR("handle is null");
    return -1;
  }
  return dlclose(handle);
}