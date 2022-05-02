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
      "No " + std::string(key) + " provided or is invalid type or value.";
  throw std::invalid_argument(message);
}

ReleaseMode StringToReleaseMode(std::string release_mode) {
  if (release_mode == "ReleaseMode.RELEASE") {
    return ReleaseMode::kRelease;
  } else if (release_mode == "ReleaseMode.LOOP") {
    return ReleaseMode::kLoop;
  } else if (release_mode == "ReleaseMode.STOP") {
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

    std::string player_id, mode;
    if (!GetValueFromEncodableMap(arguments, "playerId", player_id)) {
      result->Error(kInvalidArgument, "No playerId provided.");
      return;
    }
    GetValueFromEncodableMap(arguments, "mode", mode);

    AudioPlayer *player = GetAudioPlayer(player_id, mode);

    try {
      const auto &method_name = method_call.method_name();
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
        result->Success(flutter::EncodableValue(1));
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
        result->Success(flutter::EncodableValue(1));
      } else if (method_name == "resume") {
        player->Play();
        result->Success(flutter::EncodableValue(1));
      } else if (method_name == "pause") {
        player->Pause();
        result->Success(flutter::EncodableValue(1));
      } else if (method_name == "stop") {
        player->Stop();
        result->Success(flutter::EncodableValue(1));
      } else if (method_name == "release") {
        player->Release();
        result->Success(flutter::EncodableValue(1));
      } else if (method_name == "seek") {
        player->Seek(GetRequiredArg<int32_t>(arguments, "position"));
        result->Success(flutter::EncodableValue(1));
      } else if (method_name == "setVolume") {
        player->SetVolume(GetRequiredArg<double>(arguments, "volume"));
        result->Success(flutter::EncodableValue(1));
      } else if (method_name == "setUrl") {
        player->SetUrl(GetRequiredArg<std::string>(arguments, "url"));
        result->Success(flutter::EncodableValue(1));
      } else if (method_name == "setPlaybackRate") {
        player->SetPlaybackRate(
            GetRequiredArg<double>(arguments, "playbackRate"));
        result->Success(flutter::EncodableValue(1));
      } else if (method_name == "setReleaseMode") {
        std::string release_mode =
            GetRequiredArg<std::string>(arguments, "releaseMode");
        player->SetReleaseMode(StringToReleaseMode(release_mode));
        result->Success(flutter::EncodableValue(1));
      } else if (method_name == "getDuration") {
        result->Success(flutter::EncodableValue(player->GetDuration()));
      } else if (method_name == "getCurrentPosition") {
        result->Success(flutter::EncodableValue(player->GetCurrentPosition()));
      } else {
        result->NotImplemented();
      }
    } catch (const std::invalid_argument &error) {
      result->Error(kInvalidArgument, error.what());
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

    UpdatePositionListener update_position_listener =
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
        player_id, low_latency, prepared_listener, update_position_listener,
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
