// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_PERMISSION_MANAGER_H_
#define FLUTTER_PLUGIN_PERMISSION_MANAGER_H_

#include <tizen_error.h>

#include <string>

// Keep in sync with the enum values implemented in:
// https://github.com/Baseflow/flutter-geolocator/blob/master/geolocator_platform_interface/lib/src/enums/location_permission.dart
// https://github.com/Baseflow/flutter-geolocator/blob/master/geolocator_android/android/src/main/java/com/baseflow/geolocator/permission/LocationPermission.java
enum class PermissionStatus {
  kDenied = 0,
  kDeniedForever = 1,
  kWhileInUse = 2,
  kAlways = 3,
};

class PermissionManagerError {
 public:
  PermissionManagerError(std::string error_message)
      : error_message_(error_message) {}
  PermissionManagerError(int error_code)
      : error_message_(get_error_message(error_code)) {}
  std::string GetErrorString() const { return error_message_; }

 private:
  std::string error_message_;
};

class PermissionManager {
 public:
  PermissionManager() {}
  ~PermissionManager() {}

  PermissionStatus CheckPermission(const std::string &privilege);

  PermissionStatus RequestPermission(const std::string &privilege);
};

#endif  // FLUTTER_PLUGIN_PERMISSION_MANAGER_H_
