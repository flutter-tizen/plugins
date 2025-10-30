// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "plusplayer_platform.h"

#include <app_manager.h>
#include <system_info.h>

#include <sstream>
#include <unordered_set>

#include "log.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace video_player_avplay_tizen {

struct UserData {
  flutter::EncodableList *track_selections;
  plusplayer_track_type_e type;
  PlusPlayerPlatform *instance;
};

struct VideoSizeData {
  int32_t *w;
  int32_t *h;
  bool *found;
  PlusPlayerPlatform *instance;
};

struct ActiveTrackData {
  flutter::EncodableList *tracks;
  PlusPlayerPlatform *instance;
};

bool PlusPlayerPlatform::TrackInfoCb(const plusplayer_track_h track_h,
                                     void *user_data) {
  UserData *data = static_cast<UserData *>(user_data);
  plusplayer_track_type_e type;
  if (data->instance->plusplayer_capi_proxy_->plusplayer_capi_get_track_type(
          track_h, &type) != PLUSPLAYER_ERROR_TYPE_NONE) {
    return true;  // continue iteration
  }
  if (data->type == type) {
    if (type == PLUSPLAYER_TRACK_TYPE_VIDEO) {
      data->track_selections->push_back(
          data->instance->ParseVideoTrack(track_h));
    } else if (type == PLUSPLAYER_TRACK_TYPE_AUDIO) {
      data->track_selections->push_back(
          data->instance->ParseAudioTrack(track_h));
    } else if (type == PLUSPLAYER_TRACK_TYPE_SUBTITLE) {
      data->track_selections->push_back(
          data->instance->ParseSubtitleTrack(track_h));
    }
  }
  return true;
}

bool PlusPlayerPlatform::ActiveTrackInfoCb(const plusplayer_track_h track_h,
                                           void *user_data) {
  ActiveTrackData *data = static_cast<ActiveTrackData *>(user_data);
  plusplayer_track_type_e type;
  if (data->instance->plusplayer_capi_proxy_->plusplayer_capi_get_track_type(
          track_h, &type) != PLUSPLAYER_ERROR_TYPE_NONE) {
    return true;  // continue iteration
  }
  if (type == PLUSPLAYER_TRACK_TYPE_AUDIO) {
    data->tracks->push_back(data->instance->ParseAudioTrack(track_h));
  } else if (type == PLUSPLAYER_TRACK_TYPE_VIDEO) {
    data->tracks->push_back(data->instance->ParseVideoTrack(track_h));
  } else {
    data->tracks->push_back(data->instance->ParseSubtitleTrack(track_h));
  }
  return true;  // Continue iteration.
}

bool PlusPlayerPlatform::VideoSizeInfoCb(const plusplayer_track_h track_h,
                                         void *user_data) {
  VideoSizeData *data = static_cast<VideoSizeData *>(user_data);
  plusplayer_track_type_e type;
  if (data->instance->plusplayer_capi_proxy_->plusplayer_capi_get_track_type(
          track_h, &type) != PLUSPLAYER_ERROR_TYPE_NONE) {
    return true;  // continue iteration
  }
  if (type == PLUSPLAYER_TRACK_TYPE_VIDEO) {
    int w = 0, h = 0;
    data->instance->plusplayer_capi_proxy_->plusplayer_capi_get_track_width(
        track_h, &w);
    data->instance->plusplayer_capi_proxy_->plusplayer_capi_get_track_height(
        track_h, &h);
    *(data->w) = w;
    *(data->h) = h;
    *(data->found) = true;
    return false;  // stop iteration
  }
  return true;  // continue iteration
}

PlusPlayerPlatform::PlusPlayerPlatform(flutter::BinaryMessenger *messenger,
                                       FlutterDesktopViewRef flutter_view)
    : VideoPlayer(messenger, flutter_view) {
  memento_ = std::make_unique<PlayerMemento>();
  device_proxy_ = std::make_unique<DeviceProxy>();
  plusplayer_capi_proxy_ = std::make_unique<PlusPlayerCapiProxy>();
}

PlusPlayerPlatform::~PlusPlayerPlatform() {
  if (player_) {
    plusplayer_capi_proxy_->plusplayer_capi_stop(player_);
    plusplayer_capi_proxy_->plusplayer_capi_close(player_);
    UnRegisterCallback();
    plusplayer_capi_proxy_->plusplayer_capi_destroy(player_);
    player_ = nullptr;
  }

  if (drm_manager_) {
    drm_manager_->ReleaseDrmSession();
  }
}

void PlusPlayerPlatform::RegisterCallback() {
  plusplayer_capi_proxy_->plusplayer_capi_set_prepare_async_done_cb(
      player_, OnPrepareDone, this);
  plusplayer_capi_proxy_->plusplayer_capi_set_error_cb(player_, OnError, this);
  plusplayer_capi_proxy_->plusplayer_capi_set_error_msg_cb(player_, OnErrorMsg,
                                                           this);
  plusplayer_capi_proxy_->plusplayer_capi_set_resource_conflicted_cb(
      player_, OnResourceConflicted, this);
  plusplayer_capi_proxy_->plusplayer_capi_set_eos_cb(player_, OnEos, this);
  plusplayer_capi_proxy_->plusplayer_capi_set_seek_done_cb(player_, OnSeekDone,
                                                           this);
  plusplayer_capi_proxy_->plusplayer_capi_set_buffer_status_cb(
      player_, OnBufferStatus, this);
  plusplayer_capi_proxy_->plusplayer_capi_set_drm_init_data_cb(
      player_, OnDrmInitData, this);
  plusplayer_capi_proxy_
      ->plusplayer_capi_set_adaptive_streaming_control_event_cb(
          player_, OnAdaptiveStreamingControlEvent, this);
  plusplayer_capi_proxy_->plusplayer_capi_set_ad_event_cb(
      player_, OnADEventFromDash, this);
  plusplayer_capi_proxy_->plusplayer_capi_set_subtitle_updated_cb(
      player_, OnSubtitleData, this);
}

void PlusPlayerPlatform::UnRegisterCallback() {
  plusplayer_capi_proxy_->plusplayer_capi_set_buffer_status_cb(player_, nullptr,
                                                               this);
  plusplayer_capi_proxy_
      ->plusplayer_capi_set_adaptive_streaming_control_event_cb(player_,
                                                                nullptr, this);
  plusplayer_capi_proxy_->plusplayer_capi_set_eos_cb(player_, nullptr, this);
  plusplayer_capi_proxy_->plusplayer_capi_set_drm_init_data_cb(player_, nullptr,
                                                               this);
  plusplayer_capi_proxy_->plusplayer_capi_set_error_cb(player_, nullptr, this);
  plusplayer_capi_proxy_->plusplayer_capi_set_error_msg_cb(player_, nullptr,
                                                           this);
  plusplayer_capi_proxy_->plusplayer_capi_set_prepare_async_done_cb(
      player_, nullptr, this);
  plusplayer_capi_proxy_->plusplayer_capi_set_seek_done_cb(player_, nullptr,
                                                           this);
  plusplayer_capi_proxy_->plusplayer_capi_set_subtitle_updated_cb(
      player_, nullptr, this);
  plusplayer_capi_proxy_->plusplayer_capi_set_resource_conflicted_cb(
      player_, nullptr, this);
  plusplayer_capi_proxy_->plusplayer_capi_set_ad_event_cb(player_, nullptr,
                                                          this);
}

bool PlusPlayerPlatform::GetMemento(PlayerMemento *memento) {
  uint64_t playing_time;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_playing_time(
          player_, &playing_time) != PLUSPLAYER_ERROR_TYPE_NONE) {
    return false;
  }
  memento->playing_time = playing_time;
  memento->is_live = IsLive();
  memento->state = plusplayer_capi_proxy_->plusplayer_capi_get_state(player_);
  return true;
}

int64_t PlusPlayerPlatform::Create(const std::string &uri,
                                   const CreateMessage &create_message) {
  LOG_INFO("[PlusPlayerPlatform] Create player.");

  player_ = plusplayer_capi_proxy_->plusplayer_capi_create();

  if (!player_) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to create player.");
    return -1;
  }

  if (plusplayer_capi_proxy_->plusplayer_capi_open(player_, uri.c_str()) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to open uri :  %s.", uri.c_str());
    return -1;
  }

  url_ = uri;
  create_message_ = create_message;
  LOG_INFO("[PlusPlayerPlatform] Uri: %s", uri.c_str());

  if (!SetAppId()) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to get app id.");
    return -1;
  }

  RegisterCallback();

  int64_t drm_type = flutter_common::GetValue(create_message.drm_configs(),
                                              "drmType", (int64_t)0);
  std::string license_server_url = flutter_common::GetValue(
      create_message.drm_configs(), "licenseServerUrl", std::string());
  if (drm_type != 0) {
    if (!SetDrm(uri, drm_type, license_server_url)) {
      LOG_ERROR("[PlusPlayerPlatform] Fail to set drm.");
      return -1;
    }
  }

  if (!SetDisplay()) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to set display.");
    return -1;
  }

  SetDisplayRoi(0, 0, 1, 1);

  PreSet(create_message);

  if (plusplayer_capi_proxy_->plusplayer_capi_prepare_async(player_) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Player fail to prepare.");
    return -1;
  }
  return SetUpEventChannel();
}

void PlusPlayerPlatform::Dispose() {
  LOG_INFO("[PlusPlayerPlatform] Player disposing.");
  ClearUpEventChannel();
}

void PlusPlayerPlatform::PreSet(const CreateMessage &create_message) {
  if (create_message.streaming_property() != nullptr &&
      !create_message.streaming_property()->empty()) {
    for (const auto &[key, value] : *create_message.streaming_property()) {
      SetStreamingProperty(std::get<std::string>(key),
                           std::get<std::string>(value));
    }
  }

  std::string user_agent = flutter_common::GetValue(
      create_message.streaming_property(), "USER_AGENT", std::string());
  if (!user_agent.empty()) {
    if (plusplayer_capi_proxy_->plusplayer_capi_set_user_agent(
            player_, user_agent.c_str()) != PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR("[PlusPlayerPlatform] plusplayer_set_user_agent failed: %s.",
                user_agent.c_str());
    }
  }

  std::string cookie = flutter_common::GetValue(
      create_message.streaming_property(), "COOKIE", std::string());
  if (!cookie.empty()) {
    if (plusplayer_capi_proxy_->plusplayer_capi_set_cookie(
            player_, cookie.c_str()) != PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR("[PlusPlayerPlatform] plusplayer_set_cookie failed: %s.",
                cookie.c_str());
    }
  }

  bool is_prebuffer_mode = flutter_common::GetValue(
      create_message.player_options(), "prebufferMode", false);
  if (is_prebuffer_mode) {
    plusplayer_capi_proxy_->plusplayer_capi_set_prebuffer_mode(player_, true);
    is_prebuffer_mode_ = true;
  }

  int64_t start_position = flutter_common::GetValue(
      create_message.player_options(), "startPosition", (int64_t)0);
  if (start_position > 0) {
    LOG_INFO("[PlusPlayerPlatform] Start position: %lld", start_position);
    if (plusplayer_capi_proxy_->plusplayer_capi_seek(player_, start_position) !=
        PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_INFO(
          "[PlusPlayerPlatform] Fail to seek, it's a non-seekable content");
    }
  }
}

void PlusPlayerPlatform::SetDisplayRoi(int32_t x, int32_t y, int32_t width,
                                       int32_t height) {
  plusplayer_geometry_s roi{x, y, width, height};
  if (int ret = plusplayer_capi_proxy_->plusplayer_capi_set_display_roi(
                    player_, roi) != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Player fail to set display roi.");
    return;
  }
  memento_->display_area.x = x;
  memento_->display_area.y = y;
  memento_->display_area.width = width;
  memento_->display_area.height = height;
}

bool PlusPlayerPlatform::SetAppId() {
  char *appId;
  int ret = app_manager_get_app_id(getpid(), &appId);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to get app id: %s.",
              get_error_message(ret));
    return false;
  }

  if (plusplayer_capi_proxy_->plusplayer_capi_set_app_id(player_, appId) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to set app id");
    free(appId);
    return false;
  }
  free(appId);
  return true;
}

bool PlusPlayerPlatform::Play() {
  LOG_INFO("[PlusPlayerPlatform] Player starting.");

  plusplayer_state_e state =
      plusplayer_capi_proxy_->plusplayer_capi_get_state(player_);
  if (state < PLUSPLAYER_STATE_TRACK_SOURCE_READY) {
    LOG_ERROR("[PlusPlayerPlatform] Player is not ready.");
    return false;
  }

  if (state <= PLUSPLAYER_STATE_READY) {
    if (plusplayer_capi_proxy_->plusplayer_capi_start(player_) !=
        PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR("[PlusPlayerPlatform] Player fail to start.");
      return false;
    }
    SendIsPlayingState(true);
    return true;
  } else if (state == PLUSPLAYER_STATE_PAUSED) {
    if (plusplayer_capi_proxy_->plusplayer_capi_resume(player_) !=
        PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR("[PlusPlayerPlatform] Player fail to resume.");
      return false;
    }
    SendIsPlayingState(true);
    return true;
  }
  return false;
}

bool PlusPlayerPlatform::Activate() {
  // if (!::Activate(player_, plusplayer::kTrackTypeVideo)) {
  //   LOG_ERROR("[PlusPlayerPlatform] Fail to activate video.");
  //   return false;
  // }
  if (plusplayer_capi_proxy_->plusplayer_capi_activate_audio(player_) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to activate audio.");
    return false;
  }
  // if (!::Activate(player_, plusplayer::kTrackTypeSubtitle)) {
  //   LOG_ERROR("[PlusPlayerPlatform] Fail to activate subtitle.");
  // }

  return true;
}

bool PlusPlayerPlatform::Deactivate() {
  if (is_prebuffer_mode_) {
    plusplayer_capi_proxy_->plusplayer_capi_stop(player_);
    return true;
  }

  // if (!::Deactivate(player_, plusplayer::kTrackTypeVideo)) {
  //   LOG_ERROR("[PlusPlayerPlatform] Fail to deactivate video.");
  //   return false;
  // }
  if (plusplayer_capi_proxy_->plusplayer_capi_deactivate_audio(player_) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to deactivate audio.");
    return false;
  }
  // if (!::Deactivate(player_, plusplayer::kTrackTypeSubtitle)) {
  //   LOG_ERROR("[PlusPlayerPlatform] Fail to deactivate subtitle.");
  // }

  return true;
}

bool PlusPlayerPlatform::Pause() {
  LOG_INFO("[PlusPlayerPlatform] Player pausing.");

  plusplayer_state_e state =
      plusplayer_capi_proxy_->plusplayer_capi_get_state(player_);
  if (state < PLUSPLAYER_STATE_READY) {
    LOG_ERROR("[PlusPlayerPlatform] Player is not ready.");
    return false;
  }

  if (state != PLUSPLAYER_STATE_PLAYING) {
    LOG_INFO("[PlusPlayerPlatform] Player not playing.");
    return false;
  }

  if (plusplayer_capi_proxy_->plusplayer_capi_pause(player_) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Player fail to pause.");
    return false;
  }

  SendIsPlayingState(false);
  return true;
}

bool PlusPlayerPlatform::SetLooping(bool is_looping) {
  LOG_ERROR("[PlusPlayerPlatform] Not support to set looping.");
  return true;
}

bool PlusPlayerPlatform::SetVolume(double volume) {
  if (plusplayer_capi_proxy_->plusplayer_capi_get_state(player_) <
      PLUSPLAYER_STATE_PLAYING) {
    LOG_ERROR("[PlusPlayerPlatform] Player is in invalid state");
    return false;
  }
  // dart api volume range[0,1], plusplaer volume range[0,100]
  int new_volume = volume * 100;
  LOG_INFO("[PlusPlayerPlatform] Volume: %d", new_volume);
  // if (!::SetVolume(player_, new_volume)) {
  //   LOG_ERROR("[PlusPlayerPlatform] Fail to set volume.");
  //   return false;
  // }
  return true;
}

bool PlusPlayerPlatform::SetPlaybackSpeed(double speed) {
  LOG_INFO("[PlusPlayerPlatform] Speed: %f", speed);

  if (plusplayer_capi_proxy_->plusplayer_capi_get_state(player_) <=
      PLUSPLAYER_STATE_IDLE) {
    LOG_ERROR("[PlusPlayerPlatform] Player is not prepared.");
    return false;
  }
  if (plusplayer_capi_proxy_->plusplayer_capi_set_playback_rate(
          player_, speed) != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Player fail to set playback rate.");
    return false;
  }
  return true;
}

bool PlusPlayerPlatform::SeekTo(int64_t position,
                                SeekCompletedCallback callback) {
  LOG_INFO("[PlusPlayerPlatform] Seek to position: %lld", position);

  if (plusplayer_capi_proxy_->plusplayer_capi_get_state(player_) <
      PLUSPLAYER_STATE_READY) {
    LOG_ERROR("[PlusPlayerPlatform] Player is not ready.");
    return false;
  }

  if (on_seek_completed_) {
    LOG_ERROR("[PlusPlayerPlatform] Player is already seeking.");
    return false;
  }

  on_seek_completed_ = std::move(callback);
  if (plusplayer_capi_proxy_->plusplayer_capi_seek(player_, position) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    on_seek_completed_ = nullptr;
    LOG_ERROR("[PlusPlayerPlatform] Player fail to seek.");
    return false;
  }

  return true;
}

int64_t PlusPlayerPlatform::GetPosition() {
  uint64_t position = 0;
  plusplayer_state_e state =
      plusplayer_capi_proxy_->plusplayer_capi_get_state(player_);
  if (state == PLUSPLAYER_STATE_PLAYING || state == PLUSPLAYER_STATE_PAUSED) {
    if (plusplayer_capi_proxy_->plusplayer_capi_get_playing_time(
            player_, &position) != PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR(
          "[PlusPlayerPlatform] Player fail to get the current playing time.");
    }
  }
  return static_cast<int64_t>(position);
}

bool PlusPlayerPlatform::IsLive() {
  bool is_live = false;
  if (plusplayer_capi_proxy_->plusplayer_capi_is_live_streaming(
          player_, &is_live) != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Player fail to get is live.");
    return false;
  }
  return is_live;
}

std::pair<int64_t, int64_t> PlusPlayerPlatform::GetLiveDuration() {
  uint64_t start = 0;
  uint64_t end = 0;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_dvr_seekable_range(
          player_, &start, &end) != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Player fail to get live duration.");
    return std::make_pair(0, 0);
  }
  return std::make_pair(start, end);
}

std::pair<int64_t, int64_t> PlusPlayerPlatform::GetDuration() {
  if (IsLive()) {
    return GetLiveDuration();
  } else {
    int64_t duration = 0;
    if (plusplayer_capi_proxy_->plusplayer_capi_get_duration(
            player_, &duration) != PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR("[PlusPlayerPlatform] Player fail to get the duration.");
      return std::make_pair(0, 0);
    }
    return std::make_pair(0, duration);
  }
}

void PlusPlayerPlatform::GetVideoSize(int32_t *width, int32_t *height) {
  if (plusplayer_capi_proxy_->plusplayer_capi_get_state(player_) >=
      PLUSPLAYER_STATE_TRACK_SOURCE_READY) {
    VideoSizeData data{width, height, nullptr};
    bool found = false;
    data.found = &found;
    data.instance = this;

    if (plusplayer_capi_proxy_->plusplayer_capi_get_foreach_active_track(
            player_, VideoSizeInfoCb, &data) != PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR("[PlusPlayerPlatform] Player fail to get active track info.");
      return;
    }

    if (!found) {
      LOG_ERROR("[PlusPlayerPlatform] Player fail to get video size.");
    } else {
      LOG_INFO("[PlusPlayerPlatform] Video width: %d, height: %d.", *width,
               *height);
    }
  }
}

bool PlusPlayerPlatform::IsReady() {
  return PLUSPLAYER_STATE_READY ==
         plusplayer_capi_proxy_->plusplayer_capi_get_state(player_);
}

bool PlusPlayerPlatform::SetDisplay() {
  void *native_window = GetWindowHandle();
  if (!native_window) {
    LOG_ERROR("[PlusPlayerPlatform] Could not get a native window handle.");
    return false;
  }
  int x = 0, y = 0, width = 0, height = 0;
  ecore_wl2_window_proxy_->ecore_wl2_window_geometry_get(native_window, &x, &y,
                                                         &width, &height);
  uint32_t resource_id = FlutterDesktopViewGetResourceId(flutter_view_);
  if (resource_id == 0) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to get resource id.");
    return false;
  }

  plusplayer_geometry_s roi{x, y, width, height};
  if (plusplayer_capi_proxy_->plusplayer_capi_set_display_subsurface(
          player_, PLUSPLAYER_DISPLAY_TYPE_OVERLAY, resource_id, roi) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Player fail to set display.");
    return false;
  }

  if (plusplayer_capi_proxy_->plusplayer_capi_set_display_mode(
          player_, PLUSPLAYER_DISPLAY_MODE_DST_ROI) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Player fail to set display mode.");
    return false;
  }

  return true;
}

flutter::EncodableValue PlusPlayerPlatform::ParseVideoTrack(
    const plusplayer_track_h video_track) {
  flutter::EncodableMap video_track_result = {};
  video_track_result.insert_or_assign(flutter::EncodableValue("trackType"),
                                      flutter::EncodableValue("video"));
  int track_index = 0;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_track_index(
          video_track, &track_index) == PLUSPLAYER_ERROR_TYPE_NONE) {
    video_track_result.insert_or_assign(flutter::EncodableValue("trackId"),
                                        flutter::EncodableValue(track_index));
  }

  const char *track_mimetype;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_track_mimetype(
          video_track, &track_mimetype) == PLUSPLAYER_ERROR_TYPE_NONE) {
    video_track_result.insert_or_assign(
        flutter::EncodableValue("mimetype"),
        flutter::EncodableValue(track_mimetype));
  }

  int track_width = 0;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_track_width(
          video_track, &track_width) == PLUSPLAYER_ERROR_TYPE_NONE) {
    video_track_result.insert_or_assign(flutter::EncodableValue("width"),
                                        flutter::EncodableValue(track_width));
  }

  int track_height = 0;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_track_height(
          video_track, &track_height) == PLUSPLAYER_ERROR_TYPE_NONE) {
    video_track_result.insert_or_assign(flutter::EncodableValue("height"),
                                        flutter::EncodableValue(track_height));
  }

  int track_bitrate = 0;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_track_bitrate(
          video_track, &track_bitrate) == PLUSPLAYER_ERROR_TYPE_NONE) {
    video_track_result.insert_or_assign(flutter::EncodableValue("bitrate"),
                                        flutter::EncodableValue(track_bitrate));
  }
  LOG_DEBUG(
      "[PlusPlayerPlatform] video track info : trackId : %d, mimetype : %s, "
      "width : "
      "%d, height : %d, birate : %d",
      track_index, track_mimetype, track_width, track_height, track_bitrate);
  return flutter::EncodableValue(video_track_result);
}

flutter::EncodableValue PlusPlayerPlatform::ParseAudioTrack(
    const plusplayer_track_h audio_track) {
  flutter::EncodableMap audio_track_result = {};
  audio_track_result.insert_or_assign(flutter::EncodableValue("trackType"),
                                      flutter::EncodableValue("audio"));
  int track_index = 0;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_track_index(
          audio_track, &track_index) == PLUSPLAYER_ERROR_TYPE_NONE) {
    audio_track_result.insert_or_assign(flutter::EncodableValue("trackId"),
                                        flutter::EncodableValue(track_index));
  }

  const char *track_mimetype;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_track_mimetype(
          audio_track, &track_mimetype) == PLUSPLAYER_ERROR_TYPE_NONE) {
    audio_track_result.insert_or_assign(
        flutter::EncodableValue("mimetype"),
        flutter::EncodableValue(track_mimetype));
  }

  const char *track_lang_code;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_track_lang_code(
          audio_track, &track_lang_code) == PLUSPLAYER_ERROR_TYPE_NONE) {
    audio_track_result.insert_or_assign(
        flutter::EncodableValue("language"),
        flutter::EncodableValue(track_lang_code));
  }

  int track_channels = 0;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_track_channels(
          audio_track, &track_channels) == PLUSPLAYER_ERROR_TYPE_NONE) {
    audio_track_result.insert_or_assign(
        flutter::EncodableValue("channel"),
        flutter::EncodableValue(track_channels));
  }

  int track_bitrate = 0;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_track_bitrate(
          audio_track, &track_bitrate) == PLUSPLAYER_ERROR_TYPE_NONE) {
    audio_track_result.insert_or_assign(flutter::EncodableValue("bitrate"),
                                        flutter::EncodableValue(track_bitrate));
  }
  LOG_DEBUG(
      "[PlusPlayerPlatform] audio track info : trackId : %d, mimetype : %s, "
      "language_code : %s, channel : %d, bitrate : %d",
      track_index, track_mimetype, track_lang_code, track_channels,
      track_bitrate);
  return flutter::EncodableValue(audio_track_result);
}

flutter::EncodableValue PlusPlayerPlatform::ParseSubtitleTrack(
    const plusplayer_track_h subtitle_track) {
  flutter::EncodableMap subtitle_track_result = {};
  subtitle_track_result.insert_or_assign(flutter::EncodableValue("trackType"),
                                         flutter::EncodableValue("text"));
  int track_index = 0;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_track_index(
          subtitle_track, &track_index) == PLUSPLAYER_ERROR_TYPE_NONE) {
    subtitle_track_result.insert_or_assign(
        flutter::EncodableValue("trackId"),
        flutter::EncodableValue(track_index));
  }

  const char *track_mimetype;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_track_mimetype(
          subtitle_track, &track_mimetype) == PLUSPLAYER_ERROR_TYPE_NONE) {
    subtitle_track_result.insert_or_assign(
        flutter::EncodableValue("mimetype"),
        flutter::EncodableValue(track_mimetype));
  }

  const char *track_lang_code;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_track_lang_code(
          subtitle_track, &track_lang_code) == PLUSPLAYER_ERROR_TYPE_NONE) {
    subtitle_track_result.insert_or_assign(
        flutter::EncodableValue("language"),
        flutter::EncodableValue(track_lang_code));
  }

  LOG_DEBUG(
      "[PlusPlayerPlatform] subtitle track info : trackId : %d, mimetype : %s, "
      "language_code : %s",
      track_index, track_mimetype, track_lang_code);
  return flutter::EncodableValue(subtitle_track_result);
}

flutter::EncodableList PlusPlayerPlatform::GetTrackInfo(
    std::string track_type) {
  if (!player_) {
    LOG_ERROR("[PlusPlayerPlatform] Player not created.");
    return {};
  }

  plusplayer_state_e state =
      plusplayer_capi_proxy_->plusplayer_capi_get_state(player_);
  if (state < PLUSPLAYER_STATE_TRACK_SOURCE_READY) {
    LOG_ERROR("[PlusPlayerPlatform] Player is in invalid state.");
    return {};
  }

  plusplayer_track_type_e type = ConvertTrackType(track_type);

  int track_count = 0;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_track_count(
          player_, type, &track_count) != PLUSPLAYER_ERROR_TYPE_NONE ||
      track_count <= 0) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to get track count.");
    return {};
  }

  flutter::EncodableList trackSelections = {};
  UserData user_data = {};
  user_data.track_selections = &trackSelections;
  user_data.type = type;
  user_data.instance = this;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_foreach_track(
          player_, TrackInfoCb, &user_data) != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to get track info");
    return {};
  }

  return trackSelections;
}

flutter::EncodableList PlusPlayerPlatform::GetActiveTrackInfo() {
  if (!player_) {
    LOG_ERROR("[PlusPlayerPlatform] Player not created.");
    return {};
  }

  plusplayer_state_e state =
      plusplayer_capi_proxy_->plusplayer_capi_get_state(player_);
  if (state < PLUSPLAYER_STATE_TRACK_SOURCE_READY) {
    LOG_ERROR("[PlusPlayerPlatform] Player is in invalid state.");
    return {};
  }

  flutter::EncodableList active_tracks = {};
  ActiveTrackData active_data = {};
  active_data.tracks = &active_tracks;
  active_data.instance = this;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_foreach_active_track(
          player_, ActiveTrackInfoCb, &active_data) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to get active track info");
    return {};
  }
  return active_tracks;
}

bool PlusPlayerPlatform::SetTrackSelection(int32_t track_id,
                                           std::string track_type) {
  LOG_INFO("[PlusPlayerPlatform] Track id is: %d,track type is: %s", track_id,
           track_type.c_str());

  if (!player_) {
    LOG_ERROR("[PlusPlayerPlatform] Player not created.");
    return false;
  }

  plusplayer_state_e state =
      plusplayer_capi_proxy_->plusplayer_capi_get_state(player_);
  if (state < PLUSPLAYER_STATE_TRACK_SOURCE_READY) {
    LOG_ERROR("[PlusPlayerPlatform] Player is in invalid state.");
    return false;
  }

  if (plusplayer_capi_proxy_->plusplayer_capi_select_track(
          player_, ConvertTrackType(track_type), track_id) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Player fail to select track.");
    return false;
  }
  return true;
}

bool PlusPlayerPlatform::SetDrm(const std::string &uri, int drm_type,
                                const std::string &license_server_url) {
  drm_manager_ = std::make_unique<DrmManager>();
  if (!drm_manager_->CreateDrmSession(drm_type, true)) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to create drm session.");
    return false;
  }

  int drm_handle = 0;
  if (!drm_manager_->GetDrmHandle(&drm_handle)) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to get drm handle.");
    return false;
  }

  plusplayer_drm_type_e type;
  switch (drm_type) {
    case DrmManager::DrmType::DRM_TYPE_PLAYREADAY:
      type = PLUSPLAYER_DRM_TYPE_PLAYREADY;
      break;
    case DrmManager::DrmType::DRM_TYPE_WIDEVINECDM:
      type = PLUSPLAYER_DRM_TYPE_WIDEVINE_CDM;
      break;
    default:
      type = PLUSPLAYER_DRM_TYPE_NONE;
      break;
  }

  plusplayer_drm_property_s property;
  property.handle = drm_handle;
  property.type = type;
  property.license_acquired_cb = reinterpret_cast<void *>(OnLicenseAcquired);
  property.license_acquired_userdata = this;
  property.external_decryption = false;
  plusplayer_capi_proxy_->plusplayer_capi_set_drm(player_, property);

  if (license_server_url.empty()) {
    bool success = drm_manager_->SetChallenge(uri, binary_messenger_);
    if (!success) {
      LOG_ERROR("[PlusPlayerPlatform]Fail to set challenge.");
      return false;
    }
  } else {
    if (!drm_manager_->SetChallenge(uri, license_server_url)) {
      LOG_ERROR("[PlusPlayerPlatform]Fail to set challenge.");
      return false;
    }
  }
  return true;
}

std::string PlusPlayerPlatform::GetStreamingProperty(
    const std::string &streaming_property_type) {
  if (!player_) {
    LOG_ERROR("[PlusPlayerPlatform] Player not created.");
    return "";
  }
  plusplayer_state_e state =
      plusplayer_capi_proxy_->plusplayer_capi_get_state(player_);
  if (state == PLUSPLAYER_STATE_NONE || state == PLUSPLAYER_STATE_IDLE) {
    LOG_ERROR("[PlusPlayerPlatform] Player is in invalid state[%d]", state);
    return "";
  }

  plusplayer_property_e property = ConvertPropertyType(streaming_property_type);
  if (property == static_cast<plusplayer_property_e>(-1)) {
    return GetExtraStreamingProperty(streaming_property_type);
  }

  char *streaming_property_value;
  if (plusplayer_capi_proxy_->plusplayer_capi_get_property(
          player_, property, &streaming_property_value) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to get streaming property[%s].",
              streaming_property_type.c_str());
    return "";
  }
  std::string streaming_property_result(streaming_property_value);
  free(streaming_property_value);
  return streaming_property_result;
}

std::string PlusPlayerPlatform::GetExtraStreamingProperty(
    const std::string &streaming_property_type) {
  if (streaming_property_type == "IS_LIVE") {
    return IsLive() ? "true" : "false";
  }

  if (streaming_property_type == "CURRENT_BANDWIDTH") {
    uint32_t current_bandwidth = 0;
    plusplayer_capi_proxy_->plusplayer_capi_get_current_bandwidth(
        player_, &current_bandwidth);
    return std::to_string(current_bandwidth);
  }

  if (streaming_property_type == "GET_LIVE_DURATION") {
    uint64_t start = 0;
    uint64_t end = 0;
    plusplayer_capi_proxy_->plusplayer_capi_get_dvr_seekable_range(
        player_, &start, &end);
    return std::to_string(start) + "|" + std::to_string(end);
  }
  return "";
}

bool PlusPlayerPlatform::SetBufferConfig(const std::string &key,
                                         int64_t value) {
  if (!player_) {
    LOG_ERROR("[PlusPlayerPlatform] Player not created.");
    return false;
  }

  plusplayer_state_e state =
      plusplayer_capi_proxy_->plusplayer_capi_get_state(player_);
  if (state == PLUSPLAYER_STATE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform]:Player is in invalid state[%d]", state);
    return false;
  }

  return plusplayer_capi_proxy_->plusplayer_capi_set_buffer_config(
             player_, key.c_str(), value) == PLUSPLAYER_ERROR_TYPE_NONE;
}

void PlusPlayerPlatform::SetStreamingProperty(const std::string &type,
                                              const std::string &value) {
  if (!player_) {
    LOG_ERROR("[PlusPlayerPlatform] Player not created.");
    return;
  }

  plusplayer_state_e state =
      plusplayer_capi_proxy_->plusplayer_capi_get_state(player_);
  if (state == PLUSPLAYER_STATE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Player is in invalid state[%d]", state);
    return;
  }

  // For non-DASH formats, check if property type is supported
  if (!IsDashFormat() && IsDashOnlyProperty(type)) {
    LOG_ERROR(
        "[PlusPlayerPlatform] Only support streaming property type: %s for "
        "DASH!",
        type.c_str());
    return;
  }

  LOG_INFO("[PlusPlayerPlatform] SetStreamingProp: type[%s], value[%s]",
           type.c_str(), value.c_str());
  SetPropertyInternal(type, value);
}

bool PlusPlayerPlatform::SetDisplayRotate(int64_t rotation) {
  if (!player_) {
    LOG_ERROR("[PlusPlayerPlatform] Player not created.");
    return false;
  }

  plusplayer_state_e state =
      plusplayer_capi_proxy_->plusplayer_capi_get_state(player_);
  if (state == PLUSPLAYER_STATE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Player is in invalid state[%d]", state);
    return false;
  }

  LOG_INFO("[PlusPlayerPlatform] rotation: %lld", rotation);
  return plusplayer_capi_proxy_->plusplayer_capi_set_display_rotation(
             player_, ConvertDisplayRotationType(rotation)) ==
         PLUSPLAYER_ERROR_TYPE_NONE;
}

bool PlusPlayerPlatform::SetDisplayMode(int64_t display_mode) {
  if (!player_) {
    LOG_ERROR("[PlusPlayerPlatform] Player not created.");
    return false;
  }

  plusplayer_state_e state =
      plusplayer_capi_proxy_->plusplayer_capi_get_state(player_);
  if (state == PLUSPLAYER_STATE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Player is in invalid state[%d]", state);
    return false;
  }
  LOG_INFO("[PlusPlayerPlatform] display_mode: %lld", display_mode);
  plusplayer_display_mode_e mode = ConvertDisplayMode(display_mode);
  if (mode == static_cast<plusplayer_display_mode_e>(-1)) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to convert display mode.");
    return false;
  }
  return plusplayer_capi_proxy_->plusplayer_capi_set_display_mode(
             player_, mode) == PLUSPLAYER_ERROR_TYPE_NONE;
}

bool PlusPlayerPlatform::StopAndClose() {
  LOG_INFO("[PlusPlayerPlatform] StopAndClose is called.");
  if (!player_) {
    LOG_ERROR("[PlusPlayerPlatform] Player not created.");
    return false;
  }

  is_buffering_ = false;
  plusplayer_state_e player_state =
      plusplayer_capi_proxy_->plusplayer_capi_get_state(player_);
  if (player_state < PLUSPLAYER_STATE_READY) {
    LOG_INFO("[PlusPlayerPlatform] Player already stop, nothing to do.");
    return true;
  }

  if (plusplayer_capi_proxy_->plusplayer_capi_stop(player_) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Player fail to stop.");
    return false;
  }

  if (plusplayer_capi_proxy_->plusplayer_capi_close(player_) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Player fail to close.");
    return false;
  }

  if (drm_manager_) {
    drm_manager_->ReleaseDrmSession();
    drm_manager_.reset();
  }

  return true;
}

bool PlusPlayerPlatform::Suspend() {
  LOG_INFO("[PlusPlayerPlatform] Suspend is called.");

  if (!player_) {
    LOG_ERROR("[PlusPlayerPlatform] Player not created.");
    return false;
  }

  if (is_prebuffer_mode_) {
    LOG_ERROR("[PlusPlayerPlatform] Player is in prebuffer mode, do nothing.");
    return true;
  }

  memento_.reset(new PlayerMemento());
  if (!GetMemento(memento_.get())) {
    LOG_ERROR("[PlusPlayerPlatform] Player fail to get memento.");
    return false;
  }
  LOG_INFO(
      "[PlusPlayerPlatform] Memento saved current player state: %d, position: "
      "%llu ms, "
      "is_live: %d",
      (int)memento_->state, memento_->playing_time, memento_->is_live);

  if (memento_->is_live) {
    memento_->playing_time = 0;
    if (!StopAndClose()) {
      LOG_ERROR("[PlusPlayerPlatform] Player is live, StopAndClose fail.");
      return false;
    }
    LOG_INFO("[PlusPlayerPlatform] Player is live: close done successfully.");
    return true;
  }

  int power_state = device_proxy_->device_power_get_state();
  if (power_state == POWER_STATE_STANDBY) {
    LOG_INFO("[PlusPlayerPlatform] Power state is standby.");
    if (!StopAndClose()) {
      LOG_ERROR(
          "[PlusPlayerPlatform] Player need to stop and close, but failed.");
      return false;
    }
    LOG_INFO("[PlusPlayerPlatform] Standby state: close done successfully.");
    return true;
  } else {
    LOG_INFO(
        "[PlusPlayerPlatform] Player state is not standby: %d, do nothing.",
        power_state);
  }

  plusplayer_state_e player_state =
      plusplayer_capi_proxy_->plusplayer_capi_get_state(player_);
  if (player_state <= PLUSPLAYER_STATE_TRACK_SOURCE_READY) {
    if (plusplayer_capi_proxy_->plusplayer_capi_close(player_) !=
        PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR("[PlusPlayerPlatform] Player close fail.");
      return false;
    }
    LOG_INFO("[PlusPlayerPlatform] Player is in invalid state[%d], just close.",
             player_state);
    return true;
  } else if (player_state != PLUSPLAYER_STATE_PAUSED) {
    LOG_INFO("[PlusPlayerPlatform] Player calling pause from suspend.");
    if (plusplayer_capi_proxy_->plusplayer_capi_suspend(player_) !=
        PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR(
          "[PlusPlayerPlatform] Suspend fail, in restore player instance would "
          "be "
          "created newly.");
      if (!StopAndClose())
        LOG_ERROR(
            "[PlusPlayerPlatform] Suspend error, player stop and close fail.");
      return false;
    }
    SendIsPlayingState(false);
  }
  return true;
}

bool PlusPlayerPlatform::Restore(const CreateMessage *restore_message,
                                 int64_t resume_time) {
  LOG_INFO("[PlusPlayerPlatform] Restore is called.");
  if (!player_) {
    LOG_ERROR("[PlusPlayerPlatform] Player is not initialized.");
    return false;
  }

  plusplayer_state_e player_state =
      plusplayer_capi_proxy_->plusplayer_capi_get_state(player_);
  if (player_state != PLUSPLAYER_STATE_NONE &&
      player_state != PLUSPLAYER_STATE_PAUSED &&
      player_state != PLUSPLAYER_STATE_PLAYING) {
    LOG_ERROR("[PlusPlayerPlatform] Player is in invalid state[%d].",
              player_state);
    return false;
  }

  if (!memento_) {
    LOG_ERROR(
        "[PlusPlayerPlatform] No memento to restore. Player is in invalid "
        "state[%d]",
        player_state);
    return false;
  }

  if (is_prebuffer_mode_) {
    LOG_ERROR("[PlusPlayerPlatform] Player is in prebuffer mode, do nothing.");
    return true;
  }

  if (restore_message->uri()) {
    LOG_INFO(
        "[PlusPlayerPlatform] Restore URL is not emptpy, close the existing "
        "instance.");
    if (!StopAndClose()) {
      LOG_ERROR(
          "[PlusPlayerPlatform] Player need to stop and close, but failed.");
      return false;
    }
    return RestorePlayer(restore_message, resume_time);
  }

  switch (player_state) {
    case PLUSPLAYER_STATE_NONE:
      return RestorePlayer(restore_message, resume_time);
      break;
    case PLUSPLAYER_STATE_PAUSED:
      if (plusplayer_capi_proxy_->plusplayer_capi_restore(
              player_, memento_->state) != PLUSPLAYER_ERROR_TYPE_NONE) {
        if (!StopAndClose()) {
          LOG_ERROR(
              "[PlusPlayerPlatform] Player need to stop and close, but "
              "failed.");
          return false;
        }
        return RestorePlayer(restore_message, resume_time);
      }
      if (plusplayer_capi_proxy_->plusplayer_capi_get_state(player_) ==
          PLUSPLAYER_STATE_PLAYING) {
        SendIsPlayingState(true);
      }
      break;
    case PLUSPLAYER_STATE_PLAYING:
      // might be the case that widget has called
      // restore more than once, just ignore.
      break;
    default:
      LOG_INFO(
          "[PlusPlayerPlatform] Unhandled state, dont know how to process, "
          "just return "
          "false.");
      return false;
  }
  return true;
}

bool PlusPlayerPlatform::RestorePlayer(const CreateMessage *restore_message,
                                       int64_t resume_time) {
  LOG_INFO("[PlusPlayerPlatform] RestorePlayer is called.");
  LOG_INFO("[PlusPlayerPlatform] is_live: %d", memento_->is_live);

  if (restore_message->uri()) {
    LOG_INFO("[PlusPlayerPlatform] Player previous url: %s", url_.c_str());
    LOG_INFO("[PlusPlayerPlatform] Player new url: %s",
             restore_message->uri()->c_str());
    url_ = *restore_message->uri();
    create_message_ = *restore_message;
  }

  LOG_INFO("[PlusPlayerPlatform] Player previous playing time: %llu ms",
           memento_->playing_time);
  LOG_INFO("[PlusPlayerPlatform] Player new resume time: %lld ms", resume_time);
  // resume_time < 0  ==> use previous playing time
  // resume_time == 0 ==> play from beginning
  // resume_time > 0  ==> play from resume_time(Third-party settings)
  if (resume_time >= 0)
    memento_->playing_time = static_cast<uint64_t>(resume_time);

  is_restored_ = true;
  if (Create(url_, create_message_) < 0) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to create player.");
    is_restored_ = false;
    return false;
  }
  if (memento_->playing_time > 0 &&
      plusplayer_capi_proxy_->plusplayer_capi_seek(
          player_, memento_->playing_time) != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to seek.");
  }
  SetDisplayRoi(memento_->display_area.x, memento_->display_area.y,
                memento_->display_area.width, memento_->display_area.height);

  return true;
}

bool PlusPlayerPlatform::SetData(const flutter::EncodableMap &data) {
  if (!player_) {
    LOG_ERROR("[PlusPlayerPlatform] Player not created.");
    return false;
  }
  bool result = true;
  for (const auto &pair : data) {
    std::string key = std::get<std::string>(pair.first);
    std::string value;
    if (key == "max-bandwidth") {
      value = std::to_string(std::get<int64_t>(pair.second));
    } else {
      value = std::get<std::string>(pair.second);
    }
    if (plusplayer_capi_proxy_->plusplayer_capi_set_property(
            player_, ConvertPropertyType(key), value.c_str()) !=
        PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR("[PlusPlayerPlatform] Fail to set property, key : %s",
                key.c_str());
      result = false;
    }
  }

  return result;
}

flutter::EncodableMap PlusPlayerPlatform::GetData(
    const flutter::EncodableList &data) {
  flutter::EncodableMap result;
  if (!player_) {
    LOG_ERROR("[PlusPlayerPlatform] Player not created.");
    return result;
  }

  for (const auto &encodable_key : data) {
    std::string key = std::get<std::string>(encodable_key);
    char *value;
    if (plusplayer_capi_proxy_->plusplayer_capi_get_property(
            player_, ConvertPropertyType(key), &value) !=
        PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR("[PlusPlayerPlatform] Fail to get property, key : %s",
                key.c_str());
      continue;
    }
    if (key == "max-bandwidth") {
      result.insert_or_assign(encodable_key,
                              flutter::EncodableValue(std::stoll(value)));
    } else {
      result.insert_or_assign(encodable_key,
                              flutter::EncodableValue(std::string(value)));
    }
    free(value);
  }

  return result;
}

bool PlusPlayerPlatform::UpdateDashToken(const std::string &dashToken) {
  if (!player_) {
    LOG_ERROR("[PlusPlayerPlatform] Player not created.");
    return false;
  }

  return plusplayer_capi_proxy_->plusplayer_capi_set_property(
             player_, PLUSPLAYER_PROPERTY_URL_AUTH_TOKEN, dashToken.c_str()) ==
         PLUSPLAYER_ERROR_TYPE_NONE;
}

bool PlusPlayerPlatform::OnLicenseAcquired(int *drm_handle, unsigned int length,
                                           unsigned char *pssh_data,
                                           void *user_data) {
  LOG_INFO("[PlusPlayerPlatform] License acquired.");
  PlusPlayerPlatform *self = static_cast<PlusPlayerPlatform *>(user_data);

  if (self->drm_manager_) {
    return self->drm_manager_->SecurityInitCompleteCB(drm_handle, length,
                                                      pssh_data, nullptr);
  }
  return false;
}

void PlusPlayerPlatform::OnPrepareDone(bool ret, void *user_data) {
  PlusPlayerPlatform *self = reinterpret_cast<PlusPlayerPlatform *>(user_data);

  // if (!SetDisplayVisible(self->player_, true)) {
  //   LOG_ERROR("[PlusPlayerPlatform] Fail to set display visible.");
  // }

  if (!self->is_initialized_ && ret) {
    self->SendInitialized();
  }

  if (self->is_restored_ && ret) {
    self->SendRestored();
  }
}

void PlusPlayerPlatform::OnBufferStatus(int percent, void *user_data) {
  LOG_INFO("[PlusPlayerPlatform] Buffering percent: %d.", percent);
  PlusPlayerPlatform *self = reinterpret_cast<PlusPlayerPlatform *>(user_data);

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

void PlusPlayerPlatform::OnSeekDone(void *user_data) {
  LOG_INFO("[PlusPlayerPlatform] Seek completed.");
  PlusPlayerPlatform *self = reinterpret_cast<PlusPlayerPlatform *>(user_data);

  if (self->on_seek_completed_) {
    self->on_seek_completed_();
    self->on_seek_completed_ = nullptr;
  }
}

void PlusPlayerPlatform::OnEos(void *user_data) {
  LOG_INFO("[PlusPlayerPlatform] Play completed.");
  PlusPlayerPlatform *self = reinterpret_cast<PlusPlayerPlatform *>(user_data);

  self->SendPlayCompleted();
}

void PlusPlayerPlatform::OnSubtitleData(const plusplayer_subtitle_type_e type,
                                        const uint64_t duration_in_ms,
                                        const char *data, const int size,
                                        plusplayer_subtitle_attr_s *attr_list,
                                        int attr_size, void *user_data) {
  LOG_INFO("[PlusPlayerPlatform] Subtitle updated, duration: %llu, text: %s",
           duration_in_ms, data);
  PlusPlayerPlatform *self = reinterpret_cast<PlusPlayerPlatform *>(user_data);

  flutter::EncodableList attributes_list;
  for (int i = 0; i < attr_size; i++) {
    LOG_INFO(
        "[PlusPlayerPlatform] Subtitle update: type: %d, start: %u, end: %u",
        attr_list[i].attr, attr_list[i].start_time, attr_list[i].stop_time);
    flutter::EncodableMap attributes = {
        {flutter::EncodableValue("attrType"),
         flutter::EncodableValue(attr_list[i].attr)},
        {flutter::EncodableValue("startTime"),
         flutter::EncodableValue((int64_t)attr_list[i].start_time)},
        {flutter::EncodableValue("stopTime"),
         flutter::EncodableValue((int64_t)attr_list[i].stop_time)},
    };

    switch (attr_list[i].dtype) {
      // case PLUSPLAYER_SUBTITLE_ATTR_REGION_XPOS:
      // case PLUSPLAYER_SUBTITLE_ATTR_REGION_YPOS:
      // case PLUSPLAYER_SUBTITLE_ATTR_REGION_WIDTH:
      // case PLUSPLAYER_SUBTITLE_ATTR_REGION_HEIGHT:
      // case PLUSPLAYER_SUBTITLE_ATTR_WINDOW_XPADDING:
      // case PLUSPLAYER_SUBTITLE_ATTR_WINDOW_YPADDING:
      // case PLUSPLAYER_SUBTITLE_ATTR_WINDOW_OPACITY:
      // case PLUSPLAYER_SUBTITLE_ATTR_FONT_SIZE:
      // case PLUSPLAYER_SUBTITLE_ATTR_FONT_OPACITY:
      // case PLUSPLAYER_SUBTITLE_ATTR_FONT_BG_OPACITY:
      // case PLUSPLAYER_SUBTITLE_ATTR_WEBVTT_CUE_LINE:
      // case PLUSPLAYER_SUBTITLE_ATTR_WEBVTT_CUE_SIZE:
      // case PLUSPLAYER_SUBTITLE_ATTR_WEBVTT_CUE_POSITION:
      case PLUSPLAYER_SUBTITLE_ATTR_DATA_TYPE_FLOAT: {
        float value_float = attr_list[i].value.float_value;
        LOG_INFO("[PlusPlayerPlatform] Subtitle update: value<float>: %f",
                 value_float);
        attributes[flutter::EncodableValue("attrValue")] =
            flutter::EncodableValue((double)value_float);
      } break;
      case PLUSPLAYER_SUBTITLE_ATTR_DATA_TYPE_DOUBLE: {
        double value_double = attr_list[i].value.double_value;
        LOG_INFO("[PlusPlayerPlatform] Subtitle update: value<double>: %f",
                 value_double);
        attributes[flutter::EncodableValue("attrValue")] =
            flutter::EncodableValue(value_double);
      } break;
      // case PLUSPLAYER_SUBTITLE_ATTR_FONT_FAMILY:
      // case PLUSPLAYER_SUBTITLE_ATTR_RAW_SUBTITLE:
      case PLUSPLAYER_SUBTITLE_ATTR_DATA_TYPE_STRING: {
        const char *value_chars = attr_list[i].value.str_value;
        LOG_INFO("[PlusPlayerPlatform] Subtitle update: value<char*>: %s",
                 value_chars);
        std::string value_str(value_chars);
        attributes[flutter::EncodableValue("attrValue")] =
            flutter::EncodableValue(value_str);
      } break;
      // case PLUSPLAYER_SUBTITLE_ATTR_WINDOW_LEFT_MARGIN:
      // case PLUSPLAYER_SUBTITLE_ATTR_WINDOW_RIGHT_MARGIN:
      // case PLUSPLAYER_SUBTITLE_ATTR_WINDOW_TOP_MARGIN:
      // case PLUSPLAYER_SUBTITLE_ATTR_WINDOW_BOTTOM_MARGIN:
      // case PLUSPLAYER_SUBTITLE_ATTR_WINDOW_BG_COLOR:
      // case PLUSPLAYER_SUBTITLE_ATTR_FONT_WEIGHT:
      // case PLUSPLAYER_SUBTITLE_ATTR_FONT_STYLE:
      // case PLUSPLAYER_SUBTITLE_ATTR_FONT_COLOR:
      // case PLUSPLAYER_SUBTITLE_ATTR_FONT_BG_COLOR:
      // case PLUSPLAYER_SUBTITLE_ATTR_FONT_TEXT_OUTLINE_COLOR:
      // case PLUSPLAYER_SUBTITLE_ATTR_FONT_TEXT_OUTLINE_THICKNESS:
      // case PLUSPLAYER_SUBTITLE_ATTR_FONT_TEXT_OUTLINE_BLUR_RADIUS:
      // case PLUSPLAYER_SUBTITLE_ATTR_FONT_VERTICAL_ALIGN:
      // case PLUSPLAYER_SUBTITLE_ATTR_FONT_HORIZONTAL_ALIGN:
      // case LUSPLAYER_SUBTITLE_ATTR_WEBVTT_CUE_LINE_NUM:
      // case LUSPLAYER_SUBTITLE_ATTR_WEBVTT_CUE_LINE_ALIGN:
      // case LUSPLAYER_SUBTITLE_ATTR_WEBVTT_CUE_ALIGN:
      // case PLUSPLAYER_SUBTITLE_ATTR_WEBVTT_CUE_POSITION_ALIGN:
      // case PLUSPLAYER_SUBTITLE_ATTR_WEBVTT_CUE_VERTICAL:
      // case PLUSPLAYER_SUBTITLE_ATTR_TIMESTAMP:
      case PLUSPLAYER_SUBTITLE_ATTR_DATA_TYPE_INT: {
        int value_int = attr_list[i].value.int32_value;
        LOG_INFO("[PlusPlayerPlatform] Subtitle update: value<int>: %d",
                 value_int);
        attributes[flutter::EncodableValue("attrValue")] =
            flutter::EncodableValue(value_int);
      } break;
      // case PLUSPLAYER_SUBTITLE_ATTR_WINDOW_SHOW_BG:
      case PLUSPLAYER_SUBTITLE_ATTR_DATA_TYPE_UINT: {
        uint32_t value_uint32 = attr_list[i].value.uint32_value;
        LOG_INFO("[PlusPlayerPlatform] Subtitle update: value<uint32_t>: %u",
                 value_uint32);
        attributes[flutter::EncodableValue("attrValue")] =
            flutter::EncodableValue((int64_t)value_uint32);
      } break;
      case PLUSPLAYER_SUBTITLE_ATTR_DATA_TYPE_INT64: {
        int64_t value_int64 = attr_list[i].value.int64_value;
        LOG_INFO("[PlusPlayerPlatform] Subtitle update: value<int64_t>: %lld",
                 value_int64);
        attributes[flutter::EncodableValue("attrValue")] =
            flutter::EncodableValue(value_int64);
      } break;
      case PLUSPLAYER_SUBTITLE_ATTR_DATA_TYPE_UINT64: {
        uint64_t value_uint64 = attr_list[i].value.uint64_value;
        LOG_INFO("[PlusPlayerPlatform] Subtitle update: value<uint64_t>: %lld",
                 value_uint64);
        attributes[flutter::EncodableValue("attrValue")] =
            flutter::EncodableValue((int64_t)value_uint64);
      } break;
      default:
        LOG_ERROR("[PlusPlayerPlatform] Unknown Subtitle value type: %d",
                  attr_list[i].dtype);
        break;
    }
    attributes_list.push_back(flutter::EncodableValue(attributes));
  }
  self->SendSubtitleUpdate(duration_in_ms, data, attributes_list);
}

void PlusPlayerPlatform::OnResourceConflicted(void *user_data) {
  LOG_ERROR("[PlusPlayerPlatform] Resource conflicted.");
  PlusPlayerPlatform *self = reinterpret_cast<PlusPlayerPlatform *>(user_data);

  self->SendIsPlayingState(false);
}

void PlusPlayerPlatform::OnError(plusplayer_error_type_e error_type,
                                 void *user_data) {
  LOG_ERROR("[PlusPlayerPlatform] Error code: %d", error_type);
  PlusPlayerPlatform *self = reinterpret_cast<PlusPlayerPlatform *>(user_data);

  self->SendError("[PlusPlayerPlatform] error",
                  std::string("Error: ") + GetErrorMessage(error_type));
}

void PlusPlayerPlatform::OnErrorMsg(plusplayer_error_type_e error_type,
                                    const char *error_msg, void *user_data) {
  LOG_ERROR("[PlusPlayerPlatform] Error code: %d, message: %s.", error_type,
            error_msg);
  PlusPlayerPlatform *self = reinterpret_cast<PlusPlayerPlatform *>(user_data);

  self->SendError("PlusPlayerPlatform error",
                  std::string("Error: ") + error_msg);
}

void PlusPlayerPlatform::OnDrmInitData(Plusplayer_DrmHandle *drm_handle,
                                       unsigned int len,
                                       unsigned char *pssh_data,
                                       plusplayer_track_type_e type,
                                       void *user_data) {
  LOG_INFO("[PlusPlayerPlatform] Drm init completed.");
  PlusPlayerPlatform *self = reinterpret_cast<PlusPlayerPlatform *>(user_data);

  if (self->drm_manager_) {
    if (self->drm_manager_->SecurityInitCompleteCB(drm_handle, len, pssh_data,
                                                   nullptr)) {
      self->plusplayer_capi_proxy_->plusplayer_capi_drm_license_acquired_done(
          self->player_, type);
    }
  }
}

void PlusPlayerPlatform::OnAdaptiveStreamingControlEvent(
    plusplayer_streaming_message_type_e type, plusplayer_message_param_s *msg,
    void *user_data) {
  LOG_INFO("[PlusPlayerPlatform] Message type: %d, is DrmInitData (%d)", type,
           type == PLUSPLAYER_STREAMING_MESSAGE_TYPE_DRMINITDATA);
  PlusPlayerPlatform *self = reinterpret_cast<PlusPlayerPlatform *>(user_data);

  if (type == PLUSPLAYER_STREAMING_MESSAGE_TYPE_DRMINITDATA) {
    if (!msg || 0 == msg->size) {
      LOG_ERROR("[PlusPlayerPlatform] Empty message.");
      return;
    }

    if (self->drm_manager_) {
      self->drm_manager_->UpdatePsshData(msg->data, msg->size);
    }
  }
}

void PlusPlayerPlatform::OnADEventFromDash(const char *ad_data,
                                           void *user_data) {
  if (!ad_data) {
    LOG_ERROR("[PlusPlayerPlatform] No ad_data.");
    return;
  }

  const char *prefix = "AD_INFO: ";
  const char *data = strstr(ad_data, prefix);
  if (!data) {
    LOG_ERROR("[PlusPlayerPlatform] Invalid ad_data.");
    return;
  }
  data += strlen(prefix);
  const_cast<char *>(data)[strlen(data) - 1] = '\0';
  LOG_INFO("[PlusPlayerPlatform] AD info: %s", data);

  rapidjson::Document doc;
  doc.Parse(data);
  if (doc.HasParseError()) {
    LOG_ERROR("[PlusPlayerPlatform] Fail to parse ad_data: %d.",
              doc.GetParseError());
    return;
  }

  flutter::EncodableMap ad_info = {};
  if (doc.HasMember("event") && doc["event"].IsObject()) {
    const rapidjson::Value &event = doc["event"];

    if (event.HasMember("data") && event["data"].IsObject()) {
      const rapidjson::Value &data = event["data"];

      if (data.HasMember("id") && data["id"].IsInt64()) {
        ad_info.insert_or_assign(
            flutter::EncodableValue("id"),
            flutter::EncodableValue(data["id"].GetInt64()));
      }

      if (data.HasMember("cancel_indicator") &&
          data["cancel_indicator"].IsBool()) {
        ad_info.insert_or_assign(
            flutter::EncodableValue("cancel_indicator"),
            flutter::EncodableValue(data["cancel_indicator"].GetBool()));
      }

      if (data.HasMember("start_ms") && data["start_ms"].IsInt64()) {
        ad_info.insert_or_assign(
            flutter::EncodableValue("start_ms"),
            flutter::EncodableValue(data["start_ms"].GetInt64()));
      }

      if (data.HasMember("duration_ms") && data["duration_ms"].IsInt64()) {
        ad_info.insert_or_assign(
            flutter::EncodableValue("duration_ms"),
            flutter::EncodableValue(data["duration_ms"].GetInt64()));
      }

      if (data.HasMember("end_ms") && data["end_ms"].IsInt64()) {
        ad_info.insert_or_assign(
            flutter::EncodableValue("end_ms"),
            flutter::EncodableValue(data["end_ms"].GetInt64()));
      }

      if (data.HasMember("out_of_network_indicator") &&
          data["out_of_network_indicator"].IsInt64()) {
        ad_info.insert_or_assign(
            flutter::EncodableValue("out_of_network_indicator"),
            flutter::EncodableValue(
                data["out_of_network_indicator"].GetInt64()));
      }
    }

    PlusPlayerPlatform *self =
        reinterpret_cast<PlusPlayerPlatform *>(user_data);
    self->SendADFromDash(ad_info);
  }
}

bool PlusPlayerPlatform::IsDashFormat() const {
  return create_message_.format_hint() &&
         !create_message_.format_hint()->empty() &&
         *create_message_.format_hint() == "dash";
}

bool PlusPlayerPlatform::IsDashOnlyProperty(const std::string &type) const {
  static const std::unordered_set<std::string> dash_only_properties = {
      "OPEN_HTTP_HEADER", "TOKEN", "UNWANTED_FRAMERATE", "UNWANTED_RESOLUTION",
      "UPDATE_SAME_LANGUAGE_CODE"};
  return dash_only_properties.find(type) != dash_only_properties.end();
}

void PlusPlayerPlatform::SetPropertyInternal(const std::string &type,
                                             const std::string &value) {
  plusplayer_property_e property = ConvertPropertyType(type);
  if (property != static_cast<plusplayer_property_e>(-1)) {
    if (plusplayer_capi_proxy_->plusplayer_capi_set_property(
            player_, property, value.c_str()) != PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR("[PlusPlayer] Fail to set streaming property type: %s.",
                type.c_str());
    }
  }
}

}  // namespace video_player_avplay_tizen
