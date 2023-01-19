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

#include "log.h"
#include "tizen_audio_manager.h"
#include "tizen_audio_manager_error.h"

namespace {

typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::EventChannel<flutter::EncodableValue> FlEventChannel;
typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;
typedef flutter::StreamHandler<flutter::EncodableValue> FlStreamHandler;
typedef flutter::StreamHandlerError<flutter::EncodableValue>
    FlStreamHandlerError;

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

class TizenAudioManagerStreamHandler : public FlStreamHandler {
 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    events_ = std::move(events);
    audio_manager_.RegisterVolumeChangedCallback(
        [events = events_.get()](std::string type_str, int32_t volume) {
          flutter::EncodableMap msg;
          msg[flutter::EncodableValue("type")] =
              flutter::EncodableValue(type_str);
          msg[flutter::EncodableValue("level")] =
              flutter::EncodableValue(static_cast<int32_t>(volume));
          events->Success(flutter::EncodableValue(msg));
        });
    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    audio_manager_.UnregisterVolumeChangedCallback();
    events_.reset();
    return nullptr;
  }

 private:
  TizenAudioManager audio_manager_;
  std::unique_ptr<FlEventSink> events_;
};

class TizenAudioManagerPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<TizenAudioManagerPlugin>();

    auto method_channel = std::make_unique<FlMethodChannel>(
        registrar->messenger(), "tizen/audio_manager",
        &flutter::StandardMethodCodec::GetInstance());
    method_channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    plugin->event_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "tizen/audio_manager_events",
        &flutter::StandardMethodCodec::GetInstance());
    plugin->event_channel_->SetStreamHandler(
        std::make_unique<TizenAudioManagerStreamHandler>());

    registrar->AddPlugin(std::move(plugin));
  }

  TizenAudioManagerPlugin() {}

  virtual ~TizenAudioManagerPlugin() {}

 private:
  void HandleMethodCall(const FlMethodCall &method_call,
                        std::unique_ptr<FlMethodResult> result) {
    const std::string &method_name = method_call.method_name();

    if (method_name == "getCurrentPlaybackType") {
      GetCurrentPlaybackType(std::move(result));
    } else if (method_name == "getMaxLevel") {
      const auto *arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      assert(arguments);
      GetMaxLevel(arguments, std::move(result));
    } else if (method_name == "setLevel") {
      const auto *arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      assert(arguments);
      SetLevel(arguments, std::move(result));
    } else if (method_name == "getLevel") {
      const auto *arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      assert(arguments);
      GetLevel(arguments, std::move(result));
    } else {
      result->NotImplemented();
    }
  }

  void GetCurrentPlaybackType(std::unique_ptr<FlMethodResult> result) {
    try {
      std::string type = audio_manager_.GetCurrentPlaybackType();
      result->Success(flutter::EncodableValue(type));
    } catch (const TizenAudioManagerError &error) {
      result->Error(error.code(), error.message());
    }
  }

  void GetMaxLevel(const flutter::EncodableMap *arguments,
                   std::unique_ptr<FlMethodResult> result) {
    std::string type;
    if (!GetValueFromEncodableMap(arguments, "type", type)) {
      result->Error("Invalid arguments", "No type provided.");
      return;
    }

    try {
      int32_t max = audio_manager_.GetMaxVolume(type);
      result->Success(flutter::EncodableValue(max));
    } catch (const TizenAudioManagerError &error) {
      result->Error(error.code(), error.message());
    }
  }

  void SetLevel(const flutter::EncodableMap *arguments,
                std::unique_ptr<FlMethodResult> result) {
    std::string type;
    int32_t volume;
    if (!GetValueFromEncodableMap(arguments, "type", type)) {
      result->Error("Invalid arguments", "No type provided.");
      return;
    }
    if (!GetValueFromEncodableMap(arguments, "volume", volume)) {
      result->Error("Invalid arguments", "No volume provided.");
      return;
    }

    try {
      audio_manager_.SetVolume(type, volume);
      result->Success();
    } catch (const TizenAudioManagerError &error) {
      result->Error(error.code(), error.message());
    }
  }

  void GetLevel(const flutter::EncodableMap *arguments,
                std::unique_ptr<FlMethodResult> result) {
    std::string type;
    if (!GetValueFromEncodableMap(arguments, "type", type)) {
      result->Error("Invalid arguments", "No type provided.");
      return;
    }

    try {
      int32_t volume = audio_manager_.GetVolume(type);
      result->Success(flutter::EncodableValue(volume));
    } catch (const TizenAudioManagerError &error) {
      result->Error(error.code(), error.message());
    }
  }

  TizenAudioManager audio_manager_;
  std::unique_ptr<FlEventChannel> event_channel_;
};

}  // namespace

void TizenAudioManagerPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  TizenAudioManagerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
