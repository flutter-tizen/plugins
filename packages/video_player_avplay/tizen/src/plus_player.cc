// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "plus_player.h"

#include <app_manager.h>
#include <system_info.h>

#include <sstream>
#include <unordered_map>

#include "log.h"
#include "plus_player_util.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace video_player_avplay_tizen {

PlusPlayer::PlusPlayer(flutter::BinaryMessenger *messenger,
                       FlutterDesktopViewRef flutter_view)
    : VideoPlayer(messenger, flutter_view) {
  memento_ = std::make_unique<PlayerMemento>();
  device_proxy_ = std::make_unique<DeviceProxy>();
}

PlusPlayer::~PlusPlayer() {
  if (player_) {
    plusplayer_stop(player_);
    plusplayer_close(player_);
    UnregisterListener();
    plusplayer_destroy(player_);
    player_ = nullptr;
  }

  if (drm_manager_) {
    drm_manager_->ReleaseDrmSession();
  }
}

void PlusPlayer::UnregisterListener() {
  plusplayer_set_buffer_status_cb(player_, nullptr, this);
  plusplayer_set_adaptive_streaming_control_event_cb(player_, nullptr, this);
  plusplayer_set_eos_cb(player_, nullptr, this);
  plusplayer_set_drm_init_data_cb(player_, nullptr, this);
  plusplayer_set_error_cb(player_, nullptr, this);
  plusplayer_set_error_msg_cb(player_, nullptr, this);
  plusplayer_set_prepare_async_done_cb(player_, nullptr, this);
  plusplayer_set_seek_done_cb(player_, nullptr, this);
  plusplayer_set_subtitle_updated_cb(player_, nullptr, this);
  plusplayer_set_resource_conflicted_cb(player_, nullptr, this);
  plusplayer_set_ad_event_cb(player_, nullptr, this);
}

void PlusPlayer::RegisterListener() {
  plusplayer_set_buffer_status_cb(player_, OnBufferStatus, this);
  plusplayer_set_adaptive_streaming_control_event_cb(
      player_, OnAdaptiveStreamingControlEvent, this);
  plusplayer_set_eos_cb(player_, OnEos, this);
  plusplayer_set_drm_init_data_cb(player_, OnDrmInitData, this);
  plusplayer_set_error_cb(player_, OnError, this);
  plusplayer_set_error_msg_cb(player_, OnErrorMsg, this);
  plusplayer_set_prepare_async_done_cb(player_, OnPrepareDone, this);
  plusplayer_set_seek_done_cb(player_, OnSeekDone, this);
  plusplayer_set_subtitle_updated_cb(player_, OnSubtitleData, this);
  plusplayer_set_resource_conflicted_cb(player_, OnResourceConflicted, this);
  plusplayer_set_ad_event_cb(player_, OnADEventFromDash, this);
}

int64_t PlusPlayer::Create(const std::string &uri,
                           const CreateMessage &create_message) {
  LOG_INFO("[PlusPlayer] Create player.");
  player_ = plusplayer_create();

  if (!player_) {
    LOG_ERROR("[PlusPlayer] Fail to create player.");
    return -1;
  }

  if (plusplayer_open(player_, uri.c_str()) != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayer] Fail to open uri :  %s.", uri.c_str());
    return -1;
  }

  url_ = uri;
  create_message_ = create_message;
  LOG_INFO("[PlusPlayer] Uri: %s", uri.c_str());

  if (create_message.streaming_property() != nullptr &&
      !create_message.streaming_property()->empty()) {
    for (const auto &[key, value] : *create_message.streaming_property()) {
      SetStreamingProperty(std::get<std::string>(key),
                           std::get<std::string>(value));
    }
  }

  char *appId = nullptr;
  int ret = app_manager_get_app_id(getpid(), &appId);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("[PlusPlayer] Fail to get app id: %s.", get_error_message(ret));
    return -1;
  }
  plusplayer_set_app_id(player_, appId);
  free(appId);

  RegisterListener();

  int64_t drm_type = flutter_common::GetValue(create_message.drm_configs(),
                                              "drmType", (int64_t)0);
  std::string license_server_url = flutter_common::GetValue(
      create_message.drm_configs(), "licenseServerUrl", std::string());
  if (drm_type != 0) {
    if (!SetDrm(uri, drm_type, license_server_url)) {
      LOG_ERROR("[PlusPlayer] Fail to set drm.");
      return -1;
    }
  }

  if (!SetDisplay()) {
    LOG_ERROR("[PlusPlayer] Fail to set display.");
    return -1;
  }

  SetDisplayRoi(0, 0, 1, 1);

  bool is_prebuffer_mode = flutter_common::GetValue(
      create_message.player_options(), "prebufferMode", false);
  if (is_prebuffer_mode) {
    plusplayer_set_prebuffer_mode(player_, true);
    is_prebuffer_mode_ = true;
  }

  int64_t start_position = flutter_common::GetValue(
      create_message.player_options(), "startPosition", (int64_t)0);
  if (start_position > 0) {
    LOG_INFO("[PlusPlayer] Start position: %lld", start_position);
    if (plusplayer_seek(player_, start_position) !=
        PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_INFO("[PlusPlayer] Fail to seek, it's a non-seekable content");
    }
  }

  if (plusplayer_prepare_async(player_) != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayer] Player fail to prepare.");
    return -1;
  }
  return SetUpEventChannel();
}

void PlusPlayer::Dispose() {
  LOG_INFO("[PlusPlayer] Player disposing.");
  ClearUpEventChannel();
}

void PlusPlayer::SetDisplayRoi(int32_t x, int32_t y, int32_t width,
                               int32_t height) {
  plusplayer_geometry_s roi;
  roi.x = x;
  roi.y = y;
  roi.width = width;
  roi.height = height;
  if (plusplayer_set_display_roi(player_, roi) != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayer] Player fail to set display roi.");
    return;
  }
  memento_->display_area.x = x;
  memento_->display_area.y = y;
  memento_->display_area.width = width;
  memento_->display_area.height = height;
}

bool PlusPlayer::Play() {
  LOG_INFO("[PlusPlayer] Player starting.");

  plusplayer_state_e state = plusplayer_get_state(player_);
  if (state < PLUSPLAYER_STATE_TRACK_SOURCE_READY) {
    LOG_ERROR("[PlusPlayer] Player is not ready.");
    return false;
  }

  if (state <= PLUSPLAYER_STATE_READY) {
    if (plusplayer_start(player_) != PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR("[PlusPlayer] Player fail to start.");
      return false;
    }
    return true;
  } else if (state == PLUSPLAYER_STATE_PAUSED) {
    if (plusplayer_resume(player_) != PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR("[PlusPlayer] Player fail to resume.");
      return false;
    }
    return true;
  }
  return false;
}

bool PlusPlayer::Activate() {
  if (plusplayer_activate_audio(player_) != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayer] Fail to activate.");
    return false;
  }
  return true;
}

bool PlusPlayer::Deactivate() {
  if (is_prebuffer_mode_) {
    plusplayer_stop(player_);
    return true;
  }
  if (plusplayer_deactivate_audio(player_) != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayer] Fail to deactivate video.");
    return false;
  }
  return true;
}

bool PlusPlayer::Pause() {
  LOG_INFO("[PlusPlayer] Player pausing.");

  plusplayer_state_e state = plusplayer_get_state(player_);
  if (state < PLUSPLAYER_STATE_READY) {
    LOG_ERROR("[PlusPlayer] Player is not ready.");
    return false;
  }

  if (state != PLUSPLAYER_STATE_PLAYING) {
    LOG_INFO("[PlusPlayer] Player not playing.");
    return false;
  }

  if (plusplayer_pause(player_) != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayer] Player fail to pause.");
    return false;
  }

  SendIsPlayingState(false);
  return true;
}

bool PlusPlayer::SetLooping(bool is_looping) {
  LOG_ERROR("[PlusPlayer] Not support to set looping.");
  return true;
}

bool PlusPlayer::SetVolume(double volume) { return false; }

bool PlusPlayer::SetPlaybackSpeed(double speed) {
  LOG_INFO("[PlusPlayer] Speed: %f", speed);

  if (plusplayer_get_state(player_) <= PLUSPLAYER_STATE_IDLE) {
    LOG_ERROR("[PlusPlayer] Player is not prepared.");
    return false;
  }
  if (plusplayer_set_playback_rate(player_, speed) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayer] Player fail to set playback rate.");
    return false;
  }
  return true;
}

bool PlusPlayer::SeekTo(int64_t position, SeekCompletedCallback callback) {
  LOG_INFO("[PlusPlayer] Seek to position: %lld", position);

  if (plusplayer_get_state(player_) < PLUSPLAYER_STATE_READY) {
    LOG_ERROR("[PlusPlayer] Player is not ready.");
    return false;
  }

  if (on_seek_completed_) {
    LOG_ERROR("[PlusPlayer] Player is already seeking.");
    return false;
  }

  on_seek_completed_ = std::move(callback);
  if (plusplayer_seek(player_, position) != PLUSPLAYER_ERROR_TYPE_NONE) {
    on_seek_completed_ = nullptr;
    LOG_ERROR("[PlusPlayer] Player fail to seek.");
    return false;
  }

  return true;
}

int64_t PlusPlayer::GetPosition() {
  uint64_t position = 0;
  plusplayer_state_e state = plusplayer_get_state(player_);
  if (state == PLUSPLAYER_STATE_PLAYING || state == PLUSPLAYER_STATE_PAUSED) {
    if (plusplayer_get_playing_time(player_, &position) !=
        PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR("[PlusPlayer] Player fail to get the current playing time.");
    }
  }
  return static_cast<int64_t>(position);
}

bool PlusPlayer::IsLive() {
  char *value;
  if (plusplayer_get_property(
          player_, plusplayer_property_e::PLUSPLAYER_PROPERTY_IS_LIVE,
          &value) != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayer] Player fail to get is live.");
    return false;
  }
  std::string is_live(value);
  free(value);
  return is_live == "TRUE";
}

std::pair<int64_t, int64_t> PlusPlayer::GetLiveDuration() {
  char *value;
  if (plusplayer_get_property(
          player_, plusplayer_property_e::PLUSPLAYER_PROPERTY_LIVE_DURATION,
          &value) != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayer] Player fail to get live duration.");
    return std::make_pair(0, 0);
  }
  std::string live_duration_str(value);
  free(value);
  std::vector<std::string> time_vec = Split(live_duration_str, '|');
  return std::make_pair(std::stoll(time_vec[0]), std::stoll(time_vec[1]));
}

std::pair<int64_t, int64_t> PlusPlayer::GetDuration() {
  if (IsLive()) {
    return GetLiveDuration();
  } else {
    int64_t duration = 0;
    if (plusplayer_get_duration(player_, &duration) !=
        PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR("[PlusPlayer] Player fail to get the duration.");
      return std::make_pair(0, 0);
    }
    return std::make_pair(0, duration);
  }
}

void PlusPlayer::GetVideoSize(int32_t *width, int32_t *height) {
  if (plusplayer_get_state(player_) >= PLUSPLAYER_STATE_TRACK_SOURCE_READY) {
    struct UserData {
      int32_t *w;
      int32_t *h;
      bool *found;
    };
    UserData data{width, height, nullptr};
    bool found = false;
    data.found = &found;

    plusplayer_get_foreach_active_track(
        player_,
        [](plusplayer_track_h track_h, void *user_data) -> bool {
          plusplayer_track_type_e type;
          if (plusplayer_get_track_type(track_h, &type) !=
              PLUSPLAYER_ERROR_TYPE_NONE) {
            return true;  // continue iteration
          }
          if (type == PLUSPLAYER_TRACK_TYPE_VIDEO) {
            int w = 0, h = 0;
            plusplayer_get_track_width(track_h, &w);
            plusplayer_get_track_height(track_h, &h);
            UserData *d = static_cast<UserData *>(user_data);
            *(d->w) = w;
            *(d->h) = h;
            *(d->found) = true;
            return false;  // stop iteration
          }
          return true;  // continue iteration
        },
        &data);
    if (!found) {
      LOG_ERROR("[PlusPlayer] Player fail to get video size.");
    } else {
      LOG_INFO("[PlusPlayer] Video width: %d, height: %d.", *width, *height);
    }
  }
}

bool PlusPlayer::IsReady() {
  return PLUSPLAYER_STATE_READY == plusplayer_get_state(player_);
}

bool PlusPlayer::SetDisplay() {
  void *native_window = GetWindowHandle();
  if (!native_window) {
    LOG_ERROR("[PlusPlayer] Could not get a native window handle.");
    return false;
  }
  int x = 0, y = 0, width = 0, height = 0;
  ecore_wl2_window_proxy_->ecore_wl2_window_geometry_get(native_window, &x, &y,
                                                         &width, &height);
  uint32_t resource_id = FlutterDesktopViewGetResourceId(flutter_view_);
  if (resource_id == 0) {
    LOG_ERROR("[PlusPlayer] Fail to get resource id.");
    return false;
  }
  plusplayer_geometry_s roi{x, y, width, height};
  if (plusplayer_set_display_subsurface(
          player_, plusplayer_display_type_e::PLUSPLAYER_DISPLAY_TYPE_OVERLAY,
          resource_id, roi) != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayer] Player fail to set display.");
    return false;
  }
  if (plusplayer_set_display_mode(
          player_,
          plusplayer_display_mode_e::PLUSPLAYER_DISPLAY_MODE_DST_ROI) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayer] Player fail to set display mode.");
    return false;
  }
  return true;
}

flutter::EncodableValue ParseVideoTrack(const plusplayer_track_h track) {
  flutter::EncodableMap video_track_result = {};
  video_track_result.insert_or_assign(flutter::EncodableValue("trackType"),
                                      flutter::EncodableValue("video"));

  int track_index = 0;
  plusplayer_get_track_index(track, &track_index);
  video_track_result.insert_or_assign(flutter::EncodableValue("trackId"),
                                      flutter::EncodableValue(track_index));

  const char *mimetype;
  plusplayer_get_track_mimetype(track, &mimetype);
  video_track_result.insert_or_assign(
      flutter::EncodableValue("mimetype"),
      flutter::EncodableValue(std::string(mimetype)));

  int width;
  plusplayer_get_track_width(track, &width);
  video_track_result.insert_or_assign(flutter::EncodableValue("width"),
                                      flutter::EncodableValue(width));

  int height;
  plusplayer_get_track_height(track, &height);
  video_track_result.insert_or_assign(flutter::EncodableValue("height"),
                                      flutter::EncodableValue(height));

  int bitrate;
  plusplayer_get_track_bitrate(track, &bitrate);
  video_track_result.insert_or_assign(flutter::EncodableValue("bitrate"),
                                      flutter::EncodableValue(bitrate));

  LOG_DEBUG(
      "[PlusPlayer] video track info : trackId : %d, mimetype : %s, width : "
      "%d, height : %d, birate : %d",
      track_index, mimetype, width, height, bitrate);
  return flutter::EncodableValue(video_track_result);
}

flutter::EncodableValue ParseAudioTrack(const plusplayer_track_h track) {
  flutter::EncodableMap audio_track_result = {};
  audio_track_result.insert_or_assign(flutter::EncodableValue("trackType"),
                                      flutter::EncodableValue("audio"));
  int track_index = 0;
  plusplayer_get_track_index(track, &track_index);
  audio_track_result.insert_or_assign(flutter::EncodableValue("trackId"),
                                      flutter::EncodableValue(track_index));

  const char *mimetype;
  plusplayer_get_track_mimetype(track, &mimetype);
  audio_track_result.insert_or_assign(
      flutter::EncodableValue("mimetype"),
      flutter::EncodableValue(std::string(mimetype)));

  const char *language_code = plusplayer_get_track_language_code(
      track, PLUSPLAYER_TRACK_TYPE_AUDIO, track_index);
  audio_track_result.insert_or_assign(
      flutter::EncodableValue("language"),
      flutter::EncodableValue(std::string(language_code)));

  int channel_count;
  plusplayer_get_track_channels(track, &channel_count);
  audio_track_result.insert_or_assign(flutter::EncodableValue("channel"),
                                      flutter::EncodableValue(channel_count));

  int bitrate;
  plusplayer_get_track_bitrate(track, &bitrate);
  audio_track_result.insert_or_assign(flutter::EncodableValue("bitrate"),
                                      flutter::EncodableValue(bitrate));

  LOG_DEBUG(
      "[PlusPlayer] audio track info : trackId : %d, mimetype : %s, "
      "language_code : %s, channel : %d, bitrate : %d",
      track_index, mimetype, language_code, channel_count, bitrate);

  return flutter::EncodableValue(audio_track_result);
}

flutter::EncodableValue ParseSubtitleTrack(const plusplayer_track_h track) {
  flutter::EncodableMap subtitle_track_result = {};
  subtitle_track_result.insert_or_assign(flutter::EncodableValue("trackType"),
                                         flutter::EncodableValue("text"));

  int track_index = 0;
  plusplayer_get_track_index(track, &track_index);
  subtitle_track_result.insert_or_assign(flutter::EncodableValue("trackId"),
                                         flutter::EncodableValue(track_index));

  const char *mimetype;
  plusplayer_get_track_mimetype(track, &mimetype);
  subtitle_track_result.insert_or_assign(
      flutter::EncodableValue("mimetype"),
      flutter::EncodableValue(std::string(mimetype)));

  const char *language_code = plusplayer_get_track_language_code(
      track, PLUSPLAYER_TRACK_TYPE_SUBTITLE, track_index);
  subtitle_track_result.insert_or_assign(
      flutter::EncodableValue("language"),
      flutter::EncodableValue(std::string(language_code)));

  LOG_DEBUG(
      "[PlusPlayer] subtitle track info : trackId : %d, mimetype : %s, "
      "language_code : %s",
      track_index, mimetype, language_code);
  return flutter::EncodableValue(subtitle_track_result);
}

flutter::EncodableList PlusPlayer::GetTrackInfo(std::string track_type) {
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return {};
  }

  plusplayer_state_e state = plusplayer_get_state(player_);
  if (state < PLUSPLAYER_STATE_TRACK_SOURCE_READY) {
    LOG_ERROR("[PlusPlayer] Player is in invalid state.");
    return {};
  }

  plusplayer_track_type_e type = ConvertTrackType(track_type);

  int track_count = 0;
  if (plusplayer_get_track_count(player_, type, &track_count) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayer] Fail to get track count");
    return {};
  }
  if (track_count <= 0) {
    return {};
  }
  struct UserData {
    flutter::EncodableList *track_selections;
    plusplayer_track_type_e type;
  };
  flutter::EncodableList trackSelections = {};
  UserData user_data = {};
  user_data.track_selections = &trackSelections;
  user_data.type = type;
  if (plusplayer_get_foreach_track(
          player_,
          [](plusplayer_track_h track_h, void *user_data) -> bool {
            UserData *data = static_cast<UserData *>(user_data);
            if (data->type ==
                plusplayer_track_type_e::PLUSPLAYER_TRACK_TYPE_VIDEO) {
              data->track_selections->push_back(ParseVideoTrack(track_h));
            } else if (data->type ==
                       plusplayer_track_type_e::PLUSPLAYER_TRACK_TYPE_AUDIO) {
              data->track_selections->push_back(ParseAudioTrack(track_h));
            } else if (data->type == plusplayer_track_type_e::
                                         PLUSPLAYER_TRACK_TYPE_SUBTITLE) {
              data->track_selections->push_back(ParseSubtitleTrack(track_h));
            }
            return true;
          },
          &user_data) != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayer] Fail to get track info");
    return {};
  }
  return trackSelections;
}

flutter::EncodableList PlusPlayer::GetActiveTrackInfo() {
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return {};
  }

  plusplayer_state_e state = plusplayer_get_state(player_);
  if (state < PLUSPLAYER_STATE_TRACK_SOURCE_READY) {
    LOG_ERROR("[PlusPlayer] Player is in invalid state.");
    return {};
  }

  // Use the C API to iterate over active tracks.
  flutter::EncodableList active_tracks;
  // The callback receives each active track handle.
  if (plusplayer_get_foreach_active_track(
          player_,
          [](plusplayer_track_h track_h, void *user_data) -> bool {
            plusplayer_track_type_e type;
            plusplayer_get_track_type(track_h, &type);
            flutter::EncodableList *tracks =
                static_cast<flutter::EncodableList *>(user_data);
            if (type == PLUSPLAYER_TRACK_TYPE_AUDIO) {
              tracks->push_back(ParseAudioTrack(track_h));
            } else if (type == PLUSPLAYER_TRACK_TYPE_VIDEO) {
              tracks->push_back(ParseVideoTrack(track_h));
            } else {
              tracks->push_back(ParseSubtitleTrack(track_h));
            }
            return true;  // Continue iteration.
          },
          &active_tracks) != PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayer] Fail to get active track info");
    return {};
  }
  return active_tracks;
}

bool PlusPlayer::SetTrackSelection(int32_t track_id, std::string track_type) {
  LOG_INFO("[PlusPlayer] Track id is: %d,track type is: %s", track_id,
           track_type.c_str());

  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return false;
  }

  plusplayer_state_e state = plusplayer_get_state(player_);
  if (state < PLUSPLAYER_STATE_TRACK_SOURCE_READY) {
    LOG_ERROR("[PlusPlayer] Player is in invalid state.");
    return false;
  }

  if (!plusplayer_select_track(player_, ConvertTrackType(track_type),
                               track_id)) {
    LOG_ERROR("[PlusPlayer] Player fail to select track.");
    return false;
  }
  return true;
}

bool PlusPlayer::SetDrm(const std::string &uri, int drm_type,
                        const std::string &license_server_url) {
  drm_manager_ = std::make_unique<DrmManager>();
  if (!drm_manager_->CreateDrmSession(drm_type, true)) {
    LOG_ERROR("[PlusPlayer] Fail to create drm session.");
    return false;
  }

  int drm_handle = 0;
  if (!drm_manager_->GetDrmHandle(&drm_handle)) {
    LOG_ERROR("[PlusPlayer] Fail to get drm handle.");
    return false;
  }

  plusplayer_drm_type_e type;
  switch (drm_type) {
    case DrmManager::DrmType::DRM_TYPE_PLAYREADAY:
      type = plusplayer_drm_type_e::PLUSPLAYER_DRM_TYPE_PLAYREADY;
      break;
    case DrmManager::DrmType::DRM_TYPE_WIDEVINECDM:
      type = plusplayer_drm_type_e::PLUSPLAYER_DRM_TYPE_WIDEVINE_CDM;
      break;
    default:
      type = plusplayer_drm_type_e::PLUSPLAYER_DRM_TYPE_NONE;
      break;
  }

  plusplayer_drm_property_s property;
  property.handle = drm_handle;
  property.type = type;
  property.license_acquired_cb = reinterpret_cast<void *>(OnLicenseAcquired);
  property.license_acquired_userdata = this;
  property.external_decryption = false;
  plusplayer_set_drm(player_, property);

  if (license_server_url.empty()) {
    bool success = drm_manager_->SetChallenge(uri, binary_messenger_);
    if (!success) {
      LOG_ERROR("[PlusPlayer]Fail to set challenge.");
      return false;
    }
  } else {
    if (!drm_manager_->SetChallenge(uri, license_server_url)) {
      LOG_ERROR("[PlusPlayer]Fail to set challenge.");
      return false;
    }
  }
  return true;
}

std::string PlusPlayer::GetStreamingProperty(
    const std::string &streaming_property_type) {
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return "";
  }
  plusplayer_state_e state = plusplayer_get_state(player_);
  if (state == PLUSPLAYER_STATE_NONE || state == PLUSPLAYER_STATE_IDLE) {
    LOG_ERROR("[PlusPlayer]:Player is in invalid state[%d]", state);
    return "";
  }
  char *value;
  plusplayer_property_e property = ConvertPropertyType(streaming_property_type);
  if (property == static_cast<plusplayer_property_e>(-1)) {
    LOG_ERROR("[PlusPlayer]:Player fail to convert property type");
    return "";
  }
  if (plusplayer_get_property(player_, property, &value) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayer]:Player fail to get streaming property");
    return "";
  }
  std::string result(value);
  free(value);
  return result;
}

bool PlusPlayer::SetBufferConfig(const std::string &key, int64_t value) {
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return false;
  }

  plusplayer_state_e state = plusplayer_get_state(player_);
  if (state == PLUSPLAYER_STATE_NONE) {
    LOG_ERROR("[PlusPlayer]:Player is in invalid state[%d]", state);
    return false;
  }
  const std::pair<std::string, int> config = std::make_pair(key, value);
  return plusplayer_set_buffer_config(player_, key.c_str(), value) ==
         PLUSPLAYER_ERROR_TYPE_NONE;
}

void PlusPlayer::SetStreamingProperty(const std::string &type,
                                      const std::string &value) {
  plusplayer_state_e state = plusplayer_get_state(player_);
  if (state == PLUSPLAYER_STATE_NONE) {
    LOG_ERROR("[PlusPlayer] Player is in invalid state[%d]", state);
    return;
  }
  if ((!create_message_.format_hint() ||
       create_message_.format_hint()->empty() ||
       *create_message_.format_hint() != "dash") &&
      (type == "OPEN_HTTP_HEADER" || type == "TOKEN" ||
       type == "UNWANTED_FRAMERATE" || type == "UNWANTED_RESOLUTION" ||
       type == "UPDATE_SAME_LANGUAGE_CODE")) {
    LOG_ERROR("[PlusPlayer] Only support streaming property type: %s for DASH!",
              type.c_str());
    return;
  }
  LOG_INFO("[PlusPlayer] SetStreamingProp: type[%s], value[%s]", type.c_str(),
           value.c_str());
  plusplayer_set_property(player_, ConvertPropertyType(type), value.c_str());
}

bool PlusPlayer::SetDisplayRotate(int64_t rotation) {
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return false;
  }

  plusplayer_state_e state = plusplayer_get_state(player_);
  if (state == PLUSPLAYER_STATE_NONE) {
    LOG_ERROR("[PlusPlayer] Player is in invalid state[%d]", state);
    return false;
  }

  LOG_INFO("[PlusPlayer] rotation: %lld", rotation);
  return plusplayer_set_display_rotation(
      player_, static_cast<plusplayer_display_rotation_type_e>(rotation));
}

bool PlusPlayer::SetDisplayMode(int64_t display_mode) {
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return false;
  }

  plusplayer_state_e state = plusplayer_get_state(player_);
  if (state == PLUSPLAYER_STATE_NONE) {
    LOG_ERROR("[PlusPlayer] Player is in invalid state[%d]", state);
    return false;
  }
  LOG_INFO("[PlusPlayer] display_mode: %lld", display_mode);
  if (plusplayer_set_display_mode(
          player_, static_cast<plusplayer_display_mode_e>(display_mode)) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    LOG_ERROR("[PlusPlayer] Player fail to set display mode.");
    return false;
  }
  return true;
}

bool PlusPlayer::StopAndClose() {
  LOG_INFO("[PlusPlayer] StopAndClose is called.");
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return false;
  }

  is_buffering_ = false;
  plusplayer_state_e player_state = plusplayer_get_state(player_);
  if (player_state < PLUSPLAYER_STATE_READY) {
    LOG_INFO("[PlusPlayer] Player already stop, nothing to do.");
    return true;
  }

  if (!plusplayer_stop(player_)) {
    LOG_ERROR("[PlusPlayer] Player fail to stop.");
    return false;
  }

  if (!plusplayer_close(player_)) {
    LOG_ERROR("[PlusPlayer] Player fail to close.");
    return false;
  }

  if (drm_manager_) {
    drm_manager_->ReleaseDrmSession();
    drm_manager_.reset();
  }

  return true;
}

bool PlusPlayer::Suspend() {
  LOG_INFO("[PlusPlayer] Suspend is called.");

  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return false;
  }

  if (is_prebuffer_mode_) {
    LOG_ERROR("[PlusPlayer] Player is in prebuffer mode, do nothing.");
    return true;
  }
  plusplayer_geometry_s display_area = memento_->display_area;
  memento_.reset(new PlayerMemento());
  memento_->display_area = display_area;
  if (!GetMemento(memento_.get())) {
    LOG_ERROR("[PlusPlayer] Player fail to get memento.");
    return false;
  }

  LOG_INFO(
      "[PlusPlayer] Memento saved current player state: %d, position: %llu ms, "
      "is_live: %d",
      (int)memento_->state, memento_->playing_time, memento_->is_live);

  if (memento_->is_live) {
    memento_->playing_time = 0;
    if (!StopAndClose()) {
      LOG_ERROR("[PlusPlayer] Player is live, StopAndClose fail.");
      return false;
    }
    LOG_INFO("[PlusPlayer] Player is live: close done successfully.");
    return true;
  }

  int power_state = device_proxy_->device_power_get_state();
  if (power_state == POWER_STATE_STANDBY) {
    LOG_INFO("[PlusPlayer] Power state is standby.");
    if (!StopAndClose()) {
      LOG_ERROR("[PlusPlayer] Player need to stop and close, but failed.");
      return false;
    }
    LOG_INFO("[PlusPlayer] Standby state: close done successfully.");
    return true;
  } else {
    LOG_INFO("[PlusPlayer] Player state is not standby: %d, do nothing.",
             power_state);
  }

  plusplayer_state_e player_state = plusplayer_get_state(player_);
  if (player_state <= PLUSPLAYER_STATE_TRACK_SOURCE_READY) {
    if (plusplayer_close(player_) != PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR("[PlusPlayer] Player close fail.");
      return false;
    }
    LOG_INFO("[PlusPlayer] Player is in invalid state[%d], just close.",
             player_state);
    return true;
  } else if (player_state != PLUSPLAYER_STATE_PAUSED) {
    LOG_INFO("[PlusPlayer] Player calling pause from suspend.");
    if (plusplayer_suspend(player_) == false) {
      LOG_ERROR(
          "[PlusPlayer] Suspend fail, in restore player instance would be "
          "created newly.");
      if (!StopAndClose())
        LOG_ERROR("[PlusPlayer] Suspend error, player stop and close fail.");
      return false;
    }
    SendIsPlayingState(false);
  }
  return true;
}

bool PlusPlayer::Restore(const CreateMessage *restore_message,
                         int64_t resume_time) {
  LOG_INFO("[PlusPlayer] Restore is called.");
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player is not initialized.");
    return false;
  }

  plusplayer_state_e player_state = plusplayer_get_state(player_);
  if (player_state != PLUSPLAYER_STATE_NONE &&
      player_state != PLUSPLAYER_STATE_PAUSED &&
      player_state != PLUSPLAYER_STATE_PLAYING) {
    LOG_ERROR("[PlusPlayer] Player is in invalid state[%d].", player_state);
    return false;
  }

  if (!memento_) {
    LOG_ERROR(
        "[PlusPlayer] No memento to restore. Player is in invalid state[%d]",
        player_state);
    return false;
  }

  if (is_prebuffer_mode_) {
    LOG_ERROR("[PlusPlayer] Player is in prebuffer mode, do nothing.");
    return true;
  }

  if (restore_message->uri()) {
    LOG_INFO(
        "[PlusPlayer] Restore URL is not emptpy, close the existing instance.");
    if (!StopAndClose()) {
      LOG_ERROR("[PlusPlayer] Player need to stop and close, but failed.");
      return false;
    }
    return RestorePlayer(restore_message, resume_time);
  }

  switch (player_state) {
    case PLUSPLAYER_STATE_NONE:
      return RestorePlayer(restore_message, resume_time);
      break;
    case PLUSPLAYER_STATE_PAUSED:
      if (!plusplayer_restore(player_, memento_->state)) {
        if (!StopAndClose()) {
          LOG_ERROR("[PlusPlayer] Player need to stop and close, but failed.");
          return false;
        }
        return RestorePlayer(restore_message, resume_time);
      }
      break;
    case PLUSPLAYER_STATE_PLAYING:
      // might be the case that widget has called
      // restore more than once, just ignore.
      break;
    default:
      LOG_INFO(
          "[PlusPlayer] Unhandled state, dont know how to process, just return "
          "false.");
      return false;
  }
  return true;
}

bool PlusPlayer::RestorePlayer(const CreateMessage *restore_message,
                               int64_t resume_time) {
  LOG_INFO("[PlusPlayer] RestorePlayer is called.");
  LOG_INFO("[PlusPlayer] is_live: %d", memento_->is_live);

  if (restore_message->uri()) {
    LOG_INFO("[PlusPlayer] Player previous url: %s", url_.c_str());
    LOG_INFO("[PlusPlayer] Player new url: %s",
             restore_message->uri()->c_str());
    url_ = *restore_message->uri();
    create_message_ = *restore_message;
  }

  LOG_INFO("[PlusPlayer] Player previous playing time: %llu ms",
           memento_->playing_time);
  LOG_INFO("[PlusPlayer] Player new resume time: %lld ms", resume_time);
  // resume_time < 0  ==> use previous playing time
  // resume_time == 0 ==> play from beginning
  // resume_time > 0  ==> play from resume_time(Third-party settings)
  if (resume_time >= 0)
    memento_->playing_time = static_cast<uint64_t>(resume_time);

  is_restored_ = true;
  if (Create(url_, create_message_) < 0) {
    LOG_ERROR("[PlusPlayer] Fail to create player.");
    is_restored_ = false;
    return false;
  }
  if (memento_->playing_time > 0 &&
      !plusplayer_seek(player_, memento_->playing_time)) {
    LOG_ERROR("[PlusPlayer] Fail to seek.");
  }
  SetDisplayRoi(memento_->display_area.x, memento_->display_area.y,
                memento_->display_area.width, memento_->display_area.height);

  return true;
}

bool PlusPlayer::SetData(const flutter::EncodableMap &data) {
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
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
    if (plusplayer_set_property(player_, ConvertPropertyType(key),
                                value.c_str()) != PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR("[PlusPlayer] Fail to set property, key : %s", key.c_str());
      result = false;
    }
  }
  return result;
}

flutter::EncodableMap PlusPlayer::GetData(const flutter::EncodableList &data) {
  flutter::EncodableMap result;
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return result;
  }

  for (const auto &encodable_key : data) {
    std::string key = std::get<std::string>(encodable_key);
    char *value;
    if (plusplayer_get_property(player_, ConvertPropertyType(key), &value) !=
        PLUSPLAYER_ERROR_TYPE_NONE) {
      LOG_ERROR("[PlusPlayer] Fail to get property, key : %s", key.c_str());
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

bool PlusPlayer::UpdateDashToken(const std::string &dashToken) {
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return false;
  }
  return plusplayer_set_property(
             player_, plusplayer_property_e::PLUSPLAYER_PROPERTY_TOKEN,
             dashToken.c_str()) != PLUSPLAYER_ERROR_TYPE_NONE;
}

bool PlusPlayer::OnLicenseAcquired(int *drm_handle, unsigned int length,
                                   unsigned char *pssh_data, void *user_data) {
  LOG_INFO("[PlusPlayer] License acquired.");
  PlusPlayer *self = static_cast<PlusPlayer *>(user_data);

  if (self->drm_manager_) {
    return self->drm_manager_->SecurityInitCompleteCB(drm_handle, length,
                                                      pssh_data, nullptr);
  }
  return false;
}

void PlusPlayer::OnPrepareDone(bool ret, void *user_data) {
  PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);
  /*
  if (!SetDisplayVisible(self->player_, true)) {
    LOG_ERROR("[PlusPlayer] Fail to set display visible.");
  }
*/
  if (!self->is_initialized_ && ret) {
    self->SendInitialized();
  }

  if (self->is_restored_ && ret) {
    self->SendRestored();
  }
}

void PlusPlayer::OnBufferStatus(int percent, void *user_data) {
  LOG_INFO("[PlusPlayer] Buffering percent: %d.", percent);
  PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);

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

void PlusPlayer::OnSeekDone(void *user_data) {
  LOG_INFO("[PlusPlayer] Seek completed.");
  PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);

  if (self->on_seek_completed_) {
    self->on_seek_completed_();
    self->on_seek_completed_ = nullptr;
  }
}

void PlusPlayer::OnEos(void *user_data) {
  LOG_INFO("[PlusPlayer] Play completed.");
  PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);

  self->SendPlayCompleted();
}

void PlusPlayer::OnSubtitleData(const plusplayer_subtitle_type_e type,
                                const uint64_t duration_in_ms, const char *data,
                                const int size,
                                plusplayer_subtitle_attr_s *attr_list,
                                int attr_size, void *userdata) {}

void PlusPlayer::OnResourceConflicted(void *user_data) {
  LOG_ERROR("[PlusPlayer] Resource conflicted.");
  PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);

  self->SendIsPlayingState(false);
}

void PlusPlayer::OnError(plusplayer_error_type_e error_type, void *user_data) {
  LOG_ERROR("[PlusPlayer] Error code: %d", error_type);
  PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);

  self->SendError("[PlusPlayer] error",
                  std::string("Error: ") + GetErrorMessage(error_type));
}

void PlusPlayer::OnErrorMsg(plusplayer_error_type_e error_type,
                            const char *error_msg, void *user_data) {
  LOG_ERROR("[PlusPlayer] Error code: %d, message: %s.", error_type, error_msg);
  PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);

  self->SendError("PlusPlayer error", std::string("Error: ") + error_msg);
}

void PlusPlayer::OnDrmInitData(Plusplayer_DrmHandle *drm_handle,
                               unsigned int len, unsigned char *pssh_data,
                               plusplayer_track_type_e type, void *user_data) {
  LOG_INFO("[PlusPlayer] Drm init completed.");
  PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);

  if (self->drm_manager_) {
    if (self->drm_manager_->SecurityInitCompleteCB(drm_handle, len, pssh_data,
                                                   nullptr)) {
      plusplayer_drm_license_acquired_done(self->player_, type);
    }
  }
}

void PlusPlayer::OnAdaptiveStreamingControlEvent(
    plusplayer_streaming_message_type_e message_type,
    plusplayer_message_param_s *param, void *user_data) {
  LOG_INFO("[PlusPlayer] Message type: %d, is DrmInitData (%d)", message_type,
           message_type == PLUSPLAYER_STREAMING_MESSAGE_TYPE_DRMINITDATA);
  PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);

  if (message_type == plusplayer_streaming_message_type_e::
                          PLUSPLAYER_STREAMING_MESSAGE_TYPE_DRMINITDATA) {
    if (0 == param->size) {
      LOG_ERROR("[PlusPlayer] Empty message.");
      return;
    }

    if (self->drm_manager_) {
      self->drm_manager_->UpdatePsshData(param->data, param->size);
    }
  }
}

void PlusPlayer::OnStateChangedToPlaying(void *user_data) {
  PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);
  self->SendIsPlayingState(true);
}

void PlusPlayer::OnADEventFromDash(const char *ad_data, void *user_data) {
  if (!ad_data) {
    LOG_ERROR("[PlusPlayer] No ad_data.");
    return;
  }

  const char *prefix = "AD_INFO: ";
  const char *data = strstr(ad_data, prefix);
  if (!data) {
    LOG_ERROR("[PlusPlayer] Invalid ad_data.");
    return;
  }
  data += strlen(prefix);
  const_cast<char *>(data)[strlen(data) - 1] = '\0';
  LOG_INFO("[PlusPlayer] AD info: %s", data);

  rapidjson::Document doc;
  doc.Parse(data);
  if (doc.HasParseError()) {
    LOG_ERROR("[PlusPlayer] Fail to parse ad_data: %d.", doc.GetParseError());
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

    PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);
    self->SendADFromDash(ad_info);
  }
}

bool PlusPlayer::GetMemento(PlayerMemento *memento) {
  uint64_t playing_time;
  if (plusplayer_get_playing_time(player_, &playing_time) !=
      PLUSPLAYER_ERROR_TYPE_NONE) {
    return false;
  }
  memento->playing_time = playing_time;
  memento->is_live = IsLive();
  memento->state = plusplayer_get_state(player_);
  return true;
}

}  // namespace video_player_avplay_tizen
