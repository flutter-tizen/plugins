#include "sensors_plus_plugin.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include "device_sensor.h"

typedef flutter::EventChannel<flutter::EncodableValue> FlEventChannel;
typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;
typedef flutter::StreamHandler<flutter::EncodableValue> FlStreamHandler;
typedef flutter::StreamHandlerError<flutter::EncodableValue>
    FlStreamHandlerError;

class DeviceSensorStreamHandler : public FlStreamHandler {
 public:
  DeviceSensorStreamHandler(SensorType type) : sensor_(type) {}

 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    events_ = std::move(events);

    SensorEventCallback callback = [this](SensorEvent sensor_event) -> void {
      events_->Success(flutter::EncodableValue(sensor_event));
    };
    if (!sensor_.StartListen(callback)) {
      return std::make_unique<FlStreamHandlerError>(
          std::to_string(sensor_.GetLastError()), sensor_.GetLastErrorString(),
          nullptr);
    }
    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    sensor_.StopListen();
    events_.reset();
    return nullptr;
  }

 private:
  DeviceSensor sensor_;
  std::unique_ptr<FlEventSink> events_;
};

class SensorsPlusPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<SensorsPlusPlugin>();
    plugin->SetupEventChannels(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  SensorsPlusPlugin() {}

  virtual ~SensorsPlusPlugin() {}

 private:
  void SetupEventChannels(flutter::PluginRegistrar *registrar) {
    accelerometer_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(),
        "dev.fluttercommunity.plus/sensors/accelerometer",
        &flutter::StandardMethodCodec::GetInstance());
    accelerometer_channel_->SetStreamHandler(
        std::make_unique<DeviceSensorStreamHandler>(
            SensorType::kAccelerometer));

    gyroscope_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "dev.fluttercommunity.plus/sensors/gyroscope",
        &flutter::StandardMethodCodec::GetInstance());
    gyroscope_channel_->SetStreamHandler(
        std::make_unique<DeviceSensorStreamHandler>(SensorType::kGyroscope));

    user_accel_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "dev.fluttercommunity.plus/sensors/user_accel",
        &flutter::StandardMethodCodec::GetInstance());
    user_accel_channel_->SetStreamHandler(
        std::make_unique<DeviceSensorStreamHandler>(SensorType::kUserAccel));
  }

  std::unique_ptr<FlEventChannel> accelerometer_channel_;
  std::unique_ptr<FlEventChannel> gyroscope_channel_;
  std::unique_ptr<FlEventChannel> user_accel_channel_;
};

void SensorsPlusPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  SensorsPlusPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
