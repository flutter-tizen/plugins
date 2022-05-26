// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "app_settings_manager.h"

#include <app_common.h>
#include <package_manager.h>

#include <string>

#include "log.h"

namespace {

constexpr char kSettingAppId[] = "com.samsung.clocksetting.apps";
constexpr char kLocationSettingAppId[] = "com.samsung.setting-location";
constexpr char kLocationSettingOperationId[] =
    "http://tizen.org/appcontrol/operation/configure/location";

std::string GetPackageName() {
  char* app_id;
  int ret = app_get_id(&app_id);
  if (ret != APP_ERROR_NONE) {
    LOG_ERROR("The app ID is not found.");
    return "";
  }

  package_info_h package_info;
  ret = package_info_create(app_id, &package_info);
  free(app_id);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("Failed to create a package info handle.");
    return "";
  }

  char* package_name;
  ret = package_info_get_package(package_info, &package_name);
  package_info_destroy(package_info);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("Failed to get the package name.");
    return "";
  }

  std::string result = std::string(package_name);
  free(package_name);

  return result;
}

}  // namespace

bool AppSettingsManager::OpenAppSettings() {
  std::string package_name = GetPackageName();
  if (package_name.empty()) {
    return false;
  }

  app_control_h app_control;
  int ret = app_control_create(&app_control);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOG_ERROR("Failed to create an app control handle.");
    return false;
  }

  ret = app_control_set_app_id(app_control, kSettingAppId);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOG_ERROR("Failed to set an app ID.");
    app_control_destroy(app_control);
    return false;
  }

  ret = app_control_add_extra_data(app_control, "pkgId", package_name.c_str());
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOG_ERROR("Failed to add extra data.");
    app_control_destroy(app_control);
    return false;
  }

  ret = app_control_send_launch_request(app_control, nullptr, nullptr);
  app_control_destroy(app_control);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOG_ERROR("Failed to send a launch request.");
    return false;
  }

  return true;
}

bool AppSettingsManager::OpenLocationSettings() {
  std::string package_name = GetPackageName();
  if (package_name.empty()) {
    return false;
  }

  app_control_h app_control;
  int ret = app_control_create(&app_control);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOG_ERROR("Failed to create an app control handle.");
    return false;
  }

  ret = app_control_set_app_id(app_control, kLocationSettingAppId);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOG_ERROR("Failed to set an app ID.");
    app_control_destroy(app_control);
    return false;
  }

  ret = app_control_set_operation(app_control, kLocationSettingOperationId);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOG_ERROR("Failed to set operation. (%s)", get_error_message(ret));
    app_control_destroy(app_control);
    return false;
  }

  ret = app_control_send_launch_request(app_control, nullptr, nullptr);
  app_control_destroy(app_control);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOG_ERROR("Failed to send a launch request.");
    return false;
  }

  return true;
}
