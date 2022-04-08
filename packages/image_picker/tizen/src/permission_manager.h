// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_PERMISSION_MANAGER_H_
#define FLUTTER_PLUGIN_PERMISSION_MANAGER_H_

#include <string>

// The result of permission check.
enum class PermissionStatus { kAllow, kDeny, kAsk, kError };

// The result of permission request.
enum class PermissionResult { kAllowForever, kDenyForever, kDenyOnce, kError };

class PermissionManager {
 public:
  PermissionManager() {}
  ~PermissionManager() {}

  PermissionStatus CheckPermission(const std::string &privilege);

  PermissionResult RequestPermssion(const std::string &privilege);
};

#endif  // FLUTTER_PLUGIN_PERMISSION_MANAGER_H_
