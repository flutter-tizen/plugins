// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "locaton_manager.h"

#include "log.h"

LocationManager::LocationManager() { CreateLocationManager(); }

LocationManager::~LocationManager() { DestroyLocationManager(); }

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

TizenResult LocationManager::RequestCurrentLocationOnce(
    OnLocationUpdate on_position_update, OnError on_error) {
  on_position_update_ = on_position_update;
  on_error_ = on_error;
  TizenResult ret = location_manager_request_single_location(
      manager_, 5,
      [](location_error_e error, double latitude, double longitude,
         double altitude, time_t timestamp, double speed, double direction,
         double climb, void *user_data) {
        LocationManager *self = static_cast<LocationManager *>(user_data);

        if (error != LOCATIONS_ERROR_NONE && self->on_error_) {
          self->on_error_(error);
        } else if (self->on_position_update_) {
          Location location;
          location.latitude = latitude;
          location.longitude = longitude;
          location.altitude = altitude;
          location.timestamp = timestamp;
          location.speed = speed;
          location.heading = direction;

          self->on_position_update_(location);
        }

        self->on_error_ = nullptr;
        self->on_position_update_ = nullptr;
      },
      this);
  return ret;
}

TizenResult LocationManager::CreateLocationManager() {
  return location_manager_create(LOCATIONS_METHOD_HYBRID, &manager_);
}

TizenResult LocationManager::DestroyLocationManager() {
  int ret = location_manager_destroy(manager_);
  manager_ = nullptr;
  return TizenResult(ret);
}
