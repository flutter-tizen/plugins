#include "video_player_tizen_plugin.h"

#include <app_common.h>
#include <flutter/event_channel.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <string>

#include "flutter_texture_registrar.h"
#include "log.h"
#include "message.h"
#include "video_player.h"
#include "video_player_error.h"
#include "video_player_options.h"

namespace {

class VideoPlayerTizenPlugin : public flutter::Plugin, public VideoPlayerApi {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar);

  // Creates a plugin that communicates on the given channel.
  VideoPlayerTizenPlugin(flutter::PluginRegistrar *registrar);
  virtual ~VideoPlayerTizenPlugin();

  virtual void initialize() override;
  virtual TextureMessage create(const CreateMessage &createMsg) override;
  virtual void dispose(const TextureMessage &textureMsg) override;
  virtual void setLooping(const LoopingMessage &loopingMsg) override;
  virtual void setVolume(const VolumeMessage &volumeMsg) override;
  virtual void setPlaybackSpeed(const PlaybackSpeedMessage &speedMsg) override;
  virtual void play(const TextureMessage &textureMsg) override;
  virtual void pause(const TextureMessage &textureMsg) override;
  virtual PositionMessage position(const TextureMessage &textureMsg) override;
  virtual void seekTo(const PositionMessage &positionMsg,
                      const SeekCompletedCallback &onSeekCompleted) override;
  virtual void setMixWithOthers(
      const MixWithOthersMessage &mixWithOthersMsg) override;

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

  VideoPlayerApi::setup(registrar->messenger(), this);
}

VideoPlayerTizenPlugin::~VideoPlayerTizenPlugin() { DisposeAllPlayers(); }

void VideoPlayerTizenPlugin::DisposeAllPlayers() {
  LOG_DEBUG("[VideoPlayerTizenPlugin.DisposeAllPlayers] player count: %d",
            players_.size());

  for (const auto &[id, player] : players_) {
    player->dispose();
  }
  players_.clear();
}

void VideoPlayerTizenPlugin::initialize() {
  LOG_DEBUG("[VideoPlayerTizenPlugin.initialize] initialize");

  DisposeAllPlayers();
}

TextureMessage VideoPlayerTizenPlugin::create(const CreateMessage &createMsg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin.create] asset: %s",
            createMsg.getAsset().c_str());
  LOG_DEBUG("[VideoPlayerTizenPlugin.create] uri: %s",
            createMsg.getUri().c_str());
  LOG_DEBUG("[VideoPlayerTizenPlugin.create] packageName: %s",
            createMsg.getPackageName().c_str());
  LOG_DEBUG("[VideoPlayerTizenPlugin.create] formatHint: %s",
            createMsg.getFormatHint().c_str());

  std::string uri;
  if (createMsg.getAsset().empty()) {
    uri = createMsg.getUri();
  } else {
    char *res_path = app_get_resource_path();
    if (res_path) {
      uri = uri + res_path + "flutter_assets/" + createMsg.getAsset();
      free(res_path);
    } else {
      LOG_DEBUG(
          "[VideoPlayerTizenPlugin.create] Failed to get app resource path.");
      throw VideoPlayerError("Internal error", "Failed to get resource path.");
    }
  }
  LOG_DEBUG("[VideoPlayerTizenPlugin.create] player uri: %s", uri.c_str());

  auto player = std::make_unique<VideoPlayer>(
      plugin_registrar_, texture_registrar_, uri, options_);
  int64_t texture_id = player->getTextureId();
  players_[texture_id] = std::move(player);

  TextureMessage result;
  result.setTextureId(texture_id);
  return result;
}

void VideoPlayerTizenPlugin::dispose(const TextureMessage &textureMsg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin.dispose] textureId: %ld",
            textureMsg.getTextureId());

  auto iter = players_.find(textureMsg.getTextureId());
  if (iter != players_.end()) {
    iter->second->dispose();
    players_.erase(iter);
  }
}

void VideoPlayerTizenPlugin::setLooping(const LoopingMessage &loopingMsg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin.setLooping] textureId: %ld",
            loopingMsg.getTextureId());
  LOG_DEBUG("[VideoPlayerTizenPlugin.setLooping] isLooping: %d",
            loopingMsg.getIsLooping());

  auto iter = players_.find(loopingMsg.getTextureId());
  if (iter != players_.end()) {
    iter->second->setLooping(loopingMsg.getIsLooping());
  }
}

void VideoPlayerTizenPlugin::setVolume(const VolumeMessage &volumeMsg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin.setVolume] textureId: %ld",
            volumeMsg.getTextureId());
  LOG_DEBUG("[VideoPlayerTizenPlugin.setVolume] volume: %f",
            volumeMsg.getVolume());

  auto iter = players_.find(volumeMsg.getTextureId());
  if (iter != players_.end()) {
    iter->second->setVolume(volumeMsg.getVolume());
  }
}

void VideoPlayerTizenPlugin::setPlaybackSpeed(
    const PlaybackSpeedMessage &speedMsg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin.setPlaybackSpeed] textureId: %ld",
            speedMsg.getTextureId());
  LOG_DEBUG("[VideoPlayerTizenPlugin.setPlaybackSpeed] speed: %f",
            speedMsg.getSpeed());

  auto iter = players_.find(speedMsg.getTextureId());
  if (iter != players_.end()) {
    iter->second->setPlaybackSpeed(speedMsg.getSpeed());
  }
}

void VideoPlayerTizenPlugin::play(const TextureMessage &textureMsg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin.play] textureId: %ld",
            textureMsg.getTextureId());

  auto iter = players_.find(textureMsg.getTextureId());
  if (iter != players_.end()) {
    iter->second->play();
  }
}

void VideoPlayerTizenPlugin::pause(const TextureMessage &textureMsg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin.pause] textureId: %ld",
            textureMsg.getTextureId());

  auto iter = players_.find(textureMsg.getTextureId());
  if (iter != players_.end()) {
    iter->second->pause();
  }
}

PositionMessage VideoPlayerTizenPlugin::position(
    const TextureMessage &textureMsg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin.position] textureId: %ld",
            textureMsg.getTextureId());

  PositionMessage result;
  auto iter = players_.find(textureMsg.getTextureId());
  if (iter != players_.end()) {
    result.setTextureId(textureMsg.getTextureId());
    result.setPosition(iter->second->getPosition());
  }
  return result;
}

void VideoPlayerTizenPlugin::seekTo(
    const PositionMessage &positionMsg,
    const SeekCompletedCallback &onSeekCompleted) {
  LOG_DEBUG("[VideoPlayerTizenPlugin.seekTo] textureId: %ld",
            positionMsg.getTextureId());
  LOG_DEBUG("[VideoPlayerTizenPlugin.seekTo] position: %ld",
            positionMsg.getPosition());

  auto iter = players_.find(positionMsg.getTextureId());
  if (iter != players_.end()) {
    iter->second->seekTo(positionMsg.getPosition(), onSeekCompleted);
  }
}

void VideoPlayerTizenPlugin::setMixWithOthers(
    const MixWithOthersMessage &mixWithOthersMsg) {
  LOG_DEBUG("[VideoPlayerTizenPlugin.setMixWithOthers] mixWithOthers: %d",
            mixWithOthersMsg.getMixWithOthers());

  options_.setMixWithOthers(mixWithOthersMsg.getMixWithOthers());
}

}  // namespace

void VideoPlayerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  VideoPlayerTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
