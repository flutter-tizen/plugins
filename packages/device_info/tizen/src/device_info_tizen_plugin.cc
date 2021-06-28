// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device_info_tizen_plugin.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <system_info.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "log.h"

class DeviceInfoTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "plugins.flutter.io/device_info",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<DeviceInfoTizenPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  DeviceInfoTizenPlugin() {}

  virtual ~DeviceInfoTizenPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_INFO("method : %s", method_call.method_name().data());

    char *value = nullptr;
    flutter::EncodableMap msg;

    for (auto &request : requests_) {
      const std::string &map_key = request.first;
      const std::string &tizen_key = request.second;
      std::string result("");
      int ret = system_info_get_platform_string(tizen_key.c_str(), &value);
      if (ret == SYSTEM_INFO_ERROR_NONE) {
        result = std::string(value);
        free(value);
      } else {
        LOG_ERROR("Failed to get %s from the system: %s", tizen_key.c_str(),
                  get_error_message(ret));
      }
      msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
          map_key, result));
    }

    result->Success(flutter::EncodableValue(msg));
  }

  const std::vector<std::pair<std::string, std::string>> requests_ = {
      {"modelName", "http://tizen.org/system/model_name"},
      {"cpuArch", "http://tizen.org/feature/platform.core.cpu.arch"},
      {"nativeApiVersion",
       "http://tizen.org/feature/platform.native.api.version"},
      {"platformVersion", "http://tizen.org/feature/platform.version"},
      {"webApiVersion", "http://tizen.org/feature/platform.web.api.version"},
      {"buildDate", "http://tizen.org/system/build.date"},
      {"buildId", "http://tizen.org/system/build.id"},
      {"buildString", "http://tizen.org/system/build.string"},
      {"buildTime", "http://tizen.org/system/build.time"},
      {"buildType", "http://tizen.org/system/build.type"},
      {"buildVariant", "http://tizen.org/system/build.variant"},
      {"buildRelease", "http://tizen.org/system/build.release"},
      {"deviceType", "http://tizen.org/system/device_type"},
      {"manufacturer", "http://tizen.org/system/manufacturer"},
  };
};

void DeviceInfoTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  DeviceInfoTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
