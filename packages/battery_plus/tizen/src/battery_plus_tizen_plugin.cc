// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "battery_plus_tizen_plugin.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <string>

#include "device_battery.h"

namespace {

typedef flutter::EventChannel<flutter::EncodableValue> FlEventChannel;
typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;
typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;
typedef flutter::StreamHandler<flutter::EncodableValue> FlStreamHandler;
typedef flutter::StreamHandlerError<flutter::EncodableValue>
    FlStreamHandlerError;

std::string BatteryStatusToString(BatteryStatus status) {
  switch (status) {
    case BatteryStatus::kCharging:
      return "charging";
    case BatteryStatus::kFull:
      return "full";
    case BatteryStatus::kDischarging:
      return "discharging";
    case BatteryStatus::kUnknown:
    default:
      return "unknown";
  }
}

class BatteryStatusStreamHandler : public FlStreamHandler {
 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    events_ = std::move(events);

    BatteryStatusCallback callback = [this](BatteryStatus status) -> void {
      if (status != BatteryStatus::kError) {
        events_->Success(
            flutter::EncodableValue(BatteryStatusToString(status)));
      } else {
        events_->Error(std::to_string(battery_.GetLastError()),
                       battery_.GetLastErrorString());
      }
    };
    if (!battery_.StartListen(callback)) {
      return std::make_unique<FlStreamHandlerError>(
          std::to_string(battery_.GetLastError()),
          battery_.GetLastErrorString(), nullptr);
    }

    // Send an initial event once the stream has been set up.
    callback(battery_.GetStatus());

    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    battery_.StopListen();
    events_.reset();
    return nullptr;
  }

 private:
  DeviceBattery battery_;
  std::unique_ptr<FlEventSink> events_;
};

class BatteryPlusTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<BatteryPlusTizenPlugin>();

    auto method_channel = std::make_unique<FlMethodChannel>(
        registrar->messenger(), "dev.fluttercommunity.plus/battery",
        &flutter::StandardMethodCodec::GetInstance());
    method_channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    auto event_channel = std::make_unique<FlEventChannel>(
        registrar->messenger(), "dev.fluttercommunity.plus/charging",
        &flutter::StandardMethodCodec::GetInstance());
    event_channel->SetStreamHandler(
        std::make_unique<BatteryStatusStreamHandler>());

    registrar->AddPlugin(std::move(plugin));
  }

  BatteryPlusTizenPlugin() {}

  virtual ~BatteryPlusTizenPlugin() {}

 private:
  void HandleMethodCall(const FlMethodCall &method_call,
                        std::unique_ptr<FlMethodResult> result) {
    const auto &method_name = method_call.method_name();

    if (method_name == "getBatteryLevel") {
      DeviceBattery battery;
      int32_t level = battery.GetLevel();
      if (level >= 0) {
        result->Success(flutter::EncodableValue(level));
      } else {
        result->Error(std::to_string(battery.GetLastError()),
                      battery.GetLastErrorString());
      }
    } else if (method_name == "getBatteryState") {
      DeviceBattery battery;
      BatteryStatus status = battery.GetStatus();
      if (status != BatteryStatus::kError) {
        result->Success(flutter::EncodableValue(BatteryStatusToString(status)));
      } else {
        result->Error(std::to_string(battery.GetLastError()),
                      battery.GetLastErrorString());
      }
    } else {
      result->NotImplemented();
    }
  }
};

}  // namespace

void BatteryPlusTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  BatteryPlusTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
