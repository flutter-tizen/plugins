// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player_tizen_plugin.h"

#include <app_common.h>
#include <flutter/event_channel.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <flutter_texture_registrar.h>

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

  virtual std::optional<FlutterError> Initialize() override;
  virtual ErrorOr<TextureMessage> Create(const CreateMessage &msg) override;
  virtual std::optional<FlutterError> Dispose(
      const TextureMessage &msg) override;
  virtual std::optional<FlutterError> SetLooping(
      const LoopingMessage &msg) override;
  virtual std::optional<FlutterError> SetVolume(
      const VolumeMessage &msg) override;
  virtual std::optional<FlutterError> SetPlaybackSpeed(
      const PlaybackSpeedMessage &msg) override;
  virtual std::optional<FlutterError> Play(const TextureMessage &msg) override;
  virtual ErrorOr<PositionMessage> Position(const TextureMessage &msg) override;
  virtual void SeekTo(
      const PositionMessage &msg,
      std::function<void(std::optional<FlutterError> reply)> result) override;
  virtual std::optional<FlutterError> Pause(const TextureMessage &msg) override;
  virtual std::optional<FlutterError> SetMixWithOthers(
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
}

ErrorOr<TextureMessage> VideoPlayerTizenPlugin::Create(
    const CreateMessage &msg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin] asset: %s", msg.asset()->c_str());

  std::string uri;
  if (msg.asset()->empty()) {
    uri = *msg.uri();
  } else {
    char *res_path = app_get_resource_path();
    if (res_path) {
      uri = uri + res_path + "flutter_assets/" + *msg.asset();
      free(res_path);
    } else {
      throw VideoPlayerError("Internal error", "Failed to get resource path.");
    }
  }
  LOG_DEBUG("[VideoPlayerTizenPlugin] uri: %s", uri.c_str());

  auto player = std::make_unique<VideoPlayer>(
      plugin_registrar_, texture_registrar_, uri, options_);
  int64_t texture_id = player->GetTextureId();
  players_[texture_id] = std::move(player);

  TextureMessage result;
  result.set_texture_id(texture_id);
  return result;
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Dispose(
    const TextureMessage &msg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin] textureId: %ld", msg.texture_id());

  auto iter = players_.find(msg.texture_id());
  if (iter != players_.end()) {
    iter->second->Dispose();
    players_.erase(iter);
  }
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetLooping(
    const LoopingMessage &msg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin] textureId: %ld", msg.texture_id());
  LOG_DEBUG("[VideoPlayerTizenPlugin] isLooping: %d", msg.is_looping());

  auto iter = players_.find(msg.texture_id());
  if (iter != players_.end()) {
    iter->second->SetLooping(msg.is_looping());
  }
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetVolume(
    const VolumeMessage &msg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin] textureId: %ld", msg.texture_id());
  LOG_DEBUG("[VideoPlayerTizenPlugin] volume: %f", msg.volume());

  auto iter = players_.find(msg.texture_id());
  if (iter != players_.end()) {
    iter->second->SetVolume(msg.volume());
  }
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetPlaybackSpeed(
    const PlaybackSpeedMessage &msg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin] textureId: %ld", msg.texture_id());
  LOG_DEBUG("[VideoPlayerTizenPlugin] speed: %f", msg.speed());

  auto iter = players_.find(msg.texture_id());
  if (iter != players_.end()) {
    iter->second->SetPlaybackSpeed(msg.speed());
  }
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Play(
    const TextureMessage &msg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin] textureId: %ld", msg.texture_id());

  auto iter = players_.find(msg.texture_id());
  if (iter != players_.end()) {
    iter->second->Play();
  }
}

std::optional<FlutterError> VideoPlayerTizenPlugin::Pause(
    const TextureMessage &msg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin] textureId: %ld", msg.texture_id());

  auto iter = players_.find(msg.texture_id());
  if (iter != players_.end()) {
    iter->second->Pause();
  }
}

ErrorOr<PositionMessage> VideoPlayerTizenPlugin::Position(
    const TextureMessage &msg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin] textureId: %ld", msg.texture_id());

  PositionMessage result;
  auto iter = players_.find(msg.texture_id());
  if (iter != players_.end()) {
    result.set_texture_id(msg.texture_id());
    result.set_position(iter->second->GetPosition());
  }
  return result;
}

void VideoPlayerTizenPlugin::SeekTo(
    const PositionMessage &msg,
    std::function<void(std::optional<FlutterError> reply)> result) {
  LOG_DEBUG("[VideoPlayerTizenPlugin] textureId: %ld", msg.texture_id());
  LOG_DEBUG("[VideoPlayerTizenPlugin] position: %ld", msg.position());

  auto iter = players_.find(msg.texture_id());
  if (iter != players_.end()) {
    iter->second->SeekTo(msg.position(),
                         [result]() -> void { result(std::nullopt); });
  }
}

std::optional<FlutterError> VideoPlayerTizenPlugin::SetMixWithOthers(
    const MixWithOthersMessage &msg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin] mixWithOthers: %d",
            msg.mix_with_others());

  options_.setMixWithOthers(msg.mix_with_others());
}

}  // namespace

void VideoPlayerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  VideoPlayerTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
