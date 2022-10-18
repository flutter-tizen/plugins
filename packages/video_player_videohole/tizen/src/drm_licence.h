// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VIDEO_PLAYER_PLUGIN_DRM_LICENCE_H
#define VIDEO_PLAYER_PLUGIN_DRM_LICENCE_H
typedef long DRM_RESULT;

#define DRM_SUCCESS ((DRM_RESULT)0x00000000L)
#define DRM_E_POINTER ((DRM_RESULT)0x80004003L)
#define DRM_E_INVALIDARG ((DRM_RESULT)0x80070057L)
#define DRM_E_NETWORK ((DRM_RESULT)0x91000000L)
#define DRM_E_NETWORK_CURL ((DRM_RESULT)0x91000001L)
#define DRM_E_NETWORK_HOST ((DRM_RESULT)0x91000002L)
#define DRM_E_NETWORK_CLIENT ((DRM_RESULT)0x91000003L)
#define DRM_E_NETWORK_SERVER ((DRM_RESULT)0x91000004L)
#define DRM_E_NETWORK_HEADER ((DRM_RESULT)0x91000005L)
#define DRM_E_NETWORK_REQUEST ((DRM_RESULT)0x91000006L)
#define DRM_E_NETWORK_RESPONSE ((DRM_RESULT)0x91000007L)
#define DRM_E_NETWORK_CANCELED ((DRM_RESULT)0x91000008L)

class CBmsDrmLicenseHelper {
 public:
  enum EDrmType {
    DRM_TYPE_NONE = 0,
    DRM_TYPE_PLAYREADY,
    DRM_TYPE_WIDEVINE,
  };

  struct SExtensionCtxTZ {
    char* pSoapHeader;
    char* pHttpHeader;
    char* pUserAgent;
    bool cancelRequest;

    SExtensionCtxTZ() {
      pSoapHeader = nullptr;
      pHttpHeader = nullptr;
      pUserAgent = nullptr;
      cancelRequest = false;
    }
  };

 public:
  static DRM_RESULT DoTransaction_TZ(const char* pServerUrl,
                                     const void* f_pbChallenge,
                                     unsigned long f_cbChallenge,
                                     unsigned char** f_ppbResponse,
                                     unsigned long* f_pcbResponse,
                                     EDrmType f_type, const char* f_pCookie,
                                     SExtensionCtxTZ* pExtCtx);
};

#endif  // VIDEO_PLAYER_PLUGIN_DRM_LICENCE_H
