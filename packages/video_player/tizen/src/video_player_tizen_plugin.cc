// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player_tizen_plugin.h"

#include <app_common.h>
#include <flutter/plugin_registrar.h>

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "log.h"
#include "messages.h"
#include "video_player.h"
#include "video_player_error.h"
#include "video_player_options.h"

namespace {

class VideoPlayerTizenPlugin : public flutter::Plugin,
                               public TizenVideoPlayerApi {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar);

  VideoPlayerTizenPlugin(flutter::PluginRegistrar *registrar);
  virtual ~VideoPlayerTizenPlugin();

  std::optional<FlutterError> Initialize() override;
  ErrorOr<TextureMessage> Create(const CreateMessage &msg) override;
  std::optional<FlutterError> Dispose(const TextureMessage &msg) override;
  std::optional<FlutterError> SetLooping(const LoopingMessage &msg) override;
  std::optional<FlutterError> SetVolume(const VolumeMessage &msg) override;
  std::optional<FlutterError> SetPlaybackSpeed(
      const PlaybackSpeedMessage &msg) override;
  std::optional<FlutterError> Play(const TextureMessage &msg) override;
  ErrorOr<PositionMessage> Position(const TextureMessage &msg) override;
  void SeekTo(
      const PositionMessage &msg,
      std::function<void(std::optional<FlutterError> reply)> result) override;
  std::optional<FlutterError> Pause(const TextureMessage &msg) override;
  std::optional<FlutterError> SetMixWithOthers(
      const MixWithOthersMessage &msg) override;

 private:
  void DisposeAllPlayers();

  flutter::PluginRegistrar *plugin_registrar_;
  flutter::TextureRegistrar *texture_registrar_;
  VideoPlayerOptions options_;
  std::map<int64_t, std::unique_ptr<VideoPlayer>> players_;
};

void VideoPlayerTizenPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrar *registrar) {
  auto plugin = std::make_unique<VideoPlayerTizenPlugin>(registrar);
  registrar->AddPlugin(std::move(plugin));
}

VideoPlayerTizenPlugin::VideoPlayerTizenPlugin(
    flutter::PluginRegistrar *registrar)
    : plugin_registrar_(registrar) {
  texture_registrar_ = registrar->texture_registrar();

  TizenVideoPlayerApi::SetUp(registrar->messenger(), this);
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

ErrorOr<TextureMessage> VideoPlayerTizenPlugin::Create(
    const CreateMessage &msg) {
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
  LOG_DEBUG("[VideoPlayerTizenPlugin] uri: %s", uri.c_str());

  int64_t texture_id = 0;
  try {
    auto player = std::make_unique<VideoPlayer>(
        plugin_registrar_, texture_registrar_, uri, options_);
    texture_id = player->GetTextureId();
    players_[texture_id] = std::move(player);
  } catch (const VideoPlayerError &error) {
    return FlutterError(error.code(), error.message());
  }

  TextureMessage result(texture_id);
  return result;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Dispose(
    const TextureMessage &msg) {
  auto iter = players_.find(msg.texture_id());
  if (iter != players_.end()) {
    iter->second->Dispose();
    players_.erase(iter);
  }
  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetLooping(
    const LoopingMessage &msg) {
  auto iter = players_.find(msg.texture_id());
  if (iter == players_.end()) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  try {
    iter->second->SetLooping(msg.is_looping());
  } catch (const VideoPlayerError &error) {
    return FlutterError(error.code(), error.message());
  }

  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetVolume(
    const VolumeMessage &msg) {
  auto iter = players_.find(msg.texture_id());
  if (iter == players_.end()) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  try {
    iter->second->SetVolume(msg.volume());
  } catch (const VideoPlayerError &error) {
    return FlutterError(error.code(), error.message());
  }

  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetPlaybackSpeed(
    const PlaybackSpeedMessage &msg) {
  auto iter = players_.find(msg.texture_id());
  if (iter == players_.end()) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  try {
    iter->second->SetPlaybackSpeed(msg.speed());
  } catch (const VideoPlayerError &error) {
    return FlutterError(error.code(), error.message());
  }

  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Play(
    const TextureMessage &msg) {
  auto iter = players_.find(msg.texture_id());
  if (iter == players_.end()) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  try {
    iter->second->Play();
  } catch (const VideoPlayerError &error) {
    return FlutterError(error.code(), error.message());
  }

  return std::nullopt;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Pause(
    const TextureMessage &msg) {
  auto iter = players_.find(msg.texture_id());
  if (iter == players_.end()) {
    return FlutterError("Invalid argument", "Player not found.");
  }
  try {
    iter->second->Pause();
  } catch (const VideoPlayerError &error) {
    return FlutterError(error.code(), error.message());
  }

  return std::nullopt;
}

ErrorOr<PositionMessage> VideoPlayerTizenPlugin::Position(
    const TextureMessage &msg) {
  auto iter = players_.find(msg.texture_id());
  if (iter == players_.end()) {
    return FlutterError("Invalid argument", "Player not found.");
  }

  int32_t position = 0;
  try {
    position = iter->second->GetPosition();
  } catch (const VideoPlayerError &error) {
    return FlutterError(error.code(), error.message());
  }

  PositionMessage result(msg.texture_id(), position);
  return result;
}

void VideoPlayerTizenPlugin::SeekTo(
    const PositionMessage &msg,
    std::function<void(std::optional<FlutterError> reply)> result) {
  auto iter = players_.find(msg.texture_id());
  if (iter == players_.end()) {
    result(FlutterError("Invalid argument", "Player not found."));
    return;
  }
  try {
    iter->second->SeekTo(msg.position(),
                         [result]() -> void { result(std::nullopt); });
  } catch (const VideoPlayerError &error) {
    result(FlutterError(error.code(), error.message()));
  }
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
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
