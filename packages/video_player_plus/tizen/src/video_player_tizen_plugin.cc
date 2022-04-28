// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "video_player_tizen_plugin.h"

#include <app_common.h>
#include <flutter/event_channel.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <flutter_tizen.h>

#include "flutter_texture_registrar.h"
#include "log.h"
#include "message.h"
#include "video_player.h"
#include "video_player_error.h"
#include "video_player_options.h"

class VideoPlayerTizenPlugin : public flutter::Plugin, public VideoPlayerApi {
 public:
  static void RegisterWithRegistrar(
      FlutterDesktopPluginRegistrarRef registrar_ref,
      flutter::PluginRegistrar *plugin_registrar);
  VideoPlayerTizenPlugin(FlutterDesktopPluginRegistrarRef registrar_ref,
                         flutter::PluginRegistrar *plugin_registrar);
  virtual ~VideoPlayerTizenPlugin();

  void initialize() override;
  TextureMessage create(const CreateMessage &createMsg) override;
  void dispose(const TextureMessage &textureMsg) override;
  void setLooping(const LoopingMessage &loopingMsg) override;
  void setVolume(const VolumeMessage &volumeMsg) override;
  void setPlaybackSpeed(const PlaybackSpeedMessage &speedMsg) override;
  void play(const TextureMessage &textureMsg) override;
  void pause(const TextureMessage &textureMsg) override;
  PositionMessage position(const TextureMessage &textureMsg) override;
  void seekTo(const PositionMessage &positionMsg,
              const SeekCompletedCb &onSeekCompleted) override;
  void setMixWithOthers(const MixWithOthersMessage &mixWithOthersMsg) override;
  void setDisplayRoi(const GeometryMessage &geometryMsg) override;
  bool setBufferingConfig(const BufferingConfigMessage &configMsg) override;

 private:
  void DisposeAllPlayers();
  FlutterDesktopPluginRegistrarRef registrar_ref_;
  VideoPlayerOptions options_;
  flutter::PluginRegistrar *plugin_registrar_;
  std::map<int64_t, std::unique_ptr<VideoPlayer>> players_;
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
  VideoPlayerApi::setup(plugin_registrar_->messenger(), this);
}

VideoPlayerTizenPlugin::~VideoPlayerTizenPlugin() { DisposeAllPlayers(); }

void VideoPlayerTizenPlugin::DisposeAllPlayers() {
  auto iter = players_.begin();
  while (iter != players_.end()) {
    iter->second->Dispose();
    iter++;
  }
  players_.clear();
}

void VideoPlayerTizenPlugin::initialize() { DisposeAllPlayers(); }

TextureMessage VideoPlayerTizenPlugin::create(const CreateMessage &createMsg) {
  std::unique_ptr<VideoPlayer> player =
      std::make_unique<VideoPlayer>(registrar_ref_, createMsg);
  int64_t textureId = player->Create();
  players_[textureId] = std::move(player);
  TextureMessage result;
  result.setTextureId(textureId);
  return result;
}

void VideoPlayerTizenPlugin::dispose(const TextureMessage &textureMsg) {
  auto iter = players_.find(textureMsg.getTextureId());
  if (iter != players_.end()) {
    iter->second->Dispose();
    players_.erase(iter);
  }
}

void VideoPlayerTizenPlugin::setLooping(const LoopingMessage &loopingMsg) {
  auto iter = players_.find(loopingMsg.getTextureId());
  if (iter != players_.end()) {
    iter->second->SetLooping(loopingMsg.getIsLooping());
  }
}

void VideoPlayerTizenPlugin::setVolume(const VolumeMessage &volumeMsg) {
  auto iter = players_.find(volumeMsg.getTextureId());
  if (iter != players_.end()) {
    iter->second->SetVolume(volumeMsg.getVolume());
  }
}

void VideoPlayerTizenPlugin::setPlaybackSpeed(
    const PlaybackSpeedMessage &speedMsg) {
  auto iter = players_.find(speedMsg.getTextureId());
  if (iter != players_.end()) {
    iter->second->SetPlaybackSpeed(speedMsg.getSpeed());
  }
}

void VideoPlayerTizenPlugin::play(const TextureMessage &textureMsg) {
  auto iter = players_.find(textureMsg.getTextureId());
  if (iter != players_.end()) {
    iter->second->Play();
  }
}

void VideoPlayerTizenPlugin::pause(const TextureMessage &textureMsg) {
  auto iter = players_.find(textureMsg.getTextureId());
  if (iter != players_.end()) {
    iter->second->Pause();
  }
}

PositionMessage VideoPlayerTizenPlugin::position(
    const TextureMessage &textureMsg) {
  PositionMessage result;
  auto iter = players_.find(textureMsg.getTextureId());
  if (iter != players_.end()) {
    result.setTextureId(textureMsg.getTextureId());
    result.setPosition(iter->second->GetPosition());
  }
  return result;
}

void VideoPlayerTizenPlugin::seekTo(const PositionMessage &positionMsg,
                                    const SeekCompletedCb &onSeekCompleted) {
  auto iter = players_.find(positionMsg.getTextureId());
  if (iter != players_.end()) {
    iter->second->SeekTo(positionMsg.getPosition(), onSeekCompleted);
  }
}

void VideoPlayerTizenPlugin::setDisplayRoi(const GeometryMessage &geometryMsg) {
  auto iter = players_.find(geometryMsg.getTextureId());
  if (iter != players_.end()) {
    iter->second->SetDisplayRoi(geometryMsg.getX(), geometryMsg.getY(),
                                geometryMsg.getW(), geometryMsg.getH());
  }
}

bool VideoPlayerTizenPlugin::setBufferingConfig(
    const BufferingConfigMessage &configMsg) {
  auto iter = players_.find(configMsg.GetTextureId());
  if (iter != players_.end()) {
    return iter->second->SetBufferingConfig(configMsg.GetOption(),
                                            configMsg.GetAmount());
  }
  return false;
}

void VideoPlayerTizenPlugin::setMixWithOthers(
    const MixWithOthersMessage &mixWithOthersMsg) {
  options_.SetMixWithOthers(mixWithOthersMsg.getMixWithOthers());
}

void VideoPlayerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  VideoPlayerTizenPlugin::RegisterWithRegistrar(
      registrar, flutter::PluginRegistrarManager::GetInstance()
                     ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
