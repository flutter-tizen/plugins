#include "rpc_port_plugin.h"

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <system_info.h>

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <flutter/standard_message_codec.h>

#include <functional>
#include <memory>
#include <string>
#include <sstream>
#include <unordered_map>

#include "log.h"
#include "rpc_port_stub.hh"
#include "rpc_port_proxy.hh"

namespace {
using namespace tizen;

class RpcPortPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "tizen/rpc_port",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<RpcPortPlugin>(registrar);

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  explicit RpcPortPlugin(flutter::PluginRegistrar* plugin_registrar)
      : plugin_registrar_(plugin_registrar) {
    method_handlers_ = {
        { "create",
          std::bind(&RpcPortPlugin::StubCreate, this,
              std::placeholders::_1, std::placeholders::_2) },
        { "destroy",
          std::bind(&RpcPortPlugin::StubDestroy, this,
              std::placeholders::_1, std::placeholders::_2) },
        { "setTrusted",
          std::bind(&RpcPortPlugin::SetTrusted, this,
              std::placeholders::_1, std::placeholders::_2) },
        { "addPrivilege",
          std::bind(&RpcPortPlugin::AddPrivilege, this,
              std::placeholders::_1, std::placeholders::_2) },
        { "listen",
          std::bind(&RpcPortPlugin::Listen, this,
              std::placeholders::_1, std::placeholders::_2) },
        { "stubSend",
          std::bind(&RpcPortPlugin::StubSend, this,
              std::placeholders::_1, std::placeholders::_2) },
        { "stubReceive",
          std::bind(&RpcPortPlugin::StubReceive, this,
              std::placeholders::_1, std::placeholders::_2) },
        { "connect",
          std::bind(&RpcPortPlugin::Connect, this,
              std::placeholders::_1, std::placeholders::_2)},
        { "connectSync",
          std::bind(&RpcPortPlugin::ConnectSync, this,
              std::placeholders::_1, std::placeholders::_2)},
        { "proxySend",
          std::bind(&RpcPortPlugin::ProxySend, this,
              std::placeholders::_1, std::placeholders::_2)},
        { "proxyReceive",
          std::bind(&RpcPortPlugin::ProxyReceive, this,
              std::placeholders::_1, std::placeholders::_2)},
        { "setPrivateSharingArray",
          std::bind(&RpcPortPlugin::SetPrivateSharingArray, this,
              std::placeholders::_1, std::placeholders::_2)},
        { "setPrivateSharing",
          std::bind(&RpcPortPlugin::SetPrivateSharing, this,
              std::placeholders::_1, std::placeholders::_2)},
        { "unsetPrivateSharing",
          std::bind(&RpcPortPlugin::UnsetPrivateSharing, this,
              std::placeholders::_1, std::placeholders::_2)}};
  }

  virtual ~RpcPortPlugin() {}

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
      const char *key, flutter::EncodableValue &out) {
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
    const auto &method_name = method_call.method_name();
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

  void StubCreate(const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("StubCreate");
    std::string port_name;
    if (!GetValueFromArgs<std::string>(args, "portName", port_name)) {
      result->Error("Invalid parameter");
      return;
    }

    auto found = native_stubs_.find(port_name);
    if (found != native_stubs_.end()) {
      result->Error("Already exists");
      return;
    }

    native_stubs_[port_name] = std::make_shared<tizen::RpcPortStub>(port_name);
    result->Success();
  }

  void StubDestroy(const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("Destroy");
    std::string port_name;
    if (!GetValueFromArgs<std::string>(args, "portName", port_name)) {
      result->Error("Invalid parameter");
      return;
    }

    {
      auto found = event_channels_.find(port_name);
      if (found != event_channels_.end())
        event_channels_.erase(found);
    }

    auto found = native_stubs_.find(port_name);
    if (found == native_stubs_.end()) {
      result->Error("Invalid parameter");
      return;
    }

    native_stubs_.erase(found);
    result->Success();
  }

  void SetTrusted(const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("SetTrusted");
    std::string port_name;
    bool trusted;
    if (!GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<bool>(args, "trusted", trusted)) {
      result->Error("Invalid parameter");
      return;
    }

    auto* stub = FindStub(port_name);
    if (stub == nullptr) {
      result->Error("No such stub. portName: " + port_name);
      return;
    }

    auto ret = stub->SetTrusted(trusted);
    if (!ret) {
      result->Error("SetTrusted() is failed");
      return;
    }

    result->Success();
  }

  void AddPrivilege(const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("AddPrivilege");
    std::string port_name;
    std::string privilege;
    if (!GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<std::string>(args, "privilege", privilege)) {
      result->Error("Invalid parameter");
      return;
    }

    auto* stub = FindStub(port_name);
    if (stub == nullptr) {
      result->Error("No sush stub. portName: " + port_name);
      return;
    }

    auto ret = stub->AddPrivilege(privilege);
    if (!ret) {
      result->Error("AddPrivilege() is failed");
      return;
    }

    result->Success();
  }

  void Connect(
      const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("Connect");
    std::string appid;
    std::string port_name;
    if (!GetValueFromArgs<std::string>(args, "appid", appid) ||
        !GetValueFromArgs<std::string>(args, "portName", port_name)) {
      result->Error("Invalid parameter");
      return;
    }

    auto event_channel_name = "tizen/rpc_port_proxy/" + appid + "/" + port_name;
    if (event_channels_.find(event_channel_name) != event_channels_.end()) {
      result->Error("Already connecting");
      return;
    }

    auto event_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            plugin_registrar_->messenger(), event_channel_name,
            &flutter::StandardMethodCodec::GetInstance());

    auto event_channel_handler =
        std::make_unique<flutter::StreamHandlerFunctions<>>(
            [this, appid, port_name](const flutter::EncodableValue* arguments,
                               std::unique_ptr<flutter::EventSink<>>&& events)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnListen: %s/%s", appid.c_str(), port_name.c_str());
              auto proxy = FindProxy(appid, port_name);
              if (proxy != nullptr) {
                LOG_ERROR("Already connected");
                return nullptr;
              }
              proxy = CreateProxy(appid, port_name);
              auto ret = proxy->Connect(std::move(events));
              if (!ret) {
                LOG_ERROR("Failed to connect");
                RemoveProxy(appid, port_name);
              }

              return nullptr;
            },
            [appid, port_name](const flutter::EncodableValue* arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnCancel: %s/%s", appid.c_str(), port_name.c_str());
              return nullptr;
            });

    event_channel->SetStreamHandler(std::move(event_channel_handler));
    event_channels_[event_channel_name] = std::move(event_channel);
    LOG_DEBUG("Successfully connect stream for appid(%s) port_name(%s): ",
              appid.c_str(), port_name.c_str());
    result->Success();
  }

  void ConnectSync(
      const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("ConnectSync");
    std::string appid;
    std::string port_name;
    if (!GetValueFromArgs<std::string>(args, "appid", appid) ||
        !GetValueFromArgs<std::string>(args, "portName", port_name)) {
      result->Error("Invalid parameter");
      return;
    }

    auto event_channel_name = "tizen/rpc_port_proxy/" + appid + "/" + port_name;
    if (event_channels_.find(event_channel_name) != event_channels_.end()) {
      result->Error("Already connecting");
      return;
    }

    auto event_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            plugin_registrar_->messenger(), event_channel_name,
            &flutter::StandardMethodCodec::GetInstance());

    auto event_channel_handler =
        std::make_unique<flutter::StreamHandlerFunctions<>>(
            [this, appid, port_name](const flutter::EncodableValue* arguments,
                               std::unique_ptr<flutter::EventSink<>>&& events)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnListen: %s/%s", appid.c_str(), port_name.c_str());
              auto proxy = FindProxy(appid, port_name);
              if (proxy != nullptr) {
                LOG_ERROR("Already connected");
                return nullptr;
              }
              proxy = CreateProxy(appid, port_name);
              auto ret = proxy->ConnectSync(std::move(events));
              if (!ret) {
                LOG_ERROR("Failed to connect");
                RemoveProxy(appid, port_name);
              }

              return nullptr;
            },
            [appid, port_name](const flutter::EncodableValue* arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnCancel: %s/%s", appid.c_str(), port_name.c_str());
              return nullptr;
            });

    event_channel->SetStreamHandler(std::move(event_channel_handler));
    event_channels_[event_channel_name] = std::move(event_channel);
    LOG_DEBUG("Successfully connect stream for appid(%s) port_name(%s): ",
              appid.c_str(), port_name.c_str());
    result->Success();
  }

  void Listen(const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("Listen");
    std::string port_name;
    if (!GetValueFromArgs<std::string>(args, "portName", port_name)) {
      result->Error("Invalid parameter");
      return;
    }

    if (event_channels_.find(port_name) != event_channels_.end()) {
      result->Error("Already listening");
      return;
    }

    std::string event_channel_name = "tizen/rpc_port_stub/"  + port_name;
    auto event_channel =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
            plugin_registrar_->messenger(), event_channel_name.c_str(),
            &flutter::StandardMethodCodec::GetInstance());

    auto event_channel_handler =
        std::make_unique<flutter::StreamHandlerFunctions<>>(
            [this, port_name](const flutter::EncodableValue *arguments,
                              std::unique_ptr<flutter::EventSink<>> &&events)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnListen. port_name: %s", port_name.c_str());
              auto* stub = FindStub(port_name);
              if (stub == nullptr)
                return nullptr;

              auto ret = stub->Listen(std::move(events));
              if (!ret) {
                LOG_ERROR("Listen() is failed. error: %s",
                    ret.message());
              }

              return nullptr;
            },
            [port_name](const flutter::EncodableValue *arguments)
                -> std::unique_ptr<flutter::StreamHandlerError<>> {
              LOG_DEBUG("OnCancel. port_name: %s", port_name.c_str());
              return nullptr;
            });

    event_channel->SetStreamHandler(std::move(event_channel_handler));
    event_channels_[std::move(port_name)] = std::move(event_channel);

    result->Success();
  }

  void ProxySend(const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("ProxySend");
    std::string appid;
    std::string port_name;
    int32_t port_type;
    std::vector<uint8_t> raw_data;
    if (!GetValueFromArgs<std::string>(args, "appid", appid) ||
        !GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<int32_t>(args, "portType", port_type) ||
        !GetValueFromArgs<std::vector<uint8_t>>(args, "rawData", raw_data)) {
      result->Error("Invalid parameter");
      return;
    }

    auto* proxy = FindProxy(appid, port_name);
    if (proxy == nullptr) {
      result->Error("Invalid parameter");
      return;
    }

    LOG_DEBUG("appid: %s, portName: %s, portType: %d", appid.c_str(),
              port_name.c_str(), port_type);
    RpcPort* port = nullptr;
    auto ret = proxy->GetPort(port_type, &port);
    if (!ret) {
      LOG_ERROR("Failed to get port");
      result->Error("GetPort() is failed", ret.message());
      return;
    }
    auto port_auto = std::unique_ptr<RpcPort>(port);

    RpcPortParcel parcel(raw_data);
    ret = parcel.Send(port);
    if (!ret) {
      LOG_ERROR("Failed to send parcel");
      result->Error("Send() is failed", ret.message());
      return;
    }

    result->Success();
  }

  void StubSend(const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("Send");
    std::string port_name;
    std::string instance;
    int32_t port_type;
    std::vector<uint8_t> raw_data;
    if (!GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<std::string>(args, "instance", instance) ||
        !GetValueFromArgs<int32_t>(args, "portType", port_type) ||
        !GetValueFromArgs<std::vector<uint8_t>>(args, "rawData", raw_data)) {
      result->Error("Invalid parameter");
      return;
    }

    auto* stub = FindStub(port_name);
    if (stub == nullptr) {
      result->Error("Invalid parameter");
      return;
    }

    LOG_DEBUG("portName: %s, instance: %s, portType: %d", port_name.c_str(), instance.c_str(), port_type);
    RpcPort* port = nullptr;
    auto ret = stub->GetPort(port_type, instance, &port);
    if (!ret) {
      LOG_ERROR("Failed to get port");
      result->Error("GetPort() is failed", ret.message());
      return;
    }
    auto port_auto = std::unique_ptr<RpcPort>(port);

    RpcPortParcel parcel(raw_data);
    ret = parcel.Send(port);
    if (!ret) {
      LOG_ERROR("Failed to send parcel");
      result->Error("Send() is failed", ret.message());
      return;
    }

    result->Success();
  }

  void ProxyReceive(const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("ProxyReceive");
    std::string appid;
    std::string port_name;
    int32_t port_type;
    if (!GetValueFromArgs<std::string>(args, "appid", appid) ||
        !GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<int32_t>(args, "portType", port_type)) {
      result->Error("Invalid parameter");
      return;
    }

    auto* proxy = FindProxy(appid, port_name);
    if (proxy == nullptr) {
      result->Error("Invalid parameter");
      return;
    }

    LOG_DEBUG("appid: %s, portName: %s, portType: %d", appid.c_str(),
              port_name.c_str(), port_type);
    RpcPort* port = nullptr;
    auto ret = proxy->GetPort(port_type, &port);
    if (!ret) {
      LOG_ERROR("Failed to get port");
      result->Error("GetPort() is failed", ret.message());
      return;
    }

    auto port_auto = std::unique_ptr<RpcPort>(port);

    RpcPortParcel parcel(port);
    std::vector<uint8_t> raw;
    ret = parcel.GetRaw(&raw);
    if (!ret) {
      LOG_ERROR("Failed to get raw from parcel");
      result->Error("ProxyReceive() is failed", ret.message());
      return;
    }

    flutter::EncodableMap map {
      { flutter::EncodableValue("rawData"), flutter::EncodableValue(std::move(raw)) }
    };

    result->Success(std::move(flutter::EncodableValue(map)));
  }
  void StubReceive(const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("Receive");
    std::string port_name;
    std::string instance;
    int32_t port_type;
    if (!GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<std::string>(args, "instance", instance) ||
        !GetValueFromArgs<int32_t>(args, "portType", port_type)) {
      result->Error("Invalid parameter");
      return;
    }

    auto* stub = FindStub(port_name);
    if (stub == nullptr) {
      result->Error("Invalid parameter");
      return;
    }

    LOG_DEBUG("portName: %s, instance: %s, portType: %d", port_name.c_str(), instance.c_str(), port_type);
    RpcPort* port = nullptr;
    auto ret = stub->GetPort(port_type, instance, &port);
    if (!ret) {
      LOG_ERROR("Failed to get port");
      result->Error("GetPort() is failed", ret.message());
      return;
    }
    auto port_auto = std::unique_ptr<RpcPort>(port);

    RpcPortParcel parcel(port);
    std::vector<uint8_t> raw;
    ret = parcel.GetRaw(&raw);
    if (!ret) {
      LOG_ERROR("Failed to get raw from parcel");
      result->Error("Receive() is failed", ret.message());
      return;
    }

    flutter::EncodableMap map {
      { flutter::EncodableValue("rawData"), flutter::EncodableValue(std::move(raw)) }
    };

    result->Success(std::move(flutter::EncodableValue(map)));
  }

  void SetPrivateSharing(const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("SetPrivateSharing");
    std::string port_name;
    std::string instance;
    int32_t port_type;
    std::string path;
    if (!GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<std::string>(args, "instance", instance) ||
        !GetValueFromArgs<int32_t>(args, "portType", port_type) ||
        !GetValueFromArgs<std::string>(args, "path", path)) {
      result->Error("Invalid parameter");
      return;
    }

    auto* stub = FindStub(port_name);
    if (stub == nullptr) {
      result->Error("Invalid parameter");
      return;
    }

    RpcPort* port = nullptr;
    auto ret = stub->GetPort(port_type, instance, &port);
    if (!ret) {
      result->Error("GetPort() is failed", ret.message());
      return;
    }

    auto port_auto = std::unique_ptr<RpcPort>(port);
    ret = port->SetPrivateSharing(path);
    if (!ret) {
      result->Error("SetPrivateSharing() is failed", ret.message());
      return;
    }

    result->Success();
  }

  void SetPrivateSharingArray(const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("SetPrivateSharingArray");
    std::string port_name;
    std::string instance;
    int32_t port_type;
    flutter::EncodableList list;
    if (!GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<std::string>(args, "instance", instance) ||
        !GetValueFromArgs<int32_t>(args, "portType", port_type) ||
        !GetValueFromArgs<flutter::EncodableList>(args, "paths", list)) {
      result->Error("Invalid parameter");
      return;
    }

    auto* stub = FindStub(port_name);
    if (stub == nullptr) {
      result->Error("Invalid parameter");
      return;
    }

    RpcPort* port = nullptr;
    auto ret = stub->GetPort(port_type, instance, &port);
    if (!ret) {
      result->Error("GetPort() is failed", ret.message());
      return;
    }

    std::vector<std::string> paths;
    for (auto const& value : list) {
      if (std::holds_alternative<std::string>(value)) {
        auto path = std::get<std::string>(value);
        paths.push_back(std::move(path));
      }
    }

    auto port_auto = std::unique_ptr<RpcPort>(port);
    ret = port->SetPrivateSharing(paths);
    if (!ret) {
      result->Error("SetPrivateSharing() is failed", ret.message());
      return;
    }

    result->Success();
  }

  void UnsetPrivateSharing(const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("UnsetPrivateSharing");
    std::string port_name;
    std::string instance;
    int32_t port_type;
    if (!GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<std::string>(args, "instance", instance) ||
        !GetValueFromArgs<int32_t>(args, "portType", port_type)) {
      result->Error("Invalid parameter");
      return;
    }

    auto* stub = FindStub(port_name);
    if (stub == nullptr) {
      result->Error("Invalid parameter");
      return;
    }

    RpcPort* port = nullptr;
    auto ret = stub->GetPort(port_type, instance, &port);
    if (!ret) {
      result->Error("GetPort() is failed", ret.message());
      return;
    }

    ret = port->UnsetPrivateSharing();
    if (!ret) {
      result->Error("UnsetPrivateSharing() is failed", ret.message());
      return;
    }

    result->Success();
  }

  void Disconnect(const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("Disconnect");
    std::string port_name;
    std::string instance;
    int32_t port_type;
    if (!GetValueFromArgs<std::string>(args, "portName", port_name) ||
        !GetValueFromArgs<std::string>(args, "instance", instance) ||
        !GetValueFromArgs<int32_t>(args, "portType", port_type)) {
      result->Error("Invalid parameter");
      return;
    }

    auto* stub = FindStub(port_name);
    if (stub == nullptr) {
      result->Error("Invalid parameter");
      return;
    }

    RpcPort* port = nullptr;
    auto ret = stub->GetPort(port_type, instance, &port);
    if (!ret) {
      result->Error("GetPort() is failed", ret.message());
      return;
    }

    ret = port->Disconnect();
    if (!ret) {
      result->Error("Disconnect() is failed", ret.message());
      return;
    }

    result->Success();
  }

 private:
  static std::string CreateKey(const std::string appid,
      const std::string port_name) {
    return appid + "/" + port_name;
  }

  tizen::RpcPortStub* FindStub(const std::string& port_name) {
    auto found = native_stubs_.find(port_name);
    if (found == native_stubs_.end())
      return nullptr;

    return (found->second).get();
  }

  RpcPortProxy* CreateProxy(const std::string& appid,
      const std::string& port_name) {
    auto proxy = std::make_shared<RpcPortProxy>(appid, port_name);
    auto key = CreateKey(appid, port_name);
    native_proxies_[key] = proxy;
    return proxy.get();
  }

  void RemoveProxy(const std::string& appid, const std::string& port_name) {
    auto key = CreateKey(appid, port_name);
    auto found = native_proxies_.find(key);
    if (found == native_proxies_.end())
      native_proxies_.erase(found);
  }

  tizen::RpcPortProxy* FindProxy(const std::string& appid,
      const std::string& port_name) {
    auto key = CreateKey(appid, port_name);
    auto found = native_proxies_.find(key);
    if (found == native_proxies_.end())
      return nullptr;

    return (found->second).get();
  }

 private:
  std::unordered_map<std::string, std::shared_ptr<tizen::RpcPortStub>> native_stubs_;
  std::unordered_map<std::string, std::shared_ptr<tizen::RpcPortProxy>> native_proxies_;
  std::unordered_map<std::string, std::function<void(const flutter::EncodableValue*, std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>)>> method_handlers_;
  std::unordered_map<std::string, std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>> event_channels_;
  flutter::PluginRegistrar* plugin_registrar_;
};

}  // namespace

void RpcPortPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  RpcPortPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
