// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_app_manager.h"

#include "log.h"

TizenAppManager::TizenAppManager() {
  int ret = app_manager_set_app_context_event_cb(
      [](app_context_h app_context, app_context_event_e event,
         void *user_data) {
        auto *self = static_cast<TizenAppManager *>(user_data);

        char *app_id = nullptr;
        int ret = app_context_get_app_id(app_context, &app_id);
        if (ret != APP_MANAGER_ERROR_NONE) {
          LOG_ERROR("Failed to get app ID from the context: %s",
                    get_error_message(ret));
          return;
        }
        std::string app_id_str = std::string(app_id);
        free(app_id);

        app_context_h clone_context = nullptr;
        ret = app_context_clone(&clone_context, app_context);
        if (ret != APP_MANAGER_ERROR_NONE) {
          LOG_ERROR("Failed to clone app context: %s", get_error_message(ret));
          return;
        }

        if (event == APP_CONTEXT_EVENT_LAUNCHED) {
          if (self->launch_callback_) {
            self->launch_callback_(app_id_str, clone_context);
            return;
          }
        } else if (event == APP_CONTEXT_EVENT_TERMINATED) {
          if (self->terminate_callback_) {
            self->terminate_callback_(app_id_str, clone_context);
            return;
          }
        }
        // No callback has been registered. Destroy the context immediately.
        app_context_destroy(clone_context);
      },
      this);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("Failed to set an app context event callback: %s",
              get_error_message(ret));
    last_error_ = ret;
  }
}

TizenAppManager::~TizenAppManager() {
  app_manager_unset_app_context_event_cb();
}

std::unique_ptr<TizenAppInfo> TizenAppManager::GetAppInfo(
    const std::string &app_id) {
  app_info_h app_info;
  int ret = app_manager_get_app_info(app_id.c_str(), &app_info);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("Failed to retrieve app info: %s", get_error_message(ret));
    last_error_ = ret;
    return nullptr;
  }
  return std::make_unique<TizenAppInfo>(app_info);
}

std::vector<std::unique_ptr<TizenAppInfo>> TizenAppManager::GetAllAppsInfo() {
  std::vector<std::unique_ptr<TizenAppInfo>> list;
  int ret = app_manager_foreach_app_info(
      [](app_info_h app_info, void *user_data) {
        auto *list = static_cast<std::vector<std::unique_ptr<TizenAppInfo>> *>(
            user_data);

        app_info_h clone_info = nullptr;
        int ret = app_info_clone(&clone_info, app_info);
        if (ret != APP_MANAGER_ERROR_NONE) {
          LOG_ERROR("Failed to clone app info: %s", get_error_message(ret));
          return true;
        }

        list->push_back(std::make_unique<TizenAppInfo>(clone_info));
        return true;
      },
      &list);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("Failed to retrieve all app info: %s", get_error_message(ret));
    last_error_ = ret;
  }
  return list;
}

std::optional<std::string> TizenAppManager::GetSharedResourcePath(
    const std::string &app_id) {
  char *path = nullptr;
  int ret = app_manager_get_shared_resource_path(app_id.c_str(), &path);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("Failed to get shared resource path: %s", get_error_message(ret));
    last_error_ = ret;
    return std::nullopt;
  }
  std::string result = std::string(path);
  free(path);
  return result;
}

std::optional<bool> TizenAppManager::IsAppRunning(const std::string &app_id) {
  bool is_running = false;
  int ret = app_manager_is_running(app_id.c_str(), &is_running);
  if (ret != APP_MANAGER_ERROR_NONE) {
    LOG_ERROR("Failed to check if app is running: %s", get_error_message(ret));
    last_error_ = ret;
    return std::nullopt;
  }
  return is_running;
}
