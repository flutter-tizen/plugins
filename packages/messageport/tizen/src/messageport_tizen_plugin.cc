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
#include "messageport.h"

typedef flutter::EventChannel<flutter::EncodableValue> FlEventChannel;
typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;
typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::StreamHandler<flutter::EncodableValue> FlStreamHandler;
typedef flutter::StreamHandlerError<flutter::EncodableValue>
    FlStreamHandlerError;

namespace {

template <typename T>
bool GetValueFromEncodableMap(const flutter::EncodableMap *map, const char *key,
                              T &out) {
  auto iter = map->find(flutter::EncodableValue(key));
  if (iter != map->end() && !iter->second.IsNull()) {
    if (auto *value = std::get_if<T>(&iter->second)) {
      out = *value;
      return true;
    }
  }
  return false;
}

class LocalPortStreamHandler : public FlStreamHandler {
 public:
  LocalPortStreamHandler(const std::string &port_name, bool is_trusted)
      : port_name_(port_name), is_trusted_(is_trusted) {}

  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    std::optional<MessagePortError> error =
        MessagePort::GetInstance().RegisterLocalPort(
            port_name_, std::move(events), is_trusted_);
    if (error.has_value()) {
      LOG_ERROR("Error OnListen: %s", error.value().message().c_str());
    }
    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    std::optional<MessagePortError> error =
        MessagePort::GetInstance().UnregisterLocalPort(port_name_, is_trusted_);
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

  MessageportTizenPlugin(flutter::PluginRegistrar *plugin_registrar)
      : plugin_registrar_(plugin_registrar) {}

  virtual ~MessageportTizenPlugin() {}

 private:
  void HandleMethodCall(const FlMethodCall &method_call,
                        std::unique_ptr<FlMethodResult> result) {
    const auto &method_name = method_call.method_name();
    const auto *arguments =
        std::get_if<flutter::EncodableMap>(method_call.arguments());

    if (method_name == "createLocal") {
      CreateLocal(arguments, std::move(result));
    } else if (method_name == "checkForRemote") {
      CheckForRemote(arguments, std::move(result));
    } else if (method_name == "send") {
      Send(arguments, std::move(result));
    } else {
      result->NotImplemented();
    }
  }

  void CheckForRemote(const flutter::EncodableMap *arguments,
                      std::unique_ptr<FlMethodResult> result) {
    std::string remote_app_id;
    std::string port_name;
    bool trusted = false;

    if (!GetValueFromEncodableMap(arguments, "remoteAppId", remote_app_id) ||
        !GetValueFromEncodableMap(arguments, "portName", port_name) ||
        !GetValueFromEncodableMap(arguments, "trusted", trusted)) {
      result->Error("Invalid arguments");
      return;
    }

    ErrorOr<bool> ret = MessagePort::GetInstance().CheckRemotePort(
        remote_app_id, port_name, trusted);
    if (ret.has_error()) {
      MessagePortError error = ret.error();
      result->Error(std::to_string(error.code()), error.message());
    } else {
      result->Success(flutter::EncodableValue(ret.value()));
    }
  }

  void CreateLocal(const flutter::EncodableMap *arguments,
                   std::unique_ptr<FlMethodResult> result) {
    std::string port_name;
    bool trusted = false;

    if (!GetValueFromEncodableMap(arguments, "portName", port_name) ||
        !GetValueFromEncodableMap(arguments, "trusted", trusted)) {
      result->Error("Invalid arguments");
      return;
    }

    if (MessagePort::GetInstance().IsRegisteredLocalPort(port_name, trusted)) {
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

  void Send(const flutter::EncodableMap *arguments,
            std::unique_ptr<FlMethodResult> result) {
    flutter::EncodableValue message;
    std::string remote_app_id;
    std::string port_name;
    bool trusted = false;

    auto iter = arguments->find(flutter::EncodableValue("message"));
    if (iter != arguments->end() && !iter->second.IsNull()) {
      message = iter->second;
    } else {
      result->Error("Invalid arguments");
      return;
    }

    if (!GetValueFromEncodableMap(arguments, "remoteAppId", remote_app_id) ||
        !GetValueFromEncodableMap(arguments, "portName", port_name) ||
        !GetValueFromEncodableMap(arguments, "trusted", trusted)) {
      result->Error("Invalid arguments");
      return;
    }

    std::string local_port_name;
    bool local_port_trusted = false;
    std::optional<MessagePortError> error = std::nullopt;
    if (GetValueFromEncodableMap(arguments, "localPort", local_port_name) &&
        GetValueFromEncodableMap(arguments, "localPortTrusted",
                                 local_port_trusted)) {
      error = MessagePort::GetInstance().Send(remote_app_id, port_name, message,
                                              trusted, local_port_name,
                                              local_port_trusted);
    } else {
      error = MessagePort::GetInstance().Send(remote_app_id, port_name, message,
                                              trusted);
    }

    if (error.has_value()) {
      MessagePortError message_port_error = error.value();
      result->Error(std::to_string(message_port_error.code()),
                    message_port_error.message());
      return;
    }

    result->Success();
  }

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
