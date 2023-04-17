// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player.h"

#include <app_common.h>
#include <app_manager.h>
#include <dlfcn.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/standard_method_codec.h>
#include <unistd.h>

#include <cstdarg>
#include <functional>

#include "drm_licence.h"
#include "log.h"

static int64_t gPlayerIndex = 1;

void VideoPlayer::ParseCreateMessage(const CreateMessage &create_message) {
  if (create_message.uri() != nullptr && !create_message.uri()->empty()) {
    uri_ = *create_message.uri();
    const flutter::EncodableMap *drm_configs = create_message.drm_configs();
    if (drm_configs) {
      auto drm_configs_iter =
          drm_configs->find(flutter::EncodableValue("drmType"));
      if (drm_configs_iter != drm_configs->end()) {
        if (std::holds_alternative<int>(
                drm_configs->at(flutter::EncodableValue("drmType")))) {
          drm_type_ = std::get<int>(
              drm_configs->at(flutter::EncodableValue("drmType")));
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
  } else {
    char *res_path = app_get_resource_path();
    if (res_path) {
      uri_ = uri_ + res_path + "flutter_assets/" + *create_message.asset();
      free(res_path);
    } else {
      LOG_ERROR("[VideoPlayer] Internal error", "Failed to get resource path.");
    }
  }
  LOG_INFO("[VideoPlayer] player uri: %s", uri_.c_str());
}

VideoPlayer::VideoPlayer(FlutterDesktopPluginRegistrarRef registrar_ref,
                         const CreateMessage &create_message)
    : registrar_ref_(registrar_ref) {
  ParseCreateMessage(create_message);
}

void VideoPlayer::GetChallengeData(FuncLicenseCB callback) {
  get_challenge_cb_ = callback;
}

void VideoPlayer::SetLicenseData(void *response_data, size_t response_len) {
  if (drm_manager_) {
    drm_manager_->SetLicenseData(response_data, response_len);
  }
}

bool VideoPlayer::Open(const std::string &uri) {
  int ret = player_create(&player_);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_create failed: %s", get_error_message(ret));
    return false;
  }

  if (drm_type_ != DRM_TYPE_NONE) {
    drm_manager_ = std::make_unique<DrmManager>(drm_type_, license_url_,
                                                player_, player_id_);
    if (get_challenge_cb_) {
      drm_manager_->GetChallengeData(get_challenge_cb_);
    }
    if (!drm_manager_->InitializeDrmSession(uri_)) {
      LOG_ERROR("[VideoPlayer] initial drm session failed");
      drm_manager_->ReleaseDrmSession();
    }
  }

  ret = player_set_uri(player_, uri.c_str());
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_set_uri failed: %s",
              get_error_message(ret));
    return false;
  }
  return true;
}

bool VideoPlayer::SetDisplay(FlutterDesktopPluginRegistrarRef registrar_ref) {
  FlutterDesktopViewRef view_ref =
      FlutterDesktopPluginRegistrarGetView(registrar_ref);
  if (!view_ref) {
    LOG_ERROR("[VideoPlayer] Could not get a Flutter view handle.");
    return false;
  }
  void *window = FlutterDesktopViewGetNativeHandle(view_ref);
  if (!view_ref) {
    LOG_ERROR("[VideoPlayer] Could not get a native window handle.");
    return false;
  }

  int x = 0, y = 0, w = 0, h = 0;
  void *ecore_lib_handle = dlopen("libecore_wl2.so.1", RTLD_LAZY);
  if (ecore_lib_handle) {
    FuncEcoreWl2WindowGeometryGet ecore_wl2_window_geometry_get =
        reinterpret_cast<FuncEcoreWl2WindowGeometryGet>(
            dlsym(ecore_lib_handle, "ecore_wl2_window_geometry_get"));
    if (ecore_wl2_window_geometry_get) {
      ecore_wl2_window_geometry_get(window, &x, &y, &w, &h);
    } else {
      LOG_ERROR("[VideoPlayer] Symbol not found: %s", dlerror());
      dlclose(ecore_lib_handle);
      return false;
    }
    dlclose(ecore_lib_handle);
  } else {
    LOG_ERROR("[VideoPlayer] dlopen failed: %s", dlerror());
    return false;
  }

  void *player_lib_handle = dlopen("libcapi-media-player.so.0", RTLD_LAZY);
  if (player_lib_handle) {
    FuncPlayerSetEcoreWlDisplay player_set_ecore_wl_display =
        reinterpret_cast<FuncPlayerSetEcoreWlDisplay>(
            dlsym(player_lib_handle, "player_set_ecore_wl_display"));
    if (player_set_ecore_wl_display) {
      int ret = player_set_ecore_wl_display(
          player_, PLAYER_DISPLAY_TYPE_OVERLAY, window, x, y, w, h);
      if (ret != PLAYER_ERROR_NONE) {
        LOG_ERROR("[VideoPlayer] player_set_ecore_wl_display failed: %s",
                  get_error_message(ret));
        dlclose(player_lib_handle);
        return false;
      }
    } else {
      LOG_ERROR("[VideoPlayer] Symbol not found: %s", dlerror());
    }
    dlclose(player_lib_handle);
  } else {
    LOG_ERROR("[VideoPlayer] dlopen failed: %s", dlerror());
  }

  int ret = player_set_display_mode(player_, PLAYER_DISPLAY_MODE_DST_ROI);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_display_mode failed: %s",
              get_error_message(ret));
    return false;
  }
  return true;
}

int64_t VideoPlayer::Create() {
  player_id_ = gPlayerIndex++;
  if (uri_.empty()) {
    LOG_ERROR("[VideoPlayer] uri is empty");
    return -1;
  }

  if (!Open(uri_)) {
    LOG_ERROR("[VideoPlayer] open failed, uri : %s", uri_.c_str());
    return -1;
  }

  if (!SetDisplay(registrar_ref_)) {
    LOG_ERROR("[VideoPlayer] fail to set display");
    return -1;
  }
  SetDisplayRoi(0, 0, 1, 1);

  int ret = player_set_buffering_cb(player_, OnBuffering, this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_set_buffering_cb failed: %s",
              get_error_message(ret));
  }

  ret = player_set_completed_cb(player_, OnPlayCompleted, this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_set_completed_cb failed: %s",
              get_error_message(ret));
  }

  ret = player_set_interrupted_cb(player_, onInterrupted, this);
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

  ret = player_set_error_cb(player_, OnError, this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_set_error_cb failed: %s",
              get_error_message(ret));
  }

  ret = player_prepare_async(player_, OnPrepared, this);
  if (ret != PLAYER_ERROR_NONE) {
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_prepare_async failed: %s",
              get_error_message(ret));
  }

  ret = player_set_subtitle_updated_cb(player_, OnSubtitleUpdated, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_subtitle_updated_cb failed: %s",
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
    player_destroy(player_);
    LOG_ERROR("[VideoPlayer] player_set_display_roi_area failed: %s",
              get_error_message(ret));
  }
}

VideoPlayer::~VideoPlayer() {
  if (drm_manager_) {
    drm_manager_->ReleaseDrmSession();
  }
  Dispose();
}

void VideoPlayer::Play() {
  LOG_INFO("[VideoPlayer] start player");
  player_state_e state;
  int ret = player_get_state(player_, &state);
  if (state < PLAYER_STATE_READY) {
    LOG_ERROR("[VideoPlayer] invalid state for play operation");
    return;
  }
  ret = player_start(player_);
  if ((state == PLAYER_STATE_READY || state == PLAYER_STATE_PAUSED) &&
      ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_start: %s", get_error_message(ret));
  }
}

void VideoPlayer::Pause() {
  player_state_e state;
  int ret = player_get_state(player_, &state);
  if (state <= PLAYER_STATE_READY) {
    LOG_ERROR("[VideoPlayer] invalid state for pause operation");
    return;
  }
  ret = player_pause(player_);
  if (state == PLAYER_STATE_PLAYING && ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_pause failed: %s", get_error_message(ret));
  }
}

void VideoPlayer::SetLooping(bool is_looping) {
  LOG_INFO("[VideoPlayer] isLooping: %d", is_looping);
  int ret = player_set_looping(player_, is_looping);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_looping failed: %s",
              get_error_message(ret));
  }
}

void VideoPlayer::SetVolume(double volume) {
  LOG_INFO("[VideoPlayer] volume: %f", volume);
  int ret = player_set_volume(player_, volume, volume);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_volume failed: %s",
              get_error_message(ret));
  }
}

void VideoPlayer::SetPlaybackSpeed(double speed) {
  LOG_INFO("[VideoPlayer] speed: %f", speed);
  int ret = player_set_playback_rate(player_, speed);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_playback_rate failed: %s",
              get_error_message(ret));
  }
}

void VideoPlayer::SeekTo(int position) {
  LOG_INFO("[VideoPlayer] position: %d", position);
  int ret =
      player_set_play_position(player_, position, true, OnSeekCompleted, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_set_play_position failed: %d", position);
  }
}

int VideoPlayer::GetPosition() {
  int position = 0;
  int ret = player_get_play_position(player_, &position);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[VideoPlayer] player_get_play_position failed: %s",
              get_error_message(ret));
  }
  return position;
}

void VideoPlayer::Dispose() {
  LOG_INFO("[VideoPlayer] dispose player");
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
  LOG_INFO("[VideoPlayer] setup event channel");
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
        LOG_INFO("[VideoPlayer] call listen of StreamHandler");
        event_sink_ = std::move(events);
        Initialize();
        return nullptr;
      },
      [&](const flutter::EncodableValue *arguments)
          -> std::unique_ptr<
              flutter::StreamHandlerError<flutter::EncodableValue>> {
        LOG_INFO("[VideoPlayer] call cancel of StreamHandler");
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
    LOG_INFO("[VideoPlayer] player state: %d", state);
    if (state == PLAYER_STATE_READY && !is_initialized_) {
      SendInitialized();
    }
  } else {
    LOG_ERROR("[VideoPlayer] player_get_state failed: %s",
              get_error_message(ret));
  }
}

void VideoPlayer::SendInitialized() {
  if (!is_initialized_ && !is_interrupted_ && event_sink_ != nullptr) {
    int duration = 0;
    int ret = player_get_duration(player_, &duration);
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR("[VideoPlayer] player_get_duration failed: %s",
                get_error_message(ret));
      event_sink_->Error("player_get_duration failed", get_error_message(ret));
      return;
    }
    LOG_INFO("[VideoPlayer] video duration: %d", duration);

    int width = 0, height = 0;
    ret = player_get_video_size(player_, &width, &height);
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR("[VideoPlayer] player_get_video_size failed: %s",
                get_error_message(ret));
      event_sink_->Error("player_get_video_size failed",
                         get_error_message(ret));
      return;
    }
    LOG_INFO("[VideoPlayer] video width: %d, video height: %d", width, height);

    player_display_rotation_e rotation;
    ret = player_get_display_rotation(player_, &rotation);
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR("[VideoPlayer] player_get_display_rotation failed: %s",
                get_error_message(ret));
      event_sink_->Error("player_get_display_rotation failed",
                         get_error_message(ret));
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
    LOG_INFO("[VideoPlayer] send initialized event");
    event_sink_->Success(eventValue);
  }
}

void VideoPlayer::SendBufferingStart() {
  if (event_sink_) {
    flutter::EncodableMap encodables = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("bufferingStart")}};
    flutter::EncodableValue eventValue(encodables);
    LOG_INFO("[VideoPlayer] send bufferingStart event");
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
    LOG_INFO("[VideoPlayer] send bufferingUpdate event");
    event_sink_->Success(eventValue);
  }
}

void VideoPlayer::SendBufferingEnd() {
  if (event_sink_) {
    flutter::EncodableMap encodables = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("bufferingEnd")}};
    flutter::EncodableValue eventValue(encodables);
    LOG_INFO("[VideoPlayer] send bufferingEnd event");
    event_sink_->Success(eventValue);
  }
}

void VideoPlayer::SendSubtitleUpdate(int duration, char *text) {
  if (event_sink_) {
    flutter::EncodableMap encodables = {
        {flutter::EncodableValue("event"),
         flutter::EncodableValue("subtitleUpdate")},
        {flutter::EncodableValue("duration"),
         flutter::EncodableValue(duration)},
        {flutter::EncodableValue("text"),
         flutter::EncodableValue(std::string(text))}};
    flutter::EncodableValue eventValue(encodables);
    LOG_INFO("[VideoPlayer] send SubtitleUpdate event");
    event_sink_->Success(eventValue);
  }
}

void VideoPlayer::OnSubtitleUpdated(unsigned long duration, char *text,
                                    void *user_data) {
  LOG_INFO("[VideoPlayer] video subtitle update,duration: %ld,text: %s",
           duration, text);
  VideoPlayer *player = reinterpret_cast<VideoPlayer *>(user_data);
  player->SendSubtitleUpdate(duration, text);
}

void VideoPlayer::OnPrepared(void *data) {
  VideoPlayer *player = reinterpret_cast<VideoPlayer *>(data);
  LOG_INFO("[VideoPlayer] video player is prepared");
  if (!player->is_initialized_) {
    player->SendInitialized();
  }
}

void VideoPlayer::OnBuffering(int percent, void *data) {
  LOG_INFO("[VideoPlayer] percent: %d", percent);
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
  LOG_INFO("[VideoPlayer] completed to seek");
  if (player->on_seek_completed_) {
    player->on_seek_completed_();
    player->on_seek_completed_ = nullptr;
  }
}

void VideoPlayer::OnPlayCompleted(void *data) {
  VideoPlayer *player = reinterpret_cast<VideoPlayer *>(data);
  LOG_INFO("[VideoPlayer] completed to play video");
  if (player->event_sink_) {
    flutter::EncodableMap encodables = {{flutter::EncodableValue("event"),
                                         flutter::EncodableValue("completed")}};
    flutter::EncodableValue eventValue(encodables);
    player->event_sink_->Success(eventValue);
    player->Pause();
  }
}

void VideoPlayer::OnError(int error_code, void *user_data) {
  VideoPlayer *player = reinterpret_cast<VideoPlayer *>(user_data);
  LOG_ERROR("[VideoPlayer] error code: %s", get_error_message(error_code));
  if (player->event_sink_) {
    player->event_sink_->Error(
        "Player error", std::string("Error: ") + get_error_message(error_code));
  }
}

void VideoPlayer::onInterrupted(player_interrupted_code_e code, void *data) {
  VideoPlayer *player = reinterpret_cast<VideoPlayer *>(data);
  LOG_ERROR("[VideoPlayer] interrupt code: %d", code);
  player->is_interrupted_ = true;
  if (player->event_sink_) {
    player->event_sink_->Error("Player interrupted",
                               "Video player has been interrupted.");
  }
}
