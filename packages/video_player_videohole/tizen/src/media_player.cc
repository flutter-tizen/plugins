// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media_player.h"

#include <dlfcn.h>

#include <sstream>

#include "log.h"

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

static player_stream_type_e ConvertTrackType(std::string track_type) {
  if (track_type == "video") {
    return PLAYER_STREAM_TYPE_VIDEO;
  }
  if (track_type == "audio") {
    return PLAYER_STREAM_TYPE_AUDIO;
  }
  if (track_type == "text") {
    return PLAYER_STREAM_TYPE_TEXT;
  }
  return PLAYER_STREAM_TYPE_DEFAULT;
}

MediaPlayer::MediaPlayer(flutter::BinaryMessenger *messenger,
                         FlutterDesktopViewRef flutter_view)
    : VideoPlayer(messenger, flutter_view) {
  media_player_proxy_ = std::make_unique<MediaPlayerProxy>();
  device_proxy_ = std::make_unique<DeviceProxy>();
}

MediaPlayer::~MediaPlayer() {
  if (player_) {
    player_stop(player_);
    player_unprepare(player_);
    player_unset_buffering_cb(player_);
    player_unset_completed_cb(player_);
    player_unset_interrupted_cb(player_);
    player_unset_error_cb(player_);
    player_unset_subtitle_updated_cb(player_);
    player_destroy(player_);
    player_ = nullptr;
  }
  if (drm_manager_) {
    drm_manager_->ReleaseDrmSession();
  }
}

int64_t MediaPlayer::Create(const std::string &uri,
                            const CreateMessage &create_message) {
  LOG_INFO("[MediaPlayer] uri: %s.", uri.c_str());

  if (uri.empty()) {
    LOG_ERROR("[MediaPlayer] The uri must not be empty.");
    return -1;
  }
  url_ = uri;
  create_message_ = create_message;

  int ret = player_create(&player_);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_create failed: %s.",
              get_error_message(ret));
    return -1;
  }

  std::string cookie = flutter_common::GetValue(create_message.http_headers(),
                                                "Cookie", std::string());
  if (!cookie.empty()) {
    int ret =
        player_set_streaming_cookie(player_, cookie.c_str(), cookie.size());
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR("[MediaPlayer] player_set_streaming_cookie failed: %s.",
                get_error_message(ret));
    }
  }
  std::string user_agent = flutter_common::GetValue(
      create_message.http_headers(), "User-Agent", std::string());
  if (!user_agent.empty()) {
    int ret = player_set_streaming_user_agent(player_, user_agent.c_str(),
                                              user_agent.size());
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR("[MediaPlayer] player_set_streaming_user_agent failed: %s.",
                get_error_message(ret));
    }
  }

  int64_t drm_type =
      flutter_common::GetValue(create_message.drm_configs(), "drmType", 0);
  std::string license_server_url = flutter_common::GetValue(
      create_message.drm_configs(), "licenseServerUrl", std::string());
  if (drm_type != 0) {
    if (!SetDrm(uri, drm_type, license_server_url)) {
      LOG_ERROR("[MediaPlayer] Failed to set drm.");
      return -1;
    }
  }

  if (!SetDisplay()) {
    LOG_ERROR("[MediaPlayer] Failed to set display.");
    return -1;
  }

  SetDisplayRoi(pre_display_roi_x_, pre_display_roi_y_, pre_display_roi_width_,
                pre_display_roi_height_);

  ret = player_set_uri(player_, uri.c_str());
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_uri failed : %s.",
              get_error_message(ret));
    return -1;
  }

  ret = player_set_display_visible(player_, true);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_display_visible failed : %s.",
              get_error_message(ret));
    return -1;
  }

  ret = player_set_buffering_cb(player_, OnBuffering, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_buffering_cb failed : %s.",
              get_error_message(ret));
    return -1;
  }

  ret = player_set_completed_cb(player_, OnPlayCompleted, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_completed_cb failed : %s.",
              get_error_message(ret));
    return -1;
  }

  ret = player_set_interrupted_cb(player_, OnInterrupted, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_interrupted_cb failed : %s.",
              get_error_message(ret));
    return -1;
  }

  ret = player_set_error_cb(player_, OnError, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_error_cb failed : %s.",
              get_error_message(ret));
    return -1;
  }

  ret = player_set_subtitle_updated_cb(player_, OnSubtitleUpdated, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_subtitle_updated_cb failed : %s.",
              get_error_message(ret));
    return -1;
  }

  ret = player_prepare_async(player_, OnPrepared, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_prepare_async failed : %s.",
              get_error_message(ret));
    return -1;
  }

  return SetUpEventChannel();
}

void MediaPlayer::Dispose() {
  LOG_INFO("[MediaPlayer] Disposing.");
  ClearUpEventChannel();
}

void MediaPlayer::SetDisplayRoi(int32_t x, int32_t y, int32_t width,
                                int32_t height) {
  int ret = player_set_display_roi_area(player_, x, y, width, height);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_display_roi_area failed: %s.",
              get_error_message(ret));
  }
  pre_display_roi_x_ = x;
  pre_display_roi_y_ = y;
  pre_display_roi_width_ = width;
  pre_display_roi_height_ = height;
}

bool MediaPlayer::Play() {
  LOG_INFO("[MediaPlayer] Player starting.");

  player_state_e state = PLAYER_STATE_NONE;
  int ret = player_get_state(player_, &state);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] Unable to get player state.");
  }
  if (state == PLAYER_STATE_NONE || state == PLAYER_STATE_IDLE) {
    LOG_ERROR("[MediaPlayer] Player not ready.");
    return false;
  }
  if (state == PLAYER_STATE_PLAYING) {
    LOG_INFO("[MediaPlayer] Player already playing.");
    return false;
  }
  ret = player_start(player_);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_start failed: %s.", get_error_message(ret));
    return false;
  }
  SendIsPlayingState(true);
  return true;
}

bool MediaPlayer::Pause() {
  LOG_INFO("[MediaPlayer] Player pausing.");

  player_state_e state = PLAYER_STATE_NONE;
  int ret = player_get_state(player_, &state);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] Unable to get player state.");
  }
  if (state == PLAYER_STATE_NONE || state == PLAYER_STATE_IDLE) {
    LOG_ERROR("[MediaPlayer] Player not ready.");
    return false;
  }
  if (state != PLAYER_STATE_PLAYING) {
    LOG_INFO("[MediaPlayer] Player not playing.");
    return false;
  }
  ret = player_pause(player_);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_pause failed: %s.", get_error_message(ret));
    return false;
  }
  SendIsPlayingState(false);
  return true;
}

bool MediaPlayer::SetLooping(bool is_looping) {
  LOG_INFO("[MediaPlayer] is_looping: %d.", is_looping);

  int ret = player_set_looping(player_, is_looping);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_looping failed: %s.",
              get_error_message(ret));
    return false;
  }
  return true;
}

bool MediaPlayer::SetVolume(double volume) {
  LOG_INFO("[MediaPlayer] volume: %f.", volume);

  int ret = player_set_volume(player_, volume, volume);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_volume failed: %s.",
              get_error_message(ret));
    return false;
  }
  return true;
}

bool MediaPlayer::SetPlaybackSpeed(double speed) {
  LOG_INFO("[MediaPlayer] speed: %f.", speed);

  int ret = player_set_playback_rate(player_, speed);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_playback_rate failed: %s.",
              get_error_message(ret));
    return false;
  }
  return true;
}

bool MediaPlayer::SeekTo(int64_t position, SeekCompletedCallback callback) {
  LOG_INFO("[MediaPlayer] position: %lld.", position);

  on_seek_completed_ = std::move(callback);
  int ret =
      player_set_play_position(player_, position, true, OnSeekCompleted, this);
  if (ret != PLAYER_ERROR_NONE) {
    on_seek_completed_ = nullptr;
    LOG_ERROR("[MediaPlayer] player_set_play_position failed: %s.",
              get_error_message(ret));
    return false;
  }
  return true;
}

int64_t MediaPlayer::GetPosition() {
  int position = 0;
  int ret = player_get_play_position(player_, &position);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_get_play_position failed: %s.",
              get_error_message(ret));
  }
  LOG_DEBUG("[MediaPlayer] Video current position : %d.", position);
  return position;
}

std::pair<int64_t, int64_t> MediaPlayer::GetDuration() {
  if (IsLive()) {
    return GetLiveDuration();
  } else {
    int duration = 0;
    int ret = player_get_duration(player_, &duration);
    if (ret != PLAYER_ERROR_NONE) {
      LOG_ERROR("[MediaPlayer] player_get_duration failed: %s.",
                get_error_message(ret));
    }
    LOG_INFO("[MediaPlayer] Video duration: %d.", duration);
    return std::make_pair(0, duration);
  }
}

void MediaPlayer::GetVideoSize(int32_t *width, int32_t *height) {
  int w = 0, h = 0;
  int ret = player_get_video_size(player_, &w, &h);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_get_video_size failed: %s.",
              get_error_message(ret));
  }
  LOG_INFO("[MediaPlayer] Video width: %d, height: %d.", w, h);

  player_display_rotation_e rotation = PLAYER_DISPLAY_ROTATION_NONE;
  ret = player_get_display_rotation(player_, &rotation);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_get_display_rotation failed: %s.",
              get_error_message(ret));
  }
  LOG_DEBUG("[MediaPlayer] Video rotation: %s.",
            RotationToString(rotation).c_str());
  if (rotation == PLAYER_DISPLAY_ROTATION_90 ||
      rotation == PLAYER_DISPLAY_ROTATION_270) {
    std::swap(w, h);
  }

  *width = w;
  *height = h;
}

bool MediaPlayer::IsReady() {
  player_state_e state = PLAYER_STATE_NONE;
  int ret = player_get_state(player_, &state);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_get_state failed: %s.",
              get_error_message(ret));
    return false;
  }

  LOG_INFO("[MediaPlayer] Player state : %d.", state);
  return PLAYER_STATE_READY == state;
}

bool MediaPlayer::SetDisplay() {
  void *native_window = GetWindowHandle();
  if (!native_window) {
    LOG_ERROR("[MediaPlayer] Could not get a native window handle.");
    return false;
  }

  int x = 0, y = 0, width = 0, height = 0;
  ecore_wl2_window_proxy_->ecore_wl2_window_geometry_get(native_window, &x, &y,
                                                         &width, &height);
  int ret = media_player_proxy_->player_set_ecore_wl_display(
      player_, PLAYER_DISPLAY_TYPE_OVERLAY, native_window, x, y, width, height);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_ecore_wl_display failed: %s.",
              get_error_message(ret));
    return false;
  }

  ret = player_set_display_mode(player_, PLAYER_DISPLAY_MODE_DST_ROI);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_display_mode failed: %s.",
              get_error_message(ret));
    return false;
  }
  return true;
}

bool MediaPlayer::IsLive() {
  int is_live = 0;
  int ret = media_player_proxy_->player_get_adaptive_streaming_info(
      player_, &is_live, PLAYER_ADAPTIVE_INFO_IS_LIVE);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_get_adaptive_streaming_info failed: %s",
              get_error_message(ret));
    return false;
  }
  return is_live != 0;
}

static std::vector<std::string> split(const std::string &s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> tokens;
  while (getline(ss, item, delim)) {
    tokens.push_back(item);
  }
  return tokens;
}

std::pair<int64_t, int64_t> MediaPlayer::GetLiveDuration() {
  std::string live_duration_str = "";
  char *live_duration_buff = static_cast<char *>(malloc(sizeof(char) * 64));
  memset(live_duration_buff, 0, sizeof(char) * 64);
  int ret = media_player_proxy_->player_get_adaptive_streaming_info(
      player_, (void *)&live_duration_buff, PLAYER_ADAPTIVE_INFO_LIVE_DURATION);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_get_adaptive_streaming_info failed: %s",
              get_error_message(ret));
    free(live_duration_buff);
    return std::make_pair(0, 0);
  }
  if (*live_duration_buff) {
    live_duration_str = std::string(live_duration_buff);
  }
  free(live_duration_buff);
  if (live_duration_str.empty()) {
    return std::make_pair(0, 0);
  }
  std::vector<std::string> time_vec = split(live_duration_str, '|');
  return std::make_pair(std::stoll(time_vec[0]), std::stoll(time_vec[1]));
}

flutter::EncodableList MediaPlayer::GetTrackInfo(std::string track_type) {
  player_state_e state = PLAYER_STATE_NONE;
  int ret = player_get_state(player_, &state);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_get_state failed: %s",
              get_error_message(ret));
    return {};
  }
  if (state == PLAYER_STATE_NONE || state == PLAYER_STATE_IDLE) {
    LOG_ERROR("[MediaPlayer] Player not ready.");
    return {};
  }

  player_stream_type_e type = ConvertTrackType(track_type);
  int track_count = 0;
  ret = media_player_proxy_->player_get_track_count_v2(player_, type,
                                                       &track_count);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_get_track_count_v2 failed: %s",
              get_error_message(ret));
    return {};
  }
  if (track_count <= 0) {
    return {};
  }

  flutter::EncodableList trackSelections = {};
  flutter::EncodableMap trackSelection = {};
  trackSelection.insert(
      {flutter::EncodableValue("trackType"), flutter::EncodableValue(type)});
  if (type == PLAYER_STREAM_TYPE_VIDEO) {
    LOG_INFO("[MediaPlayer] video_count: %d", track_count);

    for (int video_index = 0; video_index < track_count; video_index++) {
      player_video_track_info_v2 *video_track_info = nullptr;

      ret = media_player_proxy_->player_get_video_track_info_v2(
          player_, video_index, &video_track_info);
      if (ret != PLAYER_ERROR_NONE) {
        LOG_ERROR("[MediaPlayer] player_get_video_track_info_v2 failed: %s",
                  get_error_message(ret));
        return {};
      }
      LOG_INFO(
          "[MediaPlayer] video track info: width[%d], height[%d], "
          "bitrate[%d]",
          video_track_info->width, video_track_info->height,
          video_track_info->bit_rate);

      trackSelection.insert_or_assign(flutter::EncodableValue("trackId"),
                                      flutter::EncodableValue(video_index));
      trackSelection.insert_or_assign(
          flutter::EncodableValue("width"),
          flutter::EncodableValue(video_track_info->width));
      trackSelection.insert_or_assign(
          flutter::EncodableValue("height"),
          flutter::EncodableValue(video_track_info->height));
      trackSelection.insert_or_assign(
          flutter::EncodableValue("bitrate"),
          flutter::EncodableValue(video_track_info->bit_rate));

      trackSelections.push_back(flutter::EncodableValue(trackSelection));
    }

  } else if (type == PLAYER_STREAM_TYPE_AUDIO) {
    LOG_INFO("[MediaPlayer] audio_count: %d", track_count);

    for (int audio_index = 0; audio_index < track_count; audio_index++) {
      player_audio_track_info_v2 *audio_track_info = nullptr;

      ret = media_player_proxy_->player_get_audio_track_info_v2(
          player_, audio_index, &audio_track_info);
      if (ret != PLAYER_ERROR_NONE) {
        LOG_ERROR("[MediaPlayer] player_get_audio_track_info_v2 failed: %s",
                  get_error_message(ret));
        return {};
      }
      LOG_INFO(
          "[MediaPlayer] audio track info: language[%s], channel[%d], "
          "sample_rate[%d], bitrate[%d]",
          audio_track_info->language, audio_track_info->channel,
          audio_track_info->sample_rate, audio_track_info->bit_rate);

      trackSelection.insert_or_assign(flutter::EncodableValue("trackId"),
                                      flutter::EncodableValue(audio_index));
      trackSelection.insert_or_assign(
          flutter::EncodableValue("language"),
          flutter::EncodableValue(std::string(audio_track_info->language)));
      trackSelection.insert_or_assign(
          flutter::EncodableValue("channel"),
          flutter::EncodableValue(audio_track_info->channel));
      trackSelection.insert_or_assign(
          flutter::EncodableValue("bitrate"),
          flutter::EncodableValue(audio_track_info->bit_rate));

      trackSelections.push_back(flutter::EncodableValue(trackSelection));
    }

  } else if (type == PLAYER_STREAM_TYPE_TEXT) {
    LOG_INFO("[MediaPlayer] subtitle_count: %d", track_count);

    for (int sub_index = 0; sub_index < track_count; sub_index++) {
      player_subtitle_track_info_v2 *sub_track_info = nullptr;

      ret = media_player_proxy_->player_get_subtitle_track_info_v2(
          player_, sub_index, &sub_track_info);
      if (ret != PLAYER_ERROR_NONE) {
        LOG_ERROR("[MediaPlayer] player_get_subtitle_track_info_v2 failed: %s",
                  get_error_message(ret));
        return {};
      }
      LOG_INFO("[MediaPlayer] subtitle track info: language[%s]",
               sub_track_info->language);

      trackSelection.insert_or_assign(flutter::EncodableValue("trackId"),
                                      flutter::EncodableValue(sub_index));
      trackSelection.insert_or_assign(
          flutter::EncodableValue("language"),
          flutter::EncodableValue(std::string(sub_track_info->language)));

      trackSelections.push_back(flutter::EncodableValue(trackSelection));
    }
  }

  return trackSelections;
}

bool MediaPlayer::SetTrackSelection(int32_t track_id, std::string track_type) {
  LOG_INFO("[MediaPlayer] track_id: %d,track_type: %s", track_id,
           track_type.c_str());

  player_state_e state = PLAYER_STATE_NONE;
  int ret = player_get_state(player_, &state);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_get_state failed: %s",
              get_error_message(ret));
    return false;
  }
  if (state == PLAYER_STATE_NONE || state == PLAYER_STATE_IDLE) {
    LOG_ERROR("[MediaPlayer] Player not ready.");
    return false;
  }

  ret = player_select_track(player_, ConvertTrackType(track_type), track_id);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_select_track failed: %s",
              get_error_message(ret));
    return false;
  }

  return true;
}

bool MediaPlayer::SetDrm(const std::string &uri, int drm_type,
                         const std::string &license_server_url) {
  drm_manager_ = std::make_unique<DrmManager>();
  if (!drm_manager_->CreateDrmSession(drm_type, false)) {
    LOG_ERROR("[MediaPlayer] Failed to create drm session.");
    return false;
  }

  int drm_handle = 0;
  if (!drm_manager_->GetDrmHandle(&drm_handle)) {
    LOG_ERROR("[MediaPlayer] Failed to get drm handle.");
    return false;
  }

  int ret = media_player_proxy_->player_set_drm_handle(
      player_, PLAYER_DRM_TYPE_EME, drm_handle);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_drm_handle failed : %s.",
              get_error_message(ret));
    return false;
  }

  ret = media_player_proxy_->player_set_drm_init_complete_cb(
      player_, OnDrmSecurityInitComplete, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_drm_init_complete_cb failed : %s.",
              get_error_message(ret));
    return false;
  }

  ret = media_player_proxy_->player_set_drm_init_data_cb(
      player_, OnDrmUpdatePsshData, this);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_drm_init_complete_cb failed : %s.",
              get_error_message(ret));
    return false;
  }

  if (license_server_url.empty()) {
    bool success = drm_manager_->SetChallenge(uri, binary_messenger_);
    if (!success) {
      LOG_ERROR("[MediaPlayer] Failed to set challenge.");
      return false;
    }
  } else {
    if (!drm_manager_->SetChallenge(uri, license_server_url)) {
      LOG_ERROR("[MediaPlayer] Failed to set challenge.");
      return false;
    }
  }
  return true;
}

bool MediaPlayer::StopAndDestroy() {
  LOG_INFO("[MediaPlayer] StopAndDestroy is called.");
  if (!player_) {
    LOG_ERROR("[MediaPlayer] Player not created.");
    return false;
  }

  is_buffering_ = false;
  player_state_e player_state = PLAYER_STATE_NONE;
  int ret = player_get_state(player_, &player_state);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_get_state failed: %s.",
              get_error_message(ret));
    return false;
  }
  if (player_state == PLAYER_STATE_NONE || player_state == PLAYER_STATE_IDLE) {
    LOG_INFO("[MediaPlayer] Player already stop, nothing to do.");
    return true;
  }

  if (player_stop(player_) != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] Player fail to stop.");
    return false;
  }

  if (player_unprepare(player_) != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] Player fail to unprepare.");
    return false;
  }
  player_get_state(player_, &player_state);

  if (drm_manager_) {
    drm_manager_->ReleaseDrmSession();
    drm_manager_.reset();
  }

  if (player_destroy(player_) != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] Player fail to destroy.");
    return false;
  }
  player_ = nullptr;

  return true;
}

bool MediaPlayer::Suspend() {
  LOG_INFO("[MediaPlayer] Suspend is called.");
  if (!player_) {
    LOG_ERROR("[MediaPlayer] Player not created.");
    return false;
  }

  player_state_e player_state = PLAYER_STATE_NONE;
  int res = player_get_state(player_, &player_state);
  if (res != PLAYER_ERROR_NONE || player_state == PLAYER_STATE_NONE) {
    LOG_ERROR("[MediaPlayer] Player get state failed or in invalid state[%d].",
              player_state);
    return false;
  }

  pre_state_ = player_state;
  pre_playing_time_ = GetPosition();
  if (pre_playing_time_ < 0) {
    LOG_ERROR("[MediaPlayer] Get position failed.");
    return false;
  }
  LOG_INFO(
      "[MediaPlayer] Saved current player state: %d, playing time: %llu ms",
      pre_state_, pre_playing_time_);

  if (IsLive()) {
    pre_playing_time_ = 0;
    if (!StopAndDestroy()) {
      LOG_ERROR("[MediaPlayer] Player is live, StopAndDestroy fail.");
      return false;
    }
    LOG_INFO("[MediaPlayer] Player is live: close done successfully.");
    return true;
  }

  res = device_proxy_->device_power_get_state();
  if (res == POWER_STATE_STANDBY) {
    LOG_INFO("[MediaPlayer] Power state is standby.");
    if (!StopAndDestroy()) {
      LOG_ERROR("[MediaPlayer] Player StopAndDestroy fail.");
      return false;
    }
    LOG_INFO("[MediaPlayer] Standby state: close done successfully.");
    return true;
  } else {
    LOG_INFO("[MediaPlayer] Player state is not standby: %d, do nothing.", res);
  }

  if (player_state == PLAYER_STATE_IDLE) {
    if (!StopAndDestroy()) {
      LOG_ERROR("[MediaPlayer] Player StopAndDestroy fail.");
      return false;
    }
    LOG_INFO("[MediaPlayer] Player called in IDLE state, so stop the player.");
  } else if (player_state != PLAYER_STATE_PAUSED) {
    LOG_INFO("[MediaPlayer] Player calling pause from suspend.");
    if (!Pause()) {
      LOG_ERROR(
          "[MediaPlayer] Suspend fail, in restore player instance would be "
          "created newly.");
      if (!StopAndDestroy()) {
        LOG_ERROR("[MediaPlayer] Player StopAndDestroy fail.");
        return false;
      }
    }
  }

  return true;
}

bool MediaPlayer::Restore(const CreateMessage *restore_message,
                          int64_t resume_time) {
  LOG_INFO("[MediaPlayer] Restore is called.");

  player_state_e player_state = PLAYER_STATE_NONE;
  if (player_) {
    int ret = player_get_state(player_, &player_state);
    if (ret != PLAYER_ERROR_NONE || (player_state != PLAYER_STATE_PAUSED &&
                                     player_state != PLAYER_STATE_PLAYING)) {
      LOG_ERROR(
          "[MediaPlayer] Player get state failed or in invalid state[%d].",
          player_state);
      return false;
    }
  }

  if (restore_message->uri()) {
    LOG_INFO(
        "[MediaPlayer] Restore URL is not emptpy, close the existing "
        "instance.");
    if (player_ && !StopAndDestroy()) {
      LOG_ERROR("[MediaPlayer] Player StopAndDestroy fail.");
      return false;
    }
    return RestorePlayer(restore_message, resume_time);
  }

  switch (player_state) {
    case PLAYER_STATE_NONE:
      return RestorePlayer(restore_message, resume_time);
      break;
    case PLAYER_STATE_PAUSED:
      if (pre_state_ == PLAYER_STATE_PLAYING) {
        if (!Play()) {
          LOG_ERROR("[MediaPlayer] Player play failed.");
          return false;
        }
      }
      break;
    case PLAYER_STATE_PLAYING:
      // might be the case that widget has called
      // restore more than once, just ignore.
      break;
    default:
      LOG_INFO(
          "[MediaPlayer] Unhandled state, dont know how to process, just "
          "return "
          "false.");
      return false;
  }
  return true;
}

bool MediaPlayer::RestorePlayer(const CreateMessage *restore_message,
                                int64_t resume_time) {
  LOG_INFO("[MediaPlayer] RestorePlayer is called.");

  if (restore_message->uri()) {
    LOG_INFO("[MediaPlayer] Player previous url: %s", url_.c_str());
    LOG_INFO("[MediaPlayer] Player new url: %s",
             restore_message->uri()->c_str());
    url_ = *restore_message->uri();
    create_message_ = *restore_message;
  }

  LOG_INFO("[MediaPlayer] Player previous playing time: %llu ms",
           pre_playing_time_);
  LOG_INFO("[MediaPlayer] Player new resume time: %lld ms", resume_time);
  // resume_time < 0  ==> use previous playing time
  // resume_time == 0 ==> play from beginning
  // resume_time > 0  ==> play from resume_time(Third-party settings)
  if (resume_time >= 0) pre_playing_time_ = static_cast<uint64_t>(resume_time);

  is_restored_ = true;
  if (Create(url_, create_message_) < 0) {
    LOG_ERROR("[MediaPlayer] Fail to create player.");
    is_restored_ = false;
    return false;
  }

  return true;
}

bool MediaPlayer::SetDisplayRotate(int64_t rotation) {
  LOG_INFO("[MediaPlayer] rotation: %lld", rotation);
  int ret = player_set_display_rotation(
      player_, static_cast<player_display_rotation_e>(rotation));
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_display_rotation failed: %s",
              get_error_message(ret));
    return false;
  }
  return true;
}

void MediaPlayer::OnRestoreCompleted() {
  if (pre_playing_time_ <= 0 ||
      !SeekTo(pre_playing_time_, [this]() { SendRestored(); })) {
    SendRestored();
  }
}

void MediaPlayer::OnPrepared(void *user_data) {
  LOG_INFO("[MediaPlayer] Player prepared.");

  MediaPlayer *self = static_cast<MediaPlayer *>(user_data);
  if (!self->is_initialized_) {
    self->SendInitialized();
  }

  if (self->is_restored_) {
    self->OnRestoreCompleted();
  }
}

void MediaPlayer::OnBuffering(int percent, void *user_data) {
  LOG_INFO("[MediaPlayer] Buffering percent: %d.", percent);

  MediaPlayer *self = static_cast<MediaPlayer *>(user_data);
  if (percent == 100) {
    self->SendBufferingEnd();
    self->is_buffering_ = false;
  } else if (!self->is_buffering_ && percent <= 5) {
    self->SendBufferingStart();
    self->is_buffering_ = true;
  } else {
    self->SendBufferingUpdate(percent);
  }
}

void MediaPlayer::OnSeekCompleted(void *user_data) {
  LOG_INFO("[MediaPlayer] Seek completed.");

  MediaPlayer *self = static_cast<MediaPlayer *>(user_data);
  if (self->on_seek_completed_) {
    self->on_seek_completed_();
    self->on_seek_completed_ = nullptr;
  }
}

void MediaPlayer::OnPlayCompleted(void *user_data) {
  LOG_INFO("[MediaPlayer] Play completed.");

  MediaPlayer *self = static_cast<MediaPlayer *>(user_data);
  self->SendPlayCompleted();
  self->Pause();
}

void MediaPlayer::OnInterrupted(player_interrupted_code_e code,
                                void *user_data) {
  MediaPlayer *self = static_cast<MediaPlayer *>(user_data);
  self->SendIsPlayingState(false);
  LOG_ERROR("[MediaPlayer] Interrupt code: %d.", code);
}

void MediaPlayer::OnError(int error_code, void *user_data) {
  LOG_ERROR("An error occurred for media player, error: %d (%s).", error_code,
            get_error_message(error_code));

  MediaPlayer *self = static_cast<MediaPlayer *>(user_data);
  self->SendError("Media Player error", get_error_message(error_code));
}

void MediaPlayer::OnSubtitleUpdated(unsigned long duration, char *text,
                                    void *user_data) {
  LOG_INFO("[MediaPlayer] Subtitle updated, duration: %ld, text: %s.", duration,
           text);

  MediaPlayer *self = static_cast<MediaPlayer *>(user_data);
  self->SendSubtitleUpdate(duration, std::string(text));
}

bool MediaPlayer::OnDrmSecurityInitComplete(int *drm_handle,
                                            unsigned int length,
                                            unsigned char *pssh_data,
                                            void *user_data) {
  LOG_INFO("[MediaPlayer] Drm init completed.");

  MediaPlayer *self = static_cast<MediaPlayer *>(user_data);
  if (self->drm_manager_) {
    return self->drm_manager_->SecurityInitCompleteCB(drm_handle, length,
                                                      pssh_data, self->player_);
  }
  return false;
}

int MediaPlayer::OnDrmUpdatePsshData(drm_init_data_type init_type, void *data,
                                     int data_length, void *user_data) {
  LOG_INFO("[MediaPlayer] Drm update pssh data.");

  MediaPlayer *self = static_cast<MediaPlayer *>(user_data);
  if (self->drm_manager_) {
    return self->drm_manager_->UpdatePsshData(data, data_length);
  }
  return 0;
}
