// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "application_utils.h"

namespace application_utils {

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

bool AppMetaDataCB(const char *key, const char *value, void *user_data) {
  if (key != nullptr) {
    flutter::EncodableMap *metadata = (flutter::EncodableMap *)user_data;
    metadata->insert(
        std::pair<flutter::EncodableValue, flutter::EncodableValue>(
            std::string(key), std::string(value)));
    LOG_INFO("AppMetaDataCB key: %s, value: %s", key, value);
  }
  return true;
}

int GetAppData(app_info_h app_info, flutter::EncodableMap &value) {
  char *app_id = nullptr;
  char *pkg_id = nullptr;
  char *label = nullptr;
  char *type = nullptr;
  char *icon_path = nullptr;
  char *exec_path = nullptr;
  char *shared_res_path = nullptr;
  bool no_display = false;
  flutter::EncodableMap metadata;

  int ret = app_info_get_app_id(app_info, &app_id);
  LOG_INFO("GetAppData %s", app_id);
  if (ret != APP_MANAGER_ERROR_NONE || app_id == nullptr) {
    LOG_ERROR("get app Id error! : %s", get_error_message(ret));
    goto cleanup;
  }

  ret = app_info_get_package(app_info, &pkg_id);
  if (ret != APP_MANAGER_ERROR_NONE || pkg_id == nullptr) {
    LOG_ERROR("get package Id error! : %s", get_error_message(ret));
    goto cleanup;
  }

  ret = app_info_get_label(app_info, &label);
  if (ret != APP_MANAGER_ERROR_NONE || label == nullptr) {
    LOG_ERROR("get application label error! : %s", get_error_message(ret));
    goto cleanup;
  }

  ret = app_info_get_type(app_info, &type);
  if (ret != APP_MANAGER_ERROR_NONE || type == nullptr) {
    LOG_ERROR("get application type error! : %s", get_error_message(ret));
    goto cleanup;
  }

  ret = app_info_get_icon(app_info, &icon_path);
  if (ret != APP_MANAGER_ERROR_NONE || icon_path == nullptr) {
    // because some system app and service app don't have icon,
    // just print error log, and pass it
    LOG_ERROR("get icon path error! : %s", get_error_message(ret));
  }

  ret = app_info_get_exec(app_info, &exec_path);
  if (ret != APP_MANAGER_ERROR_NONE || exec_path == nullptr) {
    LOG_ERROR("get exec path error! : %s", get_error_message(ret));
    goto cleanup;
  }

  ret = app_manager_get_shared_resource_path(app_id, &shared_res_path);
  if (ret != APP_MANAGER_ERROR_NONE || shared_res_path == nullptr) {
    LOG_ERROR("get shared resource path error! : %s", get_error_message(ret));
    goto cleanup;
  }

  ret = app_info_is_nodisplay(app_info, &no_display);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("app_info_is_nodisplay error! : %s", get_error_message(ret));
    goto cleanup;
  }

  ret = app_info_foreach_metadata(app_info, AppMetaDataCB, (void *)&metadata);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("app_info_foreach_metadata error! : %s", get_error_message(ret));
    goto cleanup;
  }

  value.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
      "appId", std::string(app_id)));
  value.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
      "packageId", std::string(pkg_id)));
  value.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
      "label", std::string(label)));
  value.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
      "type", std::string(type)));
  if (icon_path != nullptr) {
    value.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
        "iconPath", std::string(icon_path)));
  }
  value.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
      "executablePath", std::string(exec_path)));
  value.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
      "sharedResourcePath", std::string(shared_res_path)));
  value.insert(std::pair<flutter::EncodableValue, flutter::EncodableValue>(
      "isNoDisplay", no_display));
  value.insert(std::pair<flutter::EncodableValue, flutter::EncodableMap>(
      "metadata", metadata));

cleanup:
  if (app_id) {
    free(app_id);
  }
  if (pkg_id) {
    free(pkg_id);
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
  if (exec_path) {
    free(exec_path);
  }
  if (shared_res_path) {
    free(shared_res_path);
  }

  return ret;
}

std::string AppStateToString(app_state_e state) {
  switch (state) {
    case APP_STATE_TERMINATED:
      return "terminated";
    case APP_STATE_FOREGROUND:
      return "foreground";
    case APP_STATE_BACKGROUND:
      return "background";
    case APP_STATE_SERVICE:
      return "service";
    case APP_STATE_UNDEFINED:
    default:
      return "undefined";
  }
}

std::string AppContextEventToString(app_context_event_e event) {
  switch (event) {
    case APP_CONTEXT_EVENT_LAUNCHED:
      return "launched";
    case APP_CONTEXT_EVENT_TERMINATED:
    default:
      return "terminated";
  }
}

}  // namespace application_utils
