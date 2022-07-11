// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_TIZEN_APP_INFO_H_
#define FLUTTER_PLUGIN_TIZEN_APP_INFO_H_

#include <app_manager.h>

#include <map>
#include <string>

class TizenAppInfo {
 public:
  int GetLastError() { return last_error_; }

  std::string GetLastErrorString() { return get_error_message(last_error_); }

  TizenAppInfo(app_info_h app_info) : app_info_(app_info) {}

  ~TizenAppInfo();

  std::string GetAppId();
  std::string GetPackageId();
  std::string GetLabel();
  std::string GetType();
  std::string GetIconPath();
  std::string GetExecutablePath();
  std::string GetSharedResourcePath();
  bool GetIsNoDisplay();
  void GetForEachMetadata(app_info_metadata_cb callback, void *user_data);

 private:
  app_info_h app_info_ = nullptr;
  char *app_id_ = nullptr;
  int last_error_ = APP_MANAGER_ERROR_NONE;
};

#endif
