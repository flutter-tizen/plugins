#include "video_player.h"

#include <flutter/event_stream_handler_functions.h>
#include <flutter/standard_method_codec.h>

#include <functional>

#include "log.h"
#include "video_player_error.h"

static std::string RotationToString(player_display_rotation_e rotation) {
  std::string ret;
  switch (rotation) {
    case PLAYER_DISPLAY_ROTATION_NONE:
      ret = "PLAYER_DISPLAY_ROTATION_NONE";
      break;
    case PLAYER_DISPLAY_ROTATION_90:
      ret = "PLAYER_DISPLAY_ROTATION_90";
      break;
    case PLAYER_DISPLAY_ROTATION_180:
      ret = "PLAYER_DISPLAY_ROTATION_180";
      break;
    case PLAYER_DISPLAY_ROTATION_270:
      ret = "PLAYER_DISPLAY_ROTATION_270";
      break;
  }
  return ret;
}

static std::string StateToString(player_state_e state) {
  std::string ret;
  switch (state) {
    case PLAYER_STATE_NONE:
      ret = "PLAYER_STATE_NONE";
      break;
    case PLAYER_STATE_IDLE:
      ret = "PLAYER_STATE_IDLE";
      break;
    case PLAYER_STATE_READY:
      ret = "PLAYER_STATE_READY";
      break;
    case PLAYER_STATE_PLAYING:
      ret = "PLAYER_STATE_PLAYING";
      break;
    case PLAYER_STATE_PAUSED:
      ret = "PLAYER_STATE_PAUSED";
      break;
  }
  return ret;
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
    LOG_ERROR("No vaild media packet");
    return nullptr;
  }
  tbm_surface_h surface;
  int ret = media_packet_get_tbm_surface(current_media_packet_, &surface);
  if (ret != MEDIA_PACKET_ERROR_NONE || surface == nullptr) {
    LOG_ERROR("get tbm surface failed, error: %d", ret);
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

  LOG_INFO("[VideoPlayer] register texture");
  texture_id_ = texture_registrar->RegisterTexture(texture_variant_.get());

  LOG_DEBUG("[VideoPlayer] call player_create to create player");
  int ret = player_create(&player_);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_create failed: %s", get_error_message(ret));
    throw VideoPlayerError("player_create failed", get_error_message(ret));
  }

  LOG_DEBUG("[VideoPlayer] call player_set_uri to set video path (%s)",
            uri.c_str());
  ret = player_set_uri(player_, uri.c_str());
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_set_uri failed: %s",
              get_error_message(ret));
    throw VideoPlayerError("player_set_uri failed", get_error_message(ret));
  }

  LOG_DEBUG("[VideoPlayer] call player_set_display_visible");
  ret = player_set_display_visible(player_, true);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_set_display_visible failed: %s",
              get_error_message(ret));
    throw VideoPlayerError("player_set_display_visible failed",
                           get_error_message(ret));
  }

  LOG_DEBUG(
      "[VideoPlayer] call player_set_media_packet_video_frame_decoded_cb");
  ret = player_set_media_packet_video_frame_decoded_cb(
      player_, onVideoFrameDecoded, (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR(
        "[VideoPlayer] player_set_media_packet_video_frame_decoded_cb "
        "failed: %s",
        get_error_message(ret));
    throw VideoPlayerError(
        "player_set_media_packet_video_frame_decoded_cb failed",
        get_error_message(ret));
  }

  LOG_DEBUG("[VideoPlayer] call player_set_buffering_cb");
  ret = player_set_buffering_cb(player_, onBuffering, (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_set_buffering_cb failed: %s",
              get_error_message(ret));
    throw VideoPlayerError("player_set_buffering_cb failed",
                           get_error_message(ret));
  }

  LOG_DEBUG("[VideoPlayer] call player_set_completed_cb");
  ret = player_set_completed_cb(player_, onPlayCompleted, (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_set_completed_cb failed: %s",
              get_error_message(ret));
    throw VideoPlayerError("player_set_completed_cb failed",
                           get_error_message(ret));
  }

  LOG_DEBUG("[VideoPlayer] call player_set_interrupted_cb");
  ret = player_set_interrupted_cb(player_, onInterrupted, (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_set_interrupted_cb failed: %s",
              get_error_message(ret));
    throw VideoPlayerError("player_set_interrupted_cb failed",
                           get_error_message(ret));
  }

  LOG_DEBUG("[VideoPlayer] call player_set_error_cb");
  ret = player_set_error_cb(player_, onErrorOccurred, (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_set_error_cb failed: %s",
              get_error_message(ret));
    throw VideoPlayerError("player_set_error_cb failed",
                           get_error_message(ret));
  }

  LOG_DEBUG("[VideoPlayer] call player_prepare_async");
  ret = player_prepare_async(player_, onPrepared, (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_prepare_async failed: %s",
              get_error_message(ret));
    throw VideoPlayerError("player_prepare_async failed",
                           get_error_message(ret));
  }

  setupEventChannel(plugin_registrar->messenger());
}

VideoPlayer::~VideoPlayer() {
  LOG_INFO("[VideoPlayer] destructor");
  dispose();
}

long VideoPlayer::getTextureId() { return texture_id_; }

void VideoPlayer::play() {
  LOG_DEBUG("[VideoPlayer.play] start player");
  player_state_e state;
  int ret = player_get_state(player_, &state);
  if (ret == PLAYER_ERROR_NONE) {
    LOG_INFO("[VideoPlayer.play] player state: %s",
             StateToString(state).c_str());
    if (state != PLAYER_STATE_PAUSED && state != PLAYER_STATE_READY) {
      return;
    }
  }

  ret = player_start(player_);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer.play] player_start failed: %s",
              get_error_message(ret));
    throw VideoPlayerError("player_start failed", get_error_message(ret));
  }
}

void VideoPlayer::pause() {
  LOG_DEBUG("[VideoPlayer.pause] pause player");
  player_state_e state;
  int ret = player_get_state(player_, &state);
  if (ret == PLAYER_ERROR_NONE) {
    LOG_INFO("[VideoPlayer.pause] player state: %s",
             StateToString(state).c_str());
    if (state != PLAYER_STATE_PLAYING) {
      return;
    }
  }

  ret = player_pause(player_);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer.pause] player_pause failed: %s",
              get_error_message(ret));
    throw VideoPlayerError("player_pause failed", get_error_message(ret));
  }
}

void VideoPlayer::setLooping(bool is_looping) {
  LOG_DEBUG("[VideoPlayer.setLooping] isLooping: %d", is_looping);
  int ret = player_set_looping(player_, is_looping);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer.setLooping] player_set_looping failed: %s",
              get_error_message(ret));
    throw VideoPlayerError("player_set_looping failed", get_error_message(ret));
  }
}

void VideoPlayer::setVolume(double volume) {
  LOG_DEBUG("[VideoPlayer.setVolume] volume: %f", volume);
  int ret = player_set_volume(player_, volume, volume);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer.setVolume] player_set_volume failed: %s",
              get_error_message(ret));
    throw VideoPlayerError("player_set_volume failed", get_error_message(ret));
  }
}

void VideoPlayer::setPlaybackSpeed(double speed) {
  LOG_DEBUG("[VideoPlayer.setPlaybackSpeed] speed: %f", speed);
  int ret = player_set_playback_rate(player_, speed);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR(
        "[VideoPlayer.setPlaybackSpeed] player_set_playback_rate failed: %s",
        get_error_message(ret));
    throw VideoPlayerError("player_set_playback_rate failed",
                           get_error_message(ret));
  }
}

void VideoPlayer::seekTo(int position,
                         const SeekCompletedCb &seek_completed_cb) {
  LOG_DEBUG("[VideoPlayer.seekTo] position: %d", position);
  on_seek_completed_ = seek_completed_cb;
  int ret =
      player_set_play_position(player_, position, true, onSeekCompleted, this);
  if (ret != PLAYER_ERROR_NONE) {
    on_seek_completed_ = nullptr;
    LOG_ERROR("[VideoPlayer.seekTo] player_set_play_position failed: %s",
              get_error_message(ret));
    throw VideoPlayerError("player_set_play_position failed",
                           get_error_message(ret));
  }
}

int VideoPlayer::getPosition() {
  LOG_DEBUG("[VideoPlayer.getPosition] get video player position");
  int position;
  int ret = player_get_play_position(player_, &position);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer.getPosition] player_get_play_position failed: %s",
              get_error_message(ret));
    throw VideoPlayerError("player_get_play_position failed",
                           get_error_message(ret));
  }

  LOG_DEBUG("[VideoPlayer.getPosition] position: %d", position);
  return position;
}

void VideoPlayer::dispose() {
  LOG_DEBUG("[VideoPlayer.dispose] dispose video player");
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

void VideoPlayer::setupEventChannel(flutter::BinaryMessenger *messenger) {
  LOG_DEBUG("[VideoPlayer.setupEventChannel] setup event channel");
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
        LOG_DEBUG(
            "[VideoPlayer.setupEventChannel] call listen of StreamHandler");
        event_sink_ = std::move(events);
        initialize();
        return nullptr;
      },
      [&](const flutter::EncodableValue *arguments)
          -> std::unique_ptr<
              flutter::StreamHandlerError<flutter::EncodableValue>> {
        LOG_DEBUG(
            "[VideoPlayer.setupEventChannel] call cancel of StreamHandler");
        event_sink_ = nullptr;
        return nullptr;
      });
  channel->SetStreamHandler(std::move(handler));
  event_channel_ = std::move(channel);
}

void VideoPlayer::initialize() {
  player_state_e state;
  int ret = player_get_state(player_, &state);
  if (ret == PLAYER_ERROR_NONE) {
    LOG_INFO("[VideoPlayer.initialize] player state: %s",
             StateToString(state).c_str());
    if (state == PLAYER_STATE_READY && !is_initialized_) {
      sendInitialized();
    }
  } else {
    LOG_ERROR("[VideoPlayer.initialize] player_get_state failed: %s",
              get_error_message(ret));
  }
}

void VideoPlayer::sendInitialized() {
  if (!is_initialized_ && event_sink_ != nullptr) {
    int duration;
    int ret = player_get_duration(player_, &duration);
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR("[VideoPlayer.sendInitialized] player_get_duration failed: %s",
                get_error_message(ret));
      event_sink_->Error("player_get_duration failed", get_error_message(ret));
      return;
    }
    LOG_DEBUG("[VideoPlayer.sendInitialized] video duration: %d", duration);

    int width, height;
    ret = player_get_video_size(player_, &width, &height);
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR(
          "[VideoPlayer.sendInitialized] player_get_video_size failed: %s",
          get_error_message(ret));
      event_sink_->Error("player_get_video_size failed",
                         get_error_message(ret));
      return;
    }
    LOG_DEBUG("[VideoPlayer.sendInitialized] video width: %d, height: %d",
              width, height);

    player_display_rotation_e rotation;
    ret = player_get_display_rotation(player_, &rotation);
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR(
          "[VideoPlayer.sendInitialized] player_get_display_rotation "
          "failed: %s",
          get_error_message(ret));
    } else {
      LOG_DEBUG("[VideoPlayer.sendInitialized] rotation: %s",
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
    LOG_INFO("[VideoPlayer.sendInitialized] send initialized event");
    event_sink_->Success(eventValue);
  }
}

void VideoPlayer::sendBufferingStart() {
  if (event_sink_) {
    flutter::EncodableMap encodables = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("bufferingStart")}};
    flutter::EncodableValue eventValue(encodables);
    LOG_INFO("[VideoPlayer.onBuffering] send bufferingStart event");
    event_sink_->Success(eventValue);
  }
}

void VideoPlayer::sendBufferingUpdate(int position) {
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
    LOG_INFO("[VideoPlayer.onBuffering] send bufferingUpdate event");
    event_sink_->Success(eventValue);
  }
}

void VideoPlayer::sendBufferingEnd() {
  if (event_sink_) {
    flutter::EncodableMap encodables = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("bufferingEnd")}};
    flutter::EncodableValue eventValue(encodables);
    LOG_INFO("[VideoPlayer.onBuffering] send bufferingEnd event");
    event_sink_->Success(eventValue);
  }
}

void VideoPlayer::onPrepared(void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_DEBUG("[VideoPlayer.onPrepared] video player is prepared");

  if (!player->is_initialized_) {
    player->sendInitialized();
  }
}

void VideoPlayer::onBuffering(int percent, void *data) {
  // percent isn't used for video size, it's the used storage of buffer
  LOG_DEBUG("[VideoPlayer.onBuffering] percent: %d", percent);
}

void VideoPlayer::onSeekCompleted(void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_DEBUG("[VideoPlayer.onSeekCompleted] completed to seek");

  if (player->on_seek_completed_) {
    player->on_seek_completed_();
    player->on_seek_completed_ = nullptr;
  }
}

void VideoPlayer::onPlayCompleted(void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_DEBUG("[VideoPlayer.onPlayCompleted] completed to playe video");

  if (player->event_sink_) {
    flutter::EncodableMap encodables = {{flutter::EncodableValue("event"),
                                         flutter::EncodableValue("completed")}};
    flutter::EncodableValue eventValue(encodables);
    LOG_INFO("[VideoPlayer.onPlayCompleted] send completed event");
    player->event_sink_->Success(eventValue);

    LOG_DEBUG("[VideoPlayer.onPlayCompleted] change player state to pause");
    player->pause();
  }
}

void VideoPlayer::onInterrupted(player_interrupted_code_e code, void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_DEBUG("[VideoPlayer.onInterrupted] interrupted code: %d", code);

  if (player->event_sink_) {
    LOG_INFO("[VideoPlayer.onInterrupted] send error event");
    player->event_sink_->Error("Interrupted error",
                               "Video player has been interrupted.");
  }
}

void VideoPlayer::onErrorOccurred(int code, void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_DEBUG("[VideoPlayer.onErrorOccurred] error code: %s",
            get_error_message(code));

  if (player->event_sink_) {
    LOG_INFO("[VideoPlayer.onErrorOccurred] send error event");
    player->event_sink_->Error("Player error", get_error_message(code));
  }
}

void VideoPlayer::onVideoFrameDecoded(media_packet_h packet, void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  std::lock_guard<std::mutex> lock(player->mutex_);
  if (player->current_media_packet_) {
    LOG_INFO("current media packet not null");
    media_packet_destroy(packet);
    return;
  }
  player->current_media_packet_ = packet;
  player->texture_registrar_->MarkTextureFrameAvailable(player->texture_id_);
}
