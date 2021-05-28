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

VideoPlayer::VideoPlayer(flutter::PluginRegistrar *pluginRegistrar,
                         FlutterTextureRegistrar *textureRegistrar,
                         const std::string &uri, VideoPlayerOptions &options) {
  isInitialized_ = false;
  textureRegistrar_ = textureRegistrar;

  LOG_INFO("[VideoPlayer] register texture");
  textureId_ = FlutterRegisterExternalTexture(textureRegistrar_);

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

  setupEventChannel(pluginRegistrar->messenger());
}

VideoPlayer::~VideoPlayer() {
  LOG_INFO("[VideoPlayer] destructor");
  dispose();
}

long VideoPlayer::getTextureId() { return textureId_; }

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

void VideoPlayer::setLooping(bool isLooping) {
  LOG_DEBUG("[VideoPlayer.setLooping] isLooping: %d", isLooping);
  int ret = player_set_looping(player_, isLooping);
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

void VideoPlayer::seekTo(int position) {
  LOG_DEBUG("[VideoPlayer.seekTo] position: %d", position);
  int ret = player_set_play_position(player_, position, true, nullptr, nullptr);
  if (ret != PLAYER_ERROR_NONE) {
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
  isInitialized_ = false;
  eventSink_ = nullptr;
  eventChannel_->SetStreamHandler(nullptr);

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

  if (textureRegistrar_) {
    FlutterUnregisterExternalTexture(textureRegistrar_, textureId_);
    textureRegistrar_ = nullptr;
  }
}

void VideoPlayer::setupEventChannel(flutter::BinaryMessenger *messenger) {
  LOG_DEBUG("[VideoPlayer.setupEventChannel] setup event channel");
  std::string name =
      "flutter.io/videoPlayer/videoEvents" + std::to_string(textureId_);
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
        eventSink_ = std::move(events);
        initialize();
        return nullptr;
      },
      [&](const flutter::EncodableValue *arguments)
          -> std::unique_ptr<
              flutter::StreamHandlerError<flutter::EncodableValue>> {
        LOG_DEBUG(
            "[VideoPlayer.setupEventChannel] call cancel of StreamHandler");
        eventSink_ = nullptr;
        return nullptr;
      });
  channel->SetStreamHandler(std::move(handler));
  eventChannel_ = std::move(channel);
}

void VideoPlayer::initialize() {
  player_state_e state;
  int ret = player_get_state(player_, &state);
  if (ret == PLAYER_ERROR_NONE) {
    LOG_INFO("[VideoPlayer.initialize] player state: %s",
             StateToString(state).c_str());
    if (state == PLAYER_STATE_READY && !isInitialized_) {
      sendInitialized();
    }
  } else {
    LOG_ERROR("[VideoPlayer.initialize] player_get_state failed: %s",
              get_error_message(ret));
  }
}

void VideoPlayer::sendInitialized() {
  if (!isInitialized_ && eventSink_ != nullptr) {
    int duration;
    int ret = player_get_duration(player_, &duration);
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR("[VideoPlayer.sendInitialized] player_get_duration failed: %s",
                get_error_message(ret));
      eventSink_->Error("player_get_duration failed", get_error_message(ret));
      return;
    }
    LOG_DEBUG("[VideoPlayer.sendInitialized] video duration: %d", duration);

    int width, height;
    ret = player_get_video_size(player_, &width, &height);
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR(
          "[VideoPlayer.sendInitialized] player_get_video_size failed: %s",
          get_error_message(ret));
      eventSink_->Error("player_get_video_size failed", get_error_message(ret));
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

    isInitialized_ = true;
    flutter::EncodableMap encodables = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("initialized")},
        {flutter::EncodableValue("duration"),
         flutter::EncodableValue(duration)},
        {flutter::EncodableValue("width"), flutter::EncodableValue(width)},
        {flutter::EncodableValue("height"), flutter::EncodableValue(height)}};
    flutter::EncodableValue eventValue(encodables);
    LOG_INFO("[VideoPlayer.sendInitialized] send initialized event");
    eventSink_->Success(eventValue);
  }
}

void VideoPlayer::sendBufferingStart() {
  if (eventSink_) {
    flutter::EncodableMap encodables = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("bufferingStart")}};
    flutter::EncodableValue eventValue(encodables);
    LOG_INFO("[VideoPlayer.onBuffering] send bufferingStart event");
    eventSink_->Success(eventValue);
  }
}

void VideoPlayer::sendBufferingUpdate(int position) {
  if (eventSink_) {
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
    eventSink_->Success(eventValue);
  }
}

void VideoPlayer::sendBufferingEnd() {
  if (eventSink_) {
    flutter::EncodableMap encodables = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("bufferingEnd")}};
    flutter::EncodableValue eventValue(encodables);
    LOG_INFO("[VideoPlayer.onBuffering] send bufferingEnd event");
    eventSink_->Success(eventValue);
  }
}

void VideoPlayer::onPrepared(void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_DEBUG("[VideoPlayer.onPrepared] video player is prepared");

  if (!player->isInitialized_) {
    player->sendInitialized();
  }
}

void VideoPlayer::onBuffering(int percent, void *data) {
  // percent isn't used for video size, it's the used storage of buffer
  LOG_DEBUG("[VideoPlayer.onBuffering] percent: %d", percent);
}

void VideoPlayer::onPlayCompleted(void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_DEBUG("[VideoPlayer.onPlayCompleted] completed to playe video");

  if (player->eventSink_) {
    flutter::EncodableMap encodables = {{flutter::EncodableValue("event"),
                                         flutter::EncodableValue("completed")}};
    flutter::EncodableValue eventValue(encodables);
    LOG_INFO("[VideoPlayer.onPlayCompleted] send completed event");
    player->eventSink_->Success(eventValue);
  }
}

void VideoPlayer::onInterrupted(player_interrupted_code_e code, void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_DEBUG("[VideoPlayer.onInterrupted] interrupted code: %d", code);

  if (player->eventSink_) {
    LOG_INFO("[VideoPlayer.onInterrupted] send error event");
    player->eventSink_->Error("Video player is interrupted", "");
  }
}

void VideoPlayer::onErrorOccurred(int code, void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_DEBUG("[VideoPlayer.onErrorOccurred] error code: %s",
            get_error_message(code));

  if (player->eventSink_) {
    LOG_INFO("[VideoPlayer.onErrorOccurred] send error event");
    player->eventSink_->Error("Video player had error",
                              get_error_message(code));
  }
}

void VideoPlayer::onVideoFrameDecoded(media_packet_h packet, void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  tbm_surface_h surface;
  int ret = media_packet_get_tbm_surface(packet, &surface);
  if (ret != MEDIA_PACKET_ERROR_NONE) {
    LOG_ERROR(
        "[VideoPlayer.onVideoFrameDecoded] media_packet_get_tbm_surface "
        "failed, error: %d",
        ret);
    media_packet_destroy(packet);
    return;
  }
  FlutterMarkExternalTextureFrameAvailable(player->textureRegistrar_,
                                           player->textureId_, surface);
  media_packet_destroy(packet);
}
