// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_audio_manager_plugin.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <string>

#include "audio_manager.h"
#include "audio_manager_error.h"
#include "log.h"

namespace {
typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::EventChannel<flutter::EncodableValue> FlEventChannel;
typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;

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
}  // namespace

class AudioManagerTizenPlugin : public flutter::Plugin {
 public:
  using MethodResultPtr = std::unique_ptr<FlMethodResult>;

  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<AudioManagerTizenPlugin>();
    plugin->SetupChannels(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  AudioManagerTizenPlugin() {}

  virtual ~AudioManagerTizenPlugin() { UnregisterObserver(); }

 private:
  void HandleMethodCall(const FlMethodCall &method_call,
                        MethodResultPtr result) {
    const std::string &method_name = method_call.method_name();

    if (result_) {
      result_->Error("Already active", "Cancelled by a second request.");
      return;
    }
    result_ = std::move(result);

    if (method_name == "getCurrentPlaybackType") {
      GetCurrentPlaybackType();
    } else if (method_name == "getMaxLevel") {
      const auto *arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      assert(arguments);
      GetMaxLevel(arguments);
    } else if (method_name == "setLevel") {
      const auto *arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      assert(arguments);
      SetLevel(arguments);
    } else if (method_name == "getLevel") {
      const auto *arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      assert(arguments);
      GetLevel(arguments);
    } else {
      result->NotImplemented();
    }
    result_ = nullptr;
  }

  void RegisterObserver(std::unique_ptr<FlEventSink> &&events) {
    this->events_ = std::move(events);
    try {
      audio_manager_.AddVolumeChangedCallback(
          [events = events_.get()](std::string type_str, int32_t volume) {
            flutter::EncodableMap msg;
            msg[flutter::EncodableValue("type")] =
                flutter::EncodableValue(type_str);
            msg[flutter::EncodableValue("level")] =
                flutter::EncodableValue(static_cast<int32_t>(volume));
            events->Success(flutter::EncodableValue(msg));
          });
    } catch (const AudioManagerError &error) {
      result_->Error(error.code(), error.message());
    }
  }

  void UnregisterObserver() { audio_manager_.RemoveVolumeChangedCallback(); }

  void GetCurrentPlaybackType() {
    try {
      std::string type = audio_manager_.getCurrentPlaybackType();
      result_->Success(flutter::EncodableValue(type));
    } catch (const AudioManagerError &error) {
      result_->Error(error.code(), error.message());
    }
  }

  void GetMaxLevel(const flutter::EncodableMap *arguments) {
    std::string type;
    if (!GetValueFromEncodableMap(arguments, "type", type)) {
      result_->Error("InvalidArguments", "Please check type");
      return;
    }

    try {
      int32_t max = audio_manager_.getMaxLevel(type);
      result_->Success(flutter::EncodableValue(max));
    } catch (const AudioManagerError &error) {
      result_->Error(error.code(), error.message());
    }
  }

  void SetLevel(const flutter::EncodableMap *arguments) {
    std::string type_str;
    int32_t volume;
    if (!GetValueFromEncodableMap(arguments, "type", type_str) ||
        !GetValueFromEncodableMap(arguments, "volume", volume)) {
      result_->Error("InvalidArguments", "Please check type and volume.");
      return;
    }

    try {
      audio_manager_.SetLevel(type_str, volume);
      result_->Success();
    } catch (const AudioManagerError &error) {
      result_->Error(error.code(), error.message());
    }
  }

  void GetLevel(const flutter::EncodableMap *arguments) {
    std::string type_str;
    if (!GetValueFromEncodableMap(arguments, "type", type_str)) {
      result_->Error("InvalidArguments", "Please check type");
      return;
    }

    try {
      int32_t volume;
      volume = audio_manager_.GetLevel(type_str);
      result_->Success(flutter::EncodableValue(volume));
    } catch (const AudioManagerError &error) {
      result_->Error(error.code(), error.message());
    }
  }

  void SetupChannels(flutter::PluginRegistrar *registrar) {
    auto method_channel = std::make_unique<FlMethodChannel>(
        registrar->messenger(), "tizen/audio_manager",
        &flutter::StandardMethodCodec::GetInstance());

    auto event_channel = std::make_unique<FlEventChannel>(
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
              this->RegisterObserver(std::move(events));
              return nullptr;
            },
            [this](const flutter::EncodableValue *arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              this->UnregisterObserver();
              return nullptr;
            });

    event_channel->SetStreamHandler(std::move(event_channel_handler));
  }

  std::unique_ptr<FlMethodResult> result_;
  std::unique_ptr<FlEventSink> events_;
  AudioManager audio_manager_;
};

void AudioManagerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  AudioManagerTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
