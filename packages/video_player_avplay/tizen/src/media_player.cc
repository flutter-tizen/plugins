// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media_player.h"

#include <dlfcn.h>

#include <sstream>

#include "log.h"

#define CHECK_PLAYER_RESULT(statement, return_value)                           \
  {                                                                            \
    const auto result = (statement);                                           \
    if (result != PLAYER_ERROR_NONE) {                                         \
      LOG_ERROR("[MediaPlayer] Player error : %s", get_error_message(result)); \
      return return_value;                                                     \
    }                                                                          \
  }

#define CHECK_PLAYER_RESULT_NO_RETURN(statement)                               \
  {                                                                            \
    const auto result = (statement);                                           \
    if (result != PLAYER_ERROR_NONE) {                                         \
      LOG_ERROR("[MediaPlayer] Player error : %s", get_error_message(result)); \
    }                                                                          \
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

  CHECK_PLAYER_RESULT(player_create(&player_), -1);

  std::string cookie = flutter_common::GetValue(create_message.http_headers(),
                                                "Cookie", std::string());
  if (!cookie.empty()) {
    CHECK_PLAYER_RESULT_NO_RETURN(
        player_set_streaming_cookie(player_, cookie.c_str(), cookie.size()));
  }
  std::string user_agent = flutter_common::GetValue(
      create_message.http_headers(), "User-Agent", std::string());
  if (!user_agent.empty()) {
    CHECK_PLAYER_RESULT_NO_RETURN(player_set_streaming_user_agent(
        player_, user_agent.c_str(), user_agent.size()));
  }

  std::string adaptive_info = flutter_common::GetValue(
      create_message.streaming_property(), "ADAPTIVE_INFO", std::string());
  if (!adaptive_info.empty()) {
    CHECK_PLAYER_RESULT_NO_RETURN(
        media_player_proxy_->player_set_adaptive_streaming_info(
            player_,
            const_cast<void *>(
                reinterpret_cast<const void *>(adaptive_info.c_str())),
            PLAYER_ADAPTIVE_INFO_URL_CUSTOM));
  }

  int drm_type =
      flutter_common::GetValue(create_message.drm_configs(), "drmType", 0);
  std::string license_server_url = flutter_common::GetValue(
      create_message.drm_configs(), "licenseServerUrl", std::string());
  if (drm_type != 0) {
    if (!SetDrm(uri, drm_type, license_server_url)) {
      LOG_ERROR("[MediaPlayer] Fail to set drm.");
      return -1;
    }
  }

  if (!SetDisplay()) {
    LOG_ERROR("[MediaPlayer] Failed to set display.");
    return -1;
  }

  SetDisplayRoi(0, 0, 1, 1);

  CHECK_PLAYER_RESULT(player_set_uri(player_, uri.c_str()), -1);
  CHECK_PLAYER_RESULT(player_set_display_visible(player_, true), -1);
  CHECK_PLAYER_RESULT(player_set_buffering_cb(player_, OnBuffering, this), -1);
  CHECK_PLAYER_RESULT(player_set_completed_cb(player_, OnPlayCompleted, this),
                      -1);
  CHECK_PLAYER_RESULT(player_set_interrupted_cb(player_, OnInterrupted, this),
                      -1);
  CHECK_PLAYER_RESULT(player_set_error_cb(player_, OnError, this), -1);
  CHECK_PLAYER_RESULT(
      player_set_subtitle_updated_cb(player_, OnSubtitleUpdated, this), -1);
  CHECK_PLAYER_RESULT(player_prepare_async(player_, OnPrepared, this), -1);
  return SetUpEventChannel();
}

void MediaPlayer::Dispose() {
  LOG_INFO("[MediaPlayer] Disposing.");
  ClearUpEventChannel();
}

void MediaPlayer::SetDisplayRoi(int32_t x, int32_t y, int32_t width,
                                int32_t height) {
  CHECK_PLAYER_RESULT_NO_RETURN(
      player_set_display_roi_area(player_, x, y, width, height));
}

bool MediaPlayer::Play() {
  LOG_INFO("[MediaPlayer] Player starting.");

  player_state_e state = PLAYER_STATE_NONE;
  CHECK_PLAYER_RESULT(player_get_state(player_, &state), false);
  if (state == PLAYER_STATE_NONE || state == PLAYER_STATE_IDLE) {
    LOG_ERROR("[MediaPlayer] Player not ready.");
    return false;
  }
  if (state == PLAYER_STATE_PLAYING) {
    LOG_INFO("[MediaPlayer] Player already playing.");
    return false;
  }
  CHECK_PLAYER_RESULT(player_start(player_), false);
  return true;
}

bool MediaPlayer::Pause() {
  LOG_INFO("[MediaPlayer] Player pausing.");
  player_state_e state = PLAYER_STATE_NONE;
  CHECK_PLAYER_RESULT(player_get_state(player_, &state), false);
  if (state == PLAYER_STATE_NONE || state == PLAYER_STATE_IDLE) {
    LOG_ERROR("[MediaPlayer] Player not ready.");
    return false;
  }
  if (state != PLAYER_STATE_PLAYING) {
    LOG_INFO("[MediaPlayer] Player not playing.");
    return false;
  }
  CHECK_PLAYER_RESULT(player_pause(player_), false);
  return true;
}

bool MediaPlayer::SetLooping(bool is_looping) {
  LOG_INFO("[MediaPlayer] is_looping: %d.", is_looping);
  CHECK_PLAYER_RESULT(player_set_looping(player_, is_looping), false);
  return true;
}

bool MediaPlayer::SetVolume(double volume) {
  LOG_INFO("[MediaPlayer] volume: %f.", volume);
  CHECK_PLAYER_RESULT(player_set_volume(player_, volume, volume), false);
  return true;
}

bool MediaPlayer::SetPlaybackSpeed(double speed) {
  LOG_INFO("[MediaPlayer] speed: %f.", speed);
  CHECK_PLAYER_RESULT(player_set_playback_rate(player_, speed), false);
  return true;
}

bool MediaPlayer::SeekTo(int64_t position, SeekCompletedCallback callback) {
  LOG_INFO("[MediaPlayer] position: %lld.", position);
  on_seek_completed_ = std::move(callback);
  CHECK_PLAYER_RESULT(
      player_set_play_position(player_, position, true, OnSeekCompleted, this),
      false);
  return true;
}

int64_t MediaPlayer::GetPosition() {
  int position = 0;
  CHECK_PLAYER_RESULT_NO_RETURN(player_get_play_position(player_, &position));
  LOG_DEBUG("[MediaPlayer] Video current position : %d.", position);
  return position;
}

bool MediaPlayer::IsLive() {
  int is_live = 0;
  CHECK_PLAYER_RESULT(media_player_proxy_->player_get_adaptive_streaming_info(
                          player_, &is_live, PLAYER_ADAPTIVE_INFO_IS_LIVE),
                      false);
  return is_live != 0;
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

std::pair<int64_t, int64_t> MediaPlayer::GetDuration() {
  if (IsLive()) {
    return GetLiveDuration();
  } else {
    int duration = 0;
    CHECK_PLAYER_RESULT_NO_RETURN(player_get_duration(player_, &duration));
    LOG_INFO("[MediaPlayer] Video duration: %d.", duration);
    return std::make_pair(0, duration);
  }
}

void MediaPlayer::GetVideoSize(int32_t *width, int32_t *height) {
  int w = 0, h = 0;
  CHECK_PLAYER_RESULT_NO_RETURN(player_get_video_size(player_, &w, &h));
  LOG_INFO("[MediaPlayer] Video width: %d, height: %d.", w, h);
  player_display_rotation_e rotation = PLAYER_DISPLAY_ROTATION_NONE;
  CHECK_PLAYER_RESULT_NO_RETURN(
      player_get_display_rotation(player_, &rotation));
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
  CHECK_PLAYER_RESULT(player_get_state(player_, &state), false);
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

  CHECK_PLAYER_RESULT(media_player_proxy_->player_set_ecore_wl_display(
                          player_, PLAYER_DISPLAY_TYPE_OVERLAY, native_window,
                          x, y, width, height),
                      false);
  CHECK_PLAYER_RESULT(
      player_set_display_mode(player_, PLAYER_DISPLAY_MODE_DST_ROI), false);
  return true;
}

flutter::EncodableList MediaPlayer::GetTrackInfo(std::string track_type) {
  player_state_e state = PLAYER_STATE_NONE;
  CHECK_PLAYER_RESULT(player_get_state(player_, &state), {});
  if (state == PLAYER_STATE_NONE || state == PLAYER_STATE_IDLE) {
    LOG_ERROR("[MediaPlayer] Player not ready.");
    return {};
  }
  player_stream_type_e type = ConvertTrackType(track_type);
  int track_count = 0;
  CHECK_PLAYER_RESULT(media_player_proxy_->player_get_track_count_v2(
                          player_, type, &track_count),
                      {});
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
      CHECK_PLAYER_RESULT(media_player_proxy_->player_get_video_track_info_v2(
                              player_, video_index, &video_track_info),
                          {});
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
      CHECK_PLAYER_RESULT(media_player_proxy_->player_get_audio_track_info_v2(
                              player_, audio_index, &audio_track_info),
                          {});
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
      CHECK_PLAYER_RESULT(
          media_player_proxy_->player_get_subtitle_track_info_v2(
              player_, sub_index, &sub_track_info),
          {});
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
  CHECK_PLAYER_RESULT(player_get_state(player_, &state), false);
  if (state == PLAYER_STATE_NONE || state == PLAYER_STATE_IDLE) {
    LOG_ERROR("[MediaPlayer] Player not ready.");
    return false;
  }
  CHECK_PLAYER_RESULT(
      player_select_track(player_, ConvertTrackType(track_type), track_id),
      false);
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

  CHECK_PLAYER_RESULT(media_player_proxy_->player_set_drm_handle(
                          player_, PLAYER_DRM_TYPE_EME, drm_handle),
                      false);
  CHECK_PLAYER_RESULT(media_player_proxy_->player_set_drm_init_complete_cb(
                          player_, OnDrmSecurityInitComplete, this),
                      false);
  CHECK_PLAYER_RESULT(media_player_proxy_->player_set_drm_init_data_cb(
                          player_, OnDrmUpdatePsshData, this),
                      false);

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
