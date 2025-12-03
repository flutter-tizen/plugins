// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player_tizen_plugin.h"

#include <app_common.h>
#include <dlfcn.h>
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
#include "plus_player.h"
#include "video_player_options.h"

namespace video_player_avplay_tizen {

class VideoPlayerTizenPlugin : public flutter::Plugin,
                               public VideoPlayerAvplayApi {
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
  ErrorOr<DurationMessage> Duration(const PlayerMessage &msg) override;
  void SeekTo(
      const PositionMessage &msg,
      std::function<void(std::optional<FlutterError> reply)> result) override;
  std::optional<FlutterError> Pause(const PlayerMessage &msg) override;
  std::optional<FlutterError> SetMixWithOthers(
      const MixWithOthersMessage &msg) override;
  std::optional<FlutterError> SetDisplayGeometry(
      const GeometryMessage &msg) override;
  ErrorOr<std::string> GetStreamingProperty(
      const StreamingPropertyTypeMessage &msg) override;
  ErrorOr<bool> SetBufferConfig(const BufferConfigMessage &msg) override;
  std::optional<FlutterError> SetStreamingProperty(
      const StreamingPropertyMessage &msg) override;

  ErrorOr<bool> SetDisplayRotate(const RotationMessage &msg) override;
  ErrorOr<bool> SetDisplayMode(const DisplayModeMessage &msg) override;
  ErrorOr<bool> SetData(const DashPropertyMapMessage &msg) override;
  ErrorOr<DashPropertyMapMessage> GetData(
      const DashPropertyTypeListMessage &msg) override;
  ErrorOr<bool> UpdateDashToken(int64_t player_id,
                                const std::string &dashToken) override;
  ErrorOr<TrackMessage> GetActiveTrackInfo(const PlayerMessage &msg) override;

  std::optional<FlutterError> Suspend(int64_t player_id) override;
  std::optional<FlutterError> Restore(int64_t palyer_id,
                                      const CreateMessage *msg,
                                      int64_t resume_time) override;

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
  VideoPlayerAvplayApi::SetUp(plugin_registrar->messenger(), this);
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
  } else {
    return FlutterError("Invalid argument", "Either asset or uri must be set.");
  }

  std::unique_ptr<VideoPlayer> player = nullptr;
  if (uri.substr(0, 4) == "http") {
    player = std::make_unique<PlusPlayer>(
        plugin_registrar_->messenger(),
        FlutterDesktopPluginRegistrarGetView(registrar_ref_));
  } else {
    player = std::make_unique<MediaPlayer>(
        plugin_registrar_->messenger(),
        FlutterDesktopPluginRegistrarGetView(registrar_ref_));
  }
  int64_t player_id = player->Create(uri, msg);
  if (player_id == -1) {
    return FlutterError("Operation failed", "Failed to create a player.");
  }
  players_[player_id] = std::move(player);
  PlayerMessage result(player_id);
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

ErrorOr<DurationMessage> VideoPlayerTizenPlugin::Duration(
    const PlayerMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  DurationMessage result(msg.player_id());
  auto duration_pair = player->GetDuration();
  flutter::EncodableList duration_range{
      flutter::EncodableValue(duration_pair.first),
      flutter::EncodableValue(duration_pair.second)};
  result.set_duration_range(duration_range);
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

ErrorOr<std::string> VideoPlayerTizenPlugin::GetStreamingProperty(
    const StreamingPropertyTypeMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  return player->GetStreamingProperty(msg.streaming_property_type());
}

ErrorOr<bool> VideoPlayerTizenPlugin::SetBufferConfig(
    const BufferConfigMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  return player->SetBufferConfig(msg.buffer_config_type(),
                                 msg.buffer_config_value());
}

ErrorOr<bool> VideoPlayerTizenPlugin::SetDisplayRotate(
    const RotationMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  return player->SetDisplayRotate(msg.rotation());
}

ErrorOr<bool> VideoPlayerTizenPlugin::SetDisplayMode(
    const DisplayModeMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  return player->SetDisplayMode(msg.display_mode());
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Suspend(int64_t player_id) {
  VideoPlayer *player = FindPlayerById(player_id);
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  if (!player->Suspend()) {
    return FlutterError("Operation failed", "Player suspend error");
  }
  return std::nullopt;
}
std::optional<FlutterError> VideoPlayerTizenPlugin::Restore(
    int64_t player_id, const CreateMessage *msg, int64_t resume_time) {
  VideoPlayer *player = FindPlayerById(player_id);
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }

  if (!player->Restore(msg, resume_time)) {
    return FlutterError("Operation failed", "Player restore error");
  }
  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetStreamingProperty(
    const StreamingPropertyMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  player->SetStreamingProperty(msg.streaming_property_type(),
                               msg.streaming_property_value());
  return std::nullopt;
}

ErrorOr<bool> VideoPlayerTizenPlugin::SetData(
    const DashPropertyMapMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  return player->SetData(msg.map_data());
}

ErrorOr<DashPropertyMapMessage> VideoPlayerTizenPlugin::GetData(
    const DashPropertyTypeListMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  DashPropertyMapMessage result(msg.player_id(),
                                player->GetData(msg.type_list()));
  return result;
}

ErrorOr<bool> VideoPlayerTizenPlugin::UpdateDashToken(
    int64_t player_id, const std::string &dashToken) {
  VideoPlayer *player = FindPlayerById(player_id);
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  return player->UpdateDashToken(dashToken);
}

ErrorOr<TrackMessage> VideoPlayerTizenPlugin::GetActiveTrackInfo(
    const PlayerMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found");
  }
  TrackMessage result(msg.player_id(), player->GetActiveTrackInfo());
  return result;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetMixWithOthers(
    const MixWithOthersMessage &msg) {
  options_.SetMixWithOthers(msg.mix_with_others());
  return std::nullopt;
}

}  // namespace video_player_avplay_tizen

void VideoPlayerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  video_player_avplay_tizen::VideoPlayerTizenPlugin::RegisterWithRegistrar(
      registrar, flutter::PluginRegistrarManager::GetInstance()
                     ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
