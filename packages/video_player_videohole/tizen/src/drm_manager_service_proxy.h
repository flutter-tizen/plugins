// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_DRM_MANAGER_SERVICE_PROXY_H_
#define FLUTTER_PLUGIN_DRM_MANAGER_SERVICE_PROXY_H_

#include <player.h>

typedef enum {
  DM_ERROR_NONE = 0,                /**< Success */
  DM_ERROR_INVALID_PARAM,           /**< Invalid parameter */
  DM_ERROR_INVALID_OPERATE,         /**< Invalid operation */
  DM_ERROR_INVALID_HANDLE,          /**< Invalid handle */
  DM_ERROR_INTERNAL_ERROR,          /**< Internal error */
  DM_ERROR_TIMEOUT,                 /**< Timeout */
  DM_ERROR_MANIFEST_DOWNLOAD_ERROR, /**< Manifest download error */
  DM_ERROR_MANIFEST_PARSE_ERROR,    /**< Manifest parse error */
  DM_ERROR_FIND_NOPSSHDATA,         /**< No pssh data */

  DM_ERROR_MALLOC = 10, /**< Malloc error */
  DM_ERROR_DL,          /**< Load so error */

  DM_ERROR_INVALID_URL = 20,       /**< Invalid url */
  DM_ERROR_INVALID_SESSION,        /**< Invalid session */
  DM_ERROR_UNSUPPORTED_URL_SUFFIX, /**< Unsupported url suffix */
  DM_ERROR_INITIALIZE_FAILED,      /**< Failed to initialize DRM */

  DM_ERROR_DASH_INIT = 30, /**< DASH init failed */
  DM_ERROR_DASH_CLOSE,     /**< DASH close failed */
  DM_ERROR_DASH_OPEN,      /**< DASH open failed */

  DM_ERROR_DRM_WEB_SET = 40, /**< DRM web set failed */

  DM_ERROR_PR_HANDLE_CREATE = 50, /**< Playready handle create failed */
  DM_ERROR_PR_OPEN,               /**< Playready open failed */
  DM_ERROR_PR_DESTROY,            /**< Playready destroy failed */
  DM_ERROR_PR_GENCHALLENGE,       /**< Playready genchallenge failed */
  DM_ERROR_PR_INSTALL_LICENSE,    /**< Playready install license failed */
  DM_ERROR_PR_GETRIGHTS,          /**< Playready get rights failed */
  DM_ERROR_PR_STATUS,             /**< Playready get status failed */

  DM_ERROR_VMX_HANDLE_CREATE = 60, /**< Verimatrix handle create failed */
  DM_ERROR_VMX_FINALIZE,           /**< Verimatrix finalize failed */
  DM_ERROR_VMX_GET_UNIQUE_ID,      /**< Verimatrix get unique ID failed */

  DM_ERROR_MARLIN_OPEN = 70,   /**< Marlin open failed */
  DM_ERROR_MARLIN_CLOSE,       /**< Marlin close failed */
  DM_ERROR_MARLIN_GET_RIGHTS,  /**< Marlin get rights failed */
  DM_ERROR_MARLIN_GET_LICENSE, /**< Marlin get license failed */

  DM_ERROR_WVCDM_HANDLE_CREATE = 80,  /**< Widevinecdm handle create failed */
  DM_ERROR_WVCDM_DESTROY,             /**< Widevinecdm destroy failed */
  DM_ERROR_WVCDM_OPEN_SESSION,        /**< Widevinecdm open failed */
  DM_ERROR_WVCDM_CLOSE_SESSION,       /**< Widevinecdm close failed */
  DM_ERROR_WVCDM_GET_PROVISION,       /**< Widevinecdm get provision failed */
  DM_ERROR_WVCDM_GENERATE_KEYREQUEST, /**< Widevinecdm generate key request
                                         failed */
  DM_ERROR_WVCDM_ADD_KEY,             /**< Widevinecdm add key failed */
  DM_ERROR_WVCDM_REGISTER_EVENT,      /**< Widevinecdm register event failed */

  DM_ERROR_EME_SESSION_HANDLE_CREATE = 90, /**< EME handle create failed */
  DM_ERROR_EME_SESSION_CREATE,             /**< EME session create failed */
  DM_ERROR_EME_SESSION_DESTROY,            /**< EME session destroy failed */
  DM_ERROR_EME_SESSION_UPDATE,             /**< EME session update failed */
  DM_ERROR_EME_SESSION_REQUEST,            /**< EME session request failed */
  DM_ERROR_EME_WEB_OPERATION,              /**< EME web operation failed */
  DM_ERROR_EME_TYPE_NOT_SUPPORTED,         /**< EME type not supported */
  //...
  DM_ERROR_UNKOWN,
} dm_error_e;

typedef enum {
  PLAYER_DRM_TYPE_NONE = 0,
  PLAYER_DRM_TYPE_PLAYREADY,
  PLAYER_DRM_TYPE_MARLIN,
  PLAYER_DRM_TYPE_VERIMATRIX,
  PLAYER_DRM_TYPE_WIDEVINE_CLASSIC,
  PLAYER_DRM_TYPE_SECUREMEDIA,
  PLAYER_DRM_TYPE_SDRM,
  PLAYER_DRM_TYPE_VUDU,
  PLAYER_DRM_TYPE_WIDEVINE_CDM,
  PLAYER_DRM_TYPE_AES128,
  PLAYER_DRM_TYPE_HDCP,
  PLAYER_DRM_TYPE_DTCP,
  PLAYER_DRM_TYPE_SCSA,
  PLAYER_DRM_TYPE_CLEARKEY,
  PLAYER_DRM_TYPE_EME,
  PLAYER_DRM_TYPE_MAX_COUNT,
} player_drm_type_e;

typedef enum {
  DM_TYPE_NONE = 0,             /**< None */
  DM_TYPE_PLAYREADY = 1,        /**< Playready */
  DM_TYPE_MARLINMS3 = 2,        /**< Marlinms3 */
  DM_TYPE_VERIMATRIX = 3,       /**< Verimatrix */
  DM_TYPE_WIDEVINE_CLASSIC = 4, /**< Widevine classic */
  DM_TYPE_SECUREMEDIA = 5,      /**< Securemedia */
  DM_TYPE_SDRM = 6,             /**< SDRM */
  DM_TYPE_VUDU = 7,             /**< Vudu */
  DM_TYPE_WIDEVINE = 8,         /**< Widevine cdm */
  DM_TYPE_LYNK = 9,             /**< Lynk */
  DM_TYPE_CLEARKEY = 13,        /**< Clearkey */
  DM_TYPE_EME = 14,             /**< EME */
  //...
  DM_TYPE_MAX,
} dm_type_e;

typedef struct SetDataParam_s {
  void* param1; /**< Parameter 1 */
  void* param2; /**< Parameter 2 */
  void* param3; /**< Parameter 3 */
  void* param4; /**< Parameter 4 */
} SetDataParam_t;

typedef enum {
  DRM_TYPE_NONE,
  DRM_TYPE_PLAYREADAY,
  DRM_TYPE_WIDEVINECDM,
} DRMTYPE;

typedef enum {
  CENC = 0,
  KEYIDS = 1,
  WEBM = 2,
} drm_init_data_type;

typedef void* DRMSessionHandle_t;

typedef bool (*security_init_complete_cb)(int* drmhandle, unsigned int length,
                                          unsigned char* psshdata,
                                          void* user_data);
typedef int (*set_drm_init_data_cb)(drm_init_data_type init_type, void* data,
                                    int data_length, void* user_data);
typedef int (*FuncDMGRSetData)(DRMSessionHandle_t drm_session,
                               const char* data_type, void* input_data);
typedef int (*FuncDMGRGetData)(DRMSessionHandle_t drm_session,
                               const char* data_type, void* output_data);
typedef DRMSessionHandle_t (*FuncDMGRCreateDRMSession)(
    dm_type_e type, const char* drm_sub_type);
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

void* OpenDrmManager();
void* OpenMediaPlayer();
int InitDrmManager(void* handle);
int InitMediaPlayer(void* handle);
void CloseDrmManager(void* handle);
void CloseMediaPlayer(void* handle);

extern FuncDMGRSetData DMGRSetData;
extern FuncDMGRGetData DMGRGetData;
extern FuncDMGRCreateDRMSession DMGRCreateDRMSession;
extern FuncDMGRSecurityInitCompleteCB DMGRSecurityInitCompleteCB;
extern FuncDMGRReleaseDRMSession DMGRReleaseDRMSession;
extern FuncPlayerSetDrmHandle player_set_drm_handle;
extern FuncPlayerSetDrmInitCompleteCB player_set_drm_init_complete_cb;
extern FuncPlayerSetDrmInitDataCB player_set_drm_init_data_cb;

#endif  // FLUTTER_PLUGIN_DRM_MANAGER_SERVICE_PROXY_H_
