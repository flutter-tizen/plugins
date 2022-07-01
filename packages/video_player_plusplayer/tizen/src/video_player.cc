// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player.h"

#include <app_manager.h>
#include <dlfcn.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/standard_method_codec.h>
#include <system_info.h>
#include <unistd.h>

#include <cstdarg>
#include <functional>

#include "log.h"

static int64_t gPlayerIndex = 1;

std::string VideoPlayer::GetApplicationId() {
  char *app_id = nullptr;
  pid_t pid = getpid();
  int ret = app_manager_get_app_id(pid, &app_id);
  if (ret != APP_MANAGER_ERROR_NONE) return {};

  std::string app_id_str(app_id);
  if (app_id) free(app_id);

  return app_id_str;
}

void VideoPlayer::ParseCreateMessage(const CreateMessage &create_message) {
  uri_ = std::string(*create_message.uri());
  const flutter::EncodableMap *drm_configs = create_message.drm_configs();
  auto drm_configs_iter = drm_configs->find(flutter::EncodableValue("drmType"));
  if (drm_configs_iter != drm_configs->end()) {
    if (std::holds_alternative<int>(
            drm_configs->at(flutter::EncodableValue("drmType")))) {
      drm_type_ =
          std::get<int>(drm_configs->at(flutter::EncodableValue("drmType")));
    }
  }
  drm_configs_iter =
      drm_configs->find(flutter::EncodableValue("licenseServerUrl"));
  if (drm_configs_iter != drm_configs->end()) {
    if (std::holds_alternative<std::string>(
            drm_configs->at(flutter::EncodableValue("licenseServerUrl")))) {
      license_url_ = std::get<std::string>(
          drm_configs->at(flutter::EncodableValue("licenseServerUrl")));
    }
  }
}

VideoPlayer::VideoPlayer(FlutterDesktopPluginRegistrarRef registrar_ref,
                         const CreateMessage &create_message)
    : registrar_ref_(registrar_ref) {
  ParseCreateMessage(create_message);
}

bool VideoPlayer::Open(const std::string &uri) {
  PlusplayerWrapperProxy &instance = PlusplayerWrapperProxy::GetInstance();
  plusplayer_ = instance.CreatePlayer();
  if (plusplayer_ == nullptr) {
    LOG_ERROR("fail to create plusplayer");
    return false;
  }
  if (!instance.Open(plusplayer_, uri)) {
    LOG_ERROR("open uri(%s) failed", uri.c_str());
    return false;
  }
  instance.SetAppId(plusplayer_, GetApplicationId());
  return true;
}

void VideoPlayer::RegisterListener() {
  PlusplayerWrapperProxy &instance = PlusplayerWrapperProxy::GetInstance();
  listener_.buffering_callback = OnBuffering;
  listener_.adaptive_streaming_control_callback =
      OnPlayerAdaptiveStreamingControl;
  listener_.completed_callback = OnPlayCompleted;
  listener_.drm_init_data_callback = OnDrmInitData;
  listener_.error_callback = OnError;
  listener_.error_message_callback = OnErrorMessage;
  listener_.playing_callback = OnPlaying;
  listener_.prepared_callback = OnPrepared;
  listener_.seek_completed_callback = OnSeekCompleted;
  instance.RegisterListener(plusplayer_, &listener_, this);
}

bool VideoPlayer::SetDisplay(FlutterDesktopPluginRegistrarRef registrar_ref) {
  PlusplayerWrapperProxy &instance = PlusplayerWrapperProxy::GetInstance();
  int w, h = 0;
  if (system_info_get_platform_int("http://tizen.org/feature/screen.width",
                                   &w) != SYSTEM_INFO_ERROR_NONE ||
      system_info_get_platform_int("http://tizen.org/feature/screen.height",
                                   &h) != SYSTEM_INFO_ERROR_NONE) {
    LOG_ERROR("could not obtain the screen size.");
    return false;
  }
  return instance.SetDisplay(
      plusplayer_, plusplayer::DisplayType::kOverlay,
      instance.GetSurfaceId(
          plusplayer_,
          FlutterDesktopPluginRegistrarGetNativeWindow(registrar_ref)),
      0, 0, w, h);
}

int64_t VideoPlayer::Create() {
  PlusplayerWrapperProxy &instance = PlusplayerWrapperProxy::GetInstance();
  if (uri_.empty()) {
    LOG_ERROR("uri is empty");
    return -1;
  }
  if (!Open(uri_)) {
    LOG_ERROR("open failed, uri : %s", uri_.c_str());
    return -1;
  }
  RegisterListener();
  if (drm_type_ != DRM_TYPE_NONE && !license_url_.empty()) {
    drm_manager_ =
        std::make_unique<DrmManager>(drm_type_, license_url_, plusplayer_);
    drm_manager_->InitializeDrmSession(uri_);
  }
  if (!SetDisplay(registrar_ref_)) {
    LOG_ERROR("fail to set display");
    return -1;
  }
  SetDisplayRoi(0, 0, 1, 1);
  if (!instance.PrepareAsync(plusplayer_)) {
    LOG_ERROR("fail to prepare");
    return -1;
  }
  player_id_ = gPlayerIndex++;
  flutter::PluginRegistrar *plugin_registrar =
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar_ref_);
  SetupEventChannel(plugin_registrar->messenger());
  return player_id_;
}

void VideoPlayer::SetDisplayRoi(int x, int y, int w, int h) {
  LOG_DEBUG("setDisplayRoi x : %d, y : %d, w : %d, h : %d", x, y, w, h);
  PlusplayerWrapperProxy &instance = PlusplayerWrapperProxy::GetInstance();
  if (!instance.SetDisplayMode(plusplayer_, plusplayer::DisplayMode::kDstRoi)) {
    LOG_ERROR("fail to set display mode");
    return;
  }
  plusplayer::Geometry roi;
  roi.x = x;
  roi.y = y;
  roi.w = w;
  roi.h = h;
  if (!instance.SetDisplayRoi(plusplayer_, roi)) {
    LOG_ERROR("fail to set display roi");
  }
}

bool VideoPlayer::SetBufferingConfig(const std::string &option, int amount) {
  if (drm_manager_) {
    LOG_INFO("Do not set buffering config when playing drm");
    return false;
  }
  if (plusplayer_ == nullptr) {
    LOG_ERROR("plusplayer isn't created");
    return false;
  }
  LOG_DEBUG("plusplayer SetBufferingConfig option : %s, amount : %d",
            option.c_str(), amount);
  return PlusplayerWrapperProxy::GetInstance().SetBufferConfig(
      plusplayer_, std::pair<std::string, int>(option, amount));
}

VideoPlayer::~VideoPlayer() {
  if (drm_manager_) {
    drm_manager_->ReleaseDrmSession();
  }
  Dispose();
}

void VideoPlayer::Play() {
  PlusplayerWrapperProxy &instance = PlusplayerWrapperProxy::GetInstance();
  if (instance.GetState(plusplayer_) < plusplayer::State::kReady) {
    LOG_ERROR("invalid state for play operation");
    return;
  }

  if (instance.GetState(plusplayer_) == plusplayer::State::kReady) {
    if (!instance.Start(plusplayer_)) {
      LOG_ERROR("fail to start");
    }
  } else if (instance.GetState(plusplayer_) == plusplayer::State::kPaused) {
    if (!instance.Resume(plusplayer_)) {
      LOG_ERROR("fail to resume");
    }
  }
}

void VideoPlayer::Pause() {
  PlusplayerWrapperProxy &instance = PlusplayerWrapperProxy::GetInstance();
  if (instance.GetState(plusplayer_) <= plusplayer::State::kReady) {
    LOG_ERROR("invalid state for pause operation");
    return;
  }

  if (instance.GetState(plusplayer_) == plusplayer::State::kPlaying) {
    if (!instance.Pause(plusplayer_)) {
      LOG_ERROR("fail to pause");
    }
  }
}

void VideoPlayer::SetLooping(bool is_looping) {
  LOG_ERROR("plusplayer doesn't support to set looping");
}

void VideoPlayer::SetVolume(double volume) {
  LOG_ERROR("plusplayer doesn't support to set volume");
}

void VideoPlayer::SetPlaybackSpeed(double speed) {
  LOG_INFO("set playback speed: %f", speed);
  PlusplayerWrapperProxy &instance = PlusplayerWrapperProxy::GetInstance();
  if (!instance.SetPlaybackRate(plusplayer_, speed)) {
    LOG_ERROR("fail to set playback rate speed : %f", speed);
  }
}

void VideoPlayer::SeekTo(int position) {
  PlusplayerWrapperProxy &instance = PlusplayerWrapperProxy::GetInstance();
  if (!instance.Seek(plusplayer_, position)) {
    LOG_ERROR("fail to seek, position : %d", position);
  }
}

int VideoPlayer::GetPosition() {
  PlusplayerWrapperProxy &instance = PlusplayerWrapperProxy::GetInstance();
  plusplayer::State state = instance.GetState(plusplayer_);
  if (state == plusplayer::State::kPlaying ||
      state == plusplayer::State::kPaused) {
    uint64_t position;
    instance.GetPlayingTime(plusplayer_, &position);
    return static_cast<int>(position);
  } else {
    LOG_ERROR("fail to get playing time");
  }
}

void VideoPlayer::Dispose() {
  is_initialized_ = false;
  event_sink_ = nullptr;
  event_channel_->SetStreamHandler(nullptr);

  if (plusplayer_) {
    PlusplayerWrapperProxy &instance = PlusplayerWrapperProxy::GetInstance();
    instance.UnregisterListener(plusplayer_);
    instance.DestroyPlayer(plusplayer_);
    plusplayer_ = nullptr;
  }
}

void VideoPlayer::SetupEventChannel(flutter::BinaryMessenger *messenger) {
  LOG_INFO("[VideoPlayer.setupEventChannel] setup event channel");
  std::string name =
      "flutter.io/videoPlayer/videoEvents" + std::to_string(player_id_);
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
        LOG_INFO(
            "[VideoPlayer.setupEventChannel] call listen of StreamHandler");
        event_sink_ = std::move(events);
        Initialize();
        return nullptr;
      },
      [&](const flutter::EncodableValue *arguments)
          -> std::unique_ptr<
              flutter::StreamHandlerError<flutter::EncodableValue>> {
        LOG_INFO(
            "[VideoPlayer.setupEventChannel] call cancel of StreamHandler");
        event_sink_ = nullptr;
        return nullptr;
      });
  channel->SetStreamHandler(std::move(handler));
  event_channel_ = std::move(channel);
}

void VideoPlayer::Initialize() {
  PlusplayerWrapperProxy &instance = PlusplayerWrapperProxy::GetInstance();
  plusplayer::State state = instance.GetState(plusplayer_);
  LOG_INFO("[VideoPlayer.initialize] player state: %d", state);
  if (state == plusplayer::State::kReady && !is_initialized_) {
    SendInitialized();
  }
}

void VideoPlayer::SendInitialized() {
  if (!is_initialized_ && event_sink_ != nullptr) {
    PlusplayerWrapperProxy &instance = PlusplayerWrapperProxy::GetInstance();
    int64_t duration;
    if (!instance.GetDuration(plusplayer_, &duration)) {
      event_sink_->Error("PlusPlayer", "GetDuration operation failed");
      return;
    }
    LOG_INFO("[VideoPlayer.sendInitialized] video duration: %lld", duration);

    int width, height = 0;
    std::vector<plusplayer::Track> tracks =
        instance.GetActiveTrackInfo(plusplayer_);
    for (auto track : tracks) {
      if (track.type == plusplayer::TrackType::kTrackTypeVideo) {
        width = track.width;
        height = track.height;
      }
    }
    LOG_INFO("video widht: %d, video height: %d", width, height);
    if (width == 0 || height == 0) {
      event_sink_->Error("PlusPlayer", "Fail to get video size");
    }

    plusplayer::DisplayRotation rotate;
    if (!instance.GetDisplayRotate(plusplayer_, &rotate)) {
      event_sink_->Error("PlusPlayer", "GetDisplayRotate operation failed");
    } else {
      if (rotate == plusplayer::DisplayRotation::kRotate90 ||
          rotate == plusplayer::DisplayRotation::kRotate270) {
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

void VideoPlayer::SendBufferingStart() {
  if (event_sink_) {
    flutter::EncodableMap encodables = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("bufferingStart")}};
    flutter::EncodableValue eventValue(encodables);
    LOG_INFO("[VideoPlayer.onBuffering] send bufferingStart event");
    event_sink_->Success(eventValue);
  }
}

void VideoPlayer::SendBufferingUpdate(int position) {
  if (event_sink_) {
    flutter::EncodableMap encodables = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("bufferingUpdate")},
        {flutter::EncodableValue("values"), flutter::EncodableValue(position)}};
    flutter::EncodableValue eventValue(encodables);
    LOG_INFO("[VideoPlayer.onBuffering] send bufferingUpdate event");
    event_sink_->Success(eventValue);
  }
}

void VideoPlayer::SendBufferingEnd() {
  if (event_sink_) {
    flutter::EncodableMap encodables = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("bufferingEnd")}};
    flutter::EncodableValue eventValue(encodables);
    LOG_INFO("[VideoPlayer.onBuffering] send bufferingEnd event");
    event_sink_->Success(eventValue);
  }
}

void VideoPlayer::OnPrepared(bool ret, void *data) {
  VideoPlayer *player = reinterpret_cast<VideoPlayer *>(data);
  LOG_DEBUG("[VideoPlayer.onPrepared] ret == %d", ret);

  if (!player->is_initialized_ && ret) {
    player->SendInitialized();
  }
}

void VideoPlayer::OnBuffering(int percent, void *data) {
  LOG_DEBUG("[VideoPlayer.onBuffering] percent: %d", percent);
  VideoPlayer *player = reinterpret_cast<VideoPlayer *>(data);
  if (percent == 100) {
    player->SendBufferingEnd();
    player->is_buffering_ = false;
  } else if (!player->is_buffering_ && percent <= 5) {
    player->SendBufferingStart();
    player->is_buffering_ = true;
  } else {
    player->SendBufferingUpdate(percent);
  }
}

void VideoPlayer::OnSeekCompleted(void *data) {
  VideoPlayer *player = reinterpret_cast<VideoPlayer *>(data);
  LOG_DEBUG("[VideoPlayer.onSeekCompleted] completed to seek");
}

void VideoPlayer::OnPlayCompleted(void *data) {
  VideoPlayer *player = reinterpret_cast<VideoPlayer *>(data);

  if (player->event_sink_) {
    flutter::EncodableMap encodables = {{flutter::EncodableValue("event"),
                                         flutter::EncodableValue("completed")}};
    flutter::EncodableValue eventValue(encodables);
    player->event_sink_->Success(eventValue);
    LOG_DEBUG("[VideoPlayer.onPlayCompleted] change player state to pause");
    player->Pause();
  }
}

void VideoPlayer::OnPlaying(void *data) {}

void VideoPlayer::OnError(const plusplayer::ErrorType &error_code,
                          void *user_data) {
  LOG_ERROR("ErrorType : %d", error_code);
}

void VideoPlayer::OnErrorMessage(const plusplayer::ErrorType &error_code,
                                 const char *error_msg, void *user_data) {
  LOG_ERROR("ErrorType : %d, error_msg : %s", error_code, error_msg);
}

void VideoPlayer::OnPlayerAdaptiveStreamingControl(
    const plusplayer::StreamingMessageType &type,
    const plusplayer::MessageParam &msg, void *user_data) {
  VideoPlayer *player = reinterpret_cast<VideoPlayer *>(user_data);
  if (player && type == plusplayer::StreamingMessageType::kDrmInitData) {
    player->drm_manager_->OnPlayerAdaptiveStreamingControl(msg);
  }
}

void VideoPlayer::OnDrmInitData(int *drmhandle, unsigned int len,
                                unsigned char *psshdata,
                                plusplayer::TrackType type, void *userdata) {
  VideoPlayer *player = reinterpret_cast<VideoPlayer *>(userdata);
  if (player) {
    player->drm_manager_->OnDrmInit(drmhandle, len, psshdata, type);
  }
}
