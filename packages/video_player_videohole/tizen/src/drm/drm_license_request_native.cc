// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "drm_license_request_native.h"

#include <string>
#include <vector>

#include "drm_license_helper.h"
#include "log.h"

constexpr int kMessageQuit = -1;
constexpr int kMessageRequestLicense = 0;

struct Message {
  Eina_Thread_Queue_Msg head;
  int event;
  DataForLicenseProcess* data_for_request = nullptr;
};

LicenseResponsePipe::LicenseResponsePipe(
    OnLicenseRequestDone on_license_request_done_callback)
    : on_license_request_done_callback_(on_license_request_done_callback) {
  license_response_pipe_ = ecore_pipe_add(
      [](void* data, void* buffer, unsigned int nbyte) -> void {
        auto* self = static_cast<LicenseResponsePipe*>(data);
        self->ExecuteResponse();
      },
      this);
}

void LicenseResponsePipe::ExecuteResponse() {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  while (!license_response_queue_.empty()) {
    if (on_license_request_done_callback_) {
      auto response_data = license_response_queue_.front();
      on_license_request_done_callback_(response_data.first,
                                        response_data.second);
    }
    license_response_queue_.pop();
  }
}

void LicenseResponsePipe::PushLicenseResponse(
    const std::string& session_id, const std::vector<uint8_t>& response_data) {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  license_response_queue_.push(std::make_pair(session_id, response_data));
  ecore_pipe_write(license_response_pipe_, nullptr, 0);
}

LicenseResponsePipe::~LicenseResponsePipe() {
  if (license_response_pipe_) {
    ecore_pipe_del(license_response_pipe_);
    license_response_pipe_ = nullptr;
  }
}

DrmLicenseRequestNative::DrmLicenseRequestNative(
    int drm_type, const std::string& license_server_url,
    OnLicenseRequestDone on_license_request_done_callback)
    : DrmLicenseRequest(on_license_request_done_callback),
      drm_type_(drm_type),
      license_server_url_(license_server_url) {
  license_response_pipe_ = std::make_shared<LicenseResponsePipe>(
      [this](const std::string& session_id,
             const std::vector<uint8_t>& response_data) {
        OnLicenseResponse(session_id, response_data);
      });
  license_request_thread_ = ecore_thread_feedback_run(RunLoop, nullptr, nullptr,
                                                      nullptr, this, EINA_TRUE);
}

void DrmLicenseRequestNative::RequestLicense(void* session_id, int message_type,
                                             void* message,
                                             int message_length) {
  if (!license_request_thread_ || ecore_thread_check(license_request_thread_)) {
    LOG_ERROR("Invalid license request thread.");
    return;
  }

  if (!license_request_queue_) {
    LOG_ERROR("Invalid license request thread queue.");
    return;
  }

  void* ref;
  Message* request_message = static_cast<Message*>(
      eina_thread_queue_send(license_request_queue_, sizeof(Message), &ref));
  request_message->event = kMessageRequestLicense;
  request_message->data_for_request =
      new DataForLicenseProcess(session_id, message, message_length);
  eina_thread_queue_send_done(license_request_queue_, ref);
}

void DrmLicenseRequestNative::StopMessageQueue() {
  if (!license_request_thread_ || ecore_thread_check(license_request_thread_)) {
    LOG_ERROR("Invalid license request thread.");
    return;
  }

  if (!license_request_queue_) {
    LOG_ERROR("Invalid license request thread queue.");
    return;
  }

  void* ref;
  Message* message = static_cast<Message*>(
      eina_thread_queue_send(license_request_queue_, sizeof(Message), &ref));
  message->event = kMessageQuit;
  eina_thread_queue_send_done(license_request_queue_, ref);
}

void DrmLicenseRequestNative::RunLoop(void* data, Ecore_Thread* thread) {
  Eina_Thread_Queue* license_request_queue = eina_thread_queue_new();
  if (!license_request_queue) {
    LOG_ERROR("Invalid license request thread queue.");
    ecore_thread_cancel(thread);
    return;
  }
  auto* self = static_cast<DrmLicenseRequestNative*>(data);
  self->license_request_queue_ = license_request_queue;
  std::weak_ptr<LicenseResponsePipe> weak_pipe = self->license_response_pipe_;
  std::string license_server_url = self->license_server_url_;
  int drm_type = self->drm_type_;
  while (!ecore_thread_check(thread)) {
    void* ref;
    Message* message = static_cast<Message*>(
        eina_thread_queue_wait(license_request_queue, &ref));
    if (message->event == kMessageQuit) {
      LOG_DEBUG("Receive kMessageQuit message");
      eina_thread_queue_wait_done(license_request_queue, ref);
      break;
    }
    if (message->data_for_request == nullptr) {
      LOG_ERROR("data_for_request is null");
      continue;
    }
    std::string challenge = message->data_for_request->message;
    std::string session_id = message->data_for_request->session_id;
    delete message->data_for_request;
    eina_thread_queue_wait_done(license_request_queue, ref);
    // Get license via the license server.
    unsigned char* response_data = nullptr;
    unsigned long response_len = 0;
    std::vector<uint8_t> response;
    DRM_RESULT ret = DrmLicenseHelper::DoTransactionTZ(
        license_server_url.c_str(), challenge.c_str(), challenge.size(),
        &response_data, &response_len,
        static_cast<DrmLicenseHelper::DrmType>(drm_type), nullptr, nullptr);

    if (DRM_SUCCESS != ret || nullptr == response_data || 0 == response_len) {
      LOG_ERROR("Fail to get respone by license server url.");
      continue;
    }
    LOG_INFO("Response length : %lu", response_len);
    if (weak_pipe.expired()) {
      LOG_DEBUG("weak_pipe expired");
      free(response_data);
      break;
    }
    auto response_vec =
        std::vector<uint8_t>(response_data, response_data + response_len);
    weak_pipe.lock()->PushLicenseResponse(session_id, response_vec);
    free(response_data);
  }
  if (license_request_queue) {
    eina_thread_queue_free(license_request_queue);
  }
}

DrmLicenseRequestNative::~DrmLicenseRequestNative() {
  StopMessageQueue();
  if (license_request_thread_) {
    ecore_thread_cancel(license_request_thread_);
    license_request_thread_ = nullptr;
  }
}
