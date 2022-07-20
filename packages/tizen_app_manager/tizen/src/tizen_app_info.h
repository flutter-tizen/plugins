// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_TIZEN_APP_INFO_H_
#define FLUTTER_PLUGIN_TIZEN_APP_INFO_H_

#include <app_manager.h>

#include <map>
#include <optional>
#include <string>

class TizenAppInfo {
 public:
  TizenAppInfo(app_info_h app_info) : app_info_(app_info) {}

  ~TizenAppInfo();

  std::optional<std::string> GetAppId();

  std::optional<std::string> GetPackageId();

  std::optional<std::string> GetLabel();

  std::optional<std::string> GetType();

  std::optional<std::string> GetIconPath();

  std::optional<std::string> GetExecutablePath();

  std::optional<bool> IsNoDisplay();

  std::map<std::string, std::string> GetMetadata();

  int GetLastError() { return last_error_; }

  std::string GetLastErrorString() { return get_error_message(last_error_); }

 private:
  app_info_h app_info_ = nullptr;
  int last_error_ = APP_MANAGER_ERROR_NONE;
};

#endif  // FLUTTER_PLUGIN_TIZEN_APP_INFO_H_
