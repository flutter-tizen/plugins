// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_app_info.h"

typedef std::pair<flutter::EncodableValue, flutter::EncodableValue> FlValuePair;
typedef std::pair<flutter::EncodableValue, flutter::EncodableMap>
    FlValueMapPair;

void TizenAppInfo::GetAppInfo(flutter::EncodableMap &value) {
  if (!GetAppId(value)) {
    return;
  }
  if (!GetPackageId(value)) {
    return;
  }
  if (!GetLabel(value)) {
    return;
  }
  if (!GetType(value)) {
    return;
  }
  GetIconPath(value);
  if (!GetExecutablePath(value)) {
    return;
  }
  if (!GetSharedResourcePath(value)) {
    return;
  }
  if (!GetIsNoDisplay(value)) {
    return;
  }
  if (!GetForEachMetadata(value)) {
    return;
  }

  return;
}

int TizenAppInfo::GetLastError() { return last_error_; }

bool TizenAppInfo::GetAppId(flutter::EncodableMap &value) {
  int ret = app_info_get_app_id(app_info_, &app_id_);
  if (ret != APP_MANAGER_ERROR_NONE || app_id_ == nullptr) {
    LOG_ERROR("get app Id error ! : %s", get_error_message(ret));
    last_error_ = ret;
    return false;
  }
  value.insert(FlValuePair("appId", std::string(app_id_)));
  return true;
}

bool TizenAppInfo::GetPackageId(flutter::EncodableMap &value) {
  char *package_id = nullptr;

  int ret = app_info_get_package(app_info_, &package_id);
  if (ret != APP_MANAGER_ERROR_NONE || package_id == nullptr) {
    LOG_ERROR("get package Id error! : %s", get_error_message(ret));
    if (package_id) {
      free(package_id);
    }
    last_error_ = ret;
    return false;
  }
  value.insert(FlValuePair("packageId", std::string(package_id)));
  free(package_id);
  return true;
}

bool TizenAppInfo::GetLabel(flutter::EncodableMap &value) {
  char *label = nullptr;

  int ret = app_info_get_label(app_info_, &label);
  if (ret != APP_MANAGER_ERROR_NONE || label == nullptr) {
    LOG_ERROR("get application label error! : %s", get_error_message(ret));
    if (label) {
      free(label);
    }
    last_error_ = ret;
    return false;
  }
  value.insert(FlValuePair("label", std::string(label)));
  free(label);
  return true;
}

bool TizenAppInfo::GetType(flutter::EncodableMap &value) {
  char *type = nullptr;

  int ret = app_info_get_type(app_info_, &type);
  if (ret != APP_MANAGER_ERROR_NONE || type == nullptr) {
    LOG_ERROR("get application type error! : %s", get_error_message(ret));
    if (type) {
      free(type);
    }
    last_error_ = ret;
    return false;
  }
  value.insert(FlValuePair("type", std::string(type)));
  free(type);
  return true;
}

void TizenAppInfo::GetIconPath(flutter::EncodableMap &value) {
  char *icon_path = nullptr;

  int ret = app_info_get_icon(app_info_, &icon_path);
  if (ret != APP_MANAGER_ERROR_NONE || icon_path == nullptr) {
    LOG_ERROR("get icon path error! : %s", get_error_message(ret));
    // because some system app and service app don't have icon,
    // just print error log, and pass it
  } else {
    value.insert(FlValuePair("iconPath", std::string(icon_path)));
  }

  if (icon_path) {
    free(icon_path);
  }
}

bool TizenAppInfo::GetExecutablePath(flutter::EncodableMap &value) {
  char *exec_path = nullptr;

  int ret = app_info_get_exec(app_info_, &exec_path);
  if (ret != APP_MANAGER_ERROR_NONE || exec_path == nullptr) {
    LOG_ERROR("get exec path error! : %s", get_error_message(ret));
    if (exec_path) {
      free(exec_path);
    }
    last_error_ = ret;
    return false;
  }
  value.insert(FlValuePair("executablePath", std::string(exec_path)));
  free(exec_path);
  return true;
}

bool TizenAppInfo::GetSharedResourcePath(flutter::EncodableMap &value) {
  char *shared_res_path = nullptr;

  int ret = app_manager_get_shared_resource_path(app_id_, &shared_res_path);
  if (ret != APP_MANAGER_ERROR_NONE || shared_res_path == nullptr) {
    LOG_ERROR("get shared resource path error! : %s", get_error_message(ret));
    if (shared_res_path) {
      free(shared_res_path);
    }
    last_error_ = ret;
    return false;
  }
  value.insert(FlValuePair("sharedResourcePath", std::string(shared_res_path)));
  free(shared_res_path);
  return true;
}

bool TizenAppInfo::GetIsNoDisplay(flutter::EncodableMap &value) {
  bool no_display = false;

  int ret = app_info_is_nodisplay(app_info_, &no_display);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("app_info_is_nodisplay error! : %s", get_error_message(ret));
    last_error_ = ret;
    return false;
  }
  value.insert(FlValuePair("isNoDisplay", no_display));
  return true;
}

bool TizenAppInfo::GetForEachMetadata(flutter::EncodableMap &value) {
  flutter::EncodableMap metadata;

  int ret = app_info_foreach_metadata(
      app_info_,
      [](const char *key, const char *value, void *user_data) {
        if (key) {
          flutter::EncodableMap *metadata =
              static_cast<flutter::EncodableMap *>(user_data);
          metadata->insert(FlValuePair(std::string(key), std::string(value)));
          LOG_INFO("AppMetaDataCB key: %s, value: %s", key, value);
        }
        return true;
      },
      static_cast<void *>(&metadata));
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("app_info_foreach_metadata error! : %s", get_error_message(ret));
    last_error_ = ret;
    return false;
  }
  value.insert(FlValueMapPair("metadata", metadata));
  return true;
}

TizenAppInfo::~TizenAppInfo() {
  if (app_id_) {
    free(app_id_);
  }
}
