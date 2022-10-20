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

#include "drm_licence.h"
#include "log.h"
static int64_t gPlayerIndex = 1;

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

static DeviceProfile GetDeviceProfile() {
  char *feature_profile = nullptr;
  system_info_get_platform_string("http://tizen.org/feature/profile",
                                  &feature_profile);
  if (feature_profile == nullptr) {
    return DeviceProfile::kUnknown;
  }
  std::string profile(feature_profile);
  free(feature_profile);

  if (profile == "mobile") {
    return DeviceProfile::kMobile;
  } else if (profile == "wearable") {
    return DeviceProfile::kWearable;
  } else if (profile == "tv") {
    return DeviceProfile::kTV;
  } else if (profile == "common") {
    return DeviceProfile::kCommon;
  }
  return DeviceProfile::kUnknown;
}

VideoPlayer::VideoPlayer(FlutterDesktopPluginRegistrarRef registrar_ref,
                         const CreateMessage &create_message)
    : registrar_ref_(registrar_ref) {
  ParseCreateMessage(create_message);
}

bool VideoPlayer::Open(const std::string &uri) {
  int ret = player_create(&player_);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("fail to create media player: %s", get_error_message(ret));
    return false;
  }

  if (drm_type_ != DRM_TYPE_NONE && !license_url_.empty()) {
    drm_manager_ =
        std::make_unique<DrmManager>(drm_type_, license_url_, player_);
    drm_manager_->InitializeDrmSession(uri_);
  }

  ret = player_set_uri(player_, uri.c_str());
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("set uri(%s) failed", get_error_message(ret));
    return false;
  }
  return true;
}

bool VideoPlayer::SetDisplay(FlutterDesktopPluginRegistrarRef registrar_ref) {
  int w, h = 0;
  int ret;
  if (system_info_get_platform_int("http://tizen.org/feature/screen.width",
                                   &w) != SYSTEM_INFO_ERROR_NONE ||
      system_info_get_platform_int("http://tizen.org/feature/screen.height",
                                   &h) != SYSTEM_INFO_ERROR_NONE) {
    LOG_ERROR("could not obtain the screen size.");
    return false;
  }

  FlutterDesktopViewRef view_ref =
      FlutterDesktopPluginRegistrarGetView(registrar_ref);
  if (view_ref == nullptr) {
    LOG_ERROR("could not get window view handle");
    return false;
  }

  if (GetDeviceProfile() == kWearable) {
    ret = player_set_display(player_, PLAYER_DISPLAY_TYPE_OVERLAY,
                             FlutterDesktopViewGetNativeHandle(view_ref));
  } else {
    ret = -1;
    void *libHandle = dlopen("libcapi-media-player.so.0", RTLD_LAZY);
    int (*player_set_ecore_wl_display)(
        player_h player, player_display_type_e type, void *ecore_wl_window,
        int x, int y, int width, int height);
    if (libHandle) {
      *(void **)(&player_set_ecore_wl_display) =
          dlsym(libHandle, "player_set_ecore_wl_display");
      if (player_set_ecore_wl_display) {
        ret = player_set_ecore_wl_display(
            player_, PLAYER_DISPLAY_TYPE_OVERLAY,
            FlutterDesktopViewGetNativeHandle(view_ref), 0, 0, w, h);
      } else {
        LOG_ERROR("[VideoPlayer] Symbol not found: %s ", dlerror());
      }
      dlclose(libHandle);
    } else {
      LOG_ERROR("[VideoPlayer] dlopen failed: %s ", dlerror());
    }
  }
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_set_ecore_wl_display failed: %s",
              get_error_message(ret));
    return false;
  }
  ret = player_set_display_mode(player_, PLAYER_DISPLAY_MODE_DST_ROI);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("fail to set display mode :%s", get_error_message(ret));
    return false;
  }
  return true;
}

int64_t VideoPlayer::Create() {
  int ret;
  player_id_ = gPlayerIndex++;
  if (uri_.empty()) {
    LOG_ERROR("uri is empty");
    return -1;
  }

  if (!Open(uri_)) {
    LOG_ERROR("open failed, uri : %s", uri_.c_str());
    return -1;
  }

  if (!SetDisplay(registrar_ref_)) {
    LOG_ERROR("fail to set display");
    return -1;
  }
  SetDisplayRoi(0, 0, 1, 1);

  ret = player_set_buffering_cb(player_, OnBuffering, (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_set_buffering_cb failed: %s",
              get_error_message(ret));
  }

  ret = player_set_completed_cb(player_, OnPlayCompleted, (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_set_completed_cb failed: %s",
              get_error_message(ret));
  }

  ret = player_set_interrupted_cb(player_, onInterrupted, (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_set_interrupted_cb failed: %s",
              get_error_message(ret));
  }

  ret = player_set_display_visible(player_, true);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_set_display_visible failed: %s",
              get_error_message(ret));
  }

  ret = player_set_error_cb(player_, OnError, (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_set_error_cb failed: %s",
              get_error_message(ret));
  }

  ret = player_prepare_async(player_, OnPrepared, (void *)this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_prepare_async failed: %s",
              get_error_message(ret));
  }

  flutter::PluginRegistrar *plugin_registrar =
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar_ref_);
  SetupEventChannel(plugin_registrar->messenger());
  return player_id_;
}

void VideoPlayer::SetDisplayRoi(int x, int y, int w, int h) {
  int ret = player_set_display_roi_area(player_, x, y, w, h);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("fail to set display roi :%s", get_error_message(ret));
  }
}

VideoPlayer::~VideoPlayer() {
  if (drm_manager_) {
    drm_manager_->ReleaseDrmSession();
  }
  Dispose();
}

void VideoPlayer::Play() {
  player_state_e state;
  int ret = player_get_state(player_, &state);
  if (state < PLAYER_STATE_READY) {
    LOG_ERROR("invalid state for play operation");
    return;
  }
  ret = player_start(player_);
  if (state == PLAYER_STATE_READY || state == PLAYER_STATE_PAUSED) {
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR("fail to start");
    }
  }
}

void VideoPlayer::Pause() {
  player_state_e state;
  int ret = player_get_state(player_, &state);
  if (state <= PLAYER_STATE_READY) {
    LOG_ERROR("invalid state for pause operation");
    return;
  }
  ret = player_pause(player_);
  if (state == PLAYER_STATE_PLAYING) {
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR("fail to pause");
    }
  }
}

void VideoPlayer::SetLooping(bool is_looping) {
  LOG_INFO("[VideoPlayer.setLooping] isLooping: %d", is_looping);
  int ret = player_set_looping(player_, is_looping);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer.setLooping] player_set_looping failed: %s",
              get_error_message(ret));
  }
}

void VideoPlayer::SetVolume(double volume) {
  LOG_INFO("[VideoPlayer.setVolume] volume: %f", volume);
  int ret = player_set_volume(player_, volume, volume);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer.setVolume] player_set_volume failed: %s",
              get_error_message(ret));
  }
}

void VideoPlayer::SetPlaybackSpeed(double speed) {
  LOG_INFO("set playback speed: %f", speed);
  int ret = player_set_playback_rate(player_, speed);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("fail to set playback rate speed : %f", speed);
  }
}

void VideoPlayer::SeekTo(int position) {
  LOG_INFO("[VideoPlayer.seekTo] position: %d", position);
  int ret =
      player_set_play_position(player_, position, true, OnSeekCompleted, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer.seekTo] player_set_play_position failed: %s",
              get_error_message(ret));
    LOG_ERROR("fail to seek, position : %d", position);
  }
}

int VideoPlayer::GetPosition() {
  int position;
  int ret = player_get_play_position(player_, &position);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer.getPosition] player_get_play_position failed :%s",
              get_error_message(ret));
  }
  return position;
}

void VideoPlayer::Dispose() {
  LOG_INFO("[VideoPlayer.dispose] dispose video player");
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
    player_ = nullptr;
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
  player_state_e state;
  int ret = player_get_state(player_, &state);
  if (ret == PLAYER_ERROR_NONE) {
    LOG_INFO("[VideoPlayer.initialize] player state: %d", state);
    if (state == PLAYER_STATE_READY && !is_initialized_) {
      SendInitialized();
    }
  } else {
    LOG_ERROR("[VideoPlayer.initialize] player_get_state failed: %s",
              get_error_message(ret));
  }
}

void VideoPlayer::SendInitialized() {
  if (!is_initialized_ && !is_interrupted_ && event_sink_ != nullptr) {
    int duration;
    int ret = player_get_duration(player_, &duration);
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR("[VideoPlayer.sendInitialized] player_get_duration failed:%s",
                get_error_message(ret));
      event_sink_->Error("player_get_duration failed");
      return;
    }
    LOG_INFO("[VideoPlayer.sendInitialized] video duration: %lld", duration);

    int width = 0, height = 0;
    ret = player_get_video_size(player_, &width, &height);
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR(
          "[VideoPlayer.sendInitialized] player_get_video_size failed :%s",
          get_error_message(ret));
      event_sink_->Error("player_get_video_size failed");
      return;
    }
    LOG_INFO("[VideoPlayer.sendInitialized] video widht: %d, video height: %d",
             width, height);

    player_display_rotation_e rotation;
    ret = player_get_display_rotation(player_, &rotation);
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR(
          "[VideoPlayer.sendInitialized] player_get_display_rotation failed: "
          "%s",
          get_error_message(ret));
      event_sink_->Error("GetDisplayRotate operation failed");
    } else {
      if (rotation == player_display_rotation_e::PLAYER_DISPLAY_ROTATION_90 ||
          rotation == player_display_rotation_e::PLAYER_DISPLAY_ROTATION_270) {
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

void VideoPlayer::OnPrepared(void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_INFO("[VideoPlayer.onPrepared] video player is prepared");
  if (!player->is_initialized_) {
    player->SendInitialized();
  }
}

void VideoPlayer::OnBuffering(int percent, void *data) {
  LOG_INFO("[VideoPlayer.onBuffering] percent: %d", percent);
  VideoPlayer *player = (VideoPlayer *)data;
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
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_INFO("[VideoPlayer.onSeekCompleted] completed to seek");
  if (player->on_seek_completed_) {
    player->on_seek_completed_();
    player->on_seek_completed_ = nullptr;
  }
}

void VideoPlayer::OnPlayCompleted(void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_INFO("[VideoPlayer.onPlayCompleted] completed to play video");
  if (player->event_sink_) {
    flutter::EncodableMap encodables = {{flutter::EncodableValue("event"),
                                         flutter::EncodableValue("completed")}};
    flutter::EncodableValue eventValue(encodables);
    player->event_sink_->Success(eventValue);
    LOG_DEBUG("[VideoPlayer.onPlayCompleted] change player state to pause");
    player->Pause();
  }
}

void VideoPlayer::OnError(int error_code, void *user_data) {
  VideoPlayer *player = (VideoPlayer *)user_data;
  LOG_ERROR("[VideoPlayer.onError] error code: %s",
            get_error_message(error_code));
  if (player->event_sink_) {
    player->event_sink_->Error("Video player had error",
                               get_error_message(error_code));
  }
}

void VideoPlayer::onInterrupted(player_interrupted_code_e code, void *data) {
  VideoPlayer *player = (VideoPlayer *)data;
  LOG_ERROR("[VideoPlayer.onErrorOccurred] error code: %s",
            get_error_message(code));
  if (player->event_sink_) {
    player->event_sink_->Error("Video player had error",
                               get_error_message(code));
  }
}
