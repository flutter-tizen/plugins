// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "service_manager.h"

#include <locations.h>

ServiceStatus ServiceManager::CheckServiceStatus(Permission permission) {
  if (permission == Permission::kLocation ||
      permission == Permission::kLocationAlways ||
      permission == Permission::kLocationWhenInUse) {
    bool gps_enabled, wps_enabled;
    if (location_manager_is_enabled_method(
            LOCATIONS_METHOD_GPS, &gps_enabled) != LOCATIONS_ERROR_NONE) {
      gps_enabled = false;
    }
    if (location_manager_is_enabled_method(
            LOCATIONS_METHOD_WPS, &wps_enabled) != LOCATIONS_ERROR_NONE) {
      wps_enabled = false;
    }

    if (gps_enabled || wps_enabled) {
      return ServiceStatus::kEnabled;
    } else {
      return ServiceStatus::kDisabled;
    }
  }
  return ServiceStatus::kNotApplicable;
}
