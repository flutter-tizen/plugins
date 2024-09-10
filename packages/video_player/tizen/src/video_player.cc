// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player.h"

#include <dlfcn.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/standard_method_codec.h>

#include <algorithm>

#include "log.h"
#include "video_player_error.h"

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
  auto *player = static_cast<VideoPlayer *>(data);

  std::lock_guard<std::mutex> lock(player->mutex_);
  player->is_rendering_ = false;
  player->OnRenderingCompleted();
}

FlutterDesktopGpuSurfaceDescriptor *VideoPlayer::ObtainGpuSurface(
    size_t width, size_t height) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!current_media_packet_) {
    LOG_ERROR("[VideoPlayer] current media packet not valid.");
    is_rendering_ = false;
    OnRenderingCompleted();
    return nullptr;
  }

  tbm_surface_h surface;
  int ret = media_packet_get_tbm_surface(current_media_packet_, &surface);
  if (ret != MEDIA_PACKET_ERROR_NONE || !surface) {
    LOG_ERROR("[VideoPlayer] Failed to get a tbm surface, error: %d", ret);
    is_rendering_ = false;
    media_packet_destroy(current_media_packet_);
    current_media_packet_ = nullptr;
    OnRenderingCompleted();
    return nullptr;
  }
  gpu_surface_->handle = surface;
  gpu_surface_->width = width;
  gpu_surface_->height = height;
  gpu_surface_->release_context = this;
  gpu_surface_->release_callback = ReleaseMediaPacket;
  return gpu_surface_.get();
}

void VideoPlayer::InitScreenSaverApi() {
  screensaver_handle_ = dlopen("libcapi-screensaver.so", RTLD_LAZY);
  if (!screensaver_handle_) {
    LOG_ERROR("[VideoPlayer] dlopen failed: %s", dlerror());
    return;
  }

  screensaver_reset_timeout_ = reinterpret_cast<ScreensaverResetTimeout>(
      dlsym(screensaver_handle_, "screensaver_reset_timeout"));
  if (!screensaver_reset_timeout_) {
    LOG_ERROR("[VideoPlayer] Symbol not found: %s", dlerror());
    return;
  }

  ScreensaverOverrideReset screensaver_override_reset =
      reinterpret_cast<ScreensaverOverrideReset>(
          dlsym(screensaver_handle_, "screensaver_override_reset"));
  if (!screensaver_override_reset) {
    LOG_ERROR("[VideoPlayer] Symbol not found: %s", dlerror());
    return;
  }

  int ret = screensaver_override_reset(false);
  if (ret != 0) {
    LOG_ERROR("screensaver_override_reset failed: %s", get_error_message(ret));
    return;
  }
}

VideoPlayer::VideoPlayer(flutter::PluginRegistrar *plugin_registrar,
                         flutter::TextureRegistrar *texture_registrar,
                         const std::string &uri, VideoPlayerOptions &options) {
  sink_event_pipe_ = ecore_pipe_add(
      [](void *data, void *buffer, unsigned int nbyte) -> void {
        auto *self = static_cast<VideoPlayer *>(data);
        self->SendPendingEvents();
      },
      this);

  texture_registrar_ = texture_registrar;

  texture_variant_ =
      std::make_unique<flutter::TextureVariant>(flutter::GpuSurfaceTexture(
          kFlutterDesktopGpuSurfaceTypeNone,
          [this](size_t width,
                 size_t height) -> const FlutterDesktopGpuSurfaceDescriptor * {
            return this->ObtainGpuSurface(width, height);
          }));
  gpu_surface_ = std::make_unique<FlutterDesktopGpuSurfaceDescriptor>();
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

#ifdef TV_PROFILE
  InitScreenSaverApi();
#endif
  SetUpEventChannel(plugin_registrar->messenger());
}

VideoPlayer::~VideoPlayer() {
  if (player_) {
    player_unset_media_packet_video_frame_decoded_cb(player_);
    player_unset_buffering_cb(player_);
    player_unset_completed_cb(player_);
    player_unset_interrupted_cb(player_);
    player_unset_error_cb(player_);
    player_unprepare(player_);
    player_stop(player_);
    player_destroy(player_);
    player_ = nullptr;
  }
}

void VideoPlayer::SendPendingEvents() {
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

void VideoPlayer::PushEvent(const flutter::EncodableValue &encodable_value) {
  if (!event_sink_) {
    LOG_ERROR("[VideoPlayer] event sink is nullptr.");
    return;
  }
  std::lock_guard<std::mutex> lock(queue_mutex_);
  encodable_event_queue_.push(encodable_value);
  ecore_pipe_write(sink_event_pipe_, nullptr, 0);
}

void VideoPlayer::SendError(const std::string &error_code,
                            const std::string &error_message) {
  if (!event_sink_) {
    LOG_ERROR("[VideoPlayer] event sink is nullptr.");
    return;
  }
  std::lock_guard<std::mutex> lock(queue_mutex_);
  error_event_queue_.push(std::make_pair(error_code, error_message));
  ecore_pipe_write(sink_event_pipe_, nullptr, 0);
}

void VideoPlayer::Play() {
  LOG_DEBUG("[VideoPlayer] Player starting.");

  player_state_e state = PLAYER_STATE_NONE;
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
#ifdef TV_PROFILE
  timer_ = ecore_timer_add(30, ResetScreensaverTimeout, this);
#endif

  SendIsPlayingStateUpdate(true);
}

void VideoPlayer::Pause() {
  LOG_DEBUG("[VideoPlayer] Player pausing.");

  player_state_e state = PLAYER_STATE_NONE;
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

  if (timer_) {
    LOG_DEBUG("[VideoPlayer] Delete ecore timer.");
    ecore_timer_del(timer_);
    timer_ = nullptr;
  }

  SendIsPlayingStateUpdate(false);
}

void VideoPlayer::SetLooping(bool is_looping) {
  LOG_DEBUG("[VideoPlayer] is_looping: %d", is_looping);

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

void VideoPlayer::SeekTo(int32_t position, SeekCompletedCallback callback) {
  LOG_DEBUG("[VideoPlayer] position: %d", position);

  on_seek_completed_ = std::move(callback);
  int ret =
      player_set_play_position(player_, position, true, OnSeekCompleted, this);
  if (ret != PLAYER_ERROR_NONE) {
    on_seek_completed_ = nullptr;
    throw VideoPlayerError("player_set_play_position failed",
                           get_error_message(ret));
  }
}

int32_t VideoPlayer::GetPosition() {
  int position = 0;
  int ret = player_get_play_position(player_, &position);
  if (ret != PLAYER_ERROR_NONE) {
    throw VideoPlayerError("player_get_play_position failed",
                           get_error_message(ret));
  }
  return position;
}

void VideoPlayer::Dispose() {
  LOG_DEBUG("[VideoPlayer] Player disposing.");

  std::lock_guard<std::mutex> lock(mutex_);
  is_initialized_ = false;

  if (sink_event_pipe_) {
    ecore_pipe_del(sink_event_pipe_);
  }

  event_sink_ = nullptr;
  event_channel_->SetStreamHandler(nullptr);

  while (!packet_queue_.empty()) {
    media_packet_destroy(packet_queue_.front());
    packet_queue_.pop();
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
    texture_registrar_->UnregisterTexture(texture_id_, nullptr);
    texture_registrar_ = nullptr;
  }

  if (screensaver_handle_) {
    dlclose(screensaver_handle_);
    screensaver_handle_ = nullptr;
  }

  if (timer_) {
    ecore_timer_del(timer_);
    timer_ = nullptr;
  }
}

void VideoPlayer::SetUpEventChannel(flutter::BinaryMessenger *messenger) {
  std::string channel_name =
      "flutter.io/videoPlayer/videoEvents" + std::to_string(texture_id_);
  auto channel =
      std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
          messenger, channel_name,
          &flutter::StandardMethodCodec::GetInstance());
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
  player_state_e state = PLAYER_STATE_NONE;
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
    int duration = 0;
    int ret = player_get_duration(player_, &duration);
    if (ret != PLAYER_ERROR_NONE) {
      SendError("player_get_duration failed", get_error_message(ret));
      return;
    }
    LOG_DEBUG("[VideoPlayer] Video duration: %d", duration);

    int width = 0, height = 0;
    ret = player_get_video_size(player_, &width, &height);
    if (ret != PLAYER_ERROR_NONE) {
      SendError("player_get_video_size failed", get_error_message(ret));
      return;
    }
    LOG_DEBUG("[VideoPlayer] Video width: %d, height: %d", width, height);

    player_display_rotation_e rotation = PLAYER_DISPLAY_ROTATION_NONE;
    ret = player_get_display_rotation(player_, &rotation);
    if (ret != PLAYER_ERROR_NONE) {
      SendError("player_get_display_rotation failed", get_error_message(ret));
    } else {
      LOG_DEBUG("[VideoPlayer] rotation: %s",
                RotationToString(rotation).c_str());
      if (rotation == PLAYER_DISPLAY_ROTATION_90 ||
          rotation == PLAYER_DISPLAY_ROTATION_270) {
        std::swap(width, height);
      }
    }

    // TODO(jsuya):Some streaming resources may have a duration of 0 during the
    // initialization step. If the duration is 0, it may affect the progress of
    // video_player and cause unnecessary errors. Therefore, set it to 1
    // temporarily. the future. In the future, It can be updated depending on
    // the loading status.
    if (width != 0 && height != 0 && duration == 0) {
      duration = 1;
    }

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

void VideoPlayer::SendIsPlayingStateUpdate(bool is_playing) {
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"),
       flutter::EncodableValue("isPlayingStateUpdate")},
      {flutter::EncodableValue("isPlaying"),
       flutter::EncodableValue(is_playing)},
  };
  PushEvent(flutter::EncodableValue(result));
}

Eina_Bool VideoPlayer::ResetScreensaverTimeout(void *data) {
  LOG_DEBUG("[VideoPlayer] Reset screen saver timeout.");

  auto *player = static_cast<VideoPlayer *>(data);
  if (!player->screensaver_reset_timeout_) {
    return ECORE_CALLBACK_CANCEL;
  }
  int ret = player->screensaver_reset_timeout_();
  if (ret != 0) {
    LOG_ERROR("screensaver_reset_timeout failed: %s", get_error_message(ret));
    return ECORE_CALLBACK_CANCEL;
  }

  return ECORE_CALLBACK_RENEW;
}

void VideoPlayer::OnPrepared(void *data) {
  LOG_DEBUG("[VideoPlayer] Player prepared.");

  auto *player = static_cast<VideoPlayer *>(data);
  if (!player->is_initialized_) {
    player->SendInitialized();
  }
}

void VideoPlayer::OnBuffering(int percent, void *data) {
  LOG_DEBUG("[VideoPlayer] percent: %d", percent);
}

void VideoPlayer::OnSeekCompleted(void *data) {
  LOG_DEBUG("[VideoPlayer] Seek completed.");

  auto *player = static_cast<VideoPlayer *>(data);
  if (player->on_seek_completed_) {
    player->on_seek_completed_();
    player->on_seek_completed_ = nullptr;
  }
}

void VideoPlayer::OnPlayCompleted(void *data) {
  LOG_DEBUG("[VideoPlayer] Play completed.");

  auto *player = static_cast<VideoPlayer *>(data);
  flutter::EncodableMap result = {
      {flutter::EncodableValue("event"), flutter::EncodableValue("completed")},
  };
  player->PushEvent(flutter::EncodableValue(result));

  player->Pause();
}

void VideoPlayer::OnInterrupted(player_interrupted_code_e code, void *data) {
  LOG_ERROR("[VideoPlayer] Interrupt code: %d", code);

  auto *player = static_cast<VideoPlayer *>(data);
  player->SendError("Interrupted error", "Video player has been interrupted.");
}

void VideoPlayer::OnError(int error_code, void *data) {
  LOG_ERROR("[VideoPlayer] Error code: %d (%s)", error_code,
            get_error_message(error_code));

  auto *player = static_cast<VideoPlayer *>(data);
  player->SendError("Player error",
                    std::string("Error: ") + get_error_message(error_code));
}

void VideoPlayer::OnVideoFrameDecoded(media_packet_h packet, void *data) {
  auto *player = static_cast<VideoPlayer *>(data);
  std::lock_guard<std::mutex> lock(player->mutex_);
  if (!player->is_initialized_) {
    LOG_INFO("[VideoPlayer] player not initialized.");
    media_packet_destroy(packet);
    return;
  }
  player->packet_queue_.push(packet);
  player->RequestRendering();
}

void VideoPlayer::RequestRendering() {
  if (packet_queue_.empty() || is_rendering_) {
    return;
  }
  if (texture_registrar_->MarkTextureFrameAvailable(texture_id_)) {
    is_rendering_ = true;
    previous_media_packet_ = current_media_packet_;
    current_media_packet_ = packet_queue_.front();
    packet_queue_.pop();
    while (!packet_queue_.empty()) {
      media_packet_destroy(packet_queue_.front());
      packet_queue_.pop();
    }
  }
}

void VideoPlayer::OnRenderingCompleted() {
  if (previous_media_packet_) {
    media_packet_destroy(previous_media_packet_);
    previous_media_packet_ = nullptr;
  }
  RequestRendering();
}
