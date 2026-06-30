// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player.h"

#include <flutter/event_stream_handler_functions.h>
#include <flutter/standard_method_codec.h>

#include "log.h"

namespace video_player_videohole_tizen {

static int64_t player_index = 1;

VideoPlayer::VideoPlayer(flutter::BinaryMessenger *messenger,
                         FlutterDesktopViewRef flutter_view)
    : ecore_wl2_window_proxy_(std::make_unique<EcoreWl2WindowProxy>()),
      binary_messenger_(messenger),
      flutter_view_(flutter_view) {
  // Initialize GMainContext and event dispatch state
  main_context_ = std::unique_ptr<GMainContext, GMainContextDeleter>(
      g_main_context_ref_thread_default());
  event_dispatch_state_ = std::make_shared<VideoPlayer::EventDispatchState>();
  event_dispatch_state_->player = this;
}

VideoPlayer::~VideoPlayer() {
  // Mark event dispatch state as disposed and cancel pending event source
  if (event_dispatch_state_) {
    std::lock_guard<std::mutex> lock(event_dispatch_state_->mutex);
    event_dispatch_state_->disposed = true;
    event_dispatch_state_->player = nullptr;

    if (event_dispatch_state_->pending_source_id != 0) {
      g_source_remove(event_dispatch_state_->pending_source_id);
      event_dispatch_state_->pending_source_id = 0;
    }
  }

  main_context_.reset();
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

void VideoPlayer::ScheduleSendPendingEvents() {
  std::lock_guard<std::mutex> lock(event_dispatch_state_->mutex);

  // Check conditions and deduplicate
  if (!main_context_ || !event_dispatch_state_ ||
      event_dispatch_state_->disposed ||
      event_dispatch_state_->pending_source_id != 0) {
    return;
  }

  auto *state = new std::shared_ptr<EventDispatchState>(event_dispatch_state_);

  GSource *source = g_idle_source_new();
  g_source_set_callback(
      source,
      [](gpointer data) -> gboolean {
        auto state = static_cast<std::shared_ptr<EventDispatchState> *>(data);
        VideoPlayer *player = nullptr;
        {
          std::lock_guard<std::mutex> lock((*state)->mutex);
          if (!(*state)->disposed && (*state)->player) {
            (*state)->pending_source_id = 0;
            player = (*state)->player;
          }
        }
        if (player) {
          player->ExecuteSinkEvents();
        }
        return G_SOURCE_REMOVE;
      },
      state,
      [](gpointer data) {
        delete static_cast<std::shared_ptr<EventDispatchState> *>(data);
      });

  event_dispatch_state_->pending_source_id =
      g_source_attach(source, main_context_.get());
  g_source_unref(source);
}

void VideoPlayer::PushEvent(flutter::EncodableValue encodable_value) {
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (!event_sink_) {
      LOG_ERROR("[VideoPlayer] event sink is nullptr.");
      return;
    }
    encodable_event_queue_.push(encodable_value);
  }
  ScheduleSendPendingEvents();
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

void VideoPlayer::SendError(const std::string &error_code,
                            const std::string &error_message) {
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (!event_sink_) {
      LOG_ERROR("[VideoPlayer] event sink is nullptr.");
      return;
    }
    error_event_queue_.push(std::make_pair(error_code, error_message));
  }
  ScheduleSendPendingEvents();
}

void *VideoPlayer::GetWindowHandle() {
  return FlutterDesktopViewGetNativeHandle(flutter_view_);
}

}  // namespace video_player_videohole_tizen
