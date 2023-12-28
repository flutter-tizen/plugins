// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "drm_license_request_channel.h"

#include <flutter/method_result_functions.h>
#include <flutter/standard_method_codec.h>

#include <utility>

#include "log.h"

DrmLicenseRequestChannel::DrmLicenseRequestChannel(
    flutter::BinaryMessenger *binary_messenger,
    OnLicenseRequestDone on_license_request_done_callback)
    : DrmLicenseRequest(on_license_request_done_callback),
      request_license_channel_(
          std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
              binary_messenger, "dev.flutter.videoplayer.drm",
              &flutter::StandardMethodCodec::GetInstance())) {
  license_request_pipe_ = ecore_pipe_add(
      [](void *data, void *buffer, unsigned int nbyte) -> void {
        auto *self = static_cast<DrmLicenseRequestChannel *>(data);
        self->ExecuteRequest();
      },
      this);
}

void DrmLicenseRequestChannel::RequestLicense(void *session_id,
                                              int message_type, void *message,
                                              int message_length) {
  DataForLicenseProcess process_message(session_id, message, message_length);
  PushLicenseRequest(process_message);
}

void DrmLicenseRequestChannel::ExecuteRequest() {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  while (!license_request_queue_.empty()) {
    DataForLicenseProcess data = license_request_queue_.front();
    RequestLicense(data.session_id, data.message);
    license_request_queue_.pop();
  }
}

void DrmLicenseRequestChannel::PushLicenseRequest(
    const DataForLicenseProcess &data) {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  license_request_queue_.push(data);
  ecore_pipe_write(license_request_pipe_, nullptr, 0);
}

void DrmLicenseRequestChannel::RequestLicense(const std::string &session_id,
                                              const std::string &message) {
  LOG_INFO("[DrmLicenseRequestChannel] Start request license.");

  if (request_license_channel_ == nullptr) {
    LOG_ERROR("[DrmLicenseRequestChannel] request license channel is null.");
    return;
  }

  std::vector<uint8_t> message_vec(message.begin(), message.end());
  flutter::EncodableMap args_map = {
      {flutter::EncodableValue("message"),
       flutter::EncodableValue(message_vec)},
  };
  auto result_handler =
      std::make_unique<flutter::MethodResultFunctions<flutter::EncodableValue>>(

          [session_id, this](const flutter::EncodableValue *success_value) {
            std::vector<uint8_t> response;
            if (std::holds_alternative<std::vector<uint8_t>>(*success_value)) {
              response = std::get<std::vector<uint8_t>>(*success_value);
            } else {
              LOG_ERROR("[DrmLicenseRequestChannel] Fail to get response.");
              return;
            }
            LOG_INFO("[DrmLicenseRequestChannel] Response length : %d",
                     response.size());
            OnLicenseResponse(session_id, response);
          },
          nullptr, nullptr);
  request_license_channel_->InvokeMethod(
      "requestLicense",
      std::make_unique<flutter::EncodableValue>(
          flutter::EncodableValue(args_map)),
      std::move(result_handler));
}

DrmLicenseRequestChannel::~DrmLicenseRequestChannel() {
  if (license_request_pipe_) {
    ecore_pipe_del(license_request_pipe_);
    license_request_pipe_ = nullptr;
  }
}
