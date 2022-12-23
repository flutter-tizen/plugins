// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

#include "tizen_rpc_port_plugin.h"

#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <string>
#include <variant>

#include "log.h"
#include "rpc_port_proxy.h"
#include "rpc_port_stub.h"

namespace {

using namespace tizen;

typedef flutter::EventChannel<flutter::EncodableValue> FlEventChannel;
typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;
typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;
typedef flutter::StreamHandler<flutter::EncodableValue> FlStreamHandler;
typedef flutter::StreamHandlerError<flutter::EncodableValue>
    FlStreamHandlerError;

class RpcStreamHandlerError : public FlStreamHandlerError {
 public:
  RpcStreamHandlerError(const std::string& error_code,
                        const std::string& error_message,
                        const flutter::EncodableValue* error_details)
      : error_code_(error_code),
        error_message_(error_message),
        FlStreamHandlerError(error_code_, error_message_, error_details) {}

 private:
  std::string error_code_;
  std::string error_message_;
};

template <typename T>
static bool GetValueFromEncodableMap(const flutter::EncodableMap* map,
                                     const char* key, T& out) {
  auto iter = map->find(flutter::EncodableValue(key));
  if (iter != map->end() && !iter->second.IsNull()) {
    if (auto* value = std::get_if<T>(&iter->second)) {
      out = *value;
      return true;
    }
  }
  return false;
}

class TizenRpcPortPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto plugin = std::make_unique<TizenRpcPortPlugin>(registrar);

    auto channel = std::make_unique<FlMethodChannel>(
        registrar->messenger(), "tizen/rpc_port",
        &flutter::StandardMethodCodec::GetInstance());
    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto& call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  explicit TizenRpcPortPlugin(flutter::PluginRegistrar* plugin_registrar)
      : plugin_registrar_(plugin_registrar) {}

  virtual ~TizenRpcPortPlugin() {}

 private:
  void HandleMethodCall(const FlMethodCall& method_call,
                        std::unique_ptr<FlMethodResult> result) {
    const auto& method_name = method_call.method_name();
    const auto* arguments =
        std::get_if<flutter::EncodableMap>(method_call.arguments());

    if (method_name == "proxyConnect") {
      ProxyConnect(arguments, std::move(result));
    } else if (method_name == "stubListen") {
      StubListen(arguments, std::move(result));
    } else {
      result->NotImplemented();
    }
  }

  void ProxyConnect(const flutter::EncodableMap* arguments,
                    std::unique_ptr<FlMethodResult> result) {
    rpc_port_proxy_h handle = nullptr;
    std::string appid;
    std::string port_name;

    if (!GetValueFromEncodableMap(arguments, "handle",
                                  reinterpret_cast<int64_t&>(handle)) ||
        !GetValueFromEncodableMap(arguments, "appid", appid) ||
        !GetValueFromEncodableMap(arguments, "portName", port_name)) {
      result->Error("Invalid arguments");
      return;
    }

    if (!proxy_channel_) {
      proxy_channel_ = std::make_unique<FlEventChannel>(
          plugin_registrar_->messenger(), "tizen/rpc_port_proxy",
          &flutter::StandardMethodCodec::GetInstance());

      auto event_handler = std::make_unique<flutter::StreamHandlerFunctions<>>(
          [handle, appid, port_name](const flutter::EncodableValue* arguments,
                                     std::unique_ptr<FlEventSink>&& events)
              -> std::unique_ptr<flutter::StreamHandlerError<>> {
            RpcPortProxyManager::Init(std::move(events));

            auto ret = RpcPortProxyManager::Connect(handle, appid, port_name);
            if (!ret) {
              return std::make_unique<RpcStreamHandlerError>(
                  std::to_string(ret.error_code), ret.message(), nullptr);
            }
            return nullptr;
          },
          [this](const flutter::EncodableValue* arguments)
              -> std::unique_ptr<flutter::StreamHandlerError<>> {
            proxy_channel_ = nullptr;
            return nullptr;
          });

      proxy_channel_->SetStreamHandler(std::move(event_handler));
    } else {
      auto ret = RpcPortProxyManager::Connect(handle, appid, port_name);
      if (!ret) {
        result->Error(std::to_string(ret.error_code), ret.message());
        return;
      }
    }

    LOG_DEBUG("Successfully connect stream for appid(%s) port_name(%s): ",
              appid.c_str(), port_name.c_str());
    result->Success();
  }

  void StubListen(const flutter::EncodableMap* arguments,
                  std::unique_ptr<FlMethodResult> result) {
    rpc_port_stub_h handle = nullptr;

    if (!GetValueFromEncodableMap(arguments, "handle",
                                  reinterpret_cast<int64_t&>(handle))) {
      result->Error("Invalid arguments");
      return;
    }

    if (!stub_channel_) {
      stub_channel_ = std::make_unique<FlEventChannel>(
          plugin_registrar_->messenger(), "tizen/rpc_port_stub",
          &flutter::StandardMethodCodec::GetInstance());

      auto event_handler = std::make_unique<flutter::StreamHandlerFunctions<>>(
          [handle](const flutter::EncodableValue* arguments,
                   std::unique_ptr<FlEventSink>&& events)
              -> std::unique_ptr<flutter::StreamHandlerError<>> {
            RpcPortStubManager::Init(std::move(events));

            auto ret = RpcPortStubManager::Listen(handle);
            if (!ret) {
              return std::make_unique<RpcStreamHandlerError>(
                  std::to_string(ret.error_code), ret.message(), nullptr);
            }
            return nullptr;
          },
          [](const flutter::EncodableValue* arguments)
              -> std::unique_ptr<flutter::StreamHandlerError<>> {
            return nullptr;
          });

      stub_channel_->SetStreamHandler(std::move(event_handler));
    } else {
      auto ret = RpcPortStubManager::Listen(handle);
      if (!ret) {
        result->Error(std::to_string(ret.error_code), ret.message());
        return;
      }
    }

    result->Success();
  }

 private:
  std::unique_ptr<FlEventChannel> proxy_channel_;
  std::unique_ptr<FlEventChannel> stub_channel_;
  flutter::PluginRegistrar* plugin_registrar_;
};

}  // namespace

void TizenRpcPortPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  TizenRpcPortPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
