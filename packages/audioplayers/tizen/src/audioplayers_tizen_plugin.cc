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
#include "log.h"

namespace {

const char *kInvalidArgument = "Invalid argument";

template <typename T>
static bool GetValueFromEncodableMap(const flutter::EncodableMap *map,
                                     const char *key, T &out) {
  auto iter = map->find(flutter::EncodableValue(key));
  if (iter != map->end() && !iter->second.IsNull()) {
    if (auto *value = std::get_if<T>(&iter->second)) {
      out = *value;
      return true;
    }
  }
  return false;
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

    std::string player_id, mode;
    if (!GetValueFromEncodableMap(arguments, "playerId", player_id)) {
      result->Error(kInvalidArgument, "No playerId provided.");
      return;
    }
    GetValueFromEncodableMap(arguments, "mode", mode);

    AudioPlayer *player = GetAudioPlayer(player_id, mode);

    const auto &method_name = method_call.method_name();
    try {
      if (method_name == "play") {
        double volume = 0.0;
        std::string url;
        int32_t position = 0;
        if (GetValueFromEncodableMap(arguments, "volume", volume)) {
          player->SetVolume(volume);
        }
        if (GetValueFromEncodableMap(arguments, "url", url)) {
          player->SetUrl(url);
        }
        player->Play();
        if (GetValueFromEncodableMap(arguments, "position", position)) {
          player->Seek(position);
        }
      } else if (method_name == "playBytes") {
        double volume = 0.0;
        std::vector<uint8_t> bytes;
        int32_t position = 0;
        if (GetValueFromEncodableMap(arguments, "volume", volume)) {
          player->SetVolume(volume);
        }
        if (GetValueFromEncodableMap(arguments, "bytes", bytes)) {
          player->SetDataSource(bytes);
        }
        player->Play();
        if (GetValueFromEncodableMap(arguments, "position", position)) {
          player->Seek(position);
        }
      } else if (method_name == "resume") {
        player->Play();
      } else if (method_name == "pause") {
        player->Pause();
      } else if (method_name == "stop") {
        player->Stop();
      } else if (method_name == "release") {
        player->Release();
      } else if (method_name == "seek") {
        int32_t position = 0;
        if (GetValueFromEncodableMap(arguments, "position", position)) {
          player->Seek(position);
        } else {
          result->Error(kInvalidArgument,
                        "No position provided or is invalid value.");
          return;
        }
      } else if (method_name == "setVolume") {
        double volume = 0.0;
        if (GetValueFromEncodableMap(arguments, "volume", volume)) {
          player->SetVolume(volume);
        } else {
          result->Error(kInvalidArgument,
                        "No volume provided or is invalid value.");
          return;
        }
      } else if (method_name == "setUrl") {
        std::string url;
        if (GetValueFromEncodableMap(arguments, "url", url)) {
          player->SetUrl(url);
        } else {
          result->Error(kInvalidArgument,
                        "No url provided or is invalid value.");
          return;
        }
      } else if (method_name == "setPlaybackRate") {
        double playback_rate = 0.0;
        if (GetValueFromEncodableMap(arguments, "playbackRate",
                                     playback_rate)) {
          player->SetPlaybackRate(playback_rate);
        } else {
          result->Error(kInvalidArgument,
                        "No playbackRate provided or is invalid value.");
          return;
        }
      } else if (method_name == "setReleaseMode") {
        std::string release_mode;
        if (GetValueFromEncodableMap(arguments, "releaseMode", release_mode)) {
          if (release_mode == "ReleaseMode.RELEASE") {
            player->SetReleaseMode(ReleaseMode::kRelease);
          } else if (release_mode == "ReleaseMode.LOOP") {
            player->SetReleaseMode(ReleaseMode::kLoop);
          } else if (release_mode == "ReleaseMode.STOP") {
            player->SetReleaseMode(ReleaseMode::kStop);
          }
        } else {
          result->Error(kInvalidArgument,
                        "No realeaseMode provided or is invalid value.");
          return;
        }
      } else if (method_name == "getDuration") {
        result->Success(flutter::EncodableValue(player->GetDuration()));
        return;
      } else if (method_name == "getCurrentPosition") {
        result->Success(flutter::EncodableValue(player->GetCurrentPosition()));
        return;
      } else {
        result->NotImplemented();
        return;
      }
      result->Success(flutter::EncodableValue(1));
    } catch (const AudioPlayerError &error) {
      result->Error(error.GetCode(), error.GetMessage());
    }
  }

  AudioPlayer *GetAudioPlayer(const std::string &player_id,
                              const std::string &mode) {
    auto iter = audio_players_.find(player_id);
    if (iter != audio_players_.end()) {
      return iter->second.get();
    }

    PreparedListener prepared_listener =
        [channel = channel_.get()](const std::string &player_id, int duration) {
          flutter::EncodableMap wrapped = {{flutter::EncodableValue("playerId"),
                                            flutter::EncodableValue(player_id)},
                                           {flutter::EncodableValue("value"),
                                            flutter::EncodableValue(duration)}};
          auto arguments = std::make_unique<flutter::EncodableValue>(wrapped);
          channel->InvokeMethod("audio.onDuration", std::move(arguments));
        };

    SeekCompletedListener seek_completed_listener =
        [channel = channel_.get()](const std::string &player_id) {
          flutter::EncodableMap wrapped = {{flutter::EncodableValue("playerId"),
                                            flutter::EncodableValue(player_id)},
                                           {flutter::EncodableValue("value"),
                                            flutter::EncodableValue(true)}};
          auto arguments = std::make_unique<flutter::EncodableValue>(wrapped);
          channel->InvokeMethod("audio.onSeekComplete", std::move(arguments));
        };

    StartPlayingListener start_playing_listener =
        [channel = channel_.get()](const std::string &player_id,
                                   const int duration, const int position) {
          flutter::EncodableMap durationWrapped = {
              {flutter::EncodableValue("playerId"),
               flutter::EncodableValue(player_id)},
              {flutter::EncodableValue("value"),
               flutter::EncodableValue(duration)}};
          channel->InvokeMethod(
              "audio.onDuration",
              std::make_unique<flutter::EncodableValue>(durationWrapped));

          flutter::EncodableMap positionWrapped = {
              {flutter::EncodableValue("playerId"),
               flutter::EncodableValue(player_id)},
              {flutter::EncodableValue("value"),
               flutter::EncodableValue(position)}};
          channel->InvokeMethod(
              "audio.onCurrentPosition",
              std::make_unique<flutter::EncodableValue>(positionWrapped));
        };

    PlayCompletedListener play_completed_listener =
        [channel = channel_.get()](const std::string &player_id) {
          flutter::EncodableMap wrapped = {{flutter::EncodableValue("playerId"),
                                            flutter::EncodableValue(player_id)},
                                           {flutter::EncodableValue("value"),
                                            flutter::EncodableValue(true)}};
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

    bool low_latency = mode == "PlayerMode.LOW_LATENCY";
    auto player = std::make_unique<AudioPlayer>(
        player_id, low_latency, prepared_listener, start_playing_listener,
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
