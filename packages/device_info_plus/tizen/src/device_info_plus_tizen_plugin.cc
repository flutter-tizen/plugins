// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device_info_plus_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <runtime_info.h>
#include <storage.h>
#include <sys/statvfs.h>
#include <system_info.h>

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "log.h"

namespace {

constexpr int64_t kUnknownMetric = -1;

class DeviceInfoPlusTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "tizen/device_info_plus",
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
      result->Success(flutter::EncodableValue(GetTizenDeviceInfo()));
    } else {
      result->NotImplemented();
    }
  }

  flutter::EncodableMap GetTizenDeviceInfo() const {
    flutter::EncodableMap map;
    PopulatePlatformInfo(map);
    PopulateStorageInfo(map);
    PopulateMemoryInfo(map);
    return map;
  }

  void PopulatePlatformInfo(flutter::EncodableMap &map) const {
    for (const auto &[key, tizen_key] : tizen_keys_) {
      auto response = flutter::EncodableValue();
      int ret = SYSTEM_INFO_ERROR_NONE;
      if (key.compare(0, 6, "screen") == 0) {
        int value = 0;
        ret = system_info_get_platform_int(tizen_key.c_str(), &value);
        if (ret == SYSTEM_INFO_ERROR_NONE) {
          response = value;
        }
      } else {
        char *value = nullptr;
        ret = system_info_get_platform_string(tizen_key.c_str(), &value);
        if (ret == SYSTEM_INFO_ERROR_NONE) {
          response = std::string(value);
          free(value);
        }
      }

      if (ret != SYSTEM_INFO_ERROR_NONE) {
        LOG_ERROR("Failed to get %s from the system: %s", tizen_key.c_str(),
                  get_error_message(ret));
      }
      map[flutter::EncodableValue(key)] = response;
    }
  }

  void PopulateStorageInfo(flutter::EncodableMap &map) const {
    int64_t free_disk_size = kUnknownMetric;
    int64_t total_disk_size = kUnknownMetric;

    struct statvfs storage_stat = {};
    const int ret = storage_get_internal_memory_size(&storage_stat);
    if (ret == STORAGE_ERROR_NONE) {
      total_disk_size = static_cast<int64_t>(storage_stat.f_frsize) *
                        static_cast<int64_t>(storage_stat.f_blocks);
      free_disk_size = static_cast<int64_t>(storage_stat.f_bsize) *
                       static_cast<int64_t>(storage_stat.f_bavail);
    } else {
      LOG_ERROR("Failed to get internal storage size: %s",
                get_error_message(ret));
    }

    map[flutter::EncodableValue("freeDiskSize")] =
        flutter::EncodableValue(free_disk_size);
    map[flutter::EncodableValue("totalDiskSize")] =
        flutter::EncodableValue(total_disk_size);
  }

  void PopulateMemoryInfo(flutter::EncodableMap &map) const {
    int64_t physical_ram_size = kUnknownMetric;
    int64_t available_ram_size = kUnknownMetric;

    runtime_memory_info_s memory_info = {};
    int ret = runtime_info_get_system_memory_info(&memory_info);
    if (ret == RUNTIME_INFO_ERROR_NONE) {
      physical_ram_size = static_cast<int64_t>(memory_info.total) / 1024;

      int64_t available_kib = memory_info.free;
      if (memory_info.total >= memory_info.used) {
        available_kib = static_cast<int64_t>(memory_info.total) -
                        static_cast<int64_t>(memory_info.used);
      }
      available_ram_size = available_kib / 1024;
    } else {
      LOG_ERROR("Failed to get system memory info: %s", get_error_message(ret));

      int physical_memory_kib = 0;
      ret = runtime_info_get_physical_memory_size(&physical_memory_kib);
      if (ret == RUNTIME_INFO_ERROR_NONE) {
        physical_ram_size = static_cast<int64_t>(physical_memory_kib) / 1024;
      } else {
        LOG_ERROR("Failed to get physical memory size: %s",
                  get_error_message(ret));
      }
    }

    map[flutter::EncodableValue("physicalRamSize")] =
        flutter::EncodableValue(physical_ram_size);
    map[flutter::EncodableValue("availableRamSize")] =
        flutter::EncodableValue(available_ram_size);
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
      {"screenWidth", "http://tizen.org/feature/screen.width"},
      {"screenHeight", "http://tizen.org/feature/screen.height"},
  };
};

}  // namespace

void DeviceInfoPlusTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  DeviceInfoPlusTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
