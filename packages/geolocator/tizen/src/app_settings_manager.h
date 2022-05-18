// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_APP_SETTINGS_MANAGER_H_
#define FLUTTER_PLUGIN_APP_SETTINGS_MANAGER_H_

#include <app_control.h>

#include <string>

class AppSettingsManager {
 public:
  AppSettingsManager() : app_control_(nullptr) {}
  ~AppSettingsManager() {}

  bool OpenAppSettings();
  bool OpenLocationSetting();

 private:
  bool CreateAppControl();
  bool SetAppId(const char* app_id);
  bool SetOperation(const char* operation);
  bool AddExtraData(std::string package_name);
  bool SendLauchRequest();
  void DestroyAppControl();

  app_control_h app_control_;
};

#endif  // FLUTTER_PLUGIN_APP_SETTINGS_MANAGER_H_