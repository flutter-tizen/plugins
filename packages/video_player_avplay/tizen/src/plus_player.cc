// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "plus_player.h"

#include <app_manager.h>
#include <system_info.h>

#include <sstream>

#include "log.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace video_player_avplay_tizen {

static std::vector<std::string> split(const std::string &s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> tokens;
  while (getline(ss, item, delim)) {
    tokens.push_back(item);
  }
  return tokens;
}

static plusplayer::TrackType ConvertTrackType(std::string track_type) {
  if (track_type == "video") {
    return plusplayer::TrackType::kTrackTypeVideo;
  }
  if (track_type == "audio") {
    return plusplayer::TrackType::kTrackTypeAudio;
  }
  if (track_type == "text") {
    return plusplayer::TrackType::kTrackTypeSubtitle;
  }
  return plusplayer::TrackType::kTrackTypeMax;
}

PlusPlayer::PlusPlayer(flutter::BinaryMessenger *messenger,
                       FlutterDesktopViewRef flutter_view)
    : VideoPlayer(messenger, flutter_view) {
  memento_ = std::make_unique<plusplayer::PlayerMemento>();
  device_proxy_ = std::make_unique<DeviceProxy>();
}

PlusPlayer::~PlusPlayer() {
  if (player_) {
    Stop(player_);
    Close(player_);
    UnregisterListener(player_);
    DestroyPlayer(player_);
    player_ = nullptr;
  }

  if (drm_manager_) {
    drm_manager_->ReleaseDrmSession();
  }
}

void PlusPlayer::RegisterListener() {
  listener_.buffering_callback = OnBufferStatus;
  listener_.adaptive_streaming_control_callback =
      OnAdaptiveStreamingControlEvent;
  listener_.completed_callback = OnEos;
  listener_.drm_init_data_callback = OnDrmInitData;
  listener_.error_callback = OnError;
  listener_.error_message_callback = OnErrorMsg;
  listener_.prepared_callback = OnPrepareDone;
  listener_.seek_completed_callback = OnSeekDone;
  listener_.subtitle_data_callback = OnSubtitleData;
  listener_.playing_callback = OnStateChangedToPlaying;
  listener_.resource_conflicted_callback = OnResourceConflicted;
  listener_.ad_event_callback = OnADEventFromDash;
  ::RegisterListener(player_, &listener_, this);
}

int64_t PlusPlayer::Create(const std::string &uri,
                           const CreateMessage &create_message) {
  LOG_INFO("[PlusPlayer] Create player.");

  std::string video_format;

  if (create_message.format_hint() && !create_message.format_hint()->empty()) {
    video_format = *create_message.format_hint();
  }

  if (video_format == "dash") {
    player_ = CreatePlayer(plusplayer::PlayerType::kDASH);
  } else {
    player_ = CreatePlayer(plusplayer::PlayerType::kDefault);
  }

  if (!player_) {
    LOG_ERROR("[PlusPlayer] Fail to create player.");
    return -1;
  }

  if (!Open(player_, uri)) {
    LOG_ERROR("[PlusPlayer] Fail to open uri :  %s.", uri.c_str());
    return -1;
  }
  url_ = uri;
  create_message_ = create_message;
  LOG_INFO("[PlusPlayer] Uri: %s", uri.c_str());

  SetStreamingProperty("UPDATE_SAME_LANGUAGE_CODE", "1");
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
  SetAppId(player_, std::string(appId));
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
    SetPrebufferMode(player_, true);
    is_prebuffer_mode_ = true;
  }

  int64_t start_position = flutter_common::GetValue(
      create_message.player_options(), "startPosition", (int64_t)0);
  if (start_position > 0) {
    LOG_INFO("[PlusPlayer] Start position: %lld", start_position);
    if (!Seek(player_, start_position)) {
      LOG_INFO("[PlusPlayer] Fail to seek, it's a non-seekable content");
    }
  }

  if (!PrepareAsync(player_)) {
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
  plusplayer::Geometry roi;
  roi.x = x;
  roi.y = y;
  roi.w = width;
  roi.h = height;
  if (!::SetDisplayRoi(player_, roi)) {
    LOG_ERROR("[PlusPlayer] Player fail to set display roi.");
  }
}

bool PlusPlayer::Play() {
  LOG_INFO("[PlusPlayer] Player starting.");

  plusplayer::State state = GetState(player_);
  if (state < plusplayer::State::kTrackSourceReady) {
    LOG_ERROR("[PlusPlayer] Player is not ready.");
    return false;
  }

  if (state <= plusplayer::State::kReady) {
    if (!Start(player_)) {
      LOG_ERROR("[PlusPlayer] Player fail to start.");
      return false;
    }
    return true;
  } else if (state == plusplayer::State::kPaused) {
    if (!Resume(player_)) {
      LOG_ERROR("[PlusPlayer] Player fail to resume.");
      return false;
    }
    return true;
  }
  return false;
}

bool PlusPlayer::Activate() {
  if (!::Activate(player_, plusplayer::kTrackTypeVideo)) {
    LOG_ERROR("[PlusPlayer] Fail to activate video.");
    return false;
  }
  if (!::Activate(player_, plusplayer::kTrackTypeAudio)) {
    LOG_ERROR("[PlusPlayer] Fail to activate audio.");
    return false;
  }
  if (!::Activate(player_, plusplayer::kTrackTypeSubtitle)) {
    LOG_ERROR("[PlusPlayer] Fail to activate subtitle.");
  }

  return true;
}

bool PlusPlayer::Deactivate() {
  if (is_prebuffer_mode_) {
    Stop(player_);
    return true;
  }

  if (!::Deactivate(player_, plusplayer::kTrackTypeVideo)) {
    LOG_ERROR("[PlusPlayer] Fail to deactivate video.");
    return false;
  }
  if (!::Deactivate(player_, plusplayer::kTrackTypeAudio)) {
    LOG_ERROR("[PlusPlayer] Fail to deactivate audio.");
    return false;
  }
  if (!::Deactivate(player_, plusplayer::kTrackTypeSubtitle)) {
    LOG_ERROR("[PlusPlayer] Fail to deactivate subtitle.");
  }

  return true;
}

bool PlusPlayer::Pause() {
  LOG_INFO("[PlusPlayer] Player pausing.");

  plusplayer::State state = GetState(player_);
  if (state < plusplayer::State::kReady) {
    LOG_ERROR("[PlusPlayer] Player is not ready.");
    return false;
  }

  if (state != plusplayer::State::kPlaying) {
    LOG_INFO("[PlusPlayer] Player not playing.");
    return false;
  }

  if (!::Pause(player_)) {
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

bool PlusPlayer::SetVolume(double volume) {
  if (GetState(player_) < plusplayer::State::kPlaying) {
    LOG_ERROR("[PlusPlayer] Player is in invalid state");
    return false;
  }
  // dart api volume range[0,1], plusplaer volume range[0,100]
  int new_volume = volume * 100;
  LOG_INFO("[PlusPlayer] Volume: %d", new_volume);
  if (!::SetVolume(player_, new_volume)) {
    LOG_ERROR("[PlusPlayer] Fail to set volume.");
    return false;
  }
  return true;
}

bool PlusPlayer::SetPlaybackSpeed(double speed) {
  LOG_INFO("[PlusPlayer] Speed: %f", speed);

  if (GetState(player_) <= plusplayer::State::kIdle) {
    LOG_ERROR("[PlusPlayer] Player is not prepared.");
    return false;
  }
  if (!SetPlaybackRate(player_, speed)) {
    LOG_ERROR("[PlusPlayer] Player fail to set playback rate.");
    return false;
  }
  return true;
}

bool PlusPlayer::SeekTo(int64_t position, SeekCompletedCallback callback) {
  LOG_INFO("[PlusPlayer] Seek to position: %lld", position);

  if (GetState(player_) < plusplayer::State::kReady) {
    LOG_ERROR("[PlusPlayer] Player is not ready.");
    return false;
  }

  if (on_seek_completed_) {
    LOG_ERROR("[PlusPlayer] Player is already seeking.");
    return false;
  }

  on_seek_completed_ = std::move(callback);
  if (!Seek(player_, position)) {
    on_seek_completed_ = nullptr;
    LOG_ERROR("[PlusPlayer] Player fail to seek.");
    return false;
  }

  return true;
}

int64_t PlusPlayer::GetPosition() {
  uint64_t position = 0;
  plusplayer::State state = GetState(player_);
  if (state == plusplayer::State::kPlaying ||
      state == plusplayer::State::kPaused) {
    if (!GetPlayingTime(player_, &position)) {
      LOG_ERROR("[PlusPlayer] Player fail to get the current playing time.");
    }
  }
  return static_cast<int64_t>(position);
}

bool PlusPlayer::IsLive() {
  plusplayer::PlayerMemento memento;
  if (!GetMemento(player_, &memento)) {
    LOG_ERROR("[PlusPlayer] Player fail to get memento.");
    return false;
  }

  return memento.is_live;
}

std::pair<int64_t, int64_t> PlusPlayer::GetLiveDuration() {
  std::string live_duration_str =
      ::GetStreamingProperty(player_, "GET_LIVE_DURATION");
  if (live_duration_str.empty()) {
    LOG_ERROR("[PlusPlayer] Player fail to get live duration.");
    return std::make_pair(0, 0);
  }

  std::vector<std::string> time_vec = split(live_duration_str, '|');
  return std::make_pair(std::stoll(time_vec[0]), std::stoll(time_vec[1]));
}

std::pair<int64_t, int64_t> PlusPlayer::GetDuration() {
  if (IsLive()) {
    return GetLiveDuration();
  } else {
    int64_t duration = 0;
    if (!::GetDuration(player_, &duration)) {
      LOG_ERROR("[PlusPlayer] Player fail to get the duration.");
      return std::make_pair(0, 0);
    }
    return std::make_pair(0, duration);
  }
}

void PlusPlayer::GetVideoSize(int32_t *width, int32_t *height) {
  if (GetState(player_) >= plusplayer::State::kTrackSourceReady) {
    bool found = false;
    std::vector<plusplayer::Track> tracks = ::GetActiveTrackInfo(player_);
    for (auto track : tracks) {
      if (track.type == plusplayer::TrackType::kTrackTypeVideo) {
        *width = track.width;
        *height = track.height;
        found = true;
        break;
      }
    }
    if (!found) {
      LOG_ERROR("[PlusPlayer] Player fail to get video size.");
    } else {
      LOG_INFO("[PlusPlayer] Video width: %d, height: %d.", *width, *height);
    }
  }
}

bool PlusPlayer::IsReady() {
  return plusplayer::State::kReady == GetState(player_);
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
  bool ret = ::SetDisplay(player_, plusplayer::DisplayType::kOverlay,
                          resource_id, x, y, width, height);
  if (!ret) {
    LOG_ERROR("[PlusPlayer] Player fail to set display.");
    return false;
  }

  ret = ::SetDisplayMode(player_, plusplayer::DisplayMode::kDstRoi);
  if (!ret) {
    LOG_ERROR("[PlusPlayer] Player fail to set display mode.");
    return false;
  }

  return true;
}

flutter::EncodableValue PlusPlayer::ParseVideoTrack(
    plusplayer::Track video_track) {
  flutter::EncodableMap video_track_result = {};
  video_track_result.insert_or_assign(flutter::EncodableValue("trackType"),
                                      flutter::EncodableValue("video"));
  video_track_result.insert_or_assign(
      flutter::EncodableValue("trackId"),
      flutter::EncodableValue(video_track.index));
  video_track_result.insert_or_assign(
      flutter::EncodableValue("mimetype"),
      flutter::EncodableValue(video_track.mimetype));
  video_track_result.insert_or_assign(
      flutter::EncodableValue("width"),
      flutter::EncodableValue(video_track.width));
  video_track_result.insert_or_assign(
      flutter::EncodableValue("height"),
      flutter::EncodableValue(video_track.height));
  video_track_result.insert_or_assign(
      flutter::EncodableValue("bitrate"),
      flutter::EncodableValue(video_track.bitrate));
  LOG_DEBUG(
      "[PlusPlayer] video track info : trackId : %d, mimetype : %s, width : "
      "%d, height : %d, birate : %d",
      video_track.index, video_track.mimetype.c_str(), video_track.width,
      video_track.height, video_track.bitrate);
  return flutter::EncodableValue(video_track_result);
}

flutter::EncodableValue PlusPlayer::ParseAudioTrack(
    plusplayer::Track audio_track) {
  flutter::EncodableMap audio_track_result = {};
  audio_track_result.insert_or_assign(flutter::EncodableValue("trackType"),
                                      flutter::EncodableValue("audio"));
  audio_track_result.insert_or_assign(
      flutter::EncodableValue("trackId"),
      flutter::EncodableValue(audio_track.index));
  audio_track_result.insert_or_assign(
      flutter::EncodableValue("mimetype"),
      flutter::EncodableValue(audio_track.mimetype));
  audio_track_result.insert_or_assign(
      flutter::EncodableValue("language"),
      flutter::EncodableValue(audio_track.language_code));
  audio_track_result.insert_or_assign(
      flutter::EncodableValue("channel"),
      flutter::EncodableValue(audio_track.channels));
  audio_track_result.insert_or_assign(
      flutter::EncodableValue("bitrate"),
      flutter::EncodableValue(audio_track.bitrate));
  LOG_DEBUG(
      "[PlusPlayer] audio track info : trackId : %d, mimetype : %s, "
      "language_code : "
      "%s, channel : %d, bitrate : %d",
      audio_track.index, audio_track.mimetype.c_str(),
      audio_track.language_code.c_str(), audio_track.channels,
      audio_track.bitrate);
  return flutter::EncodableValue(audio_track_result);
}

flutter::EncodableValue PlusPlayer::ParseSubtitleTrack(
    plusplayer::Track subtitle_track) {
  flutter::EncodableMap subtitle_track_result = {};
  subtitle_track_result.insert_or_assign(flutter::EncodableValue("trackType"),
                                         flutter::EncodableValue("text"));
  subtitle_track_result.insert_or_assign(
      flutter::EncodableValue("trackId"),
      flutter::EncodableValue(subtitle_track.index));
  subtitle_track_result.insert_or_assign(
      flutter::EncodableValue("mimetype"),
      flutter::EncodableValue(subtitle_track.mimetype));
  subtitle_track_result.insert_or_assign(
      flutter::EncodableValue("language"),
      flutter::EncodableValue(subtitle_track.language_code));
  LOG_DEBUG(
      "[PlusPlayer] subtitle track info : trackId : %d, mimetype : %s, "
      "language_code : %s",
      subtitle_track.index, subtitle_track.mimetype.c_str(),
      subtitle_track.language_code.c_str());
  return flutter::EncodableValue(subtitle_track_result);
}

flutter::EncodableList PlusPlayer::GetTrackInfo(std::string track_type) {
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return {};
  }

  plusplayer::State state = GetState(player_);
  if (state < plusplayer::State::kTrackSourceReady) {
    LOG_ERROR("[PlusPlayer] Player is in invalid state.");
    return {};
  }

  plusplayer::TrackType type = ConvertTrackType(track_type);

  int track_count = GetTrackCount(player_, type);
  if (track_count <= 0) {
    return {};
  }

  const std::vector<plusplayer::Track> track_info = ::GetTrackInfo(player_);
  if (track_info.empty()) {
    return {};
  }

  flutter::EncodableList trackSelections = {};
  if (type == plusplayer::TrackType::kTrackTypeVideo) {
    LOG_INFO("[PlusPlayer] Video track count: %d", track_count);
    for (const auto &track : track_info) {
      if (track.type == plusplayer::kTrackTypeVideo) {
        trackSelections.push_back(ParseVideoTrack(track));
      }
    }
  } else if (type == plusplayer::TrackType::kTrackTypeAudio) {
    LOG_INFO("[PlusPlayer] Audio track count: %d", track_count);
    for (const auto &track : track_info) {
      if (track.type == plusplayer::kTrackTypeAudio) {
        trackSelections.push_back(ParseAudioTrack(track));
      }
    }
  } else if (type == plusplayer::TrackType::kTrackTypeSubtitle) {
    LOG_INFO("[PlusPlayer] Subtitle track count: %d", track_count);
    for (const auto &track : track_info) {
      if (track.type == plusplayer::kTrackTypeSubtitle) {
        trackSelections.push_back(
            flutter::EncodableValue(ParseSubtitleTrack(track)));
      }
    }
  }

  return trackSelections;
}

flutter::EncodableList PlusPlayer::GetActiveTrackInfo() {
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return {};
  }

  plusplayer::State state = GetState(player_);
  if (state < plusplayer::State::kTrackSourceReady) {
    LOG_ERROR("[PlusPlayer] Player is in invalid state.");
    return {};
  }

  const std::vector<plusplayer::Track> track_info =
      ::GetActiveTrackInfo(player_);

  if (track_info.empty()) {
    return {};
  }

  flutter::EncodableList active_tracks = {};
  for (const auto &track : track_info) {
    if (track.type == plusplayer::kTrackTypeVideo) {
      active_tracks.push_back(ParseVideoTrack(track));
    } else if (track.type == plusplayer::kTrackTypeAudio) {
      active_tracks.push_back(ParseAudioTrack(track));
    } else if (track.type == plusplayer::kTrackTypeSubtitle) {
      active_tracks.push_back(ParseSubtitleTrack(track));
    }
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

  plusplayer::State state = GetState(player_);
  if (state < plusplayer::State::kTrackSourceReady) {
    LOG_ERROR("[PlusPlayer] Player is in invalid state.");
    return false;
  }

  if (!SelectTrack(player_, ConvertTrackType(track_type), track_id)) {
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

  plusplayer::drm::Type type;
  switch (drm_type) {
    case DrmManager::DrmType::DRM_TYPE_PLAYREADAY:
      type = plusplayer::drm::Type::kPlayready;
      break;
    case DrmManager::DrmType::DRM_TYPE_WIDEVINECDM:
      type = plusplayer::drm::Type::kWidevineCdm;
      break;
    default:
      type = plusplayer::drm::Type::kNone;
      break;
  }

  plusplayer::drm::Property property;
  property.handle = drm_handle;
  property.type = type;
  property.license_acquired_cb =
      reinterpret_cast<plusplayer::drm::LicenseAcquiredCb>(OnLicenseAcquired);
  property.license_acquired_userdata =
      reinterpret_cast<plusplayer::drm::UserData>(this);
  property.external_decryption = false;
  ::SetDrm(player_, property);

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
  plusplayer::State state = GetState(player_);
  if (state == plusplayer::State::kNone || state == plusplayer::State::kIdle) {
    LOG_ERROR("[PlusPlayer]:Player is in invalid state[%d]", state);
    return "";
  }
  return ::GetStreamingProperty(player_, streaming_property_type);
}

bool PlusPlayer::SetBufferConfig(const std::string &key, int64_t value) {
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return false;
  }

  plusplayer::State state = GetState(player_);
  if (state == plusplayer::State::kNone) {
    LOG_ERROR("[PlusPlayer]:Player is in invalid state[%d]", state);
    return false;
  }
  const std::pair<std::string, int> config = std::make_pair(key, value);
  return ::SetBufferConfig(player_, config);
}

void PlusPlayer::SetStreamingProperty(const std::string &type,
                                      const std::string &value) {
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return;
  }
  plusplayer::State state = GetState(player_);
  if (state == plusplayer::State::kNone) {
    LOG_ERROR("[PlusPlayer] Player is in invalid state[%d]", state);
    return;
  }

  LOG_INFO("[PlusPlayer] SetStreamingProp: type[%s], value[%s]", type.c_str(),
           value.c_str());
  ::SetStreamingProperty(player_, type, value);
}

bool PlusPlayer::SetDisplayRotate(int64_t rotation) {
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return false;
  }

  plusplayer::State state = GetState(player_);
  if (state == plusplayer::State::kNone) {
    LOG_ERROR("[PlusPlayer] Player is in invalid state[%d]", state);
    return false;
  }

  LOG_INFO("[PlusPlayer] rotation: %lld", rotation);
  return ::SetDisplayRotate(player_,
                            static_cast<plusplayer::DisplayRotation>(rotation));
}

bool PlusPlayer::SetDisplayMode(int64_t display_mode) {
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return false;
  }

  plusplayer::State state = GetState(player_);
  if (state == plusplayer::State::kNone) {
    LOG_ERROR("[PlusPlayer] Player is in invalid state[%d]", state);
    return false;
  }
  LOG_INFO("[PlusPlayer] display_mode: %lld", display_mode);
  return ::SetDisplayMode(player_,
                          static_cast<plusplayer::DisplayMode>(display_mode));
}

bool PlusPlayer::StopAndClose() {
  LOG_INFO("[PlusPlayer] StopAndClose is called.");
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return false;
  }

  is_buffering_ = false;
  plusplayer::State player_state = GetState(player_);
  if (player_state < plusplayer::State::kReady) {
    LOG_INFO("[PlusPlayer] Player already stop, nothing to do.");
    return true;
  }

  if (!::Stop(player_)) {
    LOG_ERROR("[PlusPlayer] Player fail to stop.");
    return false;
  }

  if (!::Close(player_)) {
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

  memento_.reset(new plusplayer::PlayerMemento());
  if (!GetMemento(player_, memento_.get())) {
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

  plusplayer::State player_state = GetState(player_);
  if (player_state <= plusplayer::State::kTrackSourceReady) {
    if (!::Close(player_)) {
      LOG_ERROR("[PlusPlayer] Player close fail.");
      return false;
    }
    LOG_INFO("[PlusPlayer] Player is in invalid state[%d], just close.",
             player_state);
    return true;
  } else if (player_state != plusplayer::State::kPaused) {
    LOG_INFO("[PlusPlayer] Player calling pause from suspend.");
    if (::Suspend(player_) == false) {
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

  plusplayer::State player_state = GetState(player_);
  if (player_state != plusplayer::State::kNone &&
      player_state != plusplayer::State::kPaused &&
      player_state != plusplayer::State::kPlaying) {
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
    case plusplayer::State::kNone:
      return RestorePlayer(restore_message, resume_time);
      break;
    case plusplayer::State::kPaused:
      if (!::Restore(player_, memento_->state)) {
        if (!StopAndClose()) {
          LOG_ERROR("[PlusPlayer] Player need to stop and close, but failed.");
          return false;
        }
        return RestorePlayer(restore_message, resume_time);
      }
      break;
    case plusplayer::State::kPlaying:
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
  if (memento_->playing_time > 0 && !Seek(player_, memento_->playing_time)) {
    LOG_ERROR("[PlusPlayer] Fail to seek.");
  }
  SetDisplayRoi(memento_->display_area.x, memento_->display_area.y,
                memento_->display_area.w, memento_->display_area.h);

  return true;
}

std::string BuildJsonString(const flutter::EncodableMap &data) {
  rapidjson::Document doc;
  doc.SetObject();
  rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();

  for (const auto &pair : data) {
    std::string key_str = std::get<std::string>(pair.first);
    rapidjson::Value key(key_str.c_str(), allocator);
    if (key_str == "max-bandwidth") {
      doc.AddMember(key, rapidjson::Value(std::get<int64_t>(pair.second)),
                    allocator);
    } else {
      doc.AddMember(key,
                    rapidjson::Value(std::get<std::string>(pair.second).c_str(),
                                     allocator),
                    allocator);
    }
  }
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);
  return buffer.GetString();
}

std::string BuildJsonString(const flutter::EncodableList &encodable_keys) {
  rapidjson::Document doc;
  doc.SetObject();
  rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();

  for (const auto &encodable_key : encodable_keys) {
    std::string key_str = std::get<std::string>(encodable_key);
    rapidjson::Value key(key_str.c_str(), allocator);
    if (key_str == "max-bandwidth") {
      doc.AddMember(key, 0, allocator);
    } else {
      doc.AddMember(key, "", allocator);
    }
  }
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  doc.Accept(writer);
  return buffer.GetString();
}

void ParseJsonString(std::string json_str,
                     const flutter::EncodableList &encodable_keys,
                     flutter::EncodableMap &output) {
  rapidjson::Document doc;
  doc.Parse(json_str.c_str());
  if (doc.HasParseError()) {
    LOG_ERROR("[PlusPlayer] Fail to parse json string.");
    return;
  }
  for (const auto &encodable_key : encodable_keys) {
    std::string key_str = std::get<std::string>(encodable_key);
    if (doc.HasMember(key_str.c_str())) {
      if (key_str == "max-bandwidth") {
        output.insert_or_assign(
            encodable_key,
            flutter::EncodableValue(doc[key_str.c_str()].GetInt64()));
      } else {
        output.insert_or_assign(
            encodable_key,
            flutter::EncodableValue(doc[key_str.c_str()].GetString()));
      }
    }
  }
}

bool PlusPlayer::SetData(const flutter::EncodableMap &data) {
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return false;
  }
  std::string json_data = BuildJsonString(data);
  if (json_data.empty()) {
    LOG_ERROR("[PlusPlayer] json_data is empty.");
    return false;
  }
  return ::SetData(player_, json_data);
}

flutter::EncodableMap PlusPlayer::GetData(const flutter::EncodableList &data) {
  flutter::EncodableMap result;
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return result;
  }
  std::string json_data = BuildJsonString(data);
  if (json_data.empty()) {
    LOG_ERROR("[PlusPlayer] json_data is empty.");
    return result;
  }
  if (!::GetData(player_, json_data)) {
    LOG_ERROR("[PlusPlayer] Fail to get data from player");
    return result;
  }
  ParseJsonString(json_data, data, result);
  return result;
}

bool PlusPlayer::UpdateDashToken(const std::string &dashToken) {
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return false;
  }
  return ::UpdateDashToken(player_, dashToken);
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

  if (!SetDisplayVisible(self->player_, true)) {
    LOG_ERROR("[PlusPlayer] Fail to set display visible.");
  }

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

void PlusPlayer::OnSubtitleData(char *data, const int size,
                                const plusplayer::SubtitleType &type,
                                const uint64_t duration,
                                plusplayer::SubtitleAttributeListPtr attr_list,
                                void *user_data) {
  LOG_INFO("[PlusPlayer] Subtitle updated, duration: %llu, text: %s", duration,
           data);
  PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);

  plusplayer::SubtitleAttributeList *attrs = attr_list.get();
  flutter::EncodableList attributes_list;
  for (auto attr = attrs->begin(); attr != attrs->end(); attr++) {
    LOG_INFO("[PlusPlayer] Subtitle update: type: %d, start: %u, end: %u",
             attr->type, attr->start_time, attr->stop_time);
    flutter::EncodableMap attributes = {
        {flutter::EncodableValue("attrType"),
         flutter::EncodableValue(attr->type)},
        {flutter::EncodableValue("startTime"),
         flutter::EncodableValue((int64_t)attr->start_time)},
        {flutter::EncodableValue("stopTime"),
         flutter::EncodableValue((int64_t)attr->stop_time)},
    };

    switch (attr->type) {
      case plusplayer::kSubAttrRegionXPos:
      case plusplayer::kSubAttrRegionYPos:
      case plusplayer::kSubAttrRegionWidth:
      case plusplayer::kSubAttrRegionHeight:
      case plusplayer::kSubAttrWindowXPadding:
      case plusplayer::kSubAttrWindowYPadding:
      case plusplayer::kSubAttrWindowOpacity:
      case plusplayer::kSubAttrFontSize:
      case plusplayer::kSubAttrFontOpacity:
      case plusplayer::kSubAttrFontBgOpacity:
      case plusplayer::kSubAttrWebvttCueLine:
      case plusplayer::kSubAttrWebvttCueSize:
      case plusplayer::kSubAttrWebvttCuePosition: {
        intptr_t value_temp = reinterpret_cast<intptr_t>(attr->value);
        float value_float;
        std::memcpy(&value_float, &value_temp, sizeof(float));
        LOG_INFO("[PlusPlayer] Subtitle update: value<float>: %f", value_float);
        attributes[flutter::EncodableValue("attrValue")] =
            flutter::EncodableValue((double)value_float);
      } break;
      case plusplayer::kSubAttrWindowLeftMargin:
      case plusplayer::kSubAttrWindowRightMargin:
      case plusplayer::kSubAttrWindowTopMargin:
      case plusplayer::kSubAttrWindowBottomMargin:
      case plusplayer::kSubAttrWindowBgColor:
      case plusplayer::kSubAttrFontWeight:
      case plusplayer::kSubAttrFontStyle:
      case plusplayer::kSubAttrFontColor:
      case plusplayer::kSubAttrFontBgColor:
      case plusplayer::kSubAttrFontTextOutlineColor:
      case plusplayer::kSubAttrFontTextOutlineThickness:
      case plusplayer::kSubAttrFontTextOutlineBlurRadius:
      case plusplayer::kSubAttrFontVerticalAlign:
      case plusplayer::kSubAttrFontHorizontalAlign:
      case plusplayer::kSubAttrWebvttCueLineNum:
      case plusplayer::kSubAttrWebvttCueLineAlign:
      case plusplayer::kSubAttrWebvttCueAlign:
      case plusplayer::kSubAttrWebvttCuePositionAlign:
      case plusplayer::kSubAttrWebvttCueVertical:
      case plusplayer::kSubAttrTimestamp: {
        int value_int = reinterpret_cast<int>(attr->value);
        LOG_INFO("[PlusPlayer] Subtitle update: value<int>: %d", value_int);
        attributes[flutter::EncodableValue("attrValue")] =
            flutter::EncodableValue(value_int);
      } break;
      case plusplayer::kSubAttrFontFamily:
      case plusplayer::kSubAttrRawSubtitle: {
        const char *value_chars = reinterpret_cast<const char *>(attr->value);
        LOG_INFO("[PlusPlayer] Subtitle update: value<char *>: %s",
                 value_chars);
        std::string value_string(value_chars);
        attributes[flutter::EncodableValue("attrValue")] =
            flutter::EncodableValue(value_string);
      } break;
      case plusplayer::kSubAttrWindowShowBg: {
        uint32_t value_uint32 = reinterpret_cast<uint32_t>(attr->value);
        LOG_INFO("[PlusPlayer] Subtitle update: value<uint32_t>: %u",
                 value_uint32);
        attributes[flutter::EncodableValue("attrValue")] =
            flutter::EncodableValue((int64_t)value_uint32);
      } break;
      default:
        LOG_ERROR("[PlusPlayer] Unknown Subtitle type: %d", attr->type);
        break;
    }
    attributes_list.push_back(flutter::EncodableValue(attributes));
  }
  self->SendSubtitleUpdate(duration, data, attributes_list);
}

void PlusPlayer::OnResourceConflicted(void *user_data) {
  LOG_ERROR("[PlusPlayer] Resource conflicted.");
  PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);

  self->SendIsPlayingState(false);
}

std::string GetErrorMessage(plusplayer::ErrorType error_code) {
  switch (error_code) {
    case plusplayer::ErrorType::kNone:
      return "Successful";
    case plusplayer::ErrorType::kOutOfMemory:
      return "Out of memory";
    case plusplayer::ErrorType::kInvalidParameter:
      return "Invalid parameter";
    case plusplayer::ErrorType::kNoSuchFile:
      return "No such file or directory";
    case plusplayer::ErrorType::kInvalidOperation:
      return "Invalid operation";
    case plusplayer::ErrorType::kFileNoSpaceOnDevice:
      return "No space left on the device";
    case plusplayer::ErrorType::kFeatureNotSupportedOnDevice:
      return "Not supported file on this device";
    case plusplayer::ErrorType::kSeekFailed:
      return "Seek operation failure";
    case plusplayer::ErrorType::kInvalidState:
      return "Invalid player state";
    case plusplayer::ErrorType::kNotSupportedFile:
      return "File format not supported";
    case plusplayer::ErrorType::kNotSupportedFormat:
      return "Not supported media format";
    case plusplayer::ErrorType::kInvalidUri:
      return "Invalid URI";
    case plusplayer::ErrorType::kSoundPolicy:
      return "Sound policy error";
    case plusplayer::ErrorType::kConnectionFailed:
      return "Streaming connection failed";
    case plusplayer::ErrorType::kVideoCaptureFailed:
      return "Video capture failed";
    case plusplayer::ErrorType::kDrmExpired:
      return "DRM license expired";
    case plusplayer::ErrorType::kDrmNoLicense:
      return "DRM no license";
    case plusplayer::ErrorType::kDrmFutureUse:
      return "License for future use";
    case plusplayer::ErrorType::kDrmNotPermitted:
      return "DRM format not permitted";
    case plusplayer::ErrorType::kResourceLimit:
      return "Resource limit";
    case plusplayer::ErrorType::kPermissionDenied:
      return "Permission denied";
    case plusplayer::ErrorType::kServiceDisconnected:
      return "Service disconnected";
    case plusplayer::ErrorType::kBufferSpace:
      return "No buffer space available";
    case plusplayer::ErrorType::kNotSupportedAudioCodec:
      return "Not supported audio codec but video can be played";
    case plusplayer::ErrorType::kNotSupportedVideoCodec:
      return "Not supported video codec but audio can be played";
    case plusplayer::ErrorType::kNotSupportedSubtitle:
      return "Not supported subtitle format";
    default:
      return "Not defined error";
  }
}

void PlusPlayer::OnError(const plusplayer::ErrorType &error_code,
                         void *user_data) {
  LOG_ERROR("[PlusPlayer] Error code: %d", error_code);
  PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);

  self->SendError("[PlusPlayer] error",
                  std::string("Error: ") + GetErrorMessage(error_code));
}

void PlusPlayer::OnErrorMsg(const plusplayer::ErrorType &error_code,
                            const char *error_msg, void *user_data) {
  LOG_ERROR("[PlusPlayer] Error code: %d, message: %s.", error_code, error_msg);
  PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);

  self->SendError("PlusPlayer error", std::string("Error: ") + error_msg);
}

void PlusPlayer::OnDrmInitData(int *drm_handle, unsigned int len,
                               unsigned char *pssh_data,
                               plusplayer::TrackType type, void *user_data) {
  LOG_INFO("[PlusPlayer] Drm init completed.");
  PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);

  if (self->drm_manager_) {
    if (self->drm_manager_->SecurityInitCompleteCB(drm_handle, len, pssh_data,
                                                   nullptr)) {
      DrmLicenseAcquiredDone(self->player_, type);
    }
  }
}

void PlusPlayer::OnAdaptiveStreamingControlEvent(
    const plusplayer::StreamingMessageType &type,
    const plusplayer::MessageParam &msg, void *user_data) {
  LOG_INFO("[PlusPlayer] Message type: %d, is DrmInitData (%d)", type,
           type == plusplayer::StreamingMessageType::kDrmInitData);
  PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);

  if (type == plusplayer::StreamingMessageType::kDrmInitData) {
    if (msg.data.empty() || 0 == msg.size) {
      LOG_ERROR("[PlusPlayer] Empty message.");
      return;
    }

    if (self->drm_manager_) {
      self->drm_manager_->UpdatePsshData(msg.data.data(), msg.size);
    }
  }
}

void PlusPlayer::OnClosedCaptionData(std::unique_ptr<char[]> data,
                                     const int size, void *user_data) {}

void PlusPlayer::OnCueEvent(const char *cue_data, void *user_data) {}

void PlusPlayer::OnDateRangeEvent(const char *date_range_data,
                                  void *user_data) {}

void PlusPlayer::OnStopReachEvent(bool stop_reach, void *user_data) {}

void PlusPlayer::OnCueOutContEvent(const char *cue_out_cont_data,
                                   void *user_data) {}

void PlusPlayer::OnChangeSourceDone(bool ret, void *user_data) {}

void PlusPlayer::OnStateChangedToPlaying(void *user_data) {
  PlusPlayer *self = reinterpret_cast<PlusPlayer *>(user_data);
  self->SendIsPlayingState(true);
}

void PlusPlayer::OnADEventFromDash(const char *ad_data, void *user_data) {
  const char *prefix = "AD_INFO: ";
  char *data = strstr(ad_data, prefix);
  data += strlen(prefix);
  data[strlen(data) - 1] = '\0';
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

}  // namespace video_player_avplay_tizen
