// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "audioplayers_tizen_plugin.h"

#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <string>
#include <variant>

#include "audio_player.h"
#include "audio_player_error.h"

namespace {

const char *kInvalidArgument = "Invalid argument";

template <typename T>
bool GetValueFromEncodableMap(const flutter::EncodableMap *map, const char *key,
                              T &out) {
  auto iter = map->find(flutter::EncodableValue(key));
  if (iter != map->end() && !iter->second.IsNull()) {
    if (auto *value = std::get_if<T>(&iter->second)) {
      out = *value;
      return true;
    }
  }
  return false;
}

template <typename T>
T GetRequiredArg(const flutter::EncodableMap *arguments, const char *key) {
  T value;
  if (GetValueFromEncodableMap(arguments, key, value)) {
    return value;
  }
  std::string message =
      "No " + std::string(key) + " provided or has invalid type or value.";
  throw std::invalid_argument(message);
}

ReleaseMode StringToReleaseMode(std::string release_mode) {
  if (release_mode == "ReleaseMode.release") {
    return ReleaseMode::kRelease;
  } else if (release_mode == "ReleaseMode.loop") {
    return ReleaseMode::kLoop;
  } else if (release_mode == "ReleaseMode.stop") {
    return ReleaseMode::kStop;
  }
  throw std::invalid_argument("Invalid release mode.");
}

class AudioplayersTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "xyz.luan/audioplayers",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<AudioplayersTizenPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });
    plugin->channel_ = std::move(channel);

    registrar->AddPlugin(std::move(plugin));
  }

  AudioplayersTizenPlugin() {}

  virtual ~AudioplayersTizenPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const auto *arguments =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error(kInvalidArgument, "No arguments provided.");
      return;
    }

    std::string player_id;
    if (!GetValueFromEncodableMap(arguments, "playerId", player_id)) {
      result->Error(kInvalidArgument, "No playerId provided.");
      return;
    }
    AudioPlayer *player = GetAudioPlayer(player_id);

    try {
      const std::string &method_name = method_call.method_name();
      if (method_name == "setSourceBytes") {
        std::vector<uint8_t> bytes =
            GetRequiredArg<std::vector<uint8_t>>(arguments, "bytes");
        player->SetDataSource(bytes);
        result->Success();
      } else if (method_name == "resume") {
        player->Play();
        result->Success();
      } else if (method_name == "pause") {
        player->Pause();
        result->Success();
      } else if (method_name == "stop") {
        player->Stop();
        result->Success();
      } else if (method_name == "release") {
        player->Release();
        result->Success();
      } else if (method_name == "seek") {
        player->Seek(GetRequiredArg<int32_t>(arguments, "position"));
        result->Success();
      } else if (method_name == "setVolume") {
        player->SetVolume(GetRequiredArg<double>(arguments, "volume"));
        result->Success();
      } else if (method_name == "setSourceUrl") {
        bool is_local = false;
        GetValueFromEncodableMap(arguments, "isLocal", is_local);

        std::string url = GetRequiredArg<std::string>(arguments, "url");
        const std::string file_protocol_prefix = "file://";
        if (is_local && url.find(file_protocol_prefix) == 0) {
          url = url.substr(file_protocol_prefix.length());
        }
        player->SetUrl(url);
        result->Success();
      } else if (method_name == "setPlaybackRate") {
        player->SetPlaybackRate(
            GetRequiredArg<double>(arguments, "playbackRate"));
        result->Success();
      } else if (method_name == "setReleaseMode") {
        std::string release_mode =
            GetRequiredArg<std::string>(arguments, "releaseMode");
        player->SetReleaseMode(StringToReleaseMode(release_mode));
        result->Success();
      } else if (method_name == "getDuration") {
        result->Success(flutter::EncodableValue(player->GetDuration()));
      } else if (method_name == "getCurrentPosition") {
        result->Success(flutter::EncodableValue(player->GetCurrentPosition()));
      } else if (method_name == "setPlayerMode") {
        bool low_latency =
            GetRequiredArg<std::string>(arguments, "playerMode") ==
            "PlayerMode.lowLatency";
        player->SetLatencyMode(low_latency);
        result->Success();
      } else {
        result->NotImplemented();
      }
    } catch (const std::invalid_argument &error) {
      result->Error(kInvalidArgument, error.what());
    } catch (const AudioPlayerError &error) {
      result->Error(error.code(), error.message());
    }
  }

  AudioPlayer *GetAudioPlayer(const std::string &player_id) {
    auto iter = audio_players_.find(player_id);
    if (iter != audio_players_.end()) {
      return iter->second.get();
    }

    PreparedListener prepared_listener = [channel = channel_.get()](
                                             const std::string &player_id,
                                             int32_t duration) {
      flutter::EncodableMap wrapped = {{flutter::EncodableValue("playerId"),
                                        flutter::EncodableValue(player_id)},
                                       {flutter::EncodableValue("value"),
                                        flutter::EncodableValue(duration)}};
      auto arguments = std::make_unique<flutter::EncodableValue>(wrapped);
      channel->InvokeMethod("audio.onDuration", std::move(arguments));
    };

    SeekCompletedListener seek_completed_listener =
        [channel = channel_.get()](const std::string &player_id) {
          flutter::EncodableMap wrapped = {
              {flutter::EncodableValue("playerId"),
               flutter::EncodableValue(player_id)}};
          auto arguments = std::make_unique<flutter::EncodableValue>(wrapped);
          channel->InvokeMethod("audio.onSeekComplete", std::move(arguments));
        };

    UpdatePositionListener update_position_listener =
        [channel = channel_.get()](const std::string &player_id,
                                   const int32_t duration,
                                   const int32_t position) {
          flutter::EncodableMap duration_wrapped = {
              {flutter::EncodableValue("playerId"),
               flutter::EncodableValue(player_id)},
              {flutter::EncodableValue("value"),
               flutter::EncodableValue(duration)}};
          channel->InvokeMethod(
              "audio.onDuration",
              std::make_unique<flutter::EncodableValue>(duration_wrapped));

          flutter::EncodableMap position_wrapped = {
              {flutter::EncodableValue("playerId"),
               flutter::EncodableValue(player_id)},
              {flutter::EncodableValue("value"),
               flutter::EncodableValue(position)}};
          channel->InvokeMethod(
              "audio.onCurrentPosition",
              std::make_unique<flutter::EncodableValue>(position_wrapped));
        };

    PlayCompletedListener play_completed_listener =
        [channel = channel_.get()](const std::string &player_id) {
          flutter::EncodableMap wrapped = {
              {flutter::EncodableValue("playerId"),
               flutter::EncodableValue(player_id)}};
          auto arguments = std::make_unique<flutter::EncodableValue>(wrapped);
          channel->InvokeMethod("audio.onComplete", std::move(arguments));
        };

    ErrorListener error_listener = [channel = channel_.get()](
                                       const std::string &player_id,
                                       const std::string &message) {
      flutter::EncodableMap wrapped = {
          {flutter::EncodableValue("playerId"),
           flutter::EncodableValue(player_id)},
          {flutter::EncodableValue("value"), flutter::EncodableValue(message)}};
      auto arguments = std::make_unique<flutter::EncodableValue>(wrapped);
      channel->InvokeMethod("audio.onError", std::move(arguments));
    };

    auto player = std::make_unique<AudioPlayer>(
        player_id, prepared_listener, update_position_listener,
        seek_completed_listener, play_completed_listener, error_listener);
    audio_players_[player_id] = std::move(player);

    return audio_players_[player_id].get();
  }

  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel_;
  std::map<std::string, std::unique_ptr<AudioPlayer>> audio_players_;
};

}  // namespace

void AudioplayersTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  AudioplayersTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
