#include "app_settings_manager.h"

#include <app_common.h>
#include <app_control.h>
#include <package_manager.h>

#include "log.h"

namespace {
constexpr char kPackageID[] = "pkgId";
constexpr char kSettingAppID[] = "com.samsung.clocksetting.apps";

class AppPermissions {
 public:
  AppPermissions() { Init(); }

  ~AppPermissions() { Deinit(); }

  bool Launch(const std::string& package_name) {
    if (package_name.length() == 0) {
      return false;
    }

    int ret = app_control_add_extra_data(app_control_, kPackageID,
                                         package_name.c_str());
    if (ret != APP_CONTROL_ERROR_NONE) {
      LOG_ERROR("Failed to add key[%s]:value[%s]", kPackageID,
                package_name.c_str());
      return false;
    }

    ret = app_control_send_launch_request(app_control_, nullptr, nullptr);
    if (ret != APP_CONTROL_ERROR_NONE) {
      LOG_ERROR("Failed to send launch request setting");
      return false;
    }
    return true;
  }

 private:
  void Init() {
    int ret = app_control_create(&app_control_);
    if (ret != APP_CONTROL_ERROR_NONE) {
      LOG_ERROR("Failed to create app control handle");
    }

    ret = app_control_set_app_id(app_control_, kSettingAppID);
    if (ret != APP_CONTROL_ERROR_NONE) {
      LOG_ERROR("Failed to set app id[%s]", kSettingAppID);
    }
  }

  void Deinit() {
    if (app_control_) {
      app_control_destroy(app_control_);
      app_control_ = nullptr;
    }
  }

  app_control_h app_control_{nullptr};
};

class PackageName {
 public:
  PackageName() { Init(); }

  ~PackageName() { Deinit(); }

  char* Get() { return package_name_; }

 private:
  void Init() {
    int ret = app_get_id(&app_id_);
    if (ret != APP_ERROR_NONE || app_id_ == nullptr) {
      LOG_ERROR("Failed to get app id");
      return;
    }

    ret = package_info_create(app_id_, &package_info_);
    if (ret != PACKAGE_MANAGER_ERROR_NONE || package_info_ == nullptr) {
      LOG_ERROR("Failed to create package info handle");
    }
    ret = package_info_get_package(package_info_, &package_name_);
    if (ret != PACKAGE_MANAGER_ERROR_NONE || package_name_ == nullptr) {
      LOG_ERROR("Failed to get package name");
    }
  }

  void Deinit() {
    if (app_id_) {
      free(app_id_);
      app_id_ = nullptr;
    }

    if (package_info_) {
      package_info_destroy(package_info_);
      package_info_ = nullptr;
    }

    if (package_name_) {
      free(package_name_);
      package_name_ = nullptr;
    }
  }

  char* app_id_{nullptr};
  package_info_h package_info_{nullptr};
  char* package_name_{nullptr};
};
}  // namespace

AppSettingsManager::AppSettingsManager() {
  app_permissions_ = std::make_unique<AppPermissions>();
  PackageName pkg_name;
  char* name = pkg_name.Get();
  if (name == nullptr) {
    return;
  }
  package_name_ = std::string(name);
}

AppSettingsManager::~AppSettingsManager() {}

bool AppSettingsManager::OpenAppSettings() {
  return app_permissions_->Launch(package_name_);
}
