// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_DRM_LICENSE_REQUEST_CHANNEL_H_
#define FLUTTER_PLUGIN_DRM_LICENSE_REQUEST_CHANNEL_H_

#include <Ecore.h>
#include <flutter/method_channel.h>

#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include "drm_license_request.h"

struct DataForLicenseProcess {
  DataForLicenseProcess(void *session_id, void *message, int message_length)
      : session_id(static_cast<char *>(session_id)),
        message(static_cast<char *>(message), message_length) {}
  std::string session_id;
  std::string message;
};

class DrmLicenseRequestChannel : public DrmLicenseRequest {
 public:
  explicit DrmLicenseRequestChannel(
      flutter::BinaryMessenger *binary_messenger,
      OnLicenseRequestDone on_license_request_done_callback);
  void RequestLicense(void *session_id, int message_type, void *message,
                      int message_length) override;
  ~DrmLicenseRequestChannel();
  void OnLicenseResponse(const std::string &session_id,
                         const std::vector<uint8_t> &response_data) override;

 private:
  void ExecuteRequest();
  void PushLicenseRequestData(const DataForLicenseProcess &data);
  void RequestLicense(const std::string &session_id,
                      const std::string &message);
  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>>
      request_license_channel_;
  std::mutex queue_mutex_;
  Ecore_Pipe *license_request_pipe_ = nullptr;
  std::queue<DataForLicenseProcess> license_request_queue_;
};
#endif
