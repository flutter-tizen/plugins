// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

#include "tizen_rpc_port_plugin.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_message_codec.h>
#include <flutter/standard_method_codec.h>
#include <system_info.h>

#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

#include "log.h"
#include "rpc_port_proxy.h"
#include "rpc_port_stub.h"

namespace {
using namespace tizen;

class TizenRpcPortPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "tizen/rpc_port",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<TizenRpcPortPlugin>(registrar);

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto& call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  explicit TizenRpcPortPlugin(flutter::PluginRegistrar* plugin_registrar)
      : plugin_registrar_(plugin_registrar) {
    method_handlers_ = {
        {"stubListen", std::bind(&TizenRpcPortPlugin::StubListen, this,
                                 std::placeholders::_1, std::placeholders::_2)},
        {"proxyConnect",
         std::bind(&TizenRpcPortPlugin::ProxyConnect, this,
                   std::placeholders::_1, std::placeholders::_2)}};
  }

  virtual ~TizenRpcPortPlugin() {}

 private:
  template <typename T>
  bool GetValueFromArgs(const flutter::EncodableValue* args, const char* key,
                        T& out) {
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

  bool GetEncodableValueFromArgs(const flutter::EncodableValue* args,
                                 const char* key,
                                 flutter::EncodableValue& out) {
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
      const flutter::MethodCall<flutter::EncodableValue>& method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const auto& method_name = method_call.method_name();
    auto found = method_handlers_.find(method_name);
    if (found == method_handlers_.end()) {
      LOG_ERROR("%s it not implmeneted", method_name.c_str());
      result->NotImplemented();
      return;
    }

    LOG_DEBUG("HandleMethodCall. method_name: %s", method_name.c_str());
    auto method = found->second;
    const flutter::EncodableValue* args = method_call.arguments();
    method(args, std::move(result));
  }

  void ProxyConnect(
      const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("Connect");
    rpc_port_proxy_h handle = nullptr;
    std::string appid;
    std::string port_name;

    if (!GetValueFromArgs<int64_t>(args, "handle",
                                   reinterpret_cast<int64_t&>(handle)) ||
        !GetValueFromArgs<std::string>(args, "appid", appid) ||
        !GetValueFromArgs<std::string>(args, "portName", port_name)) {
      result->Error("Invalid parameter");
      return;
    }

    if (proxy_channel_ == nullptr) {
      proxy_channel_ =
          std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
              plugin_registrar_->messenger(), "tizen/rpc_port_proxy",
              &flutter::StandardMethodCodec::GetInstance());

      auto event_channel_handler =
          std::make_unique<flutter::StreamHandlerFunctions<>>(
              [handle, appid, port_name](
                  const flutter::EncodableValue* arguments,
                  std::unique_ptr<flutter::EventSink<>>&& events)
                  -> std::unique_ptr<flutter::StreamHandlerError<>> {
                LOG_DEBUG("OnListen event channel");
                RpcPortProxyManager::GetInst().Init(std::move(events));
                auto ret = RpcPortProxyManager::GetInst().Connect(handle, appid,
                                                                  port_name);
                if (!ret) {
                  LOG_ERROR("connect failed.");
                }

                return nullptr;
              },
              [this](const flutter::EncodableValue* arguments)
                  -> std::unique_ptr<flutter::StreamHandlerError<>> {
                LOG_DEBUG("OnCancel event channel");
                proxy_channel_ = nullptr;
                return nullptr;
              });

      proxy_channel_->SetStreamHandler(std::move(event_channel_handler));
    } else {
      auto ret =
          RpcPortProxyManager::GetInst().Connect(handle, appid, port_name);
      if (!ret) {
        result->Error("connect failed.");
        return;
      }
    }

    LOG_DEBUG("Successfully connect stream for appid(%s) port_name(%s): ",
              appid.c_str(), port_name.c_str());
    result->Success();
  }

  void StubListen(
      const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("StubListen");
    rpc_port_stub_h handle = nullptr;

    if (!GetValueFromArgs<int64_t>(args, "handle",
                                   reinterpret_cast<int64_t&>(handle))) {
      result->Error("Invalid parameter");
      return;
    }
    LOG_ERROR("ptr: %p", handle);
    if (stub_channel_ == nullptr) {
      stub_channel_ =
          std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
              plugin_registrar_->messenger(), "tizen/rpc_port_stub",
              &flutter::StandardMethodCodec::GetInstance());

      auto event_channel_handler =
          std::make_unique<flutter::StreamHandlerFunctions<>>(
              [handle](const flutter::EncodableValue* arguments,
                       std::unique_ptr<flutter::EventSink<>>&& events)
                  -> std::unique_ptr<flutter::StreamHandlerError<>> {
                LOG_DEBUG("OnListen stub event channel:");
                RpcPortStubManager::GetInst().Init(std::move(events));

                auto ret = RpcPortStubManager::GetInst().Listen(handle);
                if (!ret) {
                  LOG_ERROR("connect failed.");
                }

                return nullptr;
              },
              [](const flutter::EncodableValue* arguments)
                  -> std::unique_ptr<flutter::StreamHandlerError<>> {
                LOG_DEBUG("OnCancel stub event channel");
                return nullptr;
              });

      stub_channel_->SetStreamHandler(std::move(event_channel_handler));
    } else {
      auto ret = RpcPortStubManager::GetInst().Listen(handle);
      if (!ret) {
        result->Error("listen failed");
        return;
      }
    }

    result->Success();
  }

 private:
  std::unordered_map<
      std::string,
      std::function<void(
          const flutter::EncodableValue*,
          std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>)>>
      method_handlers_;
  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>
      proxy_channel_;
  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>> stub_channel_;
  flutter::PluginRegistrar* plugin_registrar_;
};

}  // namespace

void TizenRpcPortPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  TizenRpcPortPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
