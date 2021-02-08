// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "package_info_tizen_plugin.h"

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

static std::string appErrorToString(int error) {
  switch (error) {
    case APP_ERROR_NONE:
      return "APP - Successful";
    case APP_ERROR_INVALID_PARAMETER:
      return "APP - Invalid parameter";
    case APP_ERROR_OUT_OF_MEMORY:
      return "APP - Out of Memory";
    case APP_ERROR_INVALID_CONTEXT:
      return "APP - Invalid application context";
    case APP_ERROR_NO_SUCH_FILE:
      return "APP - No such file or directory";
    case APP_ERROR_NOT_SUPPORTED:
      return "APP - Not supported";
    case APP_ERROR_ALREADY_RUNNING:
      return "APP - Application is already running";
    case APP_ERROR_PERMISSION_DENIED:
      return "APP - Permission denied";
    default:
      return "APP - Unknown Error";
  }
}

static std::string packageErrorToString(int error) {
  switch (error) {
    case PACKAGE_MANAGER_ERROR_NONE:
      return "PackageManager - Successful";
    case PACKAGE_MANAGER_ERROR_INVALID_PARAMETER:
      return "PackageManager - Invalid parameter";
    case PACKAGE_MANAGER_ERROR_OUT_OF_MEMORY:
      return "PackageManager - Out of Memory";
    case PACKAGE_MANAGER_ERROR_IO_ERROR:
      return "PackageManager - Internal I/O error";
    case PACKAGE_MANAGER_ERROR_NO_SUCH_PACKAGE:
      return "PackageManager - No such package";
    case PACKAGE_MANAGER_ERROR_SYSTEM_ERROR:
      return "PackageManager - Severe system error";
    case PACKAGE_MANAGER_ERROR_PERMISSION_DENIED:
      return "PackageManager - Permission denied";
    default:
      return "PackageManager - Unknown Error";
  }
}

class PackageInfoTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "plugins.flutter.io/package_info",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<PackageInfoTizenPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  PackageInfoTizenPlugin() {}

  virtual ~PackageInfoTizenPlugin() {}

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

    char *app_id = NULL;
    char *pkg_name = NULL;
    char *version = NULL;
    char *label = NULL;
    package_info_h package_info = NULL;
    flutter::EncodableMap msg;

    int ret = app_get_id(&app_id);
    if (ret != APP_ERROR_NONE || app_id == NULL) {
      result->Error(std::to_string(ret), "Failed to get app_id",
                    flutter::EncodableValue(appErrorToString(ret)));
      goto cleanup;
    }
    LOG_INFO("app id : %s\n", app_id);

    ret = package_info_create(app_id, &package_info);
    if (ret != PACKAGE_MANAGER_ERROR_NONE || package_info == NULL) {
      result->Error(std::to_string(ret), "Failed to create package_info",
                    flutter::EncodableValue(packageErrorToString(ret)));
      goto cleanup;
    }

    ret = package_info_get_label(package_info, &label);
    if (ret != PACKAGE_MANAGER_ERROR_NONE || label == NULL) {
      result->Error(std::to_string(ret), "Failed to get app name",
                    flutter::EncodableValue(packageErrorToString(ret)));
      goto cleanup;
    }
    LOG_INFO("package label : %s\n", label);

    ret = package_info_get_package(package_info, &pkg_name);
    if (ret != PACKAGE_MANAGER_ERROR_NONE || pkg_name == NULL) {
      result->Error(std::to_string(ret), "Failed to get package name",
                    flutter::EncodableValue(packageErrorToString(ret)));
      goto cleanup;
    }
    LOG_INFO("package name : %s\n", pkg_name);

    ret = package_info_get_version(package_info, &version);
    if (ret != PACKAGE_MANAGER_ERROR_NONE || version == NULL) {
      result->Error(std::to_string(ret), "Failed to get version",
                    flutter::EncodableValue(packageErrorToString(ret)));
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

void PackageInfoTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  PackageInfoTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
