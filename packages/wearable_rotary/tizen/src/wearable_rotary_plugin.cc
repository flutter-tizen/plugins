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

#define ROTARY_CHANNEL_NAME "flutter.wearable_rotary.channel"

class WearableRotaryPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    LOG_DEBUG("RegisterWithRegistrar for WearableRotaryPlugin");
    auto plugin = std::make_unique<WearableRotaryPlugin>();
    plugin->setupEventChannels(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  WearableRotaryPlugin() {}

  virtual ~WearableRotaryPlugin() {}

 private:
  void setupEventChannels(flutter::PluginRegistrar *registrar) {
    channel_ = std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
        registrar->messenger(), ROTARY_CHANNEL_NAME,
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
                LOG_ERROR("[WearableRotaryPlugin] failed_to_add_callback");
                events->Error("failed_to_add_callback");
                return nullptr;
              }
              events_ = std::move(events);
              return nullptr;
            },
            [this](const flutter::EncodableValue *arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnCancel");
              void *ret = eext_rotary_event_handler_del(RotaryEventCallBack);
              if (!ret) {
                LOG_ERROR("[WearableRotaryPlugin] failed_to_del_callback");
                if (events_ != nullptr) {
                  events_->Error("failed_to_del_callback");
                }
                return nullptr;
              }
              events_ = nullptr;
              return nullptr;
            });
    channel_->SetStreamHandler(std::move(wearable_rotary_channel_handler));
  }

  static Eina_Bool RotaryEventCallBack(void *data,
                                       Eext_Rotary_Event_Info *dir) {
    auto *self = reinterpret_cast<WearableRotaryPlugin *>(data);
    bool clockwise = dir->direction == EEXT_ROTARY_DIRECTION_CLOCKWISE;
    self->events_->Success(flutter::EncodableValue(clockwise));
    return EINA_TRUE;
  }

  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>> channel_;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> events_;
};

void WearableRotaryPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  WearableRotaryPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
