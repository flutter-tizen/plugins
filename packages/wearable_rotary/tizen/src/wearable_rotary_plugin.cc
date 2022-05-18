// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wearable_rotary_plugin.h"

#include <efl-extension/wearable/circle/efl_extension_rotary.h>
#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <memory>

#include "log.h"

static constexpr char kChannelName[] = "flutter.wearable_rotary.channel";

class WearableRotaryPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<WearableRotaryPlugin>();
    plugin->SetupEventChannel(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  WearableRotaryPlugin() {}

  virtual ~WearableRotaryPlugin() {}

 private:
  void SetupEventChannel(flutter::PluginRegistrar *registrar) {
    event_channel_ =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            registrar->messenger(), kChannelName,
            &flutter::StandardMethodCodec::GetInstance());
    auto wearable_rotary_channel_handler =
        std::make_unique<flutter::StreamHandlerFunctions<>>(
            [this](const flutter::EncodableValue *arguments,
                   std::unique_ptr<flutter::EventSink<>> &&events)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnListen");
              Eina_Bool ret =
                  eext_rotary_event_handler_add(RotaryEventCallBack, this);
              if (ret == EINA_FALSE) {
                events->Error("failed to add callback");
                return nullptr;
              }
              events_ = std::move(events);
              return nullptr;
            },
            [this](const flutter::EncodableValue *arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnCancel");
              eext_rotary_event_handler_del(RotaryEventCallBack);
              events_ = nullptr;
              return nullptr;
            });
    event_channel_->SetStreamHandler(
        std::move(wearable_rotary_channel_handler));
  }

  static Eina_Bool RotaryEventCallBack(void *data,
                                       Eext_Rotary_Event_Info *info) {
    auto *self = reinterpret_cast<WearableRotaryPlugin *>(data);
    bool clockwise = info->direction == EEXT_ROTARY_DIRECTION_CLOCKWISE;
    self->events_->Success(flutter::EncodableValue(clockwise));
    return EINA_TRUE;
  }

 private:
  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>
      event_channel_;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> events_;
};

void WearableRotaryPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  WearableRotaryPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
