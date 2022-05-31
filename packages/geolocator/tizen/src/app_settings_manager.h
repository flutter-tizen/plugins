// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_APP_SETTINGS_MANAGER_H_
#define FLUTTER_PLUGIN_APP_SETTINGS_MANAGER_H_

class AppSettingsManager {
 public:
  AppSettingsManager() {}
  ~AppSettingsManager() {}

  bool OpenAppSettings();
  bool OpenLocationSettings();
};

#endif  // FLUTTER_PLUGIN_APP_SETTINGS_MANAGER_H_
