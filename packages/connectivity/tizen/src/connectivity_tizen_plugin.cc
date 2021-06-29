// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "connectivity_tizen_plugin.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <net_connection.h>
#include <wifi-manager.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "log.h"

class ConnectivityTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<ConnectivityTizenPlugin>();
    plugin->SetupChannels(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  ConnectivityTizenPlugin() : connection_(nullptr), events_(nullptr) {
    EnsureConnectionHandle();
  }

  virtual ~ConnectivityTizenPlugin() {
    if (connection_ != nullptr) {
      connection_destroy(connection_);
      connection_ = nullptr;
    }
  }

  void RegisterObserver(
      std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> &&events) {
    EnsureConnectionHandle();
    if (connection_set_type_changed_cb(connection_, ConnectionTypeChangedCB,
                                       this) != CONNECTION_ERROR_NONE) {
      return;
    }
    events_ = std::move(events);
  }

  void ClearObserver() {
    if (connection_ == nullptr || events_ == nullptr) return;

    connection_unset_type_changed_cb(connection_);
    events_ = nullptr;
  }

  void SendConnectivityChangedEvent(connection_type_e state) {
    if (events_ == nullptr) return;
    std::string replay = ConvertConnectionTypeToString(state);
    flutter::EncodableValue msg(replay);
    events_->Success(msg);
  }

 private:
  static void ConnectionTypeChangedCB(connection_type_e state, void *data) {
    LOG_DEBUG("connectionTypeChangedCB");
    ConnectivityTizenPlugin *plugin_pointer = (ConnectivityTizenPlugin *)data;
    plugin_pointer->SendConnectivityChangedEvent(state);
  }

  void EnsureConnectionHandle() {
    if (connection_ == nullptr) {
      if (connection_create(&connection_) != CONNECTION_ERROR_NONE) {
        connection_ = nullptr;
      }
    }
  }

  std::string ConvertConnectionTypeToString(connection_type_e net_state) {
    std::string result;
    switch (net_state) {
      case CONNECTION_TYPE_WIFI:
      case CONNECTION_TYPE_ETHERNET:
        result = "wifi";
        break;
      case CONNECTION_TYPE_CELLULAR:
        result = "mobile";
        break;
      case CONNECTION_TYPE_DISCONNECTED:
      default:
        result = "none";
    }
    return result;
  }

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    EnsureConnectionHandle();
    LOG_INFO("method : %s", method_call.method_name().data());
    std::string replay = "";
    if (method_call.method_name().compare("check") == 0) {
      connection_type_e net_state;
      if (connection_get_type(connection_, &net_state) !=
          CONNECTION_ERROR_NONE) {
        result->Error("-1", "Couldn't know current connection type");
        return;
      }
      replay = ConvertConnectionTypeToString(net_state);
    } else {
      result->Error("-1", "Not supported method");
      return;
    }
    if (replay.length() == 0) {
      result->Error("-1", "Not valid result");
      return;
    }

    flutter::EncodableValue msg(replay);
    result->Success(msg);
  }

  void SetupChannels(flutter::PluginRegistrar *registrar) {
    auto method_channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "plugins.flutter.io/connectivity",
            &flutter::StandardMethodCodec::GetInstance());
    event_channel_ =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            registrar->messenger(), "plugins.flutter.io/connectivity_status",
            &flutter::StandardMethodCodec::GetInstance());
    method_channel->SetMethodCallHandler([this](const auto &call, auto result) {
      HandleMethodCall(call, std::move(result));
    });

    auto event_channel_handler =
        std::make_unique<flutter::StreamHandlerFunctions<>>(
            [this](const flutter::EncodableValue *arguments,
                   std::unique_ptr<flutter::EventSink<>> &&events)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_INFO("OnListen");
              RegisterObserver(std::move(events));
              return nullptr;
            },
            [this](const flutter::EncodableValue *arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_INFO("OnCancel");
              ClearObserver();
              return nullptr;
            });
    event_channel_->SetStreamHandler(std::move(event_channel_handler));
  }

  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>
      event_channel_;
  connection_h connection_;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>> events_;
};

void ConnectivityTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  ConnectivityTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
