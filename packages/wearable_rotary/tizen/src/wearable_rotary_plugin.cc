// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wearable_rotary_plugin.h"

#include <efl-extension/wearable/circle/efl_extension_rotary.h>
#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <memory>

#include "log.h"

namespace {

typedef flutter::EventChannel<flutter::EncodableValue> FlEventChannel;
typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;
typedef flutter::StreamHandler<flutter::EncodableValue> FlStreamHandler;
typedef flutter::StreamHandlerError<flutter::EncodableValue>
    FlStreamHandlerError;

class WearableRotaryStreamHandler : public FlStreamHandler {
 public:
  WearableRotaryStreamHandler() {}

 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    events_ = std::move(events);

    Eina_Bool ret = eext_rotary_event_handler_add(OnRotaryEvent, this);
    if (ret == EINA_FALSE) {
      return std::make_unique<FlStreamHandlerError>(
          "Operation failed", "Failed to add rotary event handler.", nullptr);
    }
    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    eext_rotary_event_handler_del(OnRotaryEvent);
    events_.reset();
    return nullptr;
  }

 private:
  static Eina_Bool OnRotaryEvent(void *data, Eext_Rotary_Event_Info *info) {
    auto *self = static_cast<WearableRotaryStreamHandler *>(data);
    bool clockwise = (info->direction == EEXT_ROTARY_DIRECTION_CLOCKWISE);
    self->events_->Success(flutter::EncodableValue(clockwise));
    return EINA_TRUE;
  }

  std::unique_ptr<FlEventSink> events_;
};

class WearableRotaryPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<WearableRotaryPlugin>();

    plugin->event_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "flutter.wearable_rotary.channel",
        &flutter::StandardMethodCodec::GetInstance());
    plugin->event_channel_->SetStreamHandler(
        std::make_unique<WearableRotaryStreamHandler>());

    registrar->AddPlugin(std::move(plugin));
  }

  WearableRotaryPlugin() {}

  virtual ~WearableRotaryPlugin() {}

 private:
  std::unique_ptr<FlEventChannel> event_channel_;
};

}  // namespace

void WearableRotaryPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  WearableRotaryPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
