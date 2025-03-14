// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player.h"

#include <flutter/event_stream_handler_functions.h>
#include <flutter/standard_method_codec.h>

#include "log.h"

static int64_t player_index = 1;

VideoPlayer::VideoPlayer(flutter::BinaryMessenger *messenger,
                         FlutterDesktopViewRef flutter_view)
    : ecore_wl2_window_proxy_(std::make_unique<EcoreWl2WindowProxy>()),
      binary_messenger_(messenger),
      flutter_view_(flutter_view) {
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
    sink_event_pipe_ = nullptr;
  }
}

void VideoPlayer::ClearUpEventChannel() {
  is_initialized_ = false;
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
        if (IsReady()) {
          SendInitialized();
        } else {
          LOG_INFO("[VideoPlayer] Player is not ready.");
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
    LOG_ERROR("[VideoPlayer] event sink is nullptr.");
    return;
  }
  encodable_event_queue_.push(encodable_value);
  ecore_pipe_write(sink_event_pipe_, nullptr, 0);
}

void VideoPlayer::SendInitialized() {
  if (!is_initialized_ && event_sink_) {
    int32_t width = 0, height = 0;
    GetVideoSize(&width, &height);
    is_initialized_ = true;
    auto duration = GetDuration();
    flutter::EncodableList duration_range{
        flutter::EncodableValue(duration.first),
        flutter::EncodableValue(duration.second)};

    flutter::EncodableMap result = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("initialized")},
        {flutter::EncodableValue("duration"),
         flutter::EncodableValue(duration_range)},
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

void VideoPlayer::SendIsPlayingState(bool is_playing) {
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"),
       flutter::EncodableValue("isPlayingStateUpdate")},
      {flutter::EncodableValue("isPlaying"),
       flutter::EncodableValue(is_playing)},
  };
  PushEvent(flutter::EncodableValue(result));
}

void VideoPlayer::SendError(const PlayerError &error_code) {
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"), flutter::EncodableValue("error")},
      {flutter::EncodableValue("errorCode"),
       flutter::EncodableValue(static_cast<int32_t>(error_code))},
  };
  PushEvent(flutter::EncodableValue(result));
}

void *VideoPlayer::GetWindowHandle() {
  return FlutterDesktopViewGetNativeHandle(flutter_view_);
}
