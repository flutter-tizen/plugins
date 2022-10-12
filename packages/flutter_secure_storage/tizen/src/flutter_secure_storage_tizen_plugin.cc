// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_secure_storage_tizen_plugin.h"

#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <string>

#include "flutter_secure_storage.h"
#include "log.h"

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

class FlutterSecureStorageTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(),
            "plugins.it_nomads.com/flutter_secure_storage",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<FlutterSecureStorageTizenPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  FlutterSecureStorageTizenPlugin() {}

  virtual ~FlutterSecureStorageTizenPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const auto &method_name = method_call.method_name();
    const auto *arguments =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (method_name == "write") {
      if (!arguments) {
        result->Error("Invalid argument", "No arguments provided.");
        return;
      }

      std::string key;
      if (!GetValueFromEncodableMap(arguments, "key", key)) {
        result->Error("Invalid argument", "No key provided.");
        return;
      }

      std::string value;
      if (!GetValueFromEncodableMap(arguments, "value", value)) {
        result->Error("Invalid argument", "No value provided.");
        return;
      }

      std::optional<std::string> old_value = storage_.Read(key);
      if (old_value.has_value()) {
        // Delete previous data.
        storage_.Delete(key);
      }
      storage_.Write(key, value);
      result->Success();
    } else if (method_name == "read") {
      if (!arguments) {
        result->Error("Invalid argument", "No arguments provided.");
        return;
      }

      std::string key;
      if (!GetValueFromEncodableMap(arguments, "key", key)) {
        result->Error("Invalid argument", "No key provided.");
        return;
      }

      std::optional<std::string> value = storage_.Read(key);
      if (value.has_value()) {
        result->Success(flutter::EncodableValue(value.value()));
      } else {
        result->Success();
      }
    } else if (method_name == "readAll") {
      result->Success(flutter::EncodableValue(storage_.ReadAll()));
    } else if (method_name == "delete") {
      if (!arguments) {
        result->Error("Invalid argument", "No arguments provided.");
        return;
      }

      std::string key;
      if (!GetValueFromEncodableMap(arguments, "key", key)) {
        result->Error("Invalid argument", "No key provided.");
        return;
      }

      storage_.Delete(key);
      result->Success();
    } else if (method_name == "deleteAll") {
      storage_.DeleteAll();
      result->Success();
    } else if (method_name == "containsKey") {
      if (!arguments) {
        result->Error("Invalid argument", "No arguments provided.");
        return;
      }

      std::string key;
      if (!GetValueFromEncodableMap(arguments, "key", key)) {
        result->Error("Invalid argument", "No key provided.");
        return;
      }

      bool ret = storage_.ContainsKey(key);
      result->Success(flutter::EncodableValue(ret));
    } else {
      result->NotImplemented();
    }
  }

  FlutterSecureStorage storage_;
};

}  // namespace

void FlutterSecureStorageTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterSecureStorageTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
