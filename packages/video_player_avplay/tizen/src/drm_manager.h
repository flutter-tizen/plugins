// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_DRM_MANAGER_H_
#define FLUTTER_PLUGIN_DRM_MANAGER_H_

#include <Ecore.h>
#include <flutter/method_channel.h>

#include <mutex>
#include <queue>

class DrmManager {
 public:
  typedef enum {
    DRM_TYPE_NONE,
    DRM_TYPE_PLAYREADAY,
    DRM_TYPE_WIDEVINECDM,
  } DrmType;

  explicit DrmManager();
  ~DrmManager();

  bool CreateDrmSession(int drm_type, bool local_mode);
  bool SetChallenge(const std::string &media_url,
                    const std::string &license_server_url);
  bool SetChallenge(const std::string &media_url,
                    flutter::BinaryMessenger *binary_messenger);
  bool GetDrmHandle(int *handle);
  bool SecurityInitCompleteCB(int *drm_handle, unsigned int len,
                              unsigned char *pssh_data, void *user_data);
  int UpdatePsshData(const void *data, int length);
  void ReleaseDrmSession();

 private:
  struct DataForLicenseProcess {
    DataForLicenseProcess(void *session_id, void *message, int message_length)
        : session_id(static_cast<char *>(session_id)),
          message(static_cast<char *>(message), message_length) {}
    std::string session_id;
    std::string message;
  };

  void RequestLicense(std::string &session_id, std::string &message);
  void InstallKey(void *session_id, void *response_data, void *response_len);
  int SetChallenge(const std::string &media_url);

  static int OnChallengeData(void *session_id, int message_type, void *message,
                             int message_length, void *user_data);
  static void OnDrmManagerError(long error_code, char *error_message,
                                void *user_data);
  bool ProcessLicense(DataForLicenseProcess &data);
  void PushLicenseRequestData(DataForLicenseProcess &data);
  void ExecuteRequest();

  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>>
      request_license_channel_;

  void *drm_session_ = nullptr;

  int drm_type_;
  std::string license_server_url_;
  std::mutex queue_mutex_;
  Ecore_Pipe *license_request_pipe_ = nullptr;
  std::queue<DataForLicenseProcess> license_request_queue_;
};

#endif  // FLUTTER_PLUGIN_DRM_MANAGER_H_
