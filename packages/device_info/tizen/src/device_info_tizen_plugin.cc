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

static std::string systemInfoErrorToString(int error) {
  switch (error) {
    case SYSTEM_INFO_ERROR_NONE:
      return "SystemInfo - Successful";
    case SYSTEM_INFO_ERROR_INVALID_PARAMETER:
      return "SystemInfo - Invalid parameter";
    case SYSTEM_INFO_ERROR_OUT_OF_MEMORY:
      return "SystemInfo - Out of Memory";
    case SYSTEM_INFO_ERROR_IO_ERROR:
      return "SystemInfo - IO Error";
    case SYSTEM_INFO_ERROR_PERMISSION_DENIED:
      return "SystemInfo - Permission denied";
    default:
      return "SystemInfo - Unknown Error";
  }
}

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
    int ret;
    bool hasResult = false;
    std::string resultStr;
    std::string errorStr;

    // http://tizen.org/system/model_name
    ret = system_info_get_platform_string("http://tizen.org/system/model_name",
                                          &value);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
      resultStr = systemInfoErrorToString(ret);
      errorStr.append(" Failed to get model name: ");
      errorStr.append(resultStr);
    } else {
      hasResult = true;
      resultStr = std::string(value);
      LOG_INFO("model name : %s\n", value);
      free(value);
    }
    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "modelName", resultStr));

    // http://tizen.org/feature/platform.core.cpu.arch
    ret = system_info_get_platform_string(
        "http://tizen.org/feature/platform.core.cpu.arch", &value);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
      resultStr = systemInfoErrorToString(ret);
      errorStr.append(" Failed to get cpu arch: ");
      errorStr.append(resultStr);
    } else {
      hasResult = true;
      resultStr = std::string(value);
      LOG_INFO("cpu arch : %s\n", value);
      free(value);
    }
    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "cpuArch", resultStr));

    // http://tizen.org/feature/platform.native.api.version
    ret = system_info_get_platform_string(
        "http://tizen.org/feature/platform.native.api.version", &value);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
      resultStr = systemInfoErrorToString(ret);
      errorStr.append(" Failed to get native api version: ");
      errorStr.append(resultStr);
    } else {
      hasResult = true;
      resultStr = std::string(value);
      LOG_INFO("native api version : %s\n", value);
      free(value);
    }
    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "nativeApiVersion", resultStr));

    // http://tizen.org/feature/platform.version
    ret = system_info_get_platform_string(
        "http://tizen.org/feature/platform.version", &value);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
      resultStr = systemInfoErrorToString(ret);
      errorStr.append(" Failed to get platform version: ");
      errorStr.append(resultStr);
    } else {
      hasResult = true;
      resultStr = std::string(value);
      LOG_INFO("platform version : %s\n", value);
      free(value);
    }
    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "platformVersion", resultStr));

    // http://tizen.org/feature/platform.web.api.version
    ret = system_info_get_platform_string(
        "http://tizen.org/feature/platform.web.api.version", &value);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
      resultStr = systemInfoErrorToString(ret);
      errorStr.append(" Failed to get web api version: ");
      errorStr.append(resultStr);
    } else {
      hasResult = true;
      resultStr = std::string(value);
      LOG_INFO("web api version : %s\n", value);
      free(value);
    }
    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "webApiVersion", resultStr));

    // http://tizen.org/system/build.date
    ret = system_info_get_platform_string("http://tizen.org/system/build.date",
                                          &value);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
      resultStr = systemInfoErrorToString(ret);
      errorStr.append(" Failed to get build date: ");
      errorStr.append(resultStr);
    } else {
      hasResult = true;
      resultStr = std::string(value);
      LOG_INFO("build date : %s\n", value);
      free(value);
    }
    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "buildDate", resultStr));

    // http://tizen.org/system/build.id
    ret = system_info_get_platform_string("http://tizen.org/system/build.id",
                                          &value);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
      resultStr = systemInfoErrorToString(ret);
      errorStr.append(" Failed to get build id: ");
      errorStr.append(resultStr);
    } else {
      hasResult = true;
      resultStr = std::string(value);
      LOG_INFO("build id : %s\n", value);
      free(value);
    }
    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "buildId", resultStr));

    // http://tizen.org/system/build.string
    ret = system_info_get_platform_string(
        "http://tizen.org/system/build.string", &value);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
      resultStr = systemInfoErrorToString(ret);
      errorStr.append(" Failed to get build string: ");
      errorStr.append(resultStr);
    } else {
      hasResult = true;
      resultStr = std::string(value);
      LOG_INFO("build string : %s\n", value);
      free(value);
    }
    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "buildString", resultStr));

    // http://tizen.org/system/build.time
    ret = system_info_get_platform_string("http://tizen.org/system/build.time",
                                          &value);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
      resultStr = systemInfoErrorToString(ret);
      errorStr.append(" Failed to get build time: ");
      errorStr.append(resultStr);
    } else {
      hasResult = true;
      resultStr = std::string(value);
      LOG_INFO("build time : %s\n", value);
      free(value);
    }
    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "buildTime", resultStr));

    // http://tizen.org/system/build.type
    ret = system_info_get_platform_string("http://tizen.org/system/build.type",
                                          &value);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
      resultStr = systemInfoErrorToString(ret);
      errorStr.append(" Failed to get build type: ");
      errorStr.append(resultStr);
    } else {
      hasResult = true;
      resultStr = std::string(value);
      LOG_INFO("build type : %s\n", value);
      free(value);
    }
    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "buildType", resultStr));

    // http://tizen.org/system/build.variant
    ret = system_info_get_platform_string(
        "http://tizen.org/system/build.variant", &value);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
      resultStr = systemInfoErrorToString(ret);
      errorStr.append(" Failed to get build variant: ");
      errorStr.append(resultStr);
    } else {
      hasResult = true;
      resultStr = std::string(value);
      LOG_INFO("build variant : %s\n", value);
      free(value);
    }
    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "buildVariant", resultStr));

    // http://tizen.org/system/build.release
    ret = system_info_get_platform_string(
        "http://tizen.org/system/build.release", &value);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
      resultStr = systemInfoErrorToString(ret);
      errorStr.append(" Failed to get build release: ");
      errorStr.append(resultStr);
    } else {
      hasResult = true;
      resultStr = std::string(value);
      LOG_INFO("build release : %s\n", value);
      free(value);
    }
    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "buildRelease", resultStr));

    // http://tizen.org/system/device_type
    ret = system_info_get_platform_string("http://tizen.org/system/device_type",
                                          &value);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
      resultStr = systemInfoErrorToString(ret);
      errorStr.append(" Failed to get device type: ");
      errorStr.append(resultStr);
    } else {
      hasResult = true;
      resultStr = std::string(value);
      LOG_INFO("device type : %s\n", value);
      free(value);
    }
    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "deviceType", resultStr));

    // http://tizen.org/system/manufacturer
    ret = system_info_get_platform_string(
        "http://tizen.org/system/manufacturer", &value);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
      resultStr = systemInfoErrorToString(ret);
      errorStr.append(" Failed to get manufacturer: ");
      errorStr.append(resultStr);
    } else {
      hasResult = true;
      resultStr = std::string(value);
      LOG_INFO("manufacturer : %s\n", value);
      free(value);
    }
    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "manufacturer", resultStr));

    if (hasResult) {
      result->Success(flutter::EncodableValue(msg));
    } else {
      result->Error(std::to_string(-1), "Failed to get deviceinfo",
                    flutter::EncodableValue(errorStr));
    }
  }
};

void DeviceInfoTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  DeviceInfoTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
