#include "tizen_cion_plugin.h"

#include <cion.h>
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_message_codec.h>
#include <flutter/standard_method_codec.h>
#include <system_info.h>

#include <memory>
#include <string>

#include "log.h"
#include "tizen_cion_client.h"
#include "tizen_cion_group.h"
#include "tizen_cion_server.h"

namespace {
using namespace tizen;

class TizenCionPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar* registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "tizen/cion",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<TizenCionPlugin>(registrar);

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto& call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  explicit TizenCionPlugin(flutter::PluginRegistrar* plugin_registrar)
      : plugin_registrar_(plugin_registrar) {
    method_handlers_ = {
        {"groupSubscribe",
         std::bind(&TizenCionPlugin::Subscribe, this, std::placeholders::_1,
                   std::placeholders::_2)},
        {"serverListen",
         std::bind(&TizenCionPlugin::ServerListen, this, std::placeholders::_1,
                   std::placeholders::_2)},
        {"clientTryDiscovery",
         std::bind(&TizenCionPlugin::TryDiscovery, this, std::placeholders::_1,
                   std::placeholders::_2)},
        {"clientConnect",
         std::bind(&TizenCionPlugin::ClientConnect, this, std::placeholders::_1,
                   std::placeholders::_2)}};
  }

  virtual ~TizenCionPlugin() {}

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
        LOG_DEBUG("type mismatch %s ", key);
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

  void ServerListen(
      const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    cion_server_h handle = nullptr;

    if (!GetValueFromArgs<int64_t>(args, "handle",
                                   reinterpret_cast<int64_t&>(handle))) {
      result->Error("Invalid parameter");
      return;
    }
    LOG_ERROR("ptr: %p", handle);
    if (server_channel_ == nullptr) {
      server_channel_ =
          std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
              plugin_registrar_->messenger(), "tizen/cion_server",
              &flutter::StandardMethodCodec::GetInstance());

      auto event_channel_handler =
          std::make_unique<flutter::StreamHandlerFunctions<>>(
              [handle](const flutter::EncodableValue* arguments,
                       std::unique_ptr<flutter::EventSink<>>&& events)
                  -> std::unique_ptr<flutter::StreamHandlerError<>> {
                LOG_DEBUG("OnListen server event channel:");
                CionServerManager::Init(std::move(events));

                auto ret = CionServerManager::Listen(handle);
                if (!ret) {
                  LOG_ERROR("Listen failed.");
                }

                return nullptr;
              },
              [](const flutter::EncodableValue* arguments)
                  -> std::unique_ptr<flutter::StreamHandlerError<>> {
                LOG_DEBUG("OnCancel server event channel");
                return nullptr;
              });

      server_channel_->SetStreamHandler(std::move(event_channel_handler));
    } else {
      auto ret = CionServerManager::Listen(handle);
      if (!ret) {
        result->Error("listen failed");
        return;
      }
    }

    result->Success();
  }

  void Subscribe(
      const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    cion_group_h handle = nullptr;

    if (!GetValueFromArgs<int64_t>(args, "handle",
                                   reinterpret_cast<int64_t&>(handle))) {
      result->Error("Invalid parameter");
      return;
    }

    LOG_ERROR("ptr: %p", handle);

    if (group_channel_ == nullptr) {
      group_channel_ =
          std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
              plugin_registrar_->messenger(), "tizen/cion_group",
              &flutter::StandardMethodCodec::GetInstance());

      auto event_channel_handler =
          std::make_unique<flutter::StreamHandlerFunctions<>>(
              [handle](const flutter::EncodableValue* arguments,
                       std::unique_ptr<flutter::EventSink<>>&& events)
                  -> std::unique_ptr<flutter::StreamHandlerError<>> {
                LOG_DEBUG("OnListen group event channel:");
                CionGroupManager::Init(std::move(events));

                auto ret = CionGroupManager::Subscribe(handle);
                if (!ret) {
                  LOG_ERROR("Listen failed.");
                }

                return nullptr;
              },
              [](const flutter::EncodableValue* arguments)
                  -> std::unique_ptr<flutter::StreamHandlerError<>> {
                LOG_DEBUG("OnCancel group event channel");
                return nullptr;
              });

      group_channel_->SetStreamHandler(std::move(event_channel_handler));
    } else {
      auto ret = CionGroupManager::Subscribe(handle);
      if (!ret) {
        result->Error("listen failed");
        return;
      }
    }

    result->Success();
  }

  void TryDiscovery(
      const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    cion_client_h handle = nullptr;
    if (!GetValueFromArgs<int64_t>(args, "handle",
                                   reinterpret_cast<int64_t&>(handle))) {
      result->Error("Invalid parameter");
      return;
    }

    if (client_channel_ == nullptr) {
      client_channel_ =
          std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
              plugin_registrar_->messenger(), "tizen/cion_client",
              &flutter::StandardMethodCodec::GetInstance());

      auto event_channel_handler =
          std::make_unique<flutter::StreamHandlerFunctions<>>(
              [handle](const flutter::EncodableValue* arguments,
                       std::unique_ptr<flutter::EventSink<>>&& events)
                  -> std::unique_ptr<flutter::StreamHandlerError<>> {
                LOG_DEBUG("OnListen event channel");
                CionClientManager::Init(std::move(events));
                auto ret = CionClientManager::TryDiscovery(handle);
                if (!ret) {
                  LOG_ERROR("discovery failed.");
                }

                return nullptr;
              },
              [this](const flutter::EncodableValue* arguments)
                  -> std::unique_ptr<flutter::StreamHandlerError<>> {
                LOG_DEBUG("OnCancel event channel");
                client_channel_ = nullptr;
                return nullptr;
              });

      client_channel_->SetStreamHandler(std::move(event_channel_handler));
    } else {
      auto ret = CionClientManager::TryDiscovery(handle);
      if (!ret) {
        result->Error("discovery failed.");
        return;
      }
    }
    result->Success();
  }

  void ClientConnect(
      const flutter::EncodableValue* args,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    cion_client_h handle = nullptr;
    cion_peer_info_h peer_handle = nullptr;

    if (!GetValueFromArgs<int64_t>(args, "handle",
                                   reinterpret_cast<int64_t&>(handle))) {
      result->Error("Invalid parameter");
      return;
    }

    if (!GetValueFromArgs<int64_t>(args, "peerInfoHandle",
                                   reinterpret_cast<int64_t&>(peer_handle))) {
      if (!GetValueFromArgs<int32_t>(args, "peerInfoHandle",
                                     reinterpret_cast<int32_t&>(peer_handle))) {
        result->Error("Invalid parameter");
        return;
      }
    }

    if (client_channel_ == nullptr) {
      result->Error("connect failed.");
      return;
    } else {
      auto ret = CionClientManager::Connect(handle, peer_handle);
      if (!ret) {
        result->Error("connect failed.");
        return;
      }
    }

    LOG_DEBUG("Success");
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
      client_channel_;
  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>
      group_channel_;
  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>
      server_channel_;
  flutter::PluginRegistrar* plugin_registrar_;
};
}  // namespace

void TizenCionPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  TizenCionPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
