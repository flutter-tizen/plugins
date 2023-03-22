// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VIDEO_PLAYER_VIDEOHOLE_PLUGIN_DRM_LICENCE_H_
#define VIDEO_PLAYER_VIDEOHOLE_PLUGIN_DRM_LICENCE_H_
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

class DrmLicenseHelper {
 public:
  enum DrmType {
    DRM_TYPE_NONE = 0,
    DRM_TYPE_PLAYREADY,
    DRM_TYPE_WIDEVINE,
  };

  struct SExtensionCtxTZ {
    char* http_soap_header;
    char* http_header;
    char* http_user_agent;
    bool cancel_request;

    SExtensionCtxTZ() {
      http_soap_header = nullptr;
      http_header = nullptr;
      http_user_agent = nullptr;
      cancel_request = false;
    }
  };

  static DRM_RESULT DoTransactionTZ(const char* http_server_url,
                                    const void* challenge,
                                    unsigned long challenge_len,
                                    unsigned char** response,
                                    unsigned long* response_len, DrmType type,
                                    const char* http_cookie,
                                    SExtensionCtxTZ* http_ext_ctx);
};

#endif  // VIDEO_PLAYER_VIDEOHOLE_PLUGIN_DRM_LICENCE_H_
