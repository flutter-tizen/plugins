// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_audio_manager_plugin.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <sound_manager.h>

#include <memory>
#include <string>

#include "log.h"

namespace {

constexpr std::array<std::pair<std::string_view, sound_type_e>, 8> kSoundTypes{
    {{"system", SOUND_TYPE_SYSTEM},
     {"notification", SOUND_TYPE_NOTIFICATION},
     {"alarm", SOUND_TYPE_ALARM},
     {"ringtone", SOUND_TYPE_RINGTONE},
     {"media", SOUND_TYPE_MEDIA},
     {"call", SOUND_TYPE_CALL},
     {"voip", SOUND_TYPE_VOIP},
     {"voice", SOUND_TYPE_VOICE}}};

bool ConvertSoundTypeToString(sound_type_e type, std::string &str) {
  auto iter = kSoundTypes.begin();

  for (; iter != kSoundTypes.end(); iter++) {
    if (iter->second == type) {
      str = iter->first;
      return true;
    }
  }

  return false;
}

bool ConvertStringToSoundType(const std::string &str, sound_type_e &type) {
  auto iter = kSoundTypes.begin();

  for (; iter != kSoundTypes.end(); iter++) {
    if (iter->first == str) {
      type = iter->second;
      return true;
    }
  }

  return false;
}

template <typename T>
bool ExtractValueFromMap(const flutter::EncodableValue &arguments,
                         const char *key, T &out_value) {
  if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(arguments);
    auto iter = map.find(flutter::EncodableValue(key));
    if (iter != map.end() && !iter->second.IsNull()) {
      if (auto pval = std::get_if<T>(&iter->second)) {
        out_value = *pval;
        return true;
      }
    }
  }
  return false;
}

}  // namespace

class AudioManagerTizenPlugin : public flutter::Plugin {
 public:
  using MethodResultPtr =
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>;

  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<AudioManagerTizenPlugin>();
    plugin->SetupChannels(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  AudioManagerTizenPlugin() {}

  virtual ~AudioManagerTizenPlugin() { UnregisterObserver(); }

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      MethodResultPtr result) {
    LOG_INFO("method: %s", method_call.method_name().c_str());

    const auto &arguments = *method_call.arguments();

    if (method_call.method_name().compare("getCurrentPlaybackType") == 0) {
      GetCurrentPlaybackType(std::move(result));
    } else if (method_call.method_name().compare("getMaxLevel") == 0) {
      GetMaxLevel(arguments, std::move(result));
    } else if (method_call.method_name().compare("setLevel") == 0) {
      SetLevel(arguments, std::move(result));
    } else if (method_call.method_name().compare("getLevel") == 0) {
      GetLevel(arguments, std::move(result));
    } else {
      result->NotImplemented();
    }
  }

  void RegisterObserver(
      std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> &&events) {
    LOG_INFO("RegisterObserver");
    this->events_ = std::move(events);
    const int ret = sound_manager_add_volume_changed_cb(
        VolumeChangedCb, static_cast<void *>(this),
        &this->volume_changed_cb_id_);

    if (ret == SOUND_MANAGER_ERROR_NONE) {
      this->volume_changed_cb_is_registered_ = true;
    } else {
      const char *message = get_error_message(ret);
      LOG_ERROR("Failed to get current playback type: %s", message);
      this->events_->Error(std::to_string(ret),
                           "Failed sound_manager_add_volume_changed_cb.",
                           flutter::EncodableValue(std::string(message)));
    }
  }

  void UnregisterObserver() {
    LOG_INFO("UnregisterObserver");

    if (this->volume_changed_cb_is_registered_) {
      sound_manager_remove_volume_changed_cb(this->volume_changed_cb_id_);
      this->events_ = nullptr;
      this->volume_changed_cb_is_registered_ = false;
    }
  }

  void GetCurrentPlaybackType(MethodResultPtr result) {
    sound_type_e type;
    const int ret = sound_manager_get_current_sound_type(&type);

    if (ret == SOUND_MANAGER_ERROR_NONE) {
      std::string type_str;

      if (ConvertSoundTypeToString(type, type_str)) {
        result->Success(flutter::EncodableValue(type_str));
      } else {
        LOG_ERROR("Failed to convert sound type to string.");
        result->Error("UnknownSoundType",
                      "Failed to convert sound type to string");
      }
    } else if (ret == SOUND_MANAGER_ERROR_NO_PLAYING_SOUND) {
      result->Success(flutter::EncodableValue(std::string("none")));
    } else {
      const char *message = get_error_message(ret);
      LOG_ERROR("Failed to get current playback type: %s", message);
      result->Error(std::to_string(ret), "Failed to get current playback type.",
                    flutter::EncodableValue(std::string(message)));
    }
  }

  void GetMaxLevel(const flutter::EncodableValue &arguments,
                   MethodResultPtr result) {
    std::string type_str;
    sound_type_e type;
    int max;

    if (!ExtractValueFromMap(arguments, "type", type_str)) {
      result->Error("InvalidArguments", "Please check type");
      return;
    }

    if (!ConvertStringToSoundType(type_str, type)) {
      LOG_ERROR("Failed to convert string to sound type.");
      result->Error("UnknownSoundType",
                    "Failed to convert string to sound type.");
      return;
    }

    const int ret = sound_manager_get_max_volume(type, &max);

    if (ret == SOUND_MANAGER_ERROR_NONE) {
      result->Success(flutter::EncodableValue(max));
    } else {
      const char *message = get_error_message(ret);
      LOG_ERROR("Failed to get max volume: %s", message);
      result->Error(std::to_string(ret), "Failed to get max volume.",
                    flutter::EncodableValue(std::string(message)));
    }
  }

  void SetLevel(const flutter::EncodableValue &arguments,
                MethodResultPtr result) {
    std::string type_str;
    int32_t volume;
    sound_type_e type;

    if (!ExtractValueFromMap(arguments, "type", type_str) ||
        !ExtractValueFromMap(arguments, "volume", volume)) {
      result->Error("InvalidArguments", "Please check type and volume.");
      return;
    }

    if (!ConvertStringToSoundType(type_str, type)) {
      LOG_ERROR("Failed to convert string to sound type.");
      result->Error("UnknownSoundType",
                    "Failed to convert string to sound type.");
      return;
    }

    const int ret = sound_manager_set_volume(type, volume);

    if (ret == SOUND_MANAGER_ERROR_NONE) {
      result->Success();
    } else {
      const char *message = get_error_message(ret);
      LOG_ERROR("Failed to set volume: %s", message);
      result->Error(std::to_string(ret), "Failed to set volume.",
                    flutter::EncodableValue(std::string(message)));
    }
  }

  void GetLevel(const flutter::EncodableValue &arguments,
                MethodResultPtr result) {
    std::string type_str;
    sound_type_e type;
    int volume;

    if (!ExtractValueFromMap(arguments, "type", type_str)) {
      result->Error("InvalidArguments", "Please check type");
      return;
    }

    if (!ConvertStringToSoundType(type_str, type)) {
      LOG_ERROR("Failed to convert string to sound type.");
      result->Error("UnknownSoundType",
                    "Failed to convert string to sound type.");
      return;
    }

    const int ret = sound_manager_get_volume(type, &volume);

    if (ret == SOUND_MANAGER_ERROR_NONE) {
      result->Success(flutter::EncodableValue(volume));
    } else {
      const char *message = get_error_message(ret);
      LOG_ERROR("Failed to get volume: %s", message);
      result->Error(std::to_string(ret), "Failed to get volume.",
                    flutter::EncodableValue(std::string(message)));
    }
  }

  void SetupChannels(flutter::PluginRegistrar *registrar) {
    auto method_channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "tizen/audio_manager",
            &flutter::StandardMethodCodec::GetInstance());

    auto event_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            registrar->messenger(), "tizen/audio_manager_events",
            &flutter::StandardMethodCodec::GetInstance());

    method_channel->SetMethodCallHandler([this](const auto &call, auto result) {
      this->HandleMethodCall(call, std::move(result));
    });

    auto event_channel_handler =
        std::make_unique<flutter::StreamHandlerFunctions<>>(
            [this](const flutter::EncodableValue *arguments,
                   std::unique_ptr<flutter::EventSink<>> &&events)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_INFO("OnListen");
              this->RegisterObserver(std::move(events));
              return nullptr;
            },
            [this](const flutter::EncodableValue *arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_INFO("OnCancel");
              this->UnregisterObserver();
              return nullptr;
            });

    event_channel->SetStreamHandler(std::move(event_channel_handler));
  }

  static void VolumeChangedCb(sound_type_e type, unsigned int volume,
                              void *user_data) {
    auto plugin = static_cast<AudioManagerTizenPlugin *>(user_data);
    std::string type_str;

    if (ConvertSoundTypeToString(type, type_str)) {
      flutter::EncodableMap msg;
      msg[flutter::EncodableValue("type")] = flutter::EncodableValue(type_str);
      msg[flutter::EncodableValue("level")] =
          flutter::EncodableValue(static_cast<int64_t>(volume));
      plugin->events_->Success(flutter::EncodableValue(msg));
    } else {
      LOG_ERROR("Failed to convert sound type to string.");
      plugin->events_->Error("UnknownSoundType",
                             "Failed to convert sound type to string");
    }
  }

  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> events_;
  bool volume_changed_cb_is_registered_ = false;
  int volume_changed_cb_id_ = 0;
};

void AudioManagerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  AudioManagerTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
