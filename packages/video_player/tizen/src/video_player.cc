#include "video_player.h"

#include <flutter/event_stream_handler_functions.h>
#include <flutter/standard_method_codec.h>

#include <functional>

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
  VideoPlayer *player = (VideoPlayer *)data;
  std::lock_guard<std::mutex> lock(player->mutex_);
  if (player->current_media_packet_) {
    media_packet_destroy(player->current_media_packet_);
    player->current_media_packet_ = nullptr;
  }
}

FlutterDesktopGpuBuffer *VideoPlayer::ObtainGpuBuffer(size_t width,
                                                      size_t height) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (!current_media_packet_) {
    LOG_ERROR("[VideoPlayer] No vaild media packet.");
    return nullptr;
  }
  tbm_surface_h surface;
  int ret = media_packet_get_tbm_surface(current_media_packet_, &surface);
  if (ret != MEDIA_PACKET_ERROR_NONE || surface == nullptr) {
    LOG_ERROR("[VideoPlayer] Failed to get a TBM surface, error: %d", ret);
    media_packet_destroy(current_media_packet_);
    current_media_packet_ = nullptr;
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

  ret = player_set_media_packet_video_frame_decoded_cb(
      player_, OnVideoFrameDecoded, (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    throw VideoPlayerError(
        "player_set_media_packet_video_frame_decoded_cb failed",
        get_error_message(ret));
  }

  ret = player_set_buffering_cb(player_, OnBuffering, (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    throw VideoPlayerError("player_set_buffering_cb failed",
                           get_error_message(ret));
  }

  ret = player_set_completed_cb(player_, OnPlayCompleted, (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    throw VideoPlayerError("player_set_completed_cb failed",
                           get_error_message(ret));
  }

  ret = player_set_interrupted_cb(player_, OnInterrupted, (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    throw VideoPlayerError("player_set_interrupted_cb failed",
                           get_error_message(ret));
  }

  ret = player_set_error_cb(player_, OnErrorOccurred, (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    throw VideoPlayerError("player_set_error_cb failed",
                           get_error_message(ret));
  }

  ret = player_prepare_async(player_, OnPrepared, (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    throw VideoPlayerError("player_prepare_async failed",
                           get_error_message(ret));
  }

  SetupEventChannel(plugin_registrar->messenger());
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

  if (current_media_packet_) {
    media_packet_destroy(current_media_packet_);
    current_media_packet_ = nullptr;
  }

  if (texture_registrar_) {
    texture_registrar_->UnregisterTexture(texture_id_);
    texture_registrar_ = nullptr;
  }
}

void VideoPlayer::SetupEventChannel(flutter::BinaryMessenger *messenger) {
  LOG_DEBUG("[VideoPlayer] setup event channel");

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
          std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> &&events)
          -> std::unique_ptr<
              flutter::StreamHandlerError<flutter::EncodableValue>> {
        LOG_DEBUG("[VideoPlayer] call listen of StreamHandler");
        event_sink_ = std::move(events);
        Initialize();
        return nullptr;
      },
      [&](const flutter::EncodableValue *arguments)
          -> std::unique_ptr<
              flutter::StreamHandlerError<flutter::EncodableValue>> {
        LOG_DEBUG("[VideoPlayer] call cancel of StreamHandler");
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
  if (!is_initialized_ && event_sink_ != nullptr) {
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
        int tmp = width;
        width = height;
        height = tmp;
      }
    }

    is_initialized_ = true;
    flutter::EncodableMap encodables = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("initialized")},
        {flutter::EncodableValue("duration"),
         flutter::EncodableValue(duration)},
        {flutter::EncodableValue("width"), flutter::EncodableValue(width)},
        {flutter::EncodableValue("height"), flutter::EncodableValue(height)}};
    flutter::EncodableValue eventValue(encodables);
    event_sink_->Success(eventValue);
  }
}

void VideoPlayer::SendBufferingStart() {
  if (event_sink_) {
    flutter::EncodableMap encodables = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("bufferingStart")}};
    flutter::EncodableValue eventValue(encodables);
    event_sink_->Success(eventValue);
  }
}

void VideoPlayer::SendBufferingUpdate(int position) {
  if (event_sink_) {
    flutter::EncodableList range = {flutter::EncodableValue(0),
                                    flutter::EncodableValue(position)};
    flutter::EncodableList rangeList = {flutter::EncodableValue(range)};
    flutter::EncodableMap encodables = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("bufferingUpdate")},
        {flutter::EncodableValue("values"),
         flutter::EncodableValue(rangeList)}};
    flutter::EncodableValue eventValue(encodables);
    event_sink_->Success(eventValue);
  }
}

void VideoPlayer::SendBufferingEnd() {
  if (event_sink_) {
    flutter::EncodableMap encodables = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("bufferingEnd")}};
    flutter::EncodableValue eventValue(encodables);
    event_sink_->Success(eventValue);
  }
}

void VideoPlayer::OnPrepared(void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_DEBUG("[VideoPlayer] player prepared");

  if (!player->is_initialized_) {
    player->SendInitialized();
  }
}

void VideoPlayer::OnBuffering(int percent, void *data) {
  LOG_DEBUG("[VideoPlayer] buffering: %d%", percent);
}

void VideoPlayer::OnSeekCompleted(void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_DEBUG("[VideoPlayer] seek completed");

  if (player->on_seek_completed_) {
    player->on_seek_completed_();
    player->on_seek_completed_ = nullptr;
  }
}

void VideoPlayer::OnPlayCompleted(void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_DEBUG("[VideoPlayer] play completed");

  if (player->event_sink_) {
    flutter::EncodableMap encodables = {{flutter::EncodableValue("event"),
                                         flutter::EncodableValue("completed")}};
    flutter::EncodableValue eventValue(encodables);
    player->event_sink_->Success(eventValue);
  }

  player->Pause();
}

void VideoPlayer::OnInterrupted(player_interrupted_code_e code, void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_DEBUG("[VideoPlayer] interrupt code: %d", code);

  if (player->event_sink_) {
    player->event_sink_->Error("Interrupted error",
                               "Video player has been interrupted.");
  }
}

void VideoPlayer::OnErrorOccurred(int code, void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_DEBUG("[VideoPlayer] error code: %d", code);

  if (player->event_sink_) {
    player->event_sink_->Error("Player error", get_error_message(code));
  }
}

void VideoPlayer::OnVideoFrameDecoded(media_packet_h packet, void *data) {
  VideoPlayer *player = (VideoPlayer *)data;

  std::lock_guard<std::mutex> lock(player->mutex_);
  if (player->current_media_packet_) {
    LOG_INFO("[VideoPlayer] Media packet already pending.");
    media_packet_destroy(packet);
    return;
  }
  player->current_media_packet_ = packet;
  player->texture_registrar_->MarkTextureFrameAvailable(player->texture_id_);
}
