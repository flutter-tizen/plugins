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

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "log.h"
#include "messageport.h"

namespace {

class MessageportTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
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
      : pluginRegistrar_(pluginRegistrar) {}

  virtual ~MessageportTizenPlugin() {}

 private:
  template <typename T>
  bool GetValueFromArgs(const flutter::EncodableValue *args, const char *key,
                        T &out) {
    if (std::holds_alternative<flutter::EncodableMap>(*args)) {
      flutter::EncodableMap map = std::get<flutter::EncodableMap>(*args);
      flutter::EncodableValue value = map[flutter::EncodableValue(key)];
      if (std::holds_alternative<T>(value)) {
        out = std::get<T>(value);
        return true;
      }
      LOG_ERROR("Key %s not found", key);
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

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("HandleMethodCall: %s", method_call.method_name().c_str());
    const flutter::EncodableValue *args = method_call.arguments();

    if (method_call.method_name().compare("createLocal") == 0) {
      CreateLocal(args, std::move(result));
    } else if (method_call.method_name().compare("createRemote") == 0) {
      CreateRemote(args, std::move(result));
    } else if (method_call.method_name().compare("send") == 0) {
      Send(args, std::move(result));
    } else {
      result->Error("Invalid method");
    }
  }

  void CreateRemote(
      const flutter::EncodableValue *args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("CreateRemote");
    std::string remote_app_id = "";
    std::string port_name = "";
    bool is_trusted = false;
    if (!GetValueFromArgs<std::string>(args, "remoteAppId", remote_app_id) ||
        !GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<bool>(args, "isTrusted", is_trusted)) {
      result->Error("Invalid parameter");
      return;
    }

    bool port_check = false;
    MessagePortResult native_result = manager.CheckRemotePort(
        remote_app_id, port_name, is_trusted, &port_check);
    if (native_result) {
      result->Success(flutter::EncodableValue(port_check));
    } else {
      result->Error("Could not create remote port", native_result.message());
    }
  }

  void CreateLocal(
      const flutter::EncodableValue *args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("CreateLocal");
    std::string port_name = "";
    bool is_trusted = false;
    if (!GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<bool>(args, "isTrusted", is_trusted)) {
      result->Error("Could not create local port", "Invalid parameter");
      return;
    }

    std::stringstream event_channel_name;
    int id = nextId_++;
    event_channel_name << "tizen/messageport" << id;

    auto event_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            pluginRegistrar_->messenger(), event_channel_name.str(),
            &flutter::StandardMethodCodec::GetInstance());

    auto event_channel_handler =
        std::make_unique<flutter::StreamHandlerFunctions<>>(
            [this, id, port_name, is_trusted](
                const flutter::EncodableValue *arguments,
                std::unique_ptr<flutter::EventSink<>> &&events)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnListen: %s, %d", port_name.c_str(), id);
              int port = -1;

              MessagePortResult native_result = manager.RegisterLocalPort(
                  port_name, std::move(events), is_trusted, &port);
              if (native_result) {
                local_dart_ports_to_native_ports_[id] = port;
              }
              return nullptr;
            },
            [this, id](const flutter::EncodableValue *arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnCancel: %d", id);
              if (local_dart_ports_to_native_ports_.find(id) ==
                  local_dart_ports_to_native_ports_.end()) {
                LOG_ERROR("Error OnCancel: %s",
                          "Could not find port to unregister");
                return nullptr;
              }

              MessagePortResult native_result = manager.UnregisterLocalPort(
                  local_dart_ports_to_native_ports_[id]);
              if (native_result) {
                local_dart_ports_to_native_ports_.erase(id);
                return nullptr;
              }
              LOG_ERROR("Error OnCancel: %s", native_result.message().c_str());
              return nullptr;
            });

    event_channel->SetStreamHandler(std::move(event_channel_handler));
    event_channels_.insert(std::move(event_channel));
    LOG_DEBUG("Successfully registered stream for %slocal port, id %d",
              is_trusted ? "trusted " : "", id);
    result->Success(flutter::EncodableValue(id));
  }

  void Send(
      const flutter::EncodableValue *args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    flutter::EncodableValue message(nullptr);
    std::string remote_app_id = "";
    std::string port_name = "";
    bool is_trusted = false;
    if (!GetEncodableValueFromArgs(args, "message", message) ||
        !GetValueFromArgs<std::string>(args, "remoteAppId", remote_app_id) ||
        !GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<bool>(args, "isTrusted", is_trusted)) {
      result->Error("Could not send message", "Invalid parameter");
      return;
    }

    int id = -1;
    MessagePortResult native_result;
    if (GetValueFromArgs<int>(args, "localPort", id)) {
      LOG_DEBUG("localPort: %d", id);
      if (local_dart_ports_to_native_ports_.find(id) ==
          local_dart_ports_to_native_ports_.end()) {
        result->Error("Could not send message", "Local port not registered");
        return;
      }

      native_result = manager.Send(remote_app_id, port_name, message, is_trusted,
                                   local_dart_ports_to_native_ports_[id]);
    } else {
      native_result = manager.Send(remote_app_id, port_name, message, is_trusted);
    }

    if (native_result) {
      result->Success();
    } else {
      result->Error("Could not send message", native_result.message());
    }
  }

  int nextId_ = 0;
  std::map<int, int> local_dart_ports_to_native_ports_;
  std::set<std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>>
      event_channels_;
  MessagePortManager manager;
  flutter::PluginRegistrar *pluginRegistrar_;
};

}  // namespace

void MessageportTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  MessageportTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
