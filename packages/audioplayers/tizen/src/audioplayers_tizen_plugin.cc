// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "audioplayers_tizen_plugin.h"

#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
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

typedef flutter::EventChannel<flutter::EncodableValue> FlEventChannel;
typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;
typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;
typedef flutter::StreamHandler<flutter::EncodableValue> FlStreamHandler;
typedef flutter::StreamHandlerError<flutter::EncodableValue>
    FlStreamHandlerError;

const char *kInvalidArgument = "Invalid argument";
const char kAudioDurationEvent[] = "audio.onDuration";
const char kAudioPreparedEvent[] = "audio.onPrepared";
const char kAudioSeekCompleteEvent[] = "audio.onSeekComplete";
const char kAudioCompleteEvent[] = "audio.onComplete";
const char kAudioLogEvent[] = "audio.onLog";

using OnSetEventSink =
    std::function<void(std::unique_ptr<FlEventSink> event_sink)>;

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

class AudioPlayerStreamHandler : public FlStreamHandler {
 public:
  AudioPlayerStreamHandler(OnSetEventSink on_set_event_sink)
      : on_set_event_sink_(on_set_event_sink) {}

 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    on_set_event_sink_(std::move(events));
    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    return nullptr;
  }

 private:
  OnSetEventSink on_set_event_sink_;
};

class AudioplayersTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel = std::make_unique<FlMethodChannel>(
        registrar->messenger(), "xyz.luan/audioplayers",
        &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<AudioplayersTizenPlugin>(registrar);

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });
    plugin->channel_ = std::move(channel);

    registrar->AddPlugin(std::move(plugin));
  }

  AudioplayersTizenPlugin(flutter::PluginRegistrar *registrar)
      : registrar_(registrar) {}

  virtual ~AudioplayersTizenPlugin() {}

  void SetRegistrar(flutter::PluginRegistrar *registrar) {
    registrar_ = registrar;
  }

 private:
  void HandleMethodCall(const FlMethodCall &method_call,
                        std::unique_ptr<FlMethodResult> result) {
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

    try {
      const std::string &method_name = method_call.method_name();
      if (method_name == "create") {
        CreateAudioPlayer(player_id);
        result->Success();
        return;
      } else if (method_name == "dispose") {
        DisposeAudioPlayer(player_id);
        result->Success();
        return;
      }

      AudioPlayer *player = GetAudioPlayer(player_id);
      if (!player) {
        result->Error(kInvalidArgument,
                      "No AudioPlayer" + player_id + " is exist.");
        return;
      }
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
    return nullptr;
  }

  void CreateAudioPlayer(const std::string &player_id) {
    auto event_channel = std::make_unique<FlEventChannel>(
        registrar_->messenger(), "xyz.luan/audioplayers/events/" + player_id,
        &flutter::StandardMethodCodec::GetInstance());
    event_channel->SetStreamHandler(std::make_unique<AudioPlayerStreamHandler>(
        [this, id = player_id](std::unique_ptr<FlEventSink> event_sink) {
          this->event_sinks_[id] = std::move(event_sink);
        }));

    PreparedListener prepared_listener = [this](const std::string &player_id,
                                                bool is_prepared) {
      flutter::EncodableMap map = {
          {flutter::EncodableValue("event"),
           flutter::EncodableValue(kAudioPreparedEvent)},
          {flutter::EncodableValue("value"),
           flutter::EncodableValue(is_prepared)}};
      event_sinks_[player_id]->Success(flutter::EncodableValue(map));
    };

    DurationListener duration_listener = [this](const std::string &player_id,
                                                int32_t duration) {
      flutter::EncodableMap map = {
          {flutter::EncodableValue("event"),
           flutter::EncodableValue(kAudioDurationEvent)},
          {flutter::EncodableValue("value"),
           flutter::EncodableValue(duration)}};
      event_sinks_[player_id]->Success(flutter::EncodableValue(map));
    };

    SeekCompletedListener seek_completed_listener =
        [this](const std::string &player_id) {
          flutter::EncodableMap map = {
              {flutter::EncodableValue("event"),
               flutter::EncodableValue(kAudioSeekCompleteEvent)}};
          event_sinks_[player_id]->Success(flutter::EncodableValue(map));
        };

    PlayCompletedListener play_completed_listener =
        [this](const std::string &player_id) {
          flutter::EncodableMap map = {
              {flutter::EncodableValue("event"),
               flutter::EncodableValue(kAudioCompleteEvent)}};
          event_sinks_[player_id]->Success(flutter::EncodableValue(map));
        };

    LogListener log_listener = [this](const std::string &player_id,
                                      const std::string &message) {
      flutter::EncodableMap map = {
          {flutter::EncodableValue("event"),
           flutter::EncodableValue(kAudioLogEvent)},
          {flutter::EncodableValue("value"), flutter::EncodableValue(message)}};
      event_sinks_[player_id]->Success(flutter::EncodableValue(map));
    };

    auto player = std::make_unique<AudioPlayer>(
        player_id, prepared_listener, duration_listener,
        seek_completed_listener, play_completed_listener, log_listener);
    audio_players_[player_id] = std::move(player);
  }

  void DisposeAudioPlayer(const std::string &player_id) {
    audio_players_.erase(player_id);
    event_sinks_.erase(player_id);
  }

  std::unique_ptr<FlMethodChannel> channel_;

  std::map<std::string, std::unique_ptr<AudioPlayer>> audio_players_;
  std::map<std::string, std::unique_ptr<FlEventSink>> event_sinks_;

  flutter::PluginRegistrar *registrar_;
};

}  // namespace

void AudioplayersTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  AudioplayersTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
