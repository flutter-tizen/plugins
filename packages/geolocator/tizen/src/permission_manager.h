// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PERMISSION_MANAGER_H_
#define PERMISSION_MANAGER_H_

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

class PermissionManager {
 public:
  PermissionManager();
  ~PermissionManager();

  TizenResult CheckPermissionStatus(PermissionStatus *permission_status);
};

#endif  // PERMISSION_MANAGER_H_
