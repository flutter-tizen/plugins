// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "app_settings_manager.h"

#include <app_common.h>
#include <package_manager.h>

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

  if (!CreateAppControl()) {
    DestroyAppControl();
    return false;
  }

  if (!SetAppId(kSettingAppId)) {
    DestroyAppControl();
    return false;
  }

  if (!AddExtraData(package_name)) {
    DestroyAppControl();
    return false;
  }

  if (!SendLauchRequest()) {
    DestroyAppControl();
    return false;
  }

  DestroyAppControl();
  return true;
}

bool AppSettingsManager::OpenLocationSetting() {
  if (!CreateAppControl()) {
    DestroyAppControl();
    return false;
  }

  if (!SetAppId(kLocationSettingAppId)) {
    DestroyAppControl();
    return false;
  }

  if (!SetOperation(kLocationSettingOperationId)) {
    DestroyAppControl();
    return false;
  }

  if (!SendLauchRequest()) {
    DestroyAppControl();
    return false;
  }

  DestroyAppControl();
  return true;
}

bool AppSettingsManager::CreateAppControl() {
  int ret = app_control_create(&app_control_);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOG_ERROR("Failed to create an app control handle. (%s)",
              get_error_message(ret));
    return false;
  }
  return true;
}

bool AppSettingsManager::SetAppId(const char* app_id) {
  int ret = app_control_set_app_id(app_control_, app_id);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOG_ERROR("Failed to set an app ID. (%s)", get_error_message(ret));
    return false;
  }
  return true;
}

bool AppSettingsManager::SetOperation(const char* operation) {
  int ret = app_control_set_operation(app_control_, operation);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOG_ERROR("Failed to set operation. (%s)", get_error_message(ret));
    return false;
  }
  return true;
}

bool AppSettingsManager::AddExtraData(std::string package_name) {
  int ret =
      app_control_add_extra_data(app_control_, "pkgId", package_name.c_str());
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOG_ERROR("Failed to add extra data. (%s)", get_error_message(ret));
    return false;
  }
  return true;
}

bool AppSettingsManager::SendLauchRequest() {
  int ret = app_control_send_launch_request(app_control_, nullptr, nullptr);
  if (ret != APP_CONTROL_ERROR_NONE) {
    LOG_ERROR("Failed to send a launch request. (%s)", get_error_message(ret));
    return false;
  }
  return true;
}

void AppSettingsManager::DestroyAppControl() {
  app_control_destroy(app_control_);
}