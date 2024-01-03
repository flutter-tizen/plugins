// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player_tizen_plugin.h"

#include <app_common.h>
#include <flutter/encodable_value.h>
#include <flutter/plugin_registrar.h>
#include <flutter_tizen.h>

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <variant>

#include "media_player.h"
#include "messages.h"
#include "video_player.h"
#include "video_player_options.h"

namespace {

class VideoPlayerTizenPlugin : public flutter::Plugin,
                               public VideoPlayerVideoholeApi {
 public:
  static void RegisterWithRegistrar(
      FlutterDesktopPluginRegistrarRef registrar_ref,
      flutter::PluginRegistrar *plugin_registrar);

  VideoPlayerTizenPlugin(FlutterDesktopPluginRegistrarRef registrar_ref,
                         flutter::PluginRegistrar *plugin_registrar);
  virtual ~VideoPlayerTizenPlugin();

  std::optional<FlutterError> Initialize() override;
  ErrorOr<PlayerMessage> Create(const CreateMessage &msg) override;
  std::optional<FlutterError> Dispose(const PlayerMessage &msg) override;
  ErrorOr<DurationMessage> Duration(const PlayerMessage &msg) override;
  std::optional<FlutterError> SetLooping(const LoopingMessage &msg) override;
  std::optional<FlutterError> SetVolume(const VolumeMessage &msg) override;
  std::optional<FlutterError> SetPlaybackSpeed(
      const PlaybackSpeedMessage &msg) override;
  ErrorOr<TrackMessage> Track(const TrackTypeMessage &msg) override;
  ErrorOr<bool> SetTrackSelection(const SelectedTracksMessage &msg) override;
  std::optional<FlutterError> Play(const PlayerMessage &msg) override;
  ErrorOr<bool> SetDeactivate(const PlayerMessage &msg) override;
  ErrorOr<bool> SetActivate(const PlayerMessage &msg) override;
  ErrorOr<PositionMessage> Position(const PlayerMessage &msg) override;
  void SeekTo(
      const PositionMessage &msg,
      std::function<void(std::optional<FlutterError> reply)> result) override;
  std::optional<FlutterError> Pause(const PlayerMessage &msg) override;
  std::optional<FlutterError> SetMixWithOthers(
      const MixWithOthersMessage &msg) override;
  std::optional<FlutterError> SetDisplayGeometry(
      const GeometryMessage &msg) override;

  static VideoPlayer *FindPlayerById(int64_t player_id) {
    auto iter = players_.find(player_id);
    if (iter != players_.end()) {
      return iter->second.get();
    }
    return nullptr;
  }

 private:
  void DisposeAllPlayers();

  FlutterDesktopPluginRegistrarRef registrar_ref_;
  flutter::PluginRegistrar *plugin_registrar_;
  VideoPlayerOptions options_;

  static inline std::map<int64_t, std::unique_ptr<VideoPlayer>> players_;
};

void VideoPlayerTizenPlugin::RegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar_ref,
    flutter::PluginRegistrar *plugin_registrar) {
  auto plugin =
      std::make_unique<VideoPlayerTizenPlugin>(registrar_ref, plugin_registrar);
  plugin_registrar->AddPlugin(std::move(plugin));
}

VideoPlayerTizenPlugin::VideoPlayerTizenPlugin(
    FlutterDesktopPluginRegistrarRef registrar_ref,
    flutter::PluginRegistrar *plugin_registrar)
    : registrar_ref_(registrar_ref), plugin_registrar_(plugin_registrar) {
  VideoPlayerVideoholeApi::SetUp(plugin_registrar->messenger(), this);
}

VideoPlayerTizenPlugin::~VideoPlayerTizenPlugin() { DisposeAllPlayers(); }

void VideoPlayerTizenPlugin::DisposeAllPlayers() {
  for (const auto &[id, player] : players_) {
    player->Dispose();
  }
  players_.clear();
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Initialize() {
  DisposeAllPlayers();
  return std::nullopt;
}

ErrorOr<PlayerMessage> VideoPlayerTizenPlugin::Create(
    const CreateMessage &msg) {
  if (!FlutterDesktopPluginRegistrarGetView(registrar_ref_)) {
    return FlutterError("Operation failed", "Could not get a Flutter view.");
  }
  std::string uri;
  int32_t drm_type = 0;  // DRM_TYPE_NONE
  std::string license_server_url;
  bool prebuffer_mode = false;
  std::string format;
  flutter::EncodableMap http_headers = {};

  if (msg.asset() && !msg.asset()->empty()) {
    char *res_path = app_get_resource_path();
    if (res_path) {
      uri = uri + res_path + "flutter_assets/" + *msg.asset();
      free(res_path);
    } else {
      return FlutterError("Internal error", "Failed to get resource path.");
    }
  } else if (msg.uri() && !msg.uri()->empty()) {
    uri = *msg.uri();
    if (msg.format_hint() && !msg.format_hint()->empty()) {
      format = *msg.format_hint();
    }

    const flutter::EncodableMap *drm_configs = msg.drm_configs();
    if (drm_configs) {
      auto iter = drm_configs->find(flutter::EncodableValue("drmType"));
      if (iter != drm_configs->end()) {
        if (std::holds_alternative<int32_t>(iter->second)) {
          drm_type = std::get<int32_t>(iter->second);
        }
      }
      iter = drm_configs->find(flutter::EncodableValue("licenseServerUrl"));
      if (iter != drm_configs->end()) {
        if (std::holds_alternative<std::string>(iter->second)) {
          license_server_url = std::get<std::string>(iter->second);
        }
      }
    }

    const flutter::EncodableMap *player_options = msg.player_options();
    if (player_options) {
      auto iter =
          player_options->find(flutter::EncodableValue("prebufferMode"));
      if (iter != player_options->end()) {
        if (std::holds_alternative<bool>(iter->second)) {
          prebuffer_mode = std::get<bool>(iter->second);
        }
      }
    }

    const flutter::EncodableMap *http_headers_map = msg.http_headers();
    if (http_headers_map) {
      http_headers = *http_headers_map;
    }

  } else {
    return FlutterError("Invalid argument", "Either asset or uri must be set.");
  }

  auto player = std::make_unique<MediaPlayer>(
      plugin_registrar_->messenger(),
      FlutterDesktopPluginRegistrarGetView(registrar_ref_));
  int64_t player_id = player->Create(uri, drm_type, license_server_url,
                                     prebuffer_mode, http_headers);
  if (player_id == -1) {
    return FlutterError("Operation failed", "Failed to create a player.");
  }
  players_[player_id] = std::move(player);
  PlayerMessage result(player_id);
  return result;
}

ErrorOr<DurationMessage> VideoPlayerTizenPlugin::Duration(
    const PlayerMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  DurationMessage result(msg.player_id());
  auto duration_pair = player->GetDuration();
  flutter::EncodableList duration_range{
      flutter::EncodableValue(duration_pair.first),
      flutter::EncodableValue(duration_pair.second)};
  result.set_duration_range(duration_range);
  return result;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Dispose(
    const PlayerMessage &msg) {
  auto iter = players_.find(msg.player_id());
  if (iter != players_.end()) {
    iter->second->Dispose();
    players_.erase(iter);
  }
  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetLooping(
    const LoopingMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  if (!player->SetLooping(msg.is_looping())) {
    return FlutterError("SetLooping", "Player set looping failed");
  }
  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetVolume(
    const VolumeMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  if (!player->SetVolume(msg.volume())) {
    return FlutterError("SetVolume", "Player set volume failed");
  }
  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetPlaybackSpeed(
    const PlaybackSpeedMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  if (!player->SetPlaybackSpeed(msg.speed())) {
    return FlutterError("SetPlaybackSpeed", "Player set playback speed failed");
  }
  return std::nullopt;
}

ErrorOr<TrackMessage> VideoPlayerTizenPlugin::Track(
    const TrackTypeMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());

  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }

  TrackMessage result(msg.player_id(), player->GetTrackInfo(msg.track_type()));
  return result;
}

ErrorOr<bool> VideoPlayerTizenPlugin::SetTrackSelection(
    const SelectedTracksMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  return player->SetTrackSelection(msg.track_id(), msg.track_type());
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Play(
    const PlayerMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  if (!player->Play()) {
    return FlutterError("Play", "Player play failed");
  }
  return std::nullopt;
}

ErrorOr<bool> VideoPlayerTizenPlugin::SetDeactivate(const PlayerMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  return player->Deactivate();
}

ErrorOr<bool> VideoPlayerTizenPlugin::SetActivate(const PlayerMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  return player->Activate();
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Pause(
    const PlayerMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  if (!player->Pause()) {
    return FlutterError("Pause", "Player pause failed");
  }
  return std::nullopt;
}

ErrorOr<PositionMessage> VideoPlayerTizenPlugin::Position(
    const PlayerMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  PositionMessage result(msg.player_id(), player->GetPosition());
  return result;
}

void VideoPlayerTizenPlugin::SeekTo(
    const PositionMessage &msg,
    std::function<void(std::optional<FlutterError> reply)> result) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    result(FlutterError("Invalid argument", "Player not found"));
    return;
  }
  if (!player->SeekTo(msg.position(),
                      [result]() -> void { result(std::nullopt); })) {
    result(FlutterError("SeekTo", "Player seek to failed"));
  }
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetDisplayGeometry(
    const GeometryMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  player->SetDisplayRoi(msg.x(), msg.y(), msg.width(), msg.height());
  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetMixWithOthers(
    const MixWithOthersMessage &msg) {
  options_.SetMixWithOthers(msg.mix_with_others());
  return std::nullopt;
}

}  // namespace

void VideoPlayerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  VideoPlayerTizenPlugin::RegisterWithRegistrar(
      registrar, flutter::PluginRegistrarManager::GetInstance()
                     ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
