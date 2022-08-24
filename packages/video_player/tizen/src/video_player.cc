// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player.h"

#include <flutter/event_stream_handler_functions.h>
#include <flutter/standard_method_codec.h>

#include <algorithm>

#include "log.h"
#include "video_player_error.h"

constexpr int kMessageQuit = -1;
constexpr int kMessageOnFrameDecoded = 0;
constexpr int kMessageOnRenderFinished = 1;

struct Message {
  Eina_Thread_Queue_Msg head;
  int event;
  media_packet_h media_packet;
};

static std::string RotationToString(player_display_rotation_e rotation) {
  switch (rotation) {
    case PLAYER_DISPLAY_ROTATION_NONE:
      return "PLAYER_DISPLAY_ROTATION_NONE";
    case PLAYER_DISPLAY_ROTATION_90:
      return "PLAYER_DISPLAY_ROTATION_90";
    case PLAYER_DISPLAY_ROTATION_180:
      return "PLAYER_DISPLAY_ROTATION_180";
    case PLAYER_DISPLAY_ROTATION_270:
      return "PLAYER_DISPLAY_ROTATION_270";
  }
  return std::string();
}

static std::string StateToString(player_state_e state) {
  switch (state) {
    case PLAYER_STATE_NONE:
      return "PLAYER_STATE_NONE";
    case PLAYER_STATE_IDLE:
      return "PLAYER_STATE_IDLE";
    case PLAYER_STATE_READY:
      return "PLAYER_STATE_READY";
    case PLAYER_STATE_PLAYING:
      return "PLAYER_STATE_PLAYING";
    case PLAYER_STATE_PAUSED:
      return "PLAYER_STATE_PAUSED";
  }
  return std::string();
}

void VideoPlayer::ReleaseMediaPacket(void *data) {
  auto *player = reinterpret_cast<VideoPlayer *>(data);

  std::lock_guard<std::mutex> lock(player->mutex_);
  player->is_rendering_ = false;
  player->SendRenderFinishedMessage();
}

FlutterDesktopGpuBuffer *VideoPlayer::ObtainGpuBuffer(size_t width,
                                                      size_t height) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!current_media_packet_) {
    LOG_ERROR("[VideoPlayer] current media packet not valid.");
    is_rendering_ = false;
    SendRenderFinishedMessage();
    return nullptr;
  }

  tbm_surface_h surface;
  int ret = media_packet_get_tbm_surface(current_media_packet_, &surface);
  if (ret != MEDIA_PACKET_ERROR_NONE || !surface) {
    LOG_ERROR("[VideoPlayer] Failed to get a tbm surface, error: %d", ret);
    is_rendering_ = false;
    media_packet_destroy(current_media_packet_);
    current_media_packet_ = nullptr;
    SendRenderFinishedMessage();
    return nullptr;
  }
  flutter_desktop_gpu_buffer_->buffer = surface;
  flutter_desktop_gpu_buffer_->width = width;
  flutter_desktop_gpu_buffer_->height = height;
  flutter_desktop_gpu_buffer_->release_context = this;
  flutter_desktop_gpu_buffer_->release_callback = ReleaseMediaPacket;
  return flutter_desktop_gpu_buffer_.get();
}

VideoPlayer::VideoPlayer(flutter::PluginRegistrar *plugin_registrar,
                         flutter::TextureRegistrar *texture_registrar,
                         const std::string &uri, VideoPlayerOptions &options) {
  is_initialized_ = false;
  is_rendering_ = false;
  texture_registrar_ = texture_registrar;

  texture_variant_ =
      std::make_unique<flutter::TextureVariant>(flutter::GpuBufferTexture(
          [this](size_t width,
                 size_t height) -> const FlutterDesktopGpuBuffer * {
            return this->ObtainGpuBuffer(width, height);
          }));
  flutter_desktop_gpu_buffer_ = std::make_unique<FlutterDesktopGpuBuffer>();
  texture_id_ = texture_registrar->RegisterTexture(texture_variant_.get());

  int ret = player_create(&player_);
  if (ret != PLAYER_ERROR_NONE) {
    throw VideoPlayerError("player_create failed", get_error_message(ret));
  }

  ret = player_set_uri(player_, uri.c_str());
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    throw VideoPlayerError("player_set_uri failed", get_error_message(ret));
  }

  ret = player_set_display_visible(player_, true);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    throw VideoPlayerError("player_set_display_visible failed",
                           get_error_message(ret));
  }

  packet_thread_ = ecore_thread_feedback_run(RunMediaPacketLoop, nullptr,
                                             nullptr, nullptr, this, EINA_TRUE);
  ret = player_set_media_packet_video_frame_decoded_cb(
      player_, OnVideoFrameDecoded, this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    throw VideoPlayerError(
        "player_set_media_packet_video_frame_decoded_cb failed",
        get_error_message(ret));
  }

  ret = player_set_buffering_cb(player_, OnBuffering, this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    throw VideoPlayerError("player_set_buffering_cb failed",
                           get_error_message(ret));
  }

  ret = player_set_completed_cb(player_, OnPlayCompleted, this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    throw VideoPlayerError("player_set_completed_cb failed",
                           get_error_message(ret));
  }

  ret = player_set_interrupted_cb(player_, OnInterrupted, this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    throw VideoPlayerError("player_set_interrupted_cb failed",
                           get_error_message(ret));
  }

  ret = player_set_error_cb(player_, OnError, this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    throw VideoPlayerError("player_set_error_cb failed",
                           get_error_message(ret));
  }

  ret = player_prepare_async(player_, OnPrepared, this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    throw VideoPlayerError("player_prepare_async failed",
                           get_error_message(ret));
  }

  SetUpEventChannel(plugin_registrar->messenger());
}

VideoPlayer::~VideoPlayer() { Dispose(); }

void VideoPlayer::Play() {
  LOG_DEBUG("[VideoPlayer] start player");

  player_state_e state;
  int ret = player_get_state(player_, &state);
  if (ret == PLAYER_ERROR_NONE) {
    LOG_INFO("[VideoPlayer] Player state: %s", StateToString(state).c_str());
    if (state != PLAYER_STATE_PAUSED && state != PLAYER_STATE_READY) {
      return;
    }
  }

  ret = player_start(player_);
  if (ret != PLAYER_ERROR_NONE) {
    throw VideoPlayerError("player_start failed", get_error_message(ret));
  }
}

void VideoPlayer::Pause() {
  LOG_DEBUG("[VideoPlayer] pause player");

  player_state_e state;
  int ret = player_get_state(player_, &state);
  if (ret == PLAYER_ERROR_NONE) {
    LOG_INFO("[VideoPlayer] Player state: %s", StateToString(state).c_str());
    if (state != PLAYER_STATE_PLAYING) {
      return;
    }
  }

  ret = player_pause(player_);
  if (ret != PLAYER_ERROR_NONE) {
    throw VideoPlayerError("player_pause failed", get_error_message(ret));
  }
}

void VideoPlayer::SetLooping(bool is_looping) {
  LOG_DEBUG("[VideoPlayer] isLooping: %d", is_looping);

  int ret = player_set_looping(player_, is_looping);
  if (ret != PLAYER_ERROR_NONE) {
    throw VideoPlayerError("player_set_looping failed", get_error_message(ret));
  }
}

void VideoPlayer::SetVolume(double volume) {
  LOG_DEBUG("[VideoPlayer] volume: %f", volume);

  int ret = player_set_volume(player_, volume, volume);
  if (ret != PLAYER_ERROR_NONE) {
    throw VideoPlayerError("player_set_volume failed", get_error_message(ret));
  }
}

void VideoPlayer::SetPlaybackSpeed(double speed) {
  LOG_DEBUG("[VideoPlayer] speed: %f", speed);

  int ret = player_set_playback_rate(player_, speed);
  if (ret != PLAYER_ERROR_NONE) {
    throw VideoPlayerError("player_set_playback_rate failed",
                           get_error_message(ret));
  }
}

void VideoPlayer::SeekTo(int position,
                         const SeekCompletedCallback &seek_completed_cb) {
  LOG_DEBUG("[VideoPlayer] position: %d", position);

  on_seek_completed_ = seek_completed_cb;
  int ret =
      player_set_play_position(player_, position, true, OnSeekCompleted, this);
  if (ret != PLAYER_ERROR_NONE) {
    on_seek_completed_ = nullptr;
    throw VideoPlayerError("player_set_play_position failed",
                           get_error_message(ret));
  }
}

int VideoPlayer::GetPosition() {
  int position;
  int ret = player_get_play_position(player_, &position);
  if (ret != PLAYER_ERROR_NONE) {
    throw VideoPlayerError("player_get_play_position failed",
                           get_error_message(ret));
  }

  LOG_DEBUG("[VideoPlayer] position: %d", position);
  return position;
}

void VideoPlayer::Dispose() {
  LOG_DEBUG("[VideoPlayer] dispose player");

  is_initialized_ = false;
  event_sink_ = nullptr;
  event_channel_->SetStreamHandler(nullptr);

  if (player_) {
    player_unprepare(player_);
    player_unset_media_packet_video_frame_decoded_cb(player_);
    player_unset_buffering_cb(player_);
    player_unset_completed_cb(player_);
    player_unset_interrupted_cb(player_);
    player_unset_error_cb(player_);
    player_destroy(player_);
    player_ = 0;
  }

  SendMessage(kMessageQuit, nullptr);
  if (packet_thread_) {
    ecore_thread_cancel(packet_thread_);
    packet_thread_ = nullptr;
  }

  if (current_media_packet_) {
    media_packet_destroy(current_media_packet_);
    current_media_packet_ = nullptr;
  }

  if (previous_media_packet_) {
    media_packet_destroy(previous_media_packet_);
    previous_media_packet_ = nullptr;
  }

  if (texture_registrar_) {
    texture_registrar_->UnregisterTexture(texture_id_);
    texture_registrar_ = nullptr;
  }
}

void VideoPlayer::SetUpEventChannel(flutter::BinaryMessenger *messenger) {
  LOG_DEBUG("[VideoPlayer] set up event channel");

  std::string name =
      "flutter.io/videoPlayer/videoEvents" + std::to_string(texture_id_);
  auto channel =
      std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
          messenger, name, &flutter::StandardMethodCodec::GetInstance());
  // SetStreamHandler be called after player_prepare,
  // because initialized event will be send in listen function of event channel
  auto handler = std::make_unique<
      flutter::StreamHandlerFunctions<flutter::EncodableValue>>(
      [&](const flutter::EncodableValue *arguments,
          std::unique_ptr<flutter::EventSink<>> &&events)
          -> std::unique_ptr<flutter::StreamHandlerError<>> {
        event_sink_ = std::move(events);
        Initialize();
        return nullptr;
      },
      [&](const flutter::EncodableValue *arguments)
          -> std::unique_ptr<flutter::StreamHandlerError<>> {
        event_sink_ = nullptr;
        return nullptr;
      });
  channel->SetStreamHandler(std::move(handler));

  event_channel_ = std::move(channel);
}

void VideoPlayer::Initialize() {
  player_state_e state;
  int ret = player_get_state(player_, &state);
  if (ret == PLAYER_ERROR_NONE) {
    LOG_INFO("[VideoPlayer] Player state: %s", StateToString(state).c_str());
    if (state == PLAYER_STATE_READY && !is_initialized_) {
      SendInitialized();
    }
  } else {
    LOG_ERROR("[VideoPlayer] player_get_state failed: %s",
              get_error_message(ret));
  }
}

void VideoPlayer::SendInitialized() {
  if (!is_initialized_ && event_sink_) {
    int duration;
    int ret = player_get_duration(player_, &duration);
    if (ret != PLAYER_ERROR_NONE) {
      event_sink_->Error("player_get_duration failed", get_error_message(ret));
      return;
    }
    LOG_DEBUG("[VideoPlayer] video duration: %d", duration);

    int width, height;
    ret = player_get_video_size(player_, &width, &height);
    if (ret != PLAYER_ERROR_NONE) {
      event_sink_->Error("player_get_video_size failed",
                         get_error_message(ret));
      return;
    }
    LOG_DEBUG("[VideoPlayer] video width: %d, height: %d", width, height);

    player_display_rotation_e rotation;
    ret = player_get_display_rotation(player_, &rotation);
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR(
          "[VideoPlayer] player_get_display_rotation "
          "failed: %s",
          get_error_message(ret));
    } else {
      LOG_DEBUG("[VideoPlayer] rotation: %s",
                RotationToString(rotation).c_str());
      if (rotation == PLAYER_DISPLAY_ROTATION_90 ||
          rotation == PLAYER_DISPLAY_ROTATION_270) {
        std::swap(width, height);
      }
    }

    is_initialized_ = true;
    flutter::EncodableMap result = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("initialized")},
        {flutter::EncodableValue("duration"),
         flutter::EncodableValue(duration)},
        {flutter::EncodableValue("width"), flutter::EncodableValue(width)},
        {flutter::EncodableValue("height"), flutter::EncodableValue(height)}};
    event_sink_->Success(flutter::EncodableValue(result));
  }
}

void VideoPlayer::OnPrepared(void *data) {
  auto *player = reinterpret_cast<VideoPlayer *>(data);
  LOG_DEBUG("[VideoPlayer] player prepared");

  if (!player->is_initialized_) {
    player->SendInitialized();
  }
}

void VideoPlayer::OnBuffering(int percent, void *data) {
  LOG_DEBUG("[VideoPlayer] percent: %d", percent);
}

void VideoPlayer::OnSeekCompleted(void *data) {
  auto *player = reinterpret_cast<VideoPlayer *>(data);
  LOG_DEBUG("[VideoPlayer] seek completed");

  if (player->on_seek_completed_) {
    player->on_seek_completed_();
    player->on_seek_completed_ = nullptr;
  }
}

void VideoPlayer::OnPlayCompleted(void *data) {
  auto *player = reinterpret_cast<VideoPlayer *>(data);
  LOG_DEBUG("[VideoPlayer] play completed");

  if (player->event_sink_) {
    flutter::EncodableMap result = {{flutter::EncodableValue("event"),
                                     flutter::EncodableValue("completed")}};
    player->event_sink_->Success(flutter::EncodableValue(result));
  }

  player->Pause();
}

void VideoPlayer::OnInterrupted(player_interrupted_code_e code, void *data) {
  auto *player = reinterpret_cast<VideoPlayer *>(data);
  LOG_DEBUG("[VideoPlayer] interrupt code: %d", code);

  if (player->event_sink_) {
    player->event_sink_->Error("Interrupted error",
                               "Video player has been interrupted.");
  }
}

void VideoPlayer::OnError(int code, void *data) {
  auto *player = reinterpret_cast<VideoPlayer *>(data);
  LOG_DEBUG("[VideoPlayer] error code: %d", code);

  if (player->event_sink_) {
    player->event_sink_->Error("Player error", get_error_message(code));
  }
}

void VideoPlayer::OnVideoFrameDecoded(media_packet_h packet, void *data) {
  auto *player = reinterpret_cast<VideoPlayer *>(data);
  std::lock_guard<std::mutex> lock(player->mutex_);
  player->SendMessage(kMessageOnFrameDecoded, packet);
}

void VideoPlayer::RunMediaPacketLoop(void *data, Ecore_Thread *thread) {
  auto *self = reinterpret_cast<VideoPlayer *>(data);
  Eina_Thread_Queue *packet_thread_queue = eina_thread_queue_new();
  if (!packet_thread_queue) {
    LOG_ERROR("Invalid packet queue.");
    ecore_thread_cancel(thread);
    return;
  }

  self->packet_thread_queue_ = packet_thread_queue;
  std::queue<media_packet_h> packet_queue;
  while (!ecore_thread_check(thread)) {
    void *ref;
    Message *message = static_cast<Message *>(
        eina_thread_queue_wait(packet_thread_queue, &ref));
    if (message->event == kMessageQuit) {
      LOG_ERROR("Message quit.");
      eina_thread_queue_wait_done(packet_thread_queue, ref);
      break;
    }

    if (message->event == kMessageOnFrameDecoded) {
      if (message->media_packet != nullptr) {
        packet_queue.push(message->media_packet);
      }
    }

    eina_thread_queue_wait_done(packet_thread_queue, ref);
    std::lock_guard<std::mutex> lock(self->mutex_);
    if (packet_queue.empty() || self->is_rendering_) {
      continue;
    }
    if (self->texture_registrar_->MarkTextureFrameAvailable(
            self->texture_id_)) {
      self->is_rendering_ = true;
      self->previous_media_packet_ = self->current_media_packet_;
      self->current_media_packet_ = packet_queue.front();
      packet_queue.pop();
    }
  }

  while (!packet_queue.empty()) {
    media_packet_destroy(packet_queue.front());
    packet_queue.pop();
  }

  if (packet_thread_queue) {
    eina_thread_queue_free(packet_thread_queue);
  }
}

void VideoPlayer::SendMessage(int event, media_packet_h media_packet) {
  if (!packet_thread_ || ecore_thread_check(packet_thread_)) {
    LOG_ERROR("Invalid packet thread.");
    return;
  }

  if (!packet_thread_queue_) {
    LOG_ERROR("Invalid packet thread queue.");
    return;
  }

  void *ref;
  Message *message = static_cast<Message *>(
      eina_thread_queue_send(packet_thread_queue_, sizeof(Message), &ref));
  message->event = event;
  message->media_packet = media_packet;
  eina_thread_queue_send_done(packet_thread_queue_, ref);
}

void VideoPlayer::SendRenderFinishedMessage() {
  if (previous_media_packet_) {
    media_packet_destroy(previous_media_packet_);
    previous_media_packet_ = nullptr;
  }
  SendMessage(kMessageOnRenderFinished, nullptr);
}
