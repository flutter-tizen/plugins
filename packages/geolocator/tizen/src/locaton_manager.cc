// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "locaton_manager.h"

#include <locations.h>

LocationManager::LocationManager() {}

LocationManager::~LocationManager() {}

TizenResult LocationManager::IsLocationServiceEnabled(bool *is_enabled) {
  bool gps_enabled = false;
  int ret_gps =
      location_manager_is_enabled_method(LOCATIONS_METHOD_GPS, &gps_enabled);
  if (ret_gps != LOCATIONS_ERROR_NONE) {
    return TizenResult(ret_gps);
  }

  bool wps_enabled = false;
  int ret_wps =
      location_manager_is_enabled_method(LOCATIONS_METHOD_WPS, &wps_enabled);
  if (ret_wps != LOCATIONS_ERROR_NONE) {
    return TizenResult(ret_wps);
  }

  *is_enabled = gps_enabled || wps_enabled;

  return TizenResult();
}
