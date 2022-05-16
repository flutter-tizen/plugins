// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "setting.h"

#include <app_common.h>
#include <app_control.h>
#include <package_manager.h>

namespace {
  constexpr char kSettingAppId[] = "com.samsung.clocksetting.apps";
  constexpr char kLocationSettingAppId[] = "com.samsung.setting-location";

class AppControl {
 public:
  AppControl() { app_control_create(&app_control_); }

  ~AppControl() { app_control_destroy(app_control_); }

  TizenResult AddExtraData(const std::string& key, const std::string& value) {
    return app_control_add_extra_data(app_control_, key.c_str(), value.c_str());
  }

  TizenResult SetAppId(const std::string& id) {
    return app_control_set_app_id(app_control_, id.c_str());
  }

  TizenResult SetOperation(const std::string& operation) {
    return app_control_set_operation(app_control_, operation.c_str());
  }

  TizenResult SendLauchRequest() {
    return app_control_send_launch_request(app_control_, nullptr, nullptr);
  }

 private:
  app_control_h app_control_{nullptr};
};

class PackageName {
 public:
  PackageName() { Init(); }

  ~PackageName() {}

  operator std::string() const { return package_name_; }

 private:
  void Init() {
    char* id = nullptr;
    std::string app_id;
    if (app_get_id(&id) != 0) {
      return;
    }

    app_id = id;
    free(id);

    char* package_name = nullptr;
    package_info_h package_info = nullptr;

    package_info_create(app_id.c_str(), &package_info);

    if (package_info_get_package(package_info, &package_name) != 0) {
      return;
    }

    package_info_destroy(package_info);
    package_name_ = package_name;

    free(package_name);
  }

  std::string package_name_;
};

}  // namespace

TizenResult Setting::LaunchAppSetting() {
  PackageName name;
  AppControl app_ctrl;

  app_ctrl.SetAppId(kSettingAppId);
  app_ctrl.AddExtraData("pkgId", name);

  return app_ctrl.SendLauchRequest();
}

TizenResult Setting::LaunchLocationSetting() {
  AppControl app_ctrl;

  app_ctrl.SetAppId(kLocationSettingAppId);
  app_ctrl.SetOperation(
      "http://tizen.org/appcontrol/operation/configure/location");

  return app_ctrl.SendLauchRequest();
}
