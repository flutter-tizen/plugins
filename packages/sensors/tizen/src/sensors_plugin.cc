#include "sensors_plugin.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <sensor.h>
#include <tizen.h>

#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "log.h"

#define ACCELEROMETER_CHANNEL_NAME "plugins.flutter.io/sensors/accelerometer"
#define GYROSCOPE_CHANNEL_NAME "plugins.flutter.io/sensors/gyroscope"
#define USER_ACCELEROMETER_CHANNEL_NAME "plugins.flutter.io/sensors/user_accel"

class Listener {
 public:
  Listener(
      std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> &&event_sink)
      : event_sink_(std::move(event_sink)) {}
  bool Init(sensor_type_e type) {
    sensor_h sensor;
    int ret = sensor_get_default_sensor(type, &sensor);
    if (ret != SENSOR_ERROR_NONE) {
      LOG_ERROR("%s", get_error_message(ret));
      return false;
    }
    ret = sensor_create_listener(sensor, &listener_);
    if (ret != SENSOR_ERROR_NONE) {
      LOG_ERROR("%s", get_error_message(ret));
      return false;
    }
    return true;
  }

  void Listen() {
    if (is_listening_) {
      LOG_ERROR("Already listening!");
      return;
    }

    int ret = sensor_listener_set_event_cb(
        listener_, 1000,
        [](sensor_h sensor, sensor_event_s *event, void *user_data) {
          Listener *s = (Listener *)user_data;
          std::vector<double> list;
          for (int i = 0; i < event->value_count; i++) {
            list.push_back(event->values[i]);
          }
          flutter::EncodableValue value(list);
          s->event_sink_->Success(value);
        },
        this);

    if (ret != SENSOR_ERROR_NONE) {
      LOG_ERROR("%s", get_error_message(ret));
      return;
    }
    ret = sensor_listener_start(listener_);
    if (ret != SENSOR_ERROR_NONE) {
      LOG_ERROR("%s", get_error_message(ret));
      return;
    }

    is_listening_ = true;
  }

  void Cancel() {
    if (!is_listening_) {
      LOG_ERROR("Already canceled!");
      return;
    }
    int ret = sensor_listener_stop(listener_);
    if (ret != SENSOR_ERROR_NONE) {
      LOG_ERROR("%s", get_error_message(ret));
      return;
    }

    ret = sensor_listener_unset_event_cb(listener_);
    if (ret != SENSOR_ERROR_NONE) {
      LOG_ERROR("%s", get_error_message(ret));
      return;
    }
    is_listening_ = false;
  }

  virtual ~Listener() {
    if (is_listening_) {
      Cancel();
    }
    if (listener_) {
      int ret = sensor_destroy_listener(listener_);
      if (ret != SENSOR_ERROR_NONE) {
        LOG_ERROR("%s", get_error_message(ret));
      }
    }
  }

 protected:
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> event_sink_;
  sensor_listener_h listener_{nullptr};
  bool is_listening_{false};
};

class SensorsPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    LOG_DEBUG("RegisterWithRegistrar");
    auto plugin = std::make_unique<SensorsPlugin>();
    plugin->setupEventChannels(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  SensorsPlugin() { LOG_DEBUG("Create"); }

  virtual ~SensorsPlugin() { LOG_DEBUG("Destroy"); }

 private:
  void setupEventChannels(flutter::PluginRegistrar *registrar) {
    auto accelerometer_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            registrar->messenger(), ACCELEROMETER_CHANNEL_NAME,
            &flutter::StandardMethodCodec::GetInstance());
    auto accelerometer_channel_handler =
        std::make_unique<flutter::StreamHandlerFunctions<>>(
            [this](const flutter::EncodableValue *arguments,
                   std::unique_ptr<flutter::EventSink<>> &&events)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnListen");
              accelerometer_listener_ =
                  std::make_unique<Listener>(std::move(events));
              if (accelerometer_listener_->Init(SENSOR_ACCELEROMETER)) {
                accelerometer_listener_->Listen();
              }
              return nullptr;
            },
            [this](const flutter::EncodableValue *arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnCancel");
              if (accelerometer_listener_) {
                accelerometer_listener_->Cancel();
                accelerometer_listener_ = nullptr;
              }
              return nullptr;
            });
    accelerometer_channel->SetStreamHandler(
        std::move(accelerometer_channel_handler));

    auto gyroscope_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            registrar->messenger(), GYROSCOPE_CHANNEL_NAME,
            &flutter::StandardMethodCodec::GetInstance());
    auto gyroscope_channel_handler =
        std::make_unique<flutter::StreamHandlerFunctions<>>(
            [this](const flutter::EncodableValue *arguments,
                   std::unique_ptr<flutter::EventSink<>> &&events)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnListen");
              gyroscope_listener_ =
                  std::make_unique<Listener>(std::move(events));
              if (gyroscope_listener_->Init(SENSOR_GYROSCOPE)) {
                gyroscope_listener_->Listen();
              }
              return nullptr;
            },
            [this](const flutter::EncodableValue *arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnCancel");
              if (gyroscope_listener_) {
                gyroscope_listener_->Cancel();
                gyroscope_listener_ = nullptr;
              }
              return nullptr;
            });
    gyroscope_channel->SetStreamHandler(std::move(gyroscope_channel_handler));

    auto user_accel_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            registrar->messenger(), USER_ACCELEROMETER_CHANNEL_NAME,
            &flutter::StandardMethodCodec::GetInstance());
    auto user_accel_handler =
        std::make_unique<flutter::StreamHandlerFunctions<>>(
            [this](const flutter::EncodableValue *arguments,
                   std::unique_ptr<flutter::EventSink<>> &&events)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnListen");
              user_accel_listener_ =
                  std::make_unique<Listener>(std::move(events));
              if (user_accel_listener_->Init(SENSOR_LINEAR_ACCELERATION)) {
                user_accel_listener_->Listen();
              }
              return nullptr;
            },
            [this](const flutter::EncodableValue *arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnCancel");
              if (user_accel_listener_) {
                user_accel_listener_->Cancel();
                user_accel_listener_ = nullptr;
              }
              return nullptr;
            });
    user_accel_channel->SetStreamHandler(std::move(user_accel_handler));
  }

  std::unique_ptr<Listener> accelerometer_listener_;
  std::unique_ptr<Listener> gyroscope_listener_;
  std::unique_ptr<Listener> user_accel_listener_;
};

void SensorsPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  SensorsPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
