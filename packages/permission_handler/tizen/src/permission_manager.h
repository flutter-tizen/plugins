// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_PERMISSION_MANAGER_H_
#define FLUTTER_PLUGIN_PERMISSION_MANAGER_H_

#include <string>

// The result of permission check and request.
//
// Originally defined in permission_status.dart of the platform interface
// package.
enum class PermissionStatus {
  kDenied = 0,
  kGranted = 1,
  kRestricted = 2,
  kLimited = 3,
  kPermanentlyDenied = 4,
  kError = 5,
};

class PermissionManager {
 public:
  PermissionManager() {}
  ~PermissionManager() {}

  PermissionStatus CheckPermission(const std::string &privilege);

  PermissionStatus RequestPermssion(const std::string &privilege);
};

#endif  // FLUTTER_PLUGIN_PERMISSION_MANAGER_H_
