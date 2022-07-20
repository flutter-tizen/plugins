// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_TIZEN_APP_MANAGER_H_
#define FLUTTER_PLUGIN_TIZEN_APP_MANAGER_H_

#include <app_manager.h>

#include <functional>
#include <memory>
#include <string>

#include "tizen_app_info.h"

using OnAppContextEvent =
    std::function<void(std::string app_id, void* app_context_handle)>;

class TizenAppManager {
 public:
  ~TizenAppManager();

  // Returns a unique instance of TizenAppManager.
  static TizenAppManager& GetInstance() {
    static TizenAppManager instance;
    return instance;
  }

  std::unique_ptr<TizenAppInfo> GetAppInfo(const std::string& app_id);

  std::vector<std::unique_ptr<TizenAppInfo>> GetAllAppsInfo();

  std::optional<std::string> GetSharedResourcePath(const std::string& app_id);

  bool IsAppRunning(const std::string& app_id);

  void SetAppLaunchHandler(OnAppContextEvent on_launch) {
    launch_callback_ = on_launch;
  }

  void SetAppTerminateHandler(OnAppContextEvent on_terminate) {
    terminate_callback_ = on_terminate;
  }

  int GetLastError() { return last_error_; }

  std::string GetLastErrorString() { return get_error_message(last_error_); }

 private:
  explicit TizenAppManager();

  OnAppContextEvent launch_callback_;
  OnAppContextEvent terminate_callback_;
  int last_error_ = APP_MANAGER_ERROR_NONE;
};

#endif  // FLUTTER_PLUGIN_TIZEN_APP_MANAGER_H_
