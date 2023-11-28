// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sensors_plus_plugin.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <memory>

#include "device_sensor.h"
#include "log.h"

typedef flutter::EventChannel<flutter::EncodableValue> FlEventChannel;
typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;
typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::StreamHandler<flutter::EncodableValue> FlStreamHandler;
typedef flutter::StreamHandlerError<flutter::EncodableValue>
    FlStreamHandlerError;

class DeviceSensorStreamHandler : public FlStreamHandler {
 public:
  DeviceSensorStreamHandler(DeviceSensor *sensor) : sensor_(sensor) {}

 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    events_ = std::move(events);

    SensorEventCallback callback = [this](SensorEvent sensor_event) -> void {
      events_->Success(flutter::EncodableValue(sensor_event));
    };
    if (!sensor_->StartListen(callback)) {
      events_->Error(sensor_->GetLastErrorString());
      return std::make_unique<FlStreamHandlerError>(
          std::to_string(sensor_->GetLastError()),
          sensor_->GetLastErrorString(), nullptr);
    }
    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    sensor_->StopListen();
    events_.reset();
    return nullptr;
  }

 private:
  DeviceSensor *sensor_;
  std::unique_ptr<FlEventSink> events_;
};

class SensorsPlusPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<SensorsPlusPlugin>();
    std::unique_ptr<FlMethodChannel> method_channel =
        std::make_unique<FlMethodChannel>(
            registrar->messenger(), "dev.fluttercommunity.plus/sensors/method",
            &flutter::StandardMethodCodec::GetInstance());

    method_channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });
    plugin->SetUpEventChannels(registrar, plugin.get());
    registrar->AddPlugin(std::move(plugin));
  }

  SensorsPlusPlugin() {}

  virtual ~SensorsPlusPlugin() {}

 private:
  void SetUpEventChannels(flutter::PluginRegistrar *registrar,
                          SensorsPlusPlugin *plugin) {
    accelerometer_event_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(),
        "dev.fluttercommunity.plus/sensors/accelerometer",
        &flutter::StandardMethodCodec::GetInstance());
    accelerometer_sensor_ =
        std::make_unique<DeviceSensor>(SensorType::kAccelerometer);
    accelerometer_event_channel_->SetStreamHandler(
        std::make_unique<DeviceSensorStreamHandler>(
            accelerometer_sensor_.get()));

    gyroscope_event_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "dev.fluttercommunity.plus/sensors/gyroscope",
        &flutter::StandardMethodCodec::GetInstance());
    gyroscope_sensor_ = std::make_unique<DeviceSensor>(SensorType::kGyroscope);
    gyroscope_event_channel_->SetStreamHandler(
        std::make_unique<DeviceSensorStreamHandler>(gyroscope_sensor_.get()));

    user_accelerometer_event_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "dev.fluttercommunity.plus/sensors/user_accel",
        &flutter::StandardMethodCodec::GetInstance());
    user_accelerometer_sensor_ =
        std::make_unique<DeviceSensor>(SensorType::kUserAccel);
    user_accelerometer_event_channel_->SetStreamHandler(
        std::make_unique<DeviceSensorStreamHandler>(
            user_accelerometer_sensor_.get()));

    magnetometer_event_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(),
        "dev.fluttercommunity.plus/sensors/magnetometer",
        &flutter::StandardMethodCodec::GetInstance());
    magnetometer_sensor_ =
        std::make_unique<DeviceSensor>(SensorType::kMagnetometer);
    magnetometer_event_channel_->SetStreamHandler(
        std::make_unique<DeviceSensorStreamHandler>(
            magnetometer_sensor_.get()));
  }

  void HandleMethodCall(const FlMethodCall &method_call,
                        std::unique_ptr<FlMethodResult> result) {
    const std::string &method_name = method_call.method_name();

    if (method_name == "setAccelerationSamplingPeriod") {
      if (!SetInterval(method_call, result.get())) {
        return;
      }
      accelerometer_sensor_->SetInterval(interval_ms_);
    } else if (method_name == "setGyroscopeSamplingPeriod") {
      if (!SetInterval(method_call, result.get())) {
        return;
      }
      gyroscope_sensor_->SetInterval(interval_ms_);
    } else if (method_name == "setUserAccelerometerSamplingPeriod") {
      if (!SetInterval(method_call, result.get())) {
        return;
      }
      user_accelerometer_sensor_->SetInterval(interval_ms_);
    } else if (method_name == "setMagnetometerSamplingPeriod") {
      if (!SetInterval(method_call, result.get())) {
        return;
      }
      magnetometer_sensor_->SetInterval(interval_ms_);
    } else {
      result->NotImplemented();
      return;
    }
    result->Success();
  }

  bool SetInterval(const FlMethodCall &method_call, FlMethodResult *result) {
    const auto *sampling_period = std::get_if<int32_t>(method_call.arguments());
    if (!sampling_period) {
      result->Error("Invalid argument", "No sampling period provided.");
      return false;
    }

    if (*sampling_period < 10000) {
      interval_ms_ = 10;
    } else {
      interval_ms_ =
          *sampling_period / 1000;  // sampling_period is in microsecond.
    }

    return true;
  }

  int32_t interval_ms_ = 0;
  std::unique_ptr<DeviceSensor> accelerometer_sensor_;
  std::unique_ptr<DeviceSensor> gyroscope_sensor_;
  std::unique_ptr<DeviceSensor> user_accelerometer_sensor_;
  std::unique_ptr<DeviceSensor> magnetometer_sensor_;

  std::unique_ptr<FlEventChannel> accelerometer_event_channel_;
  std::unique_ptr<FlEventChannel> gyroscope_event_channel_;
  std::unique_ptr<FlEventChannel> user_accelerometer_event_channel_;
  std::unique_ptr<FlEventChannel> magnetometer_event_channel_;
};

void SensorsPlusPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  SensorsPlusPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
