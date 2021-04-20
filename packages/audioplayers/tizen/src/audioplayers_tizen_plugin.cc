#include "audioplayers_tizen_plugin.h"

#include <Ecore.h>
#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <map>

#include "audio_player.h"
#include "audio_player_error.h"
#include "audio_player_options.h"
#include "log.h"

#define TIMEOUT 0.2

class AudioplayersTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<AudioplayersTizenPlugin>(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  AudioplayersTizenPlugin(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "xyz.luan/audioplayers",
            &flutter::StandardMethodCodec::GetInstance());
    channel->SetMethodCallHandler(
        [plugin = this](const auto &call, auto result) {
          plugin->HandleMethodCall(call, std::move(result));
        });

    channel_ = std::move(channel);
    timer_ = nullptr;
  }

  virtual ~AudioplayersTizenPlugin() {
    if (timer_) {
      ecore_timer_del(timer_);
      timer_ = nullptr;
    }
  }

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("HandleMethodCall: %s", method_call.method_name().c_str());
    const flutter::EncodableValue *args = method_call.arguments();
    if (std::holds_alternative<flutter::EncodableMap>(*args)) {
      flutter::EncodableMap encodables = std::get<flutter::EncodableMap>(*args);
      flutter::EncodableValue &player_id_value =
          encodables[flutter::EncodableValue("playerId")];
      std::string player_id;
      if (std::holds_alternative<std::string>(player_id_value)) {
        player_id = std::get<std::string>(player_id_value);
        LOG_DEBUG("Audio player ID: %s", player_id.c_str());
      } else {
        result->Error("Invalid Player ID", "Invalid Player ID for method " +
                                               method_call.method_name());
        return;
      }

      flutter::EncodableValue &mode_value =
          encodables[flutter::EncodableValue("mode")];
      std::string mode;
      if (std::holds_alternative<std::string>(mode_value)) {
        mode = std::get<std::string>(mode_value);
        LOG_DEBUG("Audio player Mode: %s", mode.c_str());
      }

      AudioPlayer *player = GetAudioPlayer(player_id, mode);
      try {
        if (method_call.method_name().compare("play") == 0) {
          flutter::EncodableValue &volume =
              encodables[flutter::EncodableValue("volume")];
          if (std::holds_alternative<double>(volume)) {
            player->SetVolume(std::get<double>(volume));
          }
          flutter::EncodableValue &url =
              encodables[flutter::EncodableValue("url")];
          if (std::holds_alternative<std::string>(url)) {
            player->SetUrl(std::get<std::string>(url));
          }
          player->Play();
          flutter::EncodableValue &position =
              encodables[flutter::EncodableValue("position")];
          if (std::holds_alternative<int32_t>(position)) {
            player->Seek(std::get<int32_t>(position));
          }
        } else if (method_call.method_name().compare("playBytes") == 0) {
          flutter::EncodableValue &volume =
              encodables[flutter::EncodableValue("volume")];
          if (std::holds_alternative<double>(volume)) {
            player->SetVolume(std::get<double>(volume));
          }
          flutter::EncodableValue &bytes =
              encodables[flutter::EncodableValue("bytes")];
          if (std::holds_alternative<std::vector<uint8_t>>(bytes)) {
            player->SetDataSource(std::get<std::vector<uint8_t>>(bytes));
          }
          player->Play();
          flutter::EncodableValue &position =
              encodables[flutter::EncodableValue("position")];
          if (std::holds_alternative<int32_t>(position)) {
            player->Seek(std::get<int32_t>(position));
          }
        } else if (method_call.method_name().compare("resume") == 0) {
          player->Play();
        } else if (method_call.method_name().compare("pause") == 0) {
          player->Pause();
        } else if (method_call.method_name().compare("stop") == 0) {
          player->Stop();
        } else if (method_call.method_name().compare("release") == 0) {
          player->Release();
        } else if (method_call.method_name().compare("seek") == 0) {
          flutter::EncodableValue &position =
              encodables[flutter::EncodableValue("position")];
          if (std::holds_alternative<int32_t>(position)) {
            player->Seek(std::get<int32_t>(position));
          } else {
            result->Error("Invalid position",
                          "seek failed because of invalid position");
          }
        } else if (method_call.method_name().compare("setVolume") == 0) {
          flutter::EncodableValue &volume =
              encodables[flutter::EncodableValue("volume")];
          if (std::holds_alternative<double>(volume)) {
            player->SetVolume(std::get<double>(volume));
          } else {
            result->Error("Invalid volume",
                          "setVolume failed because of invalid volume");
          }
        } else if (method_call.method_name().compare("setUrl") == 0) {
          flutter::EncodableValue &url =
              encodables[flutter::EncodableValue("url")];
          if (std::holds_alternative<std::string>(url)) {
            player->SetUrl(std::get<std::string>(url));
          } else {
            result->Error("Invalid url",
                          "SetUrl failed because of invalid url");
          }
        } else if (method_call.method_name().compare("setPlaybackRate") == 0) {
          flutter::EncodableValue &rate =
              encodables[flutter::EncodableValue("playbackRate")];
          if (std::holds_alternative<double>(rate)) {
            player->SetPlaybackRate(std::get<double>(rate));
          } else {
            result->Error("Invalid rate",
                          "setPlaybackRate failed because of invalid rate");
          }
        } else if (method_call.method_name().compare("setReleaseMode") == 0) {
          flutter::EncodableValue &release_mode_value =
              encodables[flutter::EncodableValue("releaseMode")];
          if (std::holds_alternative<std::string>(release_mode_value)) {
            std::string release_mode = std::get<std::string>(release_mode_value);
            if (release_mode.compare("ReleaseMode.RELEASE") == 0) {
              player->SetReleaseMode(RELEASE);
            } else if (release_mode.compare("ReleaseMode.LOOP") == 0) {
              player->SetReleaseMode(LOOP);
            } else if (release_mode.compare("ReleaseMode.STOP") == 0) {
              player->SetReleaseMode(STOP);
            }
          } else {
            result->Error(
                "Invalid ReleaseMode",
                "setReleaseMode failed because of invalid ReleaseMode");
          }
        } else if (method_call.method_name().compare("getDuration") == 0) {
          int duration = player->GetDuration();
          result->Success(flutter::EncodableValue(duration));
          return;
        } else if (method_call.method_name().compare("getCurrentPosition") ==
                   0) {
          int position = player->GetCurrentPosition();
          result->Success(flutter::EncodableValue(position));
          return;
        } else {
          result->NotImplemented();
          return;
        }
        result->Success(flutter::EncodableValue(1));
      } catch (const AudioPlayerError &e) {
        result->Error(e.GetCode(), e.GetMessage());
      }
    } else {
      result->Error("Invalid arguments", "Invalid arguments for method " +
                                             method_call.method_name());
    }
  }

  AudioPlayer *GetAudioPlayer(const std::string &player_id,
                              const std::string &mode) {
    auto iter = audio_players_.find(player_id);
    if (iter != audio_players_.end()) {
      return iter->second.get();
    }

    bool low_latency = false;
    LOG_DEBUG("mode is %s", mode.c_str());
    if (mode.compare("PlayerMode.LOW_LATENCY") == 0) {
      low_latency = true;
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
        [plugin = this](const std::string &player_id) {
          plugin->StartPositionUpdates();
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

    auto player = std::make_unique<AudioPlayer>(
        player_id, low_latency, prepared_listener, start_playing_listener,
        seek_completed_listener, play_completed_listener, error_listener);
    audio_players_[player_id] = std::move(player);
    return audio_players_[player_id].get();
  }

  void StartPositionUpdates() {
    if (!timer_) {
      LOG_DEBUG("add timer to update position of playing audio");
      timer_ = ecore_timer_add(TIMEOUT, UpdatePosition, (void *)this);
      if (timer_ == nullptr) {
        LOG_ERROR("failed to add timer for UpdatePosition");
      }
    }
  }

  static Eina_Bool UpdatePosition(void *data) {
    AudioplayersTizenPlugin *plugin = (AudioplayersTizenPlugin *)data;
    bool none_playing = true;
    auto iter = plugin->audio_players_.begin();
    while (iter != plugin->audio_players_.end()) {
      try {
        std::string player_id = iter->second->GetPlayerId();
        if (iter->second->IsPlaying()) {
          LOG_DEBUG("Audio player %s is playing", player_id.c_str());
          none_playing = false;
          flutter::EncodableMap duration = {
              {flutter::EncodableValue("playerId"),
               flutter::EncodableValue(player_id)},
              {flutter::EncodableValue("value"),
               flutter::EncodableValue(iter->second->GetDuration())}};
          plugin->channel_->InvokeMethod(
              "audio.onDuration",
              std::make_unique<flutter::EncodableValue>(duration));

          flutter::EncodableMap position = {
              {flutter::EncodableValue("playerId"),
               flutter::EncodableValue(player_id)},
              {flutter::EncodableValue("value"),
               flutter::EncodableValue(iter->second->GetCurrentPosition())}};
          plugin->channel_->InvokeMethod(
              "audio.onCurrentPosition",
              std::make_unique<flutter::EncodableValue>(position));
        }
        iter++;
      } catch (...) {
        LOG_ERROR("failed to update position for player %s",
                  iter->second->GetPlayerId().c_str());
      }
    }

    if (none_playing) {
      plugin->timer_ = nullptr;
      return ECORE_CALLBACK_CANCEL;
    } else {
      return ECORE_CALLBACK_RENEW;
    }
  }

  Ecore_Timer *timer_;
  std::unique_ptr<flutter::MethodChannel<flutter::EncodableValue>> channel_;
  std::map<std::string, std::unique_ptr<AudioPlayer>> audio_players_;
};

void AudioplayersTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  AudioplayersTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
