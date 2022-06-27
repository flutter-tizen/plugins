// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_TIZEN_APP_INFO_H_
#define FLUTTER_PLUGIN_TIZEN_APP_INFO_H_

#include <app_common.h>
#include <app_manager.h>
#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <string>

#include "log.h"

class TizenAppInfo {
 public:
  void GetAppInfo(flutter::EncodableMap &value);
  int GetLastError();

  TizenAppInfo(app_info_h app_info) : app_info_(app_info) {}

  ~TizenAppInfo();

 private:
  bool GetAppId(flutter::EncodableMap &value);
  bool GetPackageId(flutter::EncodableMap &value);
  bool GetLabel(flutter::EncodableMap &value);
  bool GetType(flutter::EncodableMap &value);
  void GetIconPath(flutter::EncodableMap &value);
  bool GetExecutablePath(flutter::EncodableMap &value);
  bool GetSharedResourcePath(flutter::EncodableMap &value);
  bool GetIsNoDisplay(flutter::EncodableMap &value);
  bool GetForEachMetadata(flutter::EncodableMap &value);

  app_info_h app_info_ = nullptr;
  char *app_id_ = nullptr;
  int last_error_ = APP_MANAGER_ERROR_NONE;
};

#endif
