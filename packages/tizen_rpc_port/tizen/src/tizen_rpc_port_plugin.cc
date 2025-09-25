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
#include <sstream>
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
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::StreamHandler<flutter::EncodableValue> FlStreamHandler;
typedef flutter::StreamHandlerError<flutter::EncodableValue>
    FlStreamHandlerError;

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

class RpcProxyStreamHandler : public FlStreamHandler {
 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue* arguments,
      std::unique_ptr<FlEventSink>&& events) override {
    LOG_DEBUG("Called OnListenInternal %p", this);
    const auto* args = std::get_if<flutter::EncodableMap>(arguments);
    if (!args) {
      return std::make_unique<FlStreamHandlerError>(
          "Invalid arguments", "The argument must be a map.", nullptr);
    }
    rpc_port_proxy_h handle = nullptr;
    std::string appid, port_name;
    if (!GetValueFromEncodableMap(args, "handle",
                                  reinterpret_cast<int64_t&>(handle))) {
      if (!GetValueFromEncodableMap(args, "handle",
                                    reinterpret_cast<int32_t&>(handle))) {
        return std::make_unique<FlStreamHandlerError>(
            "Invalid arguments", "No handle provided.", nullptr);
      }
    }

    if (!GetValueFromEncodableMap(args, "appid", appid) ||
        !GetValueFromEncodableMap(args, "portName", port_name)) {
      return std::make_unique<FlStreamHandlerError>(
          "Invalid arguments", "No appid, or portName provided.", nullptr);
    }

    LOG_DEBUG("Called OnListenInternal handle %p", handle);
    RpcPortProxyManager::Init(handle, std::move(events));
    auto ret = RpcPortProxyManager::Connect(handle, appid, port_name);
    if (!ret) {
      return std::make_unique<FlStreamHandlerError>(
          std::to_string(ret.error_code), ret.message(), nullptr);
    }
    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue* arguments) override {
    return nullptr;
  }
};

class RpcStubStreamHandler : public FlStreamHandler {
 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue* arguments,
      std::unique_ptr<FlEventSink>&& events) override {
    RpcPortStubManager::Init(std::move(events));

    const auto* args = std::get_if<flutter::EncodableMap>(arguments);
    if (!args) {
      return std::make_unique<FlStreamHandlerError>(
          "Invalid arguments", "The argument must be a map.", nullptr);
    }
    rpc_port_stub_h handle = nullptr;
    if (!GetValueFromEncodableMap(args, "handle",
                                  reinterpret_cast<int64_t&>(handle))) {
      if (!GetValueFromEncodableMap(args, "handle",
                                    reinterpret_cast<int32_t&>(handle))) {
        return std::make_unique<FlStreamHandlerError>(
            "Invalid arguments", "No handle provided.", nullptr);
      }
    }

    auto ret = RpcPortStubManager::Listen(handle);
    if (!ret) {
      return std::make_unique<FlStreamHandlerError>(
          std::to_string(ret.error_code), ret.message(), nullptr);
    }
    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue* arguments) override {
    return nullptr;
  }
};

class TizenRpcPortPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto plugin = std::make_unique<TizenRpcPortPlugin>(registrar);

    auto method_channel = std::make_unique<FlMethodChannel>(
        registrar->messenger(), "tizen/rpc_port_proxy",
        &flutter::StandardMethodCodec::GetInstance());
    method_channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto& call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });
    plugin->stub_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "tizen/rpc_port_stub",
        &flutter::StandardMethodCodec::GetInstance());
    plugin->stub_channel_->SetStreamHandler(
        std::make_unique<RpcStubStreamHandler>());

    registrar->AddPlugin(std::move(plugin));
  }

  TizenRpcPortPlugin(flutter::PluginRegistrar* plugin_registrar)
      : plugin_registrar_(plugin_registrar) {}

  virtual ~TizenRpcPortPlugin() {}

 private:
  std::set<std::unique_ptr<FlEventChannel>> proxy_channels_;
  std::unique_ptr<FlEventChannel> stub_channel_;
  flutter::PluginRegistrar* plugin_registrar_ = nullptr;

 private:
  void HandleMethodCall(const FlMethodCall& method_call,
                        std::unique_ptr<FlMethodResult> result) {
    const auto& method_name = method_call.method_name();
    const auto& arguments = *method_call.arguments();

    if (method_name == "init") {
      const auto* args = std::get_if<flutter::EncodableMap>(&arguments);
      if (!args) {
        result->Error("Invalid arguments", "The argument must be a map.");
        return;
      }

      rpc_port_proxy_h handle = nullptr;
      if (!GetValueFromEncodableMap(args, "handle",
                                    reinterpret_cast<int64_t&>(handle))) {
        if (!GetValueFromEncodableMap(args, "handle",
                                      reinterpret_cast<int32_t&>(handle))) {
          result->Error("Invalid arguments", "No handle provided.");
          return;
        }
      }

      std::string appid, port_name;
      if (!GetValueFromEncodableMap(args, "portName", port_name)) {
        result->Error("Invalid arguments", "No portName provided.");
        return;
      }

      std::string event_channel_name =
          "tizen/rpc_port_proxy/" + port_name + '/' +
          std::to_string(reinterpret_cast<int64_t>(handle));

      auto proxy_channel = std::make_unique<FlEventChannel>(
          plugin_registrar_->messenger(), event_channel_name,
          &flutter::StandardMethodCodec::GetInstance());
      proxy_channel->SetStreamHandler(
          std::make_unique<RpcProxyStreamHandler>());

      proxy_channels_.insert(std::move(proxy_channel));

      result->Success();
    } else if (method_name == "dispose") {
      result->Success();
    } else {
      result->NotImplemented();
    }
  }
};

}  // namespace

void TizenRpcPortPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  TizenRpcPortPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
