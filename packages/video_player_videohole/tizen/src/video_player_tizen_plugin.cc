// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player_tizen_plugin.h"

#include <flutter/plugin_registrar.h>
#include <flutter_tizen.h>

#include <cstdint>
#include <map>
#include <optional>

#include "messages.h"
#include "video_player.h"
#include "video_player_options.h"

namespace {

class VideoPlayerTizenPlugin : public flutter::Plugin, public VideoPlayerApi {
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
  std::optional<FlutterError> Play(const PlayerMessage &msg) override;
  ErrorOr<PositionMessage> Position(const PlayerMessage &msg) override;
  std::optional<FlutterError> SeekTo(const PositionMessage &msg) override;
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
  VideoPlayerApi::SetUp(plugin_registrar->messenger(), this);
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
  FlutterDesktopViewRef flutter_view =
      FlutterDesktopPluginRegistrarGetView(registrar_ref_);
  if (!flutter_view) {
    return FlutterError("Operation failed", "Could not get a Flutter view.");
  }
  void *native_window = FlutterDesktopViewGetNativeHandle(flutter_view);
  if (!native_window) {
    return FlutterError("Operation failed",
                        "Could not get a native window handle.");
  }
  std::unique_ptr<VideoPlayer> player =
      std::make_unique<VideoPlayer>(plugin_registrar_, native_window, msg);
  int64_t player_id = player->Create();
  if (player_id == -1) {
    return FlutterError("Operation failed", "Failed to create a player.");
  }
  players_[player_id] = std::move(player);

  PlayerMessage result;
  result.set_player_id(player_id);
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
    return FlutterError("Invalid argument", "Player not found.");
  }
  player->SetLooping(msg.is_looping());

  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetVolume(
    const VolumeMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  player->SetVolume(msg.volume());

  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetPlaybackSpeed(
    const PlaybackSpeedMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  player->SetPlaybackSpeed(msg.speed());

  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Play(
    const PlayerMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  player->Play();

  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Pause(
    const PlayerMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  player->Pause();

  return std::nullopt;
}

ErrorOr<PositionMessage> VideoPlayerTizenPlugin::Position(
    const PlayerMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found.");
  }

  PositionMessage result;
  result.set_player_id(msg.player_id());
  result.set_position(player->GetPosition());
  return result;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SeekTo(
    const PositionMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  player->SeekTo(msg.position());

  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetDisplayGeometry(
    const GeometryMessage &msg) {
  VideoPlayer *player = FindPlayerById(msg.player_id());
  if (!player) {
    return FlutterError("Invalid argument", "Player not found.");
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

intptr_t VideoPlayerTizenPluginInitDartApi(void *data) {
  return Dart_InitializeApiDL(data);
}

void VideoPlayerTizenPluginRegisterSendPort(int64_t player_id,
                                            Dart_Port send_port) {
  VideoPlayer *player = VideoPlayerTizenPlugin::FindPlayerById(player_id);
  if (player) {
    player->RegisterSendPort(send_port);
  }
}
