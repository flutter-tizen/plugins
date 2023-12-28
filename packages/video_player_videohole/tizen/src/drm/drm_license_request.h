// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_DRM_LICENSE_REQUEST_H_
#define FLUTTER_PLUGIN_DRM_LICENSE_REQUEST_H_

#include <functional>
#include <memory>
#include <string>
#include <vector>

using OnLicenseRequestDone = std::function<void(
    const std::string& session_id, const std::vector<uint8_t>& response_data)>;

struct DataForLicenseProcess {
  DataForLicenseProcess(void* session_id, void* message, int message_length)
      : session_id(static_cast<char*>(session_id)),
        message(static_cast<char*>(message), message_length) {}
  std::string session_id;
  std::string message;
};
class DrmLicenseRequest {
 public:
  explicit DrmLicenseRequest(
      OnLicenseRequestDone on_license_request_done_callback)
      : on_license_request_done_callback_(on_license_request_done_callback) {}
  virtual ~DrmLicenseRequest() {}
  virtual void RequestLicense(void* session_id, int message_type, void* message,
                              int message_length) = 0;
  void OnLicenseResponse(const std::string& session_id,
                         const std::vector<uint8_t>& response_data) {
    if (on_license_request_done_callback_) {
      on_license_request_done_callback_(session_id, response_data);
    }
  }

 protected:
  OnLicenseRequestDone on_license_request_done_callback_ = nullptr;
};
#endif  // FLUTTER_PLUGIN_DRM_LICENSE_REQUEST_H_
