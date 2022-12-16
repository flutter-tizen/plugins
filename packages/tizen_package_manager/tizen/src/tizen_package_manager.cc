// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_package_manager.h"

#include "log.h"

TizenPackageManager::TizenPackageManager() {
  int ret = package_manager_create(&package_manager_);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("package_manager_create failed: %s", get_error_message(ret));
    last_error_ = ret;
    return;
  }

  ret = package_manager_set_event_status(
      package_manager_, PACKAGE_MANAGER_STATUS_TYPE_INSTALL |
                            PACKAGE_MANAGER_STATUS_TYPE_UNINSTALL |
                            PACKAGE_MANAGER_STATUS_TYPE_UPGRADE);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("package_manager_set_event_status failed: %s",
              get_error_message(ret));
    last_error_ = ret;
    return;
  }

  ret = package_manager_set_event_cb(
      package_manager_,
      [](const char *type, const char *package,
         package_manager_event_type_e event_type,
         package_manager_event_state_e event_state, int progress,
         package_manager_error_e error, void *user_data) {
        auto *self = static_cast<TizenPackageManager *>(user_data);

        PacakgeEventState state;
        switch (event_state) {
          case PACKAGE_MANAGER_EVENT_STATE_STARTED:
            state = PacakgeEventState::kStarted;
          case PACKAGE_MANAGER_EVENT_STATE_PROCESSING:
            state = PacakgeEventState::kProcessing;
          case PACKAGE_MANAGER_EVENT_STATE_FAILED:
            state = PacakgeEventState::kFailed;
          case PACKAGE_MANAGER_EVENT_STATE_COMPLETED:
          default:
            state = PacakgeEventState::kCompleted;
        }

        if (event_type == PACKAGE_MANAGER_EVENT_TYPE_INSTALL) {
          if (self->install_callback_) {
            self->install_callback_(package, type, state, progress);
          }
        } else if (event_type == PACKAGE_MANAGER_EVENT_TYPE_UNINSTALL) {
          if (self->uninstall_callback_) {
            self->uninstall_callback_(package, type, state, progress);
          }
        } else if (event_type == PACKAGE_MANAGER_EVENT_TYPE_UPDATE) {
          if (self->update_callback_) {
            self->update_callback_(package, type, state, progress);
          }
        }
      },
      this);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("package_manager_set_event_cb failed: %s",
              get_error_message(ret));
    last_error_ = ret;
  }
}

TizenPackageManager::~TizenPackageManager() {
  if (package_manager_) {
    package_manager_unset_event_cb(package_manager_);
    package_manager_destroy(package_manager_);
  }
}

std::optional<PackageInfo> TizenPackageManager::GetPackageData(
    package_info_h handle) {
  PackageInfo result = {};

  char *name = nullptr;
  int ret = package_info_get_package(handle, &name);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("package_info_get_package failed: %s", get_error_message(ret));
    last_error_ = ret;
    return std::nullopt;
  }
  result.package_id = name;
  free(name);

  char *label = nullptr;
  ret = package_info_get_label(handle, &label);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("package_info_get_label failed: %s", get_error_message(ret));
    last_error_ = ret;
    return std::nullopt;
  }
  result.label = label;
  free(label);

  char *type = nullptr;
  ret = package_info_get_type(handle, &type);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("package_info_get_type failed: %s", get_error_message(ret));
    last_error_ = ret;
    return std::nullopt;
  }
  result.type = type;
  free(type);

  char *icon_path = nullptr;
  ret = package_info_get_icon(handle, &icon_path);
  if (icon_path) {
    result.icon_path = icon_path;
    free(icon_path);
  }

  char *version = nullptr;
  ret = package_info_get_version(handle, &version);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("package_info_get_version failed: %s", get_error_message(ret));
    last_error_ = ret;
    return std::nullopt;
  }
  result.version = version;
  free(version);

  package_info_installed_storage_type_e storage_type =
      PACKAGE_INFO_INTERNAL_STORAGE;
  ret = package_info_get_installed_storage(handle, &storage_type);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("package_info_get_installed_storage failed: %s",
              get_error_message(ret));
    last_error_ = ret;
    return std::nullopt;
  }
  if (storage_type == PACKAGE_INFO_EXTERNAL_STORAGE) {
    result.installed_storage_type = "external";
  } else {
    result.installed_storage_type = "internal";
  }

  ret = package_info_is_system_package(handle, &result.is_system);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("package_info_is_system_package failed: %s",
              get_error_message(ret));
    last_error_ = ret;
    return std::nullopt;
  }

  ret = package_info_is_preload_package(handle, &result.is_preloaded);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("package_info_is_preload_package failed: %s",
              get_error_message(ret));
    last_error_ = ret;
    return std::nullopt;
  }

  ret = package_info_is_removable_package(handle, &result.is_removable);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("package_info_is_removable_package failed: %s",
              get_error_message(ret));
    last_error_ = ret;
    return std::nullopt;
  }

  return result;
}

std::optional<PackageInfo> TizenPackageManager::GetPackageInfo(
    const std::string &package_id) {
  package_info_h handle = nullptr;
  int ret = package_info_create(package_id.c_str(), &handle);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("package_info_create failed: %s", get_error_message(ret));
    last_error_ = ret;
    return std::nullopt;
  }
  std::optional<PackageInfo> package = GetPackageData(handle);
  package_info_destroy(handle);
  return package;
}

std::optional<std::vector<PackageInfo>>
TizenPackageManager::GetAllPackagesInfo() {
  packages_.clear();
  last_error_ = PACKAGE_MANAGER_ERROR_NONE;

  int ret = package_manager_foreach_package_info(
      [](package_info_h handle, void *user_data) -> bool {
        auto *self = static_cast<TizenPackageManager *>(user_data);
        if (handle) {
          std::optional<PackageInfo> package = self->GetPackageData(handle);
          if (package.has_value()) {
            self->packages_.push_back(package.value());
          } else {
            return false;
          }
        }
        return true;
      },
      this);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("package_manager_foreach_package_info failed: %s",
              get_error_message(ret));
    last_error_ = ret;
    return std::nullopt;
  }

  if (last_error_ != PACKAGE_MANAGER_ERROR_NONE) {
    // GetPackageData() failed during the iteration.
    return std::nullopt;
  }
  return packages_;
}

bool TizenPackageManager::Install(const std::string &package_path) {
  package_manager_request_h request = nullptr;
  int ret = package_manager_request_create(&request);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("package_manager_request_create failed: %s",
              get_error_message(ret));
    last_error_ = ret;
    return false;
  }

  int request_id = 0;
  ret = package_manager_request_install(request, package_path.c_str(),
                                        &request_id);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    package_manager_request_destroy(request);
    LOG_ERROR("package_manager_request_install failed: %s",
              get_error_message(ret));
    last_error_ = ret;
    return false;
  }
  package_manager_request_destroy(request);

  return true;
}

bool TizenPackageManager::Uninstall(const std::string &package_id) {
  package_manager_request_h request = nullptr;
  int ret = package_manager_request_create(&request);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("package_manager_request_create failed: %s",
              get_error_message(ret));
    last_error_ = ret;
    return false;
  }

  ret = package_manager_request_set_type(request, "unknown");
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    package_manager_request_destroy(request);
    LOG_ERROR("package_manager_request_set_type failed: %s",
              get_error_message(ret));
    last_error_ = ret;
    return false;
  }

  int request_id = 0;
  ret = package_manager_request_uninstall(request, package_id.c_str(),
                                          &request_id);
  if (ret != PACKAGE_MANAGER_ERROR_NONE) {
    LOG_ERROR("package_manager_request_uninstall failed: %s",
              get_error_message(ret));
    package_manager_request_destroy(request);
    last_error_ = ret;
    return false;
  }
  package_manager_request_destroy(request);

  return true;
}
