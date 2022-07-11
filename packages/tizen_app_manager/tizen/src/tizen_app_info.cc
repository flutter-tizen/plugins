// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_app_info.h"

#include <map>

#include "log.h"

std::string TizenAppInfo::GetAppId() {
  int ret = app_info_get_app_id(app_info_, &app_id_);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("get app Id error ! : %s", get_error_message(ret));
    last_error_ = ret;
    return std::string();
  }
  return std::string(app_id_);
}

std::string TizenAppInfo::GetPackageId() {
  char *package_id = nullptr;

  int ret = app_info_get_package(app_info_, &package_id);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("get package Id error! : %s", get_error_message(ret));
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
    LOG_ERROR("get application label error! : %s", get_error_message(ret));
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
    LOG_ERROR("get application type error! : %s", get_error_message(ret));
    last_error_ = ret;
    return std::string();
  }
  std::string result = std::string(type);
  free(type);
  return result;
}

std::string TizenAppInfo::GetIconPath() {
  char *icon_path = nullptr;

  int ret = app_info_get_icon(app_info_, &icon_path);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("get icon path error! : %s", get_error_message(ret));
    // because some system app and service app don't have icon,
    // just print error log, and pass it
    return std::string();
  }
  std::string result = std::string(icon_path);
  free(icon_path);
  return result;
}

std::string TizenAppInfo::GetExecutablePath() {
  char *executable_path = nullptr;

  int ret = app_info_get_exec(app_info_, &executable_path);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("get exec path error! : %s", get_error_message(ret));
    last_error_ = ret;
    return std::string();
  }
  std::string result = std::string(executable_path);
  free(executable_path);
  return result;
}

std::string TizenAppInfo::GetSharedResourcePath() {
  char *shared_res_path = nullptr;

  int ret = app_manager_get_shared_resource_path(app_id_, &shared_res_path);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("get shared resource path error! : %s", get_error_message(ret));
    last_error_ = ret;
    return std::string();
  }
  std::string result = std::string(shared_res_path);
  free(shared_res_path);
  return result;
}

bool TizenAppInfo::GetIsNoDisplay() {
  bool is_no_display = false;

  int ret = app_info_is_nodisplay(app_info_, &is_no_display);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("app_info_is_nodisplay error! : %s", get_error_message(ret));
    last_error_ = ret;
  }
  return is_no_display;
}

void TizenAppInfo::GetForEachMetadata(app_info_metadata_cb callback,
                                      void *user_data) {
  int ret = app_info_foreach_metadata(app_info_, callback, user_data);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("app_info_foreach_metadata error! : %s", get_error_message(ret));
    last_error_ = ret;
    return;
  }
  return;
}

TizenAppInfo::~TizenAppInfo() {
  if (app_id_) {
    free(app_id_);
  }
}
