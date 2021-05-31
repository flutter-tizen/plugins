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
      LOG_DEBUG("Key %s not found", key);
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
    } else if (method_call.method_name().compare("checkForRemote") == 0) {
      CheckForRemote(args, std::move(result));
    } else if (method_call.method_name().compare("send") == 0) {
      Send(args, std::move(result));
    } else {
      result->Error("Invalid method");
    }
  }

  void CheckForRemote(
      const flutter::EncodableValue *args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("CheckForRemote");
    std::string remote_app_id = "";
    std::string port_name = "";
    bool trusted = false;
    if (!GetValueFromArgs<std::string>(args, "remoteAppId", remote_app_id) ||
        !GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<bool>(args, "trusted", trusted)) {
      result->Error("Invalid parameter");
      return;
    }

    bool port_check = false;
    MessagePortResult native_result = manager_.CheckRemotePort(
        remote_app_id, port_name, trusted, &port_check);
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
    bool trusted = false;
    if (!GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<bool>(args, "trusted", trusted)) {
      result->Error("Could not create local port", "Invalid parameter");
      return;
    }

    auto key = std::make_pair(port_name, trusted);
    if (native_ports_.find(key) != native_ports_.end()) {
      LOG_DEBUG("Stream handler for %s, already registered", port_name.c_str());
      result->Success();
      return;
    }

    std::stringstream event_channel_name;
    if (trusted) {
      event_channel_name << "tizen/messageport/" << port_name << "_trusted";
    } else {
      event_channel_name << "tizen/messageport/" << port_name;
    }

    auto event_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            plugin_registrar_->messenger(), event_channel_name.str(),
            &flutter::StandardMethodCodec::GetInstance());

    auto event_channel_handler =
        std::make_unique<flutter::StreamHandlerFunctions<>>(
            [this, key](const flutter::EncodableValue *arguments,
                        std::unique_ptr<flutter::EventSink<>> &&events)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnListen: %s", key.first.c_str());
              int port = -1;

              MessagePortResult native_result = manager_.RegisterLocalPort(
                  key.first, std::move(events), key.second, &port);
              if (native_result) {
                native_ports_[key] = port;
              }
              return nullptr;
            },
            [this, key](const flutter::EncodableValue *arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnCancel: %s", key.first.c_str());
              if (native_ports_.find(key) == native_ports_.end()) {
                LOG_ERROR("Error OnCancel: %s",
                          "Could not find port to unregister");
                return nullptr;
              }

              MessagePortResult native_result =
                  manager_.UnregisterLocalPort(native_ports_[key]);
              if (native_result) {
                native_ports_.erase(key);
                return nullptr;
              }
              LOG_ERROR("Error OnCancel: %s", native_result.message().c_str());
              return nullptr;
            });

    event_channel->SetStreamHandler(std::move(event_channel_handler));
    event_channels_.insert(std::move(event_channel));
    LOG_DEBUG(
        "Successfully registered stream for local port, port_name: %s, "
        "trusted: %s",
        port_name.c_str(), trusted ? "yes " : "no");
    result->Success();
  }

  void Send(
      const flutter::EncodableValue *args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
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
    MessagePortResult native_result;
    if (GetValueFromArgs<std::string>(args, "localPort", local_port_name) &&
        GetValueFromArgs<bool>(args, "localPortTrusted", local_port_trusted)) {
      LOG_DEBUG("localPort: %s, trusted: %s", local_port_name.c_str(),
                local_port_trusted ? "yes" : "no");
      auto key = std::make_pair(local_port_name, local_port_trusted);
      if (native_ports_.find(key) == native_ports_.end()) {
        result->Error("Could not send message",
                      "Local port is not registered.");
        return;
      }

      native_result = manager_.Send(remote_app_id, port_name, message, trusted,
                                    native_ports_[key]);
    } else {
      native_result = manager_.Send(remote_app_id, port_name, message, trusted);
    }

    if (native_result) {
      result->Success();
    } else {
      result->Error("Could not send message", native_result.message());
    }
  }

  // < channel_name, is_trusted > -> native number >
  std::map<std::pair<std::string, bool>, int> native_ports_;
  std::set<std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>>
      event_channels_;
  MessagePortManager manager_;
  flutter::PluginRegistrar *plugin_registrar_;
};

}  // namespace

void MessageportTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  MessageportTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
