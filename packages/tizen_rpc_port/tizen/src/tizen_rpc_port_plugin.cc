// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

#include "tizen_rpc_port_plugin.h"

#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
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

class RpcStreamHandlerError : public FlStreamHandlerError {
 public:
  RpcStreamHandlerError(const std::string& error_code,
                        const std::string& error_message,
                        const flutter::EncodableValue* error_details = nullptr)
      : error_code_(error_code),
        error_message_(error_message),
        FlStreamHandlerError(error_code_, error_message_, error_details) {}

 private:
  std::string error_code_;
  std::string error_message_;
};

class RpcProxyStreamHandler : public FlStreamHandler {
 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue* arguments,
      std::unique_ptr<FlEventSink>&& events) override {
    RpcPortProxyManager::Init(std::move(events));

    const auto* args = std::get_if<flutter::EncodableMap>(arguments);
    if (!args) {
      return std::make_unique<RpcStreamHandlerError>(
          "Invalid arguments", "The argument must be a map.");
    }
    rpc_port_proxy_h handle = nullptr;
    std::string appid, port_name;
    if (!GetValueFromEncodableMap(args, "handle",
                                  reinterpret_cast<int64_t&>(handle)) ||
        !GetValueFromEncodableMap(args, "appid", appid) ||
        !GetValueFromEncodableMap(args, "portName", port_name)) {
      return std::make_unique<RpcStreamHandlerError>(
          "Invalid arguments", "No handle, appid, or portName provided.");
    }

    auto ret = RpcPortProxyManager::Connect(handle, appid, port_name);
    if (!ret) {
      return std::make_unique<RpcStreamHandlerError>(
          std::to_string(ret.error_code), ret.message());
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
      return std::make_unique<RpcStreamHandlerError>(
          "Invalid arguments", "The argument must be a map.");
    }
    rpc_port_stub_h handle = nullptr;
    if (!GetValueFromEncodableMap(args, "handle",
                                  reinterpret_cast<int64_t&>(handle))) {
      return std::make_unique<RpcStreamHandlerError>("Invalid arguments",
                                                     "No handle provided.");
    }

    auto ret = RpcPortStubManager::Listen(handle);
    if (!ret) {
      return std::make_unique<RpcStreamHandlerError>(
          std::to_string(ret.error_code), ret.message());
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
    auto plugin = std::make_unique<TizenRpcPortPlugin>();

    plugin->proxy_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "tizen/rpc_port_proxy",
        &flutter::StandardMethodCodec::GetInstance());
    plugin->proxy_channel_->SetStreamHandler(
        std::make_unique<RpcProxyStreamHandler>());

    plugin->stub_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "tizen/rpc_port_stub",
        &flutter::StandardMethodCodec::GetInstance());
    plugin->stub_channel_->SetStreamHandler(
        std::make_unique<RpcStubStreamHandler>());

    registrar->AddPlugin(std::move(plugin));
  }

  TizenRpcPortPlugin() {}

  virtual ~TizenRpcPortPlugin() {}

 private:
  std::unique_ptr<FlEventChannel> proxy_channel_;
  std::unique_ptr<FlEventChannel> stub_channel_;
};

}  // namespace

void TizenRpcPortPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  TizenRpcPortPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
