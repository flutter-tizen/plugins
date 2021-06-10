// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "package_info_plus_tizen_plugin.h"

#include <app_common.h>
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <package_manager.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "log.h"

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
    LOG_INFO("method : %s", method_call.method_name().data());
    std::string replay = "";

    if (method_call.method_name().compare("getAll") != 0) {
      result->Error("-1", "Not supported method");
      return;
    }

    char *app_id = nullptr;
    char *pkg_name = nullptr;
    char *version = nullptr;
    char *label = nullptr;
    package_info_h package_info = nullptr;
    flutter::EncodableMap msg;

    int ret = app_get_id(&app_id);
    if (ret != APP_ERROR_NONE || app_id == nullptr) {
      result->Error(std::to_string(ret), "Failed to get app_id",
                    flutter::EncodableValue(get_error_message(ret)));
      goto cleanup;
    }
    LOG_INFO("app id : %s\n", app_id);

    ret = package_info_create(app_id, &package_info);
    if (ret != PACKAGE_MANAGER_ERROR_NONE || package_info == nullptr) {
      result->Error(std::to_string(ret), "Failed to create package_info",
                    flutter::EncodableValue(get_error_message(ret)));
      goto cleanup;
    }

    ret = package_info_get_label(package_info, &label);
    if (ret != PACKAGE_MANAGER_ERROR_NONE || label == nullptr) {
      result->Error(std::to_string(ret), "Failed to get app name",
                    flutter::EncodableValue(get_error_message(ret)));
      goto cleanup;
    }
    LOG_INFO("package label : %s\n", label);

    ret = package_info_get_package(package_info, &pkg_name);
    if (ret != PACKAGE_MANAGER_ERROR_NONE || pkg_name == nullptr) {
      result->Error(std::to_string(ret), "Failed to get package name",
                    flutter::EncodableValue(get_error_message(ret)));
      goto cleanup;
    }
    LOG_INFO("package name : %s\n", pkg_name);

    ret = package_info_get_version(package_info, &version);
    if (ret != PACKAGE_MANAGER_ERROR_NONE || version == nullptr) {
      result->Error(std::to_string(ret), "Failed to get version",
                    flutter::EncodableValue(get_error_message(ret)));
      goto cleanup;
    }
    LOG_INFO("package version : %s\n", version);

    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "appName", std::string(label)));
    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "packageName", std::string(pkg_name)));
    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "version", std::string(version)));
    msg.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "buildNumber", "Not supported property"));
    result->Success(flutter::EncodableValue(msg));

  cleanup:
    if (app_id) {
      free(app_id);
    }
    if (label) {
      free(label);
    }
    if (pkg_name) {
      free(pkg_name);
    }
    if (version) {
      free(version);
    }
    if (package_info) {
      package_info_destroy(package_info);
    }
  }
};

void PackageInfoPlusTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  PackageInfoPlusTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
