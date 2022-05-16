// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_PERMISSION_MANAGER_H_
#define FLUTTER_PLUGIN_PERMISSION_MANAGER_H_

#include <functional>

#include "tizen_result.h"

// Keep in sync with the enum values implemented in:
// https://github.com/Baseflow/flutter-geolocator/blob/master/geolocator_platform_interface/lib/src/enums/location_permission.dart
// https://github.com/Baseflow/flutter-geolocator/blob/master/geolocator_android/android/src/main/java/com/baseflow/geolocator/permission/LocationPermission.java
enum class PermissionStatus {
  kDenied = 0,
  kDeniedForever = 1,
  kWhileInUse = 2,
  kAlways = 3,
};

using OnSuccess = std::function<void(PermissionStatus)>;
using OnFailure = std::function<void(TizenResult)>;
class PermissionManager {
 public:
  PermissionManager() {};
  ~PermissionManager() {};

  TizenResult CheckPermissionStatus(PermissionStatus *permission_status);

  void RequestPermssion(const OnSuccess &on_success,
                        const OnFailure &on_failure);
};

#endif  // FLUTTER_PLUGIN_PERMISSION_MANAGER_H_
