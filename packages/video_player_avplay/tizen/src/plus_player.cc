// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "plus_player.h"

#include <app_manager.h>
#include <system_info.h>

#include <sstream>

#include "log.h"

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
}

PlusPlayer::PlusPlayer(flutter::BinaryMessenger *messenger, void *native_window,
                       std::string &video_format)
    : VideoPlayer(messenger),
      native_window_(native_window),
      video_format_(video_format) {}

PlusPlayer::~PlusPlayer() { Dispose(); }

int64_t PlusPlayer::Create(const std::string &uri, int drm_type,
                           const std::string &license_server_url,
                           bool is_prebuffer_mode,
                           flutter::EncodableMap &http_headers) {
  LOG_INFO("[PlusPlayer] Create player.");

  if (video_format_ == "dash") {
    player_ = plusplayer::PlusPlayer::Create(plusplayer::PlayerType::kDASH);
  } else {
    player_ = plusplayer::PlusPlayer::Create();
  }

  if (!player_) {
    LOG_ERROR("[PlusPlayer] Fail to create player.");
    return -1;
  }

  if (!http_headers.empty()) {
    auto iter = http_headers.find(flutter::EncodableValue("Cookie"));
    if (iter != http_headers.end()) {
      if (std::holds_alternative<std::string>(iter->second)) {
        std::string cookie = std::get<std::string>(iter->second);
        player_->SetStreamingProperty("COOKIE", cookie);
      }
    }

    iter = http_headers.find(flutter::EncodableValue("User-Agent"));
    if (iter != http_headers.end()) {
      if (std::holds_alternative<std::string>(iter->second)) {
        std::string user_agent = std::get<std::string>(iter->second);
        player_->SetStreamingProperty("USER_AGENT", user_agent);
      }
    }
  }

  if (!player_->Open(uri)) {
    LOG_ERROR("[PlusPlayer] Fail to open uri :  %s.", uri.c_str());
    return -1;
  }
  LOG_INFO("[PlusPlayer] Uri: %s", uri.c_str());

  char *appId = nullptr;
  int ret = app_manager_get_app_id(getpid(), &appId);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("[PlusPlayer] Fail to get app id: %s.", get_error_message(ret));
    return -1;
  }
  player_->SetAppId(std::string(appId));
  free(appId);

  player_->RegisterListener(this);

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

  if (is_prebuffer_mode) {
    player_->SetPrebufferMode(true);
    is_prebuffer_mode_ = true;
  }

  if (!player_->PrepareAsync()) {
    LOG_ERROR("[PlusPlayer] Player fail to prepare.");
    return -1;
  }
  return SetUpEventChannel();
}

void PlusPlayer::Dispose() {
  LOG_INFO("[PlusPlayer] Player disposing.");

  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return;
  }
  if (!player_->Stop()) {
    LOG_INFO("[PlusPlayer] Player fail to stop.");
    return;
  }

  plusplayer::State state = player_->GetState();
  if (state == plusplayer::State::kIdle || state == plusplayer::State::kNone) {
    if (!player_->Close()) {
      LOG_INFO("[PlusPlayer] Player fail to close.");
      return;
    }
  }

  if (drm_manager_) {
    drm_manager_->ReleaseDrmSession();
  }
}

void PlusPlayer::SetDisplayRoi(int32_t x, int32_t y, int32_t width,
                               int32_t height) {
  plusplayer::Geometry roi;
  roi.x = x;
  roi.y = y;
  roi.w = width;
  roi.h = height;
  if (!player_->SetDisplayRoi(roi)) {
    LOG_ERROR("[PlusPlayer] Player fail to set display roi.");
  }
}

bool PlusPlayer::Play() {
  LOG_INFO("[PlusPlayer] Player starting.");

  plusplayer::State state = player_->GetState();
  if (state < plusplayer::State::kTrackSourceReady) {
    LOG_ERROR("[PlusPlayer] Player is not ready.");
    return false;
  }

  if (state <= plusplayer::State::kReady) {
    if (!player_->Start()) {
      LOG_ERROR("[PlusPlayer] Player fail to start.");
      return false;
    }
    return true;
  } else if (state == plusplayer::State::kPaused) {
    if (!player_->Resume()) {
      LOG_ERROR("[PlusPlayer] Player fail to resume.");
      return false;
    }
    return true;
  }
  return false;
}

bool PlusPlayer::Activate() {
  if (!player_->Activate(plusplayer::kTrackTypeVideo)) {
    LOG_ERROR("[PlusPlayer] Fail to activate video.");
    return false;
  }
  if (!player_->Activate(plusplayer::kTrackTypeAudio)) {
    LOG_ERROR("[PlusPlayer] Fail to activate audio.");
    return false;
  }
  if (!player_->Activate(plusplayer::kTrackTypeSubtitle)) {
    LOG_ERROR("[PlusPlayer] Fail to activate subtitle.");
  }

  return true;
}

bool PlusPlayer::Deactivate() {
  if (is_prebuffer_mode_) {
    player_->Stop();
    return true;
  }

  if (!player_->Deactivate(plusplayer::kTrackTypeVideo)) {
    LOG_ERROR("[PlusPlayer] Fail to activate video.");
    return false;
  }
  if (!player_->Deactivate(plusplayer::kTrackTypeAudio)) {
    LOG_ERROR("[PlusPlayer] Fail to activate audio.");
    return false;
  }
  if (!player_->Deactivate(plusplayer::kTrackTypeSubtitle)) {
    LOG_ERROR("[PlusPlayer] Fail to activate subtitle.");
  }

  return true;
}

bool PlusPlayer::Pause() {
  LOG_INFO("[PlusPlayer] Player pausing.");

  plusplayer::State state = player_->GetState();
  if (state < plusplayer::State::kReady) {
    LOG_ERROR("[PlusPlayer] Player is not ready.");
    return false;
  }

  if (state != plusplayer::State::kPlaying) {
    LOG_INFO("[PlusPlayer] Player not playing.");
    return false;
  }

  if (!player_->Pause()) {
    LOG_ERROR("[PlusPlayer] Player fail to pause.");
    return false;
  }

  return true;
}

bool PlusPlayer::SetLooping(bool is_looping) {
  LOG_ERROR("[PlusPlayer] Not support to set looping.");
  return false;
}

bool PlusPlayer::SetVolume(double volume) {
  LOG_INFO("[PlusPlayer] Volume: %f", volume);

  if (player_->GetState() != plusplayer::State::kPlaying ||
      player_->GetState() != plusplayer::State::kPaused) {
    LOG_ERROR("[PlusPlayer] Player is in invalid state");
    return false;
  }

  if (!player_->SetVolume(volume)) {
    LOG_ERROR("[PlusPlayer] Fail to set volume.");
    return false;
  }
  return true;
}

bool PlusPlayer::SetPlaybackSpeed(double speed) {
  LOG_INFO("[PlusPlayer] Speed: %f", speed);

  if (player_->GetState() <= plusplayer::State::kIdle) {
    LOG_ERROR("[PlusPlayer] Player is not prepared.");
    return false;
  }
  if (!player_->SetPlaybackRate(speed)) {
    LOG_ERROR("[PlusPlayer] Player fail to set playback rate.");
    return false;
  }
  return true;
}

bool PlusPlayer::SeekTo(int64_t position, SeekCompletedCallback callback) {
  LOG_INFO("[PlusPlayer] Seek to position: %lld", position);

  if (player_->GetState() < plusplayer::State::kReady) {
    LOG_ERROR("[PlusPlayer] Player is not ready.");
    return false;
  }

  if (on_seek_completed_) {
    LOG_ERROR("[PlusPlayer] Player is already seeking.");
    return false;
  }

  on_seek_completed_ = std::move(callback);
  plusplayer::PlayerMemento memento;
  if (!player_->GetMemento(&memento)) {
    LOG_ERROR("[PlusPlayer] Player fail to get memento.");
  }

  if (memento.is_live) {
    std::string str = player_->GetStreamingProperty("GET_LIVE_DURATION");
    if (str.empty()) {
      LOG_ERROR("[PlusPlayer] Player fail to get live duration.");
      return false;
    }
    std::vector<std::string> time_str = split(str, '|');
    int64_t start_time = std::stoll(time_str[0].c_str());
    int64_t end_time = std::stoll(time_str[1].c_str());

    if (position < start_time || position > end_time) {
      on_seek_completed_ = nullptr;
      LOG_ERROR("[PlusPlayer] Position out of range.");
      return false;
    }

    if (!player_->Seek(position)) {
      on_seek_completed_ = nullptr;
      LOG_ERROR("[PlusPlayer] Player fail to seek.");
      return false;
    }
  } else {
    if (!player_->Seek(position)) {
      on_seek_completed_ = nullptr;
      LOG_ERROR("[PlusPlayer] Player fail to seek.");
      return false;
    }
  }
  return true;
}

int64_t PlusPlayer::GetPosition() {
  uint64_t position = 0;
  plusplayer::State state = player_->GetState();
  if (state == plusplayer::State::kPlaying ||
      state == plusplayer::State::kPaused) {
    if (!player_->GetPlayingTime(&position)) {
      LOG_ERROR("[PlusPlayer] Player fail to get the current playing time.");
    }
  }
  return static_cast<int64_t>(position);
}

int64_t PlusPlayer::GetDuration() {
  int64_t duration = 0;
  if (player_->GetState() >= plusplayer::State::kTrackSourceReady) {
    plusplayer::PlayerMemento memento;
    if (!player_->GetMemento(&memento)) {
      LOG_ERROR("[PlusPlayer] Player fail to get memento.");
    }

    if (memento.is_live) {
      std::string str = player_->GetStreamingProperty("GET_LIVE_DURATION");
      if (str.empty()) {
        LOG_ERROR("[PlusPlayer] Player fail to get live duration.");
        return duration;
      }
      std::vector<std::string> time_str = split(str, '|');
      int64_t start_time = std::stoll(time_str[0].c_str());
      int64_t end_time = std::stoll(time_str[1].c_str());

      duration = end_time - start_time;
    } else {
      if (!player_->GetDuration(&duration)) {
        LOG_ERROR("[PlusPlayer] Player fail to get the duration.");
      }
    }
  }

  LOG_INFO("[PlusPlayer] Video duration: %lld.", duration);
  return duration;
}

void PlusPlayer::GetVideoSize(int32_t *width, int32_t *height) {
  if (player_->GetState() >= plusplayer::State::kTrackSourceReady) {
    bool found = false;
    std::vector<plusplayer::Track> tracks = player_->GetActiveTrackInfo();
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
  return plusplayer::State::kReady == player_->GetState();
}

bool PlusPlayer::SetDisplay() {
  int x = 0, y = 0, width = 0, height = 0;
  ecore_wl2_window_proxy_->ecore_wl2_window_geometry_get(native_window_, &x, &y,
                                                         &width, &height);
  int surface_id =
      ecore_wl2_window_proxy_->ecore_wl2_window_surface_id_get(native_window_);
  if (surface_id < 0) {
    LOG_ERROR("[PlusPlayer] Fail to get surface id.");
    return false;
  }
  bool ret = player_->SetDisplay(plusplayer::DisplayType::kOverlay, surface_id,
                                 x, y, width, height);
  if (!ret) {
    LOG_ERROR("[PlusPlayer] Player fail to set display.");
    return false;
  }

  ret = player_->SetDisplayMode(plusplayer::DisplayMode::kDstRoi);
  if (!ret) {
    LOG_ERROR("[PlusPlayer] Player fail to set display mode.");
    return false;
  }

  return true;
}

flutter::EncodableList PlusPlayer::GetTrackInfo(std::string track_type) {
  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return {};
  }

  plusplayer::State state = player_->GetState();
  if (state < plusplayer::State::kTrackSourceReady) {
    LOG_ERROR("[PlusPlayer] Player is in invalid state.");
    return {};
  }

  plusplayer::TrackType type = ConvertTrackType(track_type);

  int track_count = player_->GetTrackCount(type);
  if (track_count <= 0) {
    return {};
  }

  const std::vector<plusplayer::Track> track_info = player_->GetTrackInfo();
  if (track_info.empty()) {
    return {};
  }

  flutter::EncodableList trackSelections = {};
  flutter::EncodableMap trackSelection = {};
  trackSelection.insert(
      {flutter::EncodableValue("trackType"), flutter::EncodableValue(type)});
  if (type == plusplayer::TrackType::kTrackTypeVideo) {
    LOG_INFO("[PlusPlayer] Video track count: %d", track_count);
    for (const auto &track : track_info) {
      if (track.type == plusplayer::kTrackTypeVideo) {
        trackSelection.insert_or_assign(flutter::EncodableValue("trackId"),
                                        flutter::EncodableValue(track.index));
        trackSelection.insert_or_assign(flutter::EncodableValue("width"),
                                        flutter::EncodableValue(track.width));
        trackSelection.insert_or_assign(flutter::EncodableValue("height"),
                                        flutter::EncodableValue(track.height));
        trackSelection.insert_or_assign(flutter::EncodableValue("bitrate"),
                                        flutter::EncodableValue(track.bitrate));
        LOG_INFO(
            "[PlusPlayer] video track info[%d]: width[%d], height[%d], "
            "bitrate[%d]",
            track.index, track.width, track.height, track.bitrate);

        trackSelections.push_back(flutter::EncodableValue(trackSelection));
      }
    }
  } else if (type == plusplayer::TrackType::kTrackTypeAudio) {
    LOG_INFO("[PlusPlayer] Audio track count: %d", track_count);
    for (const auto &track : track_info) {
      if (track.type == plusplayer::kTrackTypeAudio) {
        trackSelection.insert_or_assign(flutter::EncodableValue("trackId"),
                                        flutter::EncodableValue(track.index));
        trackSelection.insert_or_assign(
            flutter::EncodableValue("language"),
            flutter::EncodableValue(track.language_code));
        trackSelection.insert_or_assign(
            flutter::EncodableValue("channel"),
            flutter::EncodableValue(track.channels));
        trackSelection.insert_or_assign(flutter::EncodableValue("bitrate"),
                                        flutter::EncodableValue(track.bitrate));
        LOG_INFO(
            "[PlusPlayer] Audio track info[%d]: language[%s], channel[%d], "
            "sample_rate[%d], bitrate[%d]",
            track.index, track.language_code.c_str(), track.channels,
            track.sample_rate, track.bitrate);

        trackSelections.push_back(flutter::EncodableValue(trackSelection));
      }
    }
  } else if (type == plusplayer::TrackType::kTrackTypeSubtitle) {
    LOG_INFO("[PlusPlayer] Subtitle track count: %d", track_count);
    for (const auto &track : track_info) {
      if (track.type == plusplayer::kTrackTypeSubtitle) {
        trackSelection.insert_or_assign(flutter::EncodableValue("trackId"),
                                        flutter::EncodableValue(track.index));
        trackSelection.insert_or_assign(
            flutter::EncodableValue("language"),
            flutter::EncodableValue(track.language_code));
        LOG_INFO("[PlusPlayer] Subtitle track info[%d]: language[%s]",
                 track.index, track.language_code.c_str());

        trackSelections.push_back(flutter::EncodableValue(trackSelection));
      }
    }
  }

  return trackSelections;
}

bool PlusPlayer::SetTrackSelection(int32_t track_id, std::string track_type) {
  LOG_INFO("[PlusPlayer] Track id is: %d,track type is: %s", track_id,
           track_type.c_str());

  if (!player_) {
    LOG_ERROR("[PlusPlayer] Player not created.");
    return false;
  }

  plusplayer::State state = player_->GetState();
  if (state < plusplayer::State::kTrackSourceReady) {
    LOG_ERROR("[PlusPlayer] Player is in invalid state.");
    return false;
  }

  if (!player_->SelectTrack(ConvertTrackType(track_type), track_id)) {
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
  player_->SetDrm(property);

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

void PlusPlayer::OnPrepareDone(bool ret, UserData userdata) {
  LOG_INFO("[PlusPlayer] Prepare done, result: %d.", ret);

  if (!is_initialized_ && ret) {
    SendInitialized();
  }
}

void PlusPlayer::OnBufferStatus(const int percent, UserData userdata) {
  LOG_INFO("[PlusPlayer] Buffering percent: %d.", percent);

  if (percent == 100) {
    SendBufferingEnd();
    is_buffering_ = false;
  } else if (!is_buffering_ && percent <= 5) {
    SendBufferingStart();
    is_buffering_ = true;
  } else {
    SendBufferingUpdate(percent);
  }
}

void PlusPlayer::OnSeekDone(UserData userdata) {
  LOG_INFO("[PlusPlayer] Seek completed.");

  if (on_seek_completed_) {
    on_seek_completed_();
    on_seek_completed_ = nullptr;
  }
}

void PlusPlayer::OnEos(UserData userdata) {
  LOG_INFO("[PlusPlayer] Play completed.");
  SendPlayCompleted();
}

void PlusPlayer::OnSubtitleData(std::unique_ptr<char[]> data, const int size,
                                const plusplayer::SubtitleType &type,
                                const uint64_t duration,
                                plusplayer::SubtitleAttrListPtr attr_list,
                                UserData userdata) {
  LOG_INFO("[PlusPlayer] Subtitle updated, duration: %llu, text: %s", duration,
           data.get());
  SendSubtitleUpdate(duration, data.get());
}

void PlusPlayer::OnResourceConflicted(UserData userdata) {
  LOG_ERROR("[PlusPlayer] Resource conflicted.");
  SendError("PlusPlayer error", "Resource conflicted");
}

void PlusPlayer::OnError(const plusplayer::ErrorType &error_code,
                         UserData userdata) {
  LOG_ERROR("[PlusPlayer] Error code: %d", error_code);
  SendError("[PlusPlayer] OnError", "");
}

void PlusPlayer::OnErrorMsg(const plusplayer::ErrorType &error_code,
                            const char *error_msg, UserData userdata) {
  LOG_ERROR("[PlusPlayer] Error code: %d, message: %s.", error_code, error_msg);
  SendError("PlusPlayer error", error_msg);
}

void PlusPlayer::OnDrmInitData(int *drmhandle, unsigned int len,
                               unsigned char *psshdata,
                               plusplayer::TrackType type, UserData userdata) {
  LOG_INFO("[PlusPlayer] Drm init completed");

  if (drm_manager_) {
    if (drm_manager_->SecurityInitCompleteCB(drmhandle, len, psshdata,
                                             nullptr)) {
      player_->DrmLicenseAcquiredDone(type);
    }
  }
}

void PlusPlayer::OnAdaptiveStreamingControlEvent(
    const plusplayer::StreamingMessageType &type,
    const plusplayer::MessageParam &msg, UserData userdata) {
  LOG_INFO("[PlusPlayer] Message type: %d, is DrmInitData (%d)", type,
           type == plusplayer::StreamingMessageType::kDrmInitData);

  if (type == plusplayer::StreamingMessageType::kDrmInitData) {
    if (msg.data.empty() || 0 == msg.size) {
      LOG_ERROR("[PlusPlayer] Empty message");
      return;
    }

    if (drm_manager_) {
      drm_manager_->UpdatePsshData(msg.data.data(), msg.size);
    }
  }
}

void PlusPlayer::OnClosedCaptionData(std::unique_ptr<char[]> data,
                                     const int size, UserData userdata) {}

void PlusPlayer::OnCueEvent(const char *CueData, UserData userdata) {}

void PlusPlayer::OnDateRangeEvent(const char *DateRangeData,
                                  UserData userdata) {}

void PlusPlayer::OnStopReachEvent(bool StopReach, UserData userdata) {}

void PlusPlayer::OnCueOutContEvent(const char *CueOutContData,
                                   UserData userdata) {}

void PlusPlayer::OnChangeSourceDone(bool ret, UserData userdata) {}

void PlusPlayer::OnStateChangedToPlaying(UserData userdata) {}
