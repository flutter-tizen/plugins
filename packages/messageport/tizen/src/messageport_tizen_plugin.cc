// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "messageport_tizen_plugin.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_message_codec.h>
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

class LocalPortStreamHandlerError : public FlStreamHandlerError {
 public:
  LocalPortStreamHandlerError(const std::string &error_code,
                              const std::string &error_message,
                              const flutter::EncodableValue *error_details)
      : error_code_(error_code),
        error_message_(error_message),
        FlStreamHandlerError(error_code_, error_message_, error_details) {}

 private:
  std::string error_code_;
  std::string error_message_;
};

class LocalPortStreamHandler : public FlStreamHandler {
 public:
  LocalPortStreamHandler(LocalPort *local_port) : local_port_(local_port) {}

  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    event_sink_ = std::move(events);
    std::optional<MessagePortError> error =
        local_port_->Register([this](const Message &message) {
          if (message.error.size()) {
            event_sink_->Error(message.error);
            return;
          }

          flutter::EncodableMap map;
          auto value =
              flutter::StandardMessageCodec::GetInstance().DecodeMessage(
                  message.encoded_message);
          map[flutter::EncodableValue("message")] = *(value.get());
          if (message.remort_port.app_id().size() &&
              message.remort_port.name().size()) {
            map[flutter::EncodableValue("remoteAppId")] =
                flutter::EncodableValue(message.remort_port.app_id());
            map[flutter::EncodableValue("remotePort")] =
                flutter::EncodableValue(message.remort_port.name());
            map[flutter::EncodableValue("trusted")] =
                flutter::EncodableValue(message.remort_port.is_trusted());
          }

          event_sink_->Success(flutter::EncodableValue(map));
        });

    if (error.has_value()) {
      return std::make_unique<LocalPortStreamHandlerError>(
          std::to_string(error.value().code()), error.value().message(),
          nullptr);
    }

    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    std::optional<MessagePortError> error = local_port_->Unregister();
    if (error.has_value()) {
      return std::make_unique<LocalPortStreamHandlerError>(
          std::to_string(error.value().code()), error.value().message(),
          nullptr);
    }
    return nullptr;
  }

 private:
  LocalPort *local_port_ = nullptr;
  std::unique_ptr<FlEventSink> event_sink_;
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

    RemotePort remote_port(remote_app_id, port_name, trusted);
    ErrorOr<bool> ret = remote_port.CheckRemotePort();
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

    if (FindLocalPort(port_name, trusted)) {
      result->Success();
      return;
    }

    std::unique_ptr<LocalPort> local_port =
        std::make_unique<LocalPort>(port_name, trusted);

    std::stringstream event_channel_name;
    event_channel_name << "tizen/messageport/" << port_name;
    if (trusted) {
      event_channel_name << "_trusted";
    }

    auto event_channel = std::make_unique<FlEventChannel>(
        plugin_registrar_->messenger(), event_channel_name.str(),
        &flutter::StandardMethodCodec::GetInstance());
    event_channel->SetStreamHandler(
        std::make_unique<LocalPortStreamHandler>(local_port.get()));
    event_channels_.insert(std::move(event_channel));
    local_ports_.push_back(std::move(local_port));
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
    std::unique_ptr<std::vector<uint8_t>> encoded_message =
        flutter::StandardMessageCodec::GetInstance().EncodeMessage(message);

    if (!GetValueFromEncodableMap(arguments, "remoteAppId", remote_app_id) ||
        !GetValueFromEncodableMap(arguments, "portName", port_name) ||
        !GetValueFromEncodableMap(arguments, "trusted", trusted)) {
      result->Error("Invalid arguments");
      return;
    }
    RemotePort remote_port(remote_app_id, port_name, trusted);

    std::optional<MessagePortError> error = std::nullopt;
    std::string local_port_name;
    bool local_port_trusted = false;
    if (GetValueFromEncodableMap(arguments, "localPort", local_port_name) &&
        GetValueFromEncodableMap(arguments, "localPortTrusted",
                                 local_port_trusted)) {
      LocalPort *local_port =
          FindLocalPort(local_port_name, local_port_trusted);
      if (local_port == nullptr) {
        result->Error("Invalid arguments");
        return;
      }
      error = remote_port.SendWithLocalPort(*encoded_message, local_port);
    } else {
      error = remote_port.Send(*encoded_message);
    }

    if (error.has_value()) {
      MessagePortError message_port_error = error.value();
      result->Error(std::to_string(message_port_error.code()),
                    message_port_error.message());
      return;
    }

    result->Success();
  }

  LocalPort *FindLocalPort(const std::string &port_name, bool is_trusted) {
    for (auto &port : local_ports_) {
      if (port->name() == port_name && port->is_trusted() == is_trusted) {
        return port.get();
      }
    }

    return nullptr;
  }

  std::set<std::unique_ptr<FlEventChannel>> event_channels_;
  std::vector<std::unique_ptr<LocalPort>> local_ports_;
  flutter::PluginRegistrar *plugin_registrar_ = nullptr;
};

}  // namespace

void MessageportTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  MessageportTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
