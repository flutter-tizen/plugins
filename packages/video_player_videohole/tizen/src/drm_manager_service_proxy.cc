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
    LOG_ERROR("[DrmManagerService] dlsym failed, handle is null");
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
  FuncDMGRSetData drm_set_data;
  *(void**)(&drm_set_data) = Dlsym(handle, "DMGRSetData");
  if (drm_set_data) {
    return drm_set_data(drm_session, data_type, input_data);
  }
  return DM_ERROR_UNKOWN;
}

int DMGRGetData(void* handle, DRMSessionHandle_t drm_session,
                const char* data_type, void** output_data) {
  FuncDMGRGetData drm_get_data;
  *(void**)(&drm_get_data) = Dlsym(handle, "DMGRGetData");
  if (drm_get_data) {
    return drm_get_data(drm_session, data_type, output_data);
  }
  return DM_ERROR_UNKOWN;
}

void DMGRSetDRMLocalMode(void* handle) {
  FuncDMGRSetDRMLocalMode drm_set_local_mode;
  *(void**)(&drm_set_local_mode) = Dlsym(handle, "DMGRSetDRMLocalMode");
  if (drm_set_local_mode) {
    return drm_set_local_mode();
  }
}

DRMSessionHandle_t DMGRCreateDRMSession(void* handle, dm_type_e drm_type,
                                        const char* drm_sub_type) {
  FuncDMGRCreateDRMSession drm_create_session;
  *(void**)(&drm_create_session) = Dlsym(handle, "DMGRCreateDRMSession");
  if (drm_create_session) {
    return drm_create_session(drm_type, drm_sub_type);
  }
  return nullptr;
}

FuncDMGRSecurityInitCompleteCB DMGRSecurityInitCompleteCB(void* handle) {
  FuncDMGRSecurityInitCompleteCB drm_security_init_complete_cb;
  *(void**)(&drm_security_init_complete_cb) =
      Dlsym(handle, "DMGRSecurityInitCompleteCB");
  if (drm_security_init_complete_cb) {
    return drm_security_init_complete_cb;
  }
  return nullptr;
}

int DMGRReleaseDRMSession(void* handle, DRMSessionHandle_t drm_session) {
  FuncDMGRReleaseDRMSession drm_release_session;
  *(void**)(&drm_release_session) = Dlsym(handle, "DMGRReleaseDRMSession");
  if (drm_release_session) {
    return drm_release_session(drm_session);
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
