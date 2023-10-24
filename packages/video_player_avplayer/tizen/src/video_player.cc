// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player.h"

#include <flutter/event_stream_handler_functions.h>
#include <flutter/standard_method_codec.h>

#include "log.h"
#include "pending_call.h"

static int64_t player_index = 1;

VideoPlayer::VideoPlayer(flutter::BinaryMessenger *messenger)
    : ecore_wl2_window_proxy_(std::make_unique<EcoreWl2WindowProxy>()),
      binary_messenger_(messenger) {
  sink_event_pipe_ = ecore_pipe_add(
      [](void *data, void *buffer, unsigned int nbyte) -> void {
        auto *self = static_cast<VideoPlayer *>(data);
        self->ExecuteSinkEvents();
      },
      this);
}

VideoPlayer::~VideoPlayer() {
  if (sink_event_pipe_) {
    ecore_pipe_del(sink_event_pipe_);
  }
  event_sink_ = nullptr;
  if (event_channel_) {
    event_channel_->SetStreamHandler(nullptr);
  }
}

int64_t VideoPlayer::SetUpEventChannel() {
  int64_t player_id = player_index++;
  std::string channel_name =
      "tizen/video_player/video_events_" + std::to_string(player_id);
  auto channel =
      std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
          binary_messenger_, channel_name,
          &flutter::StandardMethodCodec::GetInstance());
  auto handler = std::make_unique<
      flutter::StreamHandlerFunctions<flutter::EncodableValue>>(
      [&](const flutter::EncodableValue *arguments,
          std::unique_ptr<flutter::EventSink<>> &&events)
          -> std::unique_ptr<flutter::StreamHandlerError<>> {
        event_sink_ = std::move(events);
        if (isReady()) {
          SendInitialized();
        } else {
          LOG_INFO("Video Player is not ready.");
        }
        return nullptr;
      },
      [&](const flutter::EncodableValue *arguments)
          -> std::unique_ptr<flutter::StreamHandlerError<>> {
        event_sink_ = nullptr;
        return nullptr;
      });
  channel->SetStreamHandler(std::move(handler));
  event_channel_ = std::move(channel);
  return player_id;
}

void VideoPlayer::ExecuteSinkEvents() {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  while (!encodable_event_queue_.empty()) {
    if (event_sink_) {
      event_sink_->Success(encodable_event_queue_.front());
    }
    encodable_event_queue_.pop();
  }

  while (!error_event_queue_.empty()) {
    if (event_sink_) {
      event_sink_->Error(error_event_queue_.front().first,
                         error_event_queue_.front().second);
    }
    error_event_queue_.pop();
  }
}

void VideoPlayer::PushEvent(flutter::EncodableValue encodable_value) {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  if (event_sink_ == nullptr) {
    LOG_ERROR("event sink is nullptr");
    return;
  }
  encodable_event_queue_.push(encodable_value);
  ecore_pipe_write(sink_event_pipe_, nullptr, 0);
}

void VideoPlayer::SendInitialized() {
  if (!is_initialized_ && event_sink_) {
    int32_t width = 0, height = 0;
    int64_t duration = GetDuration();
    GetVideoSize(&width, &height);
    is_initialized_ = true;
    flutter::EncodableMap result = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("initialized")},
        {flutter::EncodableValue("duration"),
         flutter::EncodableValue(duration)},
        {flutter::EncodableValue("width"), flutter::EncodableValue(width)},
        {flutter::EncodableValue("height"), flutter::EncodableValue(height)},
    };
    PushEvent(flutter::EncodableValue(result));
  }
}

void VideoPlayer::SendBufferingStart() {
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"),
       flutter::EncodableValue("bufferingStart")},
  };
  PushEvent(flutter::EncodableValue(result));
}

void VideoPlayer::SendBufferingUpdate(int32_t value) {
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"),
       flutter::EncodableValue("bufferingUpdate")},
      {flutter::EncodableValue("value"), flutter::EncodableValue(value)},
  };
  PushEvent(flutter::EncodableValue(result));
}

void VideoPlayer::SendBufferingEnd() {
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"),
       flutter::EncodableValue("bufferingEnd")},
  };
  PushEvent(flutter::EncodableValue(result));
}

void VideoPlayer::SendSubtitleUpdate(int32_t duration,
                                     const std::string &text) {
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"),
       flutter::EncodableValue("subtitleUpdate")},
      {flutter::EncodableValue("duration"), flutter::EncodableValue(duration)},
      {flutter::EncodableValue("text"), flutter::EncodableValue(text)},
  };
  PushEvent(flutter::EncodableValue(result));
}

void VideoPlayer::SendPlayCompleted() {
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"), flutter::EncodableValue("completed")},
  };
  PushEvent(flutter::EncodableValue(result));
}

void VideoPlayer::SendError(const std::string &error_code,
                            const std::string &error_message) {
  if (event_sink_) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    error_event_queue_.push(std::make_pair(error_code, error_message));
    ecore_pipe_write(sink_event_pipe_, nullptr, 0);
  }
}

void VideoPlayer::OnLicenseChallenge(const void *challenge,
                                     unsigned long challenge_len,
                                     void **response,
                                     unsigned long *response_len) {
  const char *method_name = "onLicenseChallenge";
  size_t request_length = challenge_len;
  void *request_buffer = malloc(request_length);
  memcpy(request_buffer, challenge, challenge_len);

  size_t response_length = 0;
  PendingCall pending_call(response, &response_length);

  Dart_CObject c_send_port;
  c_send_port.type = Dart_CObject_kSendPort;
  c_send_port.value.as_send_port.id = pending_call.port();
  c_send_port.value.as_send_port.origin_id = ILLEGAL_PORT;

  Dart_CObject c_pending_call;
  c_pending_call.type = Dart_CObject_kInt64;
  c_pending_call.value.as_int64 = reinterpret_cast<int64_t>(&pending_call);

  Dart_CObject c_method_name;
  c_method_name.type = Dart_CObject_kString;
  c_method_name.value.as_string = const_cast<char *>(method_name);

  Dart_CObject c_request_data;
  c_request_data.type = Dart_CObject_kExternalTypedData;
  c_request_data.value.as_external_typed_data.type = Dart_TypedData_kUint8;
  c_request_data.value.as_external_typed_data.length = request_length;
  c_request_data.value.as_external_typed_data.data =
      static_cast<uint8_t *>(request_buffer);
  c_request_data.value.as_external_typed_data.peer = request_buffer;
  c_request_data.value.as_external_typed_data.callback =
      [](void *isolate_callback_data, void *peer) { free(peer); };

  Dart_CObject *c_request_arr[] = {&c_send_port, &c_pending_call,
                                   &c_method_name, &c_request_data};
  Dart_CObject c_request;
  c_request.type = Dart_CObject_kArray;
  c_request.value.as_array.values = c_request_arr;
  c_request.value.as_array.length =
      sizeof(c_request_arr) / sizeof(c_request_arr[0]);

  pending_call.PostAndWait(send_port_, &c_request);
  LOG_INFO("Received response of challenge (size: %d)", response_length);

  *response_len = response_length;
}
