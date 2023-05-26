// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "package_info_plus_tizen_plugin.h"

#include <app_common.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <package_manager.h>

#include <memory>
#include <string>

namespace {

class PackageInfoPlusTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "dev.fluttercommunity.plus/package_info",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<PackageInfoPlusTizenPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  PackageInfoPlusTizenPlugin() {}

  virtual ~PackageInfoPlusTizenPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const auto &method_name = method_call.method_name();

    if (method_name != "getAll") {
      result->NotImplemented();
      return;
    }

    flutter::EncodableMap map;

    char *id = nullptr;
    int ret = app_get_id(&id);
    if (ret != APP_ERROR_NONE) {
      result->Error(std::to_string(ret), "Failed to find the app ID.",
                    flutter::EncodableValue(get_error_message(ret)));
      return;
    }
    auto app_id = std::string(id);
    free(id);

    package_info_h package_info = nullptr;
    ret = package_info_create(app_id.c_str(), &package_info);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
      result->Error(std::to_string(ret),
                    "Failed to create a package_info handle.",
                    flutter::EncodableValue(get_error_message(ret)));
      return;
    }

    char *label = nullptr;
    ret = package_info_get_label(package_info, &label);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
      result->Error(std::to_string(ret), "Failed to get the app label.",
                    flutter::EncodableValue(get_error_message(ret)));
      package_info_destroy(package_info);
      return;
    }
    map[flutter::EncodableValue("appName")] =
        flutter::EncodableValue(std::string(label));
    free(label);

    char *package_name = nullptr;
    ret = package_info_get_package(package_info, &package_name);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
      result->Error(std::to_string(ret), "Failed to get the package name.",
                    flutter::EncodableValue(get_error_message(ret)));
      package_info_destroy(package_info);
      return;
    }
    map[flutter::EncodableValue("packageName")] =
        flutter::EncodableValue(std::string(package_name));
    free(package_name);

    char *version = nullptr;
    ret = package_info_get_version(package_info, &version);
    if (ret != PACKAGE_MANAGER_ERROR_NONE) {
      result->Error(std::to_string(ret), "Failed to get the package version.",
                    flutter::EncodableValue(get_error_message(ret)));
      package_info_destroy(package_info);
      return;
    }
    map[flutter::EncodableValue("version")] =
        flutter::EncodableValue(std::string(version));
    free(version);
    package_info_destroy(package_info);

    result->Success(flutter::EncodableValue(map));
  }
};

}  // namespace

void PackageInfoPlusTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  PackageInfoPlusTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
