// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef FLUTTER_PLUGIN_DRM_LICENSE_REQUEST_NATIVE_H_
#define FLUTTER_PLUGIN_DRM_LICENSE_REQUEST_NATIVE_H_

#include <Ecore.h>

#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "drm_license_request.h"

class LicenseResponsePipe {
 public:
  explicit LicenseResponsePipe(
      OnLicenseRequestDone on_license_request_done_callback);
  virtual ~LicenseResponsePipe();
  void ExecuteResponse();
  void PushLicenseResponse(const std::string& session_id,
                           const std::vector<uint8_t>& response_data);

 private:
  std::mutex queue_mutex_;
  Ecore_Pipe* license_response_pipe_ = nullptr;
  std::queue<std::pair<std::string, std::vector<uint8_t>>>
      license_response_queue_;
  OnLicenseRequestDone on_license_request_done_callback_ = nullptr;
};

class DrmLicenseRequestNative : public DrmLicenseRequest {
 public:
  explicit DrmLicenseRequestNative(
      int drm_type, const std::string& license_server_url,
      OnLicenseRequestDone on_license_request_done_callback);
  virtual ~DrmLicenseRequestNative();
  void RequestLicense(void* session_id, int message_type, void* message,
                      int message_length) override;

 private:
  static void RunLoop(void* data, Ecore_Thread* thread);
  void StopMessageQueue();
  int drm_type_;
  Ecore_Thread* license_request_thread_ = nullptr;
  Eina_Thread_Queue* license_request_queue_ = nullptr;
  std::string license_server_url_;
  std::shared_ptr<LicenseResponsePipe> license_response_pipe_;
};

#endif
