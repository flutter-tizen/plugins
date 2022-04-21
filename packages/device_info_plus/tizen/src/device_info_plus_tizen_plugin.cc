// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device_info_plus_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <system_info.h>

#include <map>
#include <memory>
#include <string>

#include "log.h"

namespace {

class DeviceInfoPlusTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "dev.fluttercommunity.plus/device_info",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<DeviceInfoPlusTizenPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  DeviceInfoPlusTizenPlugin() {}

  virtual ~DeviceInfoPlusTizenPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const auto &method_name = method_call.method_name();

    if (method_name == "getTizenDeviceInfo") {
      flutter::EncodableMap map;

      for (const auto &[key, tizen_key] : tizen_keys_) {
        auto response = flutter::EncodableValue();
        char *value = nullptr;
        int ret = system_info_get_platform_string(tizen_key.c_str(), &value);
        if (ret == SYSTEM_INFO_ERROR_NONE) {
          response = std::string(value);
          free(value);
        } else {
          LOG_ERROR("Failed to get %s from the system: %s", tizen_key.c_str(),
                    get_error_message(ret));
        }
        map[flutter::EncodableValue(key)] = response;
      }

      result->Success(flutter::EncodableValue(map));
    } else {
      result->NotImplemented();
    }
  }

  const std::map<std::string, std::string> tizen_keys_ = {
      {"modelName", "http://tizen.org/system/model_name"},
      {"cpuArch", "http://tizen.org/feature/platform.core.cpu.arch"},
      {"nativeApiVersion",
       "http://tizen.org/feature/platform.native.api.version"},
      {"platformVersion", "http://tizen.org/feature/platform.version"},
      {"webApiVersion", "http://tizen.org/feature/platform.web.api.version"},
      {"profile", "http://tizen.org/feature/profile"},
      {"buildDate", "http://tizen.org/system/build.date"},
      {"buildId", "http://tizen.org/system/build.id"},
      {"buildString", "http://tizen.org/system/build.string"},
      {"buildTime", "http://tizen.org/system/build.time"},
      {"buildType", "http://tizen.org/system/build.type"},
      {"buildVariant", "http://tizen.org/system/build.variant"},
      {"buildRelease", "http://tizen.org/system/build.release"},
      {"deviceType", "http://tizen.org/system/device_type"},
      {"manufacturer", "http://tizen.org/system/manufacturer"},
      {"platformName", "http://tizen.org/system/platform.name"},
      {"platformProcessor", "http://tizen.org/system/platform.processor"},
      {"tizenId", "http://tizen.org/system/tizenid"},
  };
};

}  // namespace

void DeviceInfoPlusTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  DeviceInfoPlusTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
