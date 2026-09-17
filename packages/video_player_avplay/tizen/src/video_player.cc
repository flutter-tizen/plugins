// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player.h"

#include <flutter/event_stream_handler_functions.h>
#include <flutter/standard_method_codec.h>

#include "log.h"

namespace video_player_avplay_tizen {

static int64_t player_index = 1;

VideoPlayer::VideoPlayer(flutter::BinaryMessenger *messenger,
                         FlutterDesktopViewRef flutter_view)
    : binary_messenger_(messenger), flutter_view_(flutter_view) {}

VideoPlayer::~VideoPlayer() {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  if (sink_event_source_) {
    g_source_remove(sink_event_source_);
  }
}

void VideoPlayer::ClearUpEventChannel() {
  is_initialized_ = false;
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    event_sink_ = nullptr;
  }
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
  sink_event_source_ = 0;
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

void VideoPlayer::RequestEventDispatch() {
  if (sink_event_source_ == 0) {
    sink_event_source_ = g_idle_add_full(
        G_PRIORITY_DEFAULT,
        [](gpointer data) -> gboolean {
          auto *self = static_cast<VideoPlayer *>(data);
          self->ExecuteSinkEvents();
          return G_SOURCE_REMOVE;
        },
        this, nullptr);
  }
}

void VideoPlayer::PushEvent(flutter::EncodableValue encodable_value) {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  if (event_sink_ == nullptr) {
    LOG_ERROR("[VideoPlayer] event sink is nullptr.");
    return;
  }
  encodable_event_queue_.push(encodable_value);
  RequestEventDispatch();
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
                                     flutter::EncodableList texts_info,
                                     flutter::EncodableMap picture_info) {
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"),
       flutter::EncodableValue("subtitleUpdate")},
      {flutter::EncodableValue("duration"), flutter::EncodableValue(duration)},
      {flutter::EncodableValue("textsInfo"),
       flutter::EncodableValue(texts_info)},
      {flutter::EncodableValue("pictureInfo"),
       flutter::EncodableValue(picture_info)},
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

void VideoPlayer::SendRestored() {
  if (is_restored_ && event_sink_) {
    is_restored_ = false;
    int32_t width = 0, height = 0;
    GetVideoSize(&width, &height);
    auto duration = GetDuration();
    flutter::EncodableList duration_range{
        flutter::EncodableValue(duration.first),
        flutter::EncodableValue(duration.second)};

    flutter::EncodableMap result = {
        {flutter::EncodableValue("event"), flutter::EncodableValue("restored")},
        {flutter::EncodableValue("duration"),
         flutter::EncodableValue(duration_range)},
        {flutter::EncodableValue("width"), flutter::EncodableValue(width)},
        {flutter::EncodableValue("height"), flutter::EncodableValue(height)},
    };
    PushEvent(flutter::EncodableValue(result));
  }
}

void VideoPlayer::SendADFromDash(flutter::EncodableMap ad_info) {
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"), flutter::EncodableValue("adFromDash")},
      {flutter::EncodableValue("adInfo"), flutter::EncodableValue(ad_info)},
  };
  PushEvent(flutter::EncodableValue(result));
}

void VideoPlayer::SendManifestInfo(std::string manifest_info) {
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"),
       flutter::EncodableValue("manifestInfoUpdated")},
      {flutter::EncodableValue("manifestInfo"),
       flutter::EncodableValue(manifest_info)},
  };
  PushEvent(flutter::EncodableValue(result));
}

void VideoPlayer::SendError(const std::string &error_code,
                            const std::string &error_message) {
  std::lock_guard<std::mutex> lock(queue_mutex_);
  if (event_sink_) {
    error_event_queue_.push(std::make_pair(error_code, error_message));
    RequestEventDispatch();
  }
}

void *VideoPlayer::GetWindowHandle() {
  return FlutterDesktopViewGetNativeHandle(flutter_view_);
}

}  // namespace video_player_avplay_tizen
