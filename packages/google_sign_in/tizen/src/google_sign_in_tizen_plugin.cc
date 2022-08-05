// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "google_sign_in_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <optional>

#include "secure_storage.h"

namespace {

const char *kInvalidArgument = "Invalid argument";

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

class GoogleSignInTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "tizen/secure_storage",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<GoogleSignInTizenPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  GoogleSignInTizenPlugin() {}

  virtual ~GoogleSignInTizenPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const auto &method_name = method_call.method_name();
    if (method_name == "destroy") {
      storage_.Destroy();
      result->Success();
      return;
    }

    const auto *arguments =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error(kInvalidArgument, "No arguments provided.");
      return;
    }
    std::string name;
    if (!GetValueFromEncodableMap(arguments, "name", name)) {
      result->Error(kInvalidArgument, "No name provided.");
      return;
    }

    if (!storage_.HasKey()) {
      storage_.CreateKey();
    }

    if (method_name == "save") {
      std::vector<uint8_t> data;
      if (!GetValueFromEncodableMap(arguments, "data", data)) {
        result->Error(kInvalidArgument, "No data provided.");
        return;
      }
      std::vector<uint8_t> iv;
      if (!GetValueFromEncodableMap(arguments, "iv", iv)) {
        result->Error(kInvalidArgument, "No initialization_vector provided.");
        return;
      }
      storage_.SaveData(name, iv, data);
      result->Success();
    } else if (method_name == "get") {
      std::optional<std::vector<uint8_t>> data = storage_.GetData(name);
      if (data.has_value()) {
        result->Success(flutter::EncodableValue(data.value()));
      } else {
        // Sends `null` to dart side if data doesn't exist.
        result->Success();
      }
    } else if (method_name == "remove") {
      storage_.RemoveData(name);
      result->Success();
    } else {
      result->NotImplemented();
    }
  }

  SecureStorage storage_;
};

}  // namespace

void GoogleSignInTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  GoogleSignInTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
