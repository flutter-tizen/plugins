// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "package_manager_utils.h"

namespace package_manager_utils {

bool ExtractValueFromMap(const flutter::EncodableValue &arguments,
                         const char *key, std::string &out_value) {
  if (std::holds_alternative<flutter::EncodableMap>(arguments)) {
    flutter::EncodableMap map = std::get<flutter::EncodableMap>(arguments);
    auto iter = map.find(flutter::EncodableValue(key));
    if (iter != map.end() && !iter->second.IsNull()) {
      if (auto pval = std::get_if<std::string>(&iter->second)) {
        out_value = *pval;
        return true;
      }
    }
  }
  return false;
}

int GetPackageData(package_info_h package_info, flutter::EncodableMap &value) {
  char *pkg_name = nullptr;
  char *label = nullptr;
  char *type = nullptr;
  char *icon_path = nullptr;
  char *version = nullptr;
  package_info_installed_storage_type_e installed_storage_type =
      PACKAGE_INFO_INTERNAL_STORAGE;
  bool is_system = false;
  bool is_preloaded = false;
  bool is_removable = true;

  int ret = package_info_get_package(package_info, &pkg_name);
  if (ret != PACKAGE_MANAGER_ERROR_NONE || pkg_name == nullptr) {
    LOG_ERROR("get package name error! : %s", get_error_message(ret));
    goto cleanup;
  }

  ret = package_info_get_label(package_info, &label);
  if (ret != PACKAGE_MANAGER_ERROR_NONE || label == nullptr) {
    LOG_ERROR("get package label error! : %s", get_error_message(ret));
    goto cleanup;
  }

  ret = package_info_get_type(package_info, &type);
  if (ret != PACKAGE_MANAGER_ERROR_NONE || type == nullptr) {
    LOG_ERROR("get package type error! : %s", get_error_message(ret));
    goto cleanup;
  }

  ret = package_info_get_icon(package_info, &icon_path);
  if (ret != PACKAGE_MANAGER_ERROR_NONE || icon_path == nullptr) {
    // because some service app doesn't have icon,
    // just print error log, and pass it
    LOG_ERROR("get icon path error! : %s", get_error_message(ret));
  }

  ret = package_info_get_version(package_info, &version);
  if (ret != PACKAGE_MANAGER_ERROR_NONE || version == nullptr) {
    LOG_ERROR("get version error! : %s", get_error_message(ret));
    goto cleanup;
  }

  ret =
      package_info_get_installed_storage(package_info, &installed_storage_type);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("get installed storage error! : %s", get_error_message(ret));
    goto cleanup;
  }

  ret = package_info_is_system_package(package_info, &is_system);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("check system package error! : %s", get_error_message(ret));
    goto cleanup;
  }

  ret = package_info_is_preload_package(package_info, &is_preloaded);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("check preload package error! : %s", get_error_message(ret));
    goto cleanup;
  }

  ret = package_info_is_removable_package(package_info, &is_removable);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("check removable package error! : %s", get_error_message(ret));
    goto cleanup;
  }

  value[flutter::EncodableValue("packageId")] =
      flutter::EncodableValue(std::string(pkg_name));
  value[flutter::EncodableValue("label")] =
      flutter::EncodableValue(std::string(label));
  value[flutter::EncodableValue("label")] =
      flutter::EncodableValue(std::string(label));
  value[flutter::EncodableValue("version")] =
      flutter::EncodableValue(std::string(version));
  value[flutter::EncodableValue("installedStorageType")] =
      flutter::EncodableValue(StorageTypeToString(installed_storage_type));
  value[flutter::EncodableValue("isSystem")] =
      flutter::EncodableValue(is_system);
  value[flutter::EncodableValue("isPreloaded")] =
      flutter::EncodableValue(is_preloaded);
  value[flutter::EncodableValue("isRemovable")] =
      flutter::EncodableValue(is_removable);
  if (icon_path != nullptr) {
    value[flutter::EncodableValue("iconPath")] =
        flutter::EncodableValue(std::string(icon_path));
  }

cleanup:
  if (pkg_name) {
    free(pkg_name);
  }
  if (label) {
    free(label);
  }
  if (type) {
    free(type);
  }
  if (icon_path) {
    free(icon_path);
  }
  if (version) {
    free(version);
  }

  return ret;
}

std::string StorageTypeToString(package_info_installed_storage_type_e value) {
  switch (value) {
    case PACKAGE_INFO_EXTERNAL_STORAGE:
      return "External storage";
    case PACKAGE_INFO_INTERNAL_STORAGE:
    default:
      return "Internal storage";
  }
}

std::string PacakgeEventTypeToString(package_manager_event_type_e type) {
  switch (type) {
    case PACKAGE_MANAGER_EVENT_TYPE_UNINSTALL:
      return "uninstall";
    case PACKAGE_MANAGER_EVENT_TYPE_UPDATE:
      return "update";
    case PACKAGE_MANAGER_EVENT_TYPE_MOVE:
      return "move";
    case PACKAGE_MANAGER_EVENT_TYPE_CLEAR:
      return "cleardata";
    case PACKAGE_MANAGER_EVENT_TYPE_INSTALL:
    default:
      return "install";
  }
}

std::string PacakgeEventStateToString(package_manager_event_state_e state) {
  switch (state) {
    case PACKAGE_MANAGER_EVENT_STATE_STARTED:
      return "started";
    case PACKAGE_MANAGER_EVENT_STATE_PROCESSING:
      return "processing";
    case PACKAGE_MANAGER_EVENT_STATE_FAILED:
      return "failed";
    case PACKAGE_MANAGER_EVENT_STATE_COMPLETED:
    default:
      return "completed";
  }
}

}  // namespace package_manager_utils
