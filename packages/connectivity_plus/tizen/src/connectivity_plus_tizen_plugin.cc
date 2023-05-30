// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "connectivity_plus_tizen_plugin.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <string>

#include "connection.h"

namespace {

typedef flutter::EventChannel<flutter::EncodableValue> FlEventChannel;
typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;
typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;
typedef flutter::StreamHandler<flutter::EncodableValue> FlStreamHandler;
typedef flutter::StreamHandlerError<flutter::EncodableValue>
    FlStreamHandlerError;

std::string ConnectionTypeToString(ConnectionType type) {
  switch (type) {
    case ConnectionType::kEthernet:
      return "ethernet";
    case ConnectionType::kWiFi:
      return "wifi";
    case ConnectionType::kMobile:
      return "mobile";
    case ConnectionType::kBluetooth:
      return "bluetooth";
    case ConnectionType::kNone:
    default:
      return "none";
  }
}

class ConnectivityStreamHandler : public FlStreamHandler {
 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    events_ = std::move(events);

    ConnectionTypeCallback callback = [this](ConnectionType type) -> void {
      if (type != ConnectionType::kError) {
        events_->Success(flutter::EncodableValue(ConnectionTypeToString(type)));
      } else {
        events_->Error(std::to_string(connection_.GetLastError()),
                       connection_.GetLastErrorString());
      }
    };
    if (!connection_.StartListen(callback)) {
      return std::make_unique<FlStreamHandlerError>(
          std::to_string(connection_.GetLastError()),
          connection_.GetLastErrorString(), nullptr);
    }

    // Send an initial event once the stream has been set up.
    callback(connection_.GetType());

    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    connection_.StopListen();
    events_.reset();
    return nullptr;
  }

 private:
  Connection connection_;
  std::unique_ptr<FlEventSink> events_;
};

class ConnectivityPlusTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<ConnectivityPlusTizenPlugin>();

    auto method_channel = std::make_unique<FlMethodChannel>(
        registrar->messenger(), "dev.fluttercommunity.plus/connectivity",
        &flutter::StandardMethodCodec::GetInstance());
    method_channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    plugin->event_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "dev.fluttercommunity.plus/connectivity_status",
        &flutter::StandardMethodCodec::GetInstance());
    plugin->event_channel_->SetStreamHandler(
        std::make_unique<ConnectivityStreamHandler>());

    registrar->AddPlugin(std::move(plugin));
  }

  ConnectivityPlusTizenPlugin() {}

  virtual ~ConnectivityPlusTizenPlugin() {}

 private:
  void HandleMethodCall(const FlMethodCall &method_call,
                        std::unique_ptr<FlMethodResult> result) {
    const auto &method_name = method_call.method_name();

    if (method_name == "check") {
      Connection connection;
      ConnectionType type = connection.GetType();
      if (type != ConnectionType::kError) {
        result->Success(flutter::EncodableValue(ConnectionTypeToString(type)));
      } else {
        result->Error(std::to_string(connection.GetLastError()),
                      connection.GetLastErrorString());
      }
    } else {
      result->NotImplemented();
    }
  }

  std::unique_ptr<FlEventChannel> event_channel_;
};

}  // namespace

void ConnectivityPlusTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  ConnectivityPlusTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
