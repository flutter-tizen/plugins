// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_DRM_LICENSE_REQUEST_H_
#define FLUTTER_PLUGIN_DRM_LICENSE_REQUEST_H_

#include <functional>
#include <string>

using OnLicenseRequestDone = std::function<void(
    const std::string& session_id, std::vector<uint8_t>& response_data)>;
class DrmLicenseRequest
    : public std::enable_shared_from_this<DrmLicenseRequest> {
 public:
  DrmLicenseRequest(OnLicenseRequestDone on_license_request_done_callback)
      : on_license_request_done_callback_(on_license_request_done_callback) {}
  virtual ~DrmLicenseRequest(){};
  virtual void RequestLicense(void* session_id, int message_type, void* message,
                              int message_length) = 0;
  virtual void OnLicenseResponse(const std::string& session_id,
                                 std::vector<uint8_t>& response_data) = 0;

 protected:
  std::shared_ptr<DrmLicenseRequest> getShared() { return shared_from_this(); }
  OnLicenseRequestDone on_license_request_done_callback_ = nullptr;
};
#endif  // FLUTTER_PLUGIN_DRM_LICENSE_REQUEST_H_
