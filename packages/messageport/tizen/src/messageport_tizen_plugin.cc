// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "messageport_tizen_plugin.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <set>
#include <sstream>
#include <string>

#include "log.h"
#include "messageport_manager.h"

typedef flutter::EventChannel<flutter::EncodableValue> FlEventChannel;
typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;
typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::StreamHandler<flutter::EncodableValue> FlStreamHandler;
typedef flutter::StreamHandlerError<flutter::EncodableValue>
    FlStreamHandlerError;

namespace {

class LocalPortStreamHandler : public FlStreamHandler {
 public:
  LocalPortStreamHandler(const std::string &port_name, bool is_trusted)
      : port_name_(port_name), is_trusted_(is_trusted) {}

  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    std::optional<MessagePortError> error =
        MessagePortManager::GetInstance().RegisterLocalPort(
            port_name_, std::move(events), is_trusted_);
    if (error.has_value()) {
      LOG_ERROR("Error OnListen: %s", error.value().message().c_str());
    }
    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    std::optional<MessagePortError> error =
        MessagePortManager::GetInstance().UnregisterLocalPort(port_name_,
                                                              is_trusted_);
    if (error.has_value()) {
      LOG_ERROR("Error OnCancel: %s", error.value().message().c_str());
    }
    return nullptr;
  }

 private:
  std::string port_name_;
  bool is_trusted_;
};

class MessageportTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel = std::make_unique<FlMethodChannel>(
        registrar->messenger(), "tizen/messageport",
        &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<MessageportTizenPlugin>(registrar);

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  MessageportTizenPlugin(flutter::PluginRegistrar *pluginRegistrar)
      : plugin_registrar_(pluginRegistrar) {}

  virtual ~MessageportTizenPlugin() {}

 private:
  template <typename T>
  bool GetValueFromArgs(const flutter::EncodableValue *args, const char *key,
                        T &out) {
    if (std::holds_alternative<flutter::EncodableMap>(*args)) {
      flutter::EncodableMap map = std::get<flutter::EncodableMap>(*args);
      if (map.find(flutter::EncodableValue(key)) != map.end()) {
        flutter::EncodableValue value = map[flutter::EncodableValue(key)];
        if (std::holds_alternative<T>(value)) {
          out = std::get<T>(value);
          return true;
        }
      }
    }
    return false;
  }

  bool GetEncodableValueFromArgs(const flutter::EncodableValue *args,
                                 const char *key,
                                 flutter::EncodableValue &out) {
    if (std::holds_alternative<flutter::EncodableMap>(*args)) {
      flutter::EncodableMap map = std::get<flutter::EncodableMap>(*args);
      if (map.find(flutter::EncodableValue(key)) != map.end()) {
        out = map[flutter::EncodableValue(key)];
        return true;
      }
    }
    return false;
  }

  void HandleMethodCall(const FlMethodCall &method_call,
                        std::unique_ptr<FlMethodResult> result) {
    const auto &method_name = method_call.method_name();
    const flutter::EncodableValue *args = method_call.arguments();

    if (method_name == "createLocal") {
      CreateLocal(args, std::move(result));
    } else if (method_name == "checkForRemote") {
      CheckForRemote(args, std::move(result));
    } else if (method_name == "send") {
      Send(args, std::move(result));
    } else {
      result->NotImplemented();
    }
  }

  void CheckForRemote(const flutter::EncodableValue *args,
                      std::unique_ptr<FlMethodResult> result) {
    std::string remote_app_id = "";
    std::string port_name = "";
    bool trusted = false;
    if (!GetValueFromArgs<std::string>(args, "remoteAppId", remote_app_id) ||
        !GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<bool>(args, "trusted", trusted)) {
      result->Error("Invalid parameter");
      return;
    }

    ErrorOr<bool> ret = MessagePortManager::GetInstance().CheckRemotePort(
        remote_app_id, port_name, trusted);
    if (ret.has_error()) {
      result->Error("Could not create remote port", ret.error().message());
    } else {
      result->Success(flutter::EncodableValue(ret.value()));
    }
  }

  void CreateLocal(const flutter::EncodableValue *args,
                   std::unique_ptr<FlMethodResult> result) {
    std::string port_name = "";
    bool trusted = false;
    if (!GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<bool>(args, "trusted", trusted)) {
      result->Error("Could not create local port", "Invalid parameter");
      return;
    }

    if (MessagePortManager::GetInstance().IsRegisteredLocalPort(port_name,
                                                                trusted)) {
      result->Success();
      return;
    }

    std::stringstream event_channel_name;
    event_channel_name << "tizen/messageport/" << port_name;
    if (trusted) {
      event_channel_name << "_trusted";
    }

    auto event_channel = std::make_unique<FlEventChannel>(
        plugin_registrar_->messenger(), event_channel_name.str(),
        &flutter::StandardMethodCodec::GetInstance());
    event_channel->SetStreamHandler(
        std::make_unique<LocalPortStreamHandler>(port_name, trusted));
    event_channels_.insert(std::move(event_channel));
    result->Success();
  }

  void Send(const flutter::EncodableValue *args,
            std::unique_ptr<FlMethodResult> result) {
    flutter::EncodableValue message(nullptr);
    std::string remote_app_id = "";
    std::string port_name = "";
    bool trusted = false;
    if (!GetEncodableValueFromArgs(args, "message", message) ||
        !GetValueFromArgs<std::string>(args, "remoteAppId", remote_app_id) ||
        !GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<bool>(args, "trusted", trusted)) {
      result->Error("Could not send message", "Invalid parameter");
      return;
    }

    std::string local_port_name;
    bool local_port_trusted = false;
    std::optional<MessagePortError> error = std::nullopt;
    if (GetValueFromArgs<std::string>(args, "localPort", local_port_name) &&
        GetValueFromArgs<bool>(args, "localPortTrusted", local_port_trusted)) {
      error = MessagePortManager::GetInstance().Send(
          remote_app_id, port_name, message, trusted, local_port_name,
          local_port_trusted);
    } else {
      error = MessagePortManager::GetInstance().Send(remote_app_id, port_name,
                                                     message, trusted);
    }

    if (error.has_value()) {
      result->Error("Could not send message", error.value().message());
      return;
    }

    result->Success();
  }

  // < channel_name, is_trusted > -> native number >
  std::set<std::unique_ptr<FlEventChannel>> event_channels_;
  flutter::PluginRegistrar *plugin_registrar_;
};

}  // namespace

void MessageportTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  MessageportTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
