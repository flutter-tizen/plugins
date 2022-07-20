// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_app_info.h"

#include "log.h"

TizenAppInfo::~TizenAppInfo() {
  if (app_info_) {
    app_info_destroy(app_info_);
  }
}

std::string TizenAppInfo::GetAppId() {
  char *app_id = nullptr;
  int ret = app_info_get_app_id(app_info_, &app_id);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("Failed to get app ID: %s", get_error_message(ret));
    last_error_ = ret;
    return std::string();
  }
  std::string result = std::string(app_id);
  free(app_id);
  return result;
}

std::string TizenAppInfo::GetPackageId() {
  char *package_id = nullptr;
  int ret = app_info_get_package(app_info_, &package_id);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("Failed to get package ID: %s", get_error_message(ret));
    last_error_ = ret;
    return std::string();
  }
  std::string result = std::string(package_id);
  free(package_id);
  return result;
}

std::string TizenAppInfo::GetLabel() {
  char *label = nullptr;
  int ret = app_info_get_label(app_info_, &label);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("Failed to get app label: %s", get_error_message(ret));
    last_error_ = ret;
    return std::string();
  }
  std::string result = std::string(label);
  free(label);
  return result;
}

std::string TizenAppInfo::GetType() {
  char *type = nullptr;
  int ret = app_info_get_type(app_info_, &type);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("Failed to get app type: %s", get_error_message(ret));
    last_error_ = ret;
    return std::string();
  }
  std::string result = std::string(type);
  free(type);
  return result;
}

std::string TizenAppInfo::GetIconPath() {
  char *path = nullptr;
  int ret = app_info_get_icon(app_info_, &path);
  if (ret != APP_MANAGER_ERROR_NONE) {
    // Some system apps and service apps may not have icons.
    LOG_INFO("Failed to get icon path: %s", get_error_message(ret));
    return std::string();
  }
  std::string result = std::string(path);
  free(path);
  return result;
}

std::string TizenAppInfo::GetExecutablePath() {
  char *path = nullptr;
  int ret = app_info_get_exec(app_info_, &path);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("Failed to get app executable path: %s", get_error_message(ret));
    last_error_ = ret;
    return std::string();
  }
  std::string result = std::string(path);
  free(path);
  return result;
}

bool TizenAppInfo::IsNoDisplay() {
  bool value = false;
  int ret = app_info_is_nodisplay(app_info_, &value);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("Failed to get nodisplay info: %s", get_error_message(ret));
    last_error_ = ret;
  }
  return value;
}

std::map<std::string, std::string> TizenAppInfo::GetMetadata() {
  std::map<std::string, std::string> map;
  int ret = app_info_foreach_metadata(
      app_info_,
      [](const char *key, const char *value, void *user_data) -> bool {
        auto *map =
            static_cast<std::map<std::string, std::string> *>(user_data);
        map->insert(std::pair<std::string, std::string>(key, value));
        return true;
      },
      &map);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("Failed to get app metadata: %s", get_error_message(ret));
    last_error_ = ret;
  }
  return map;
}
