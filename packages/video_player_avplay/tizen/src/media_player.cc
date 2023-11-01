// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media_player.h"

#include <dlfcn.h>

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
}

MediaPlayer::MediaPlayer(flutter::BinaryMessenger *messenger,
                         void *native_window)
    : VideoPlayer(messenger), native_window_(native_window) {
  media_player_proxy_ = std::make_unique<MediaPlayerProxy>();
}

MediaPlayer::~MediaPlayer() { Dispose(); }

int64_t MediaPlayer::Create(const std::string &uri, int drm_type,
                            const std::string &license_server_url,
                            bool is_prebuffer_mode,
                            flutter::EncodableMap &http_headers) {
  LOG_INFO("[MediaPlayer] uri: %s, drm_type: %d.", uri.c_str(), drm_type);

  if (uri.empty()) {
    LOG_ERROR("[MediaPlayer] The uri must not be empty.");
    return -1;
  }

  int ret = player_create(&player_);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_create failed: %s.",
              get_error_message(ret));
    return -1;
  }

  if (!http_headers.empty()) {
    auto iter = http_headers.find(flutter::EncodableValue("Cookie"));
    if (iter != http_headers.end()) {
      if (std::holds_alternative<std::string>(iter->second)) {
        std::string cookie = std::get<std::string>(iter->second);
        ret =
            player_set_streaming_cookie(player_, cookie.c_str(), cookie.size());
        if (ret != PLAYER_ERROR_NONE) {
          LOG_ERROR("[MediaPlayer] player_set_streaming_cookie failed: %s.",
                    get_error_message(ret));
        }
      }
    }

    iter = http_headers.find(flutter::EncodableValue("User-Agent"));
    if (iter != http_headers.end()) {
      if (std::holds_alternative<std::string>(iter->second)) {
        std::string user_agent = std::get<std::string>(iter->second);
        ret = player_set_streaming_user_agent(player_, user_agent.c_str(),
                                              user_agent.size());
        if (ret != PLAYER_ERROR_NONE) {
          LOG_ERROR("[MediaPlayer] player_set_streaming_user_agent failed: %s.",
                    get_error_message(ret));
        }
      }
    }
  }

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

  SetDisplayRoi(0, 0, 1, 1);

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

  if (player_) {
    if (is_initialized_) {
      player_unprepare(player_);
      is_initialized_ = false;
    }
    player_destroy(player_);
    player_ = nullptr;
  }

  // drm should be released after destroy of player
  if (drm_manager_) {
    drm_manager_->ReleaseDrmSession();
  }
}

void MediaPlayer::SetDisplayRoi(int32_t x, int32_t y, int32_t width,
                                int32_t height) {
  int ret = player_set_display_roi_area(player_, x, y, width, height);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_set_display_roi_area failed: %s.",
              get_error_message(ret));
  }
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
  LOG_INFO("[MediaPlayer] position: %d.", position);

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

int64_t MediaPlayer::GetDuration() {
  int duration = 0;
  int ret = player_get_duration(player_, &duration);
  if (ret != PLAYER_ERROR_NONE) {
    LOG_ERROR("[MediaPlayer] player_get_duration failed: %s.",
              get_error_message(ret));
  }
  LOG_INFO("[MediaPlayer] Video duration: %d.", duration);
  return duration;
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
  int x = 0, y = 0, width = 0, height = 0;
  ecore_wl2_window_proxy_->ecore_wl2_window_geometry_get(native_window_, &x, &y,
                                                         &width, &height);
  int ret = media_player_proxy_->player_set_ecore_wl_display(
      player_, PLAYER_DISPLAY_TYPE_OVERLAY, native_window_, x, y, width,
      height);
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

void MediaPlayer::OnPrepared(void *user_data) {
  LOG_INFO("[MediaPlayer] Player prepared.");

  MediaPlayer *self = static_cast<MediaPlayer *>(user_data);
  if (!self->is_initialized_) {
    self->SendInitialized();
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
  LOG_ERROR("[MediaPlayer] Interrupt code: %d.", code);

  MediaPlayer *self = static_cast<MediaPlayer *>(user_data);
  self->SendError("Interrupted error", "Media player has been interrupted.");
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
