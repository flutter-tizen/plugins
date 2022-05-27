// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "location_manager.h"

#include "log.h"

LocationManager::LocationManager() {
  location_manager_create(LOCATIONS_METHOD_HYBRID, &manager_);
  location_manager_create(LOCATIONS_METHOD_HYBRID,
                          &manager_for_current_location_);
}

LocationManager::~LocationManager() {
  location_manager_destroy(manager_);
  location_manager_destroy(manager_for_current_location_);
}

bool LocationManager::IsLocationServiceEnabled() {
  bool gps_enabled = false;
  int ret =
      location_manager_is_enabled_method(LOCATIONS_METHOD_GPS, &gps_enabled);
  if (LOCATIONS_ERROR_NONE != ret) {
    throw LocationManagerError(ret);
  }

  bool wps_enabled = false;
  ret = location_manager_is_enabled_method(LOCATIONS_METHOD_WPS, &wps_enabled);
  if (LOCATIONS_ERROR_NONE != ret) {
    throw LocationManagerError(ret);
  }
  return gps_enabled || wps_enabled;
}

Position LocationManager::GetLastKnownPosition() {
  double altitude = 0.0;
  double latitude = 0.0;
  double longitude = 0.0;
  double climb = 0.0;
  double direction = 0.0;
  double speed = 0.0;
  double horizontal = 0.0;
  double vertical = 0.0;
  location_accuracy_level_e level = LOCATIONS_ACCURACY_NONE;
  time_t timestamp = 0;

  int ret = location_manager_get_last_location(
      manager_, &altitude, &latitude, &longitude, &climb, &direction, &speed,
      &level, &horizontal, &vertical, &timestamp);
  if (LOCATIONS_ERROR_NONE != ret) {
    throw LocationManagerError(ret);
  }
  Position position;
  position.longitude = longitude;
  position.latitude = latitude;
  position.timestamp = timestamp;
  position.accuracy = horizontal;
  position.altitude = altitude;
  position.heading = direction;
  position.speed = speed;

  return position;
}

void LocationManager::GetCurrentPosition(LocationCallback on_location,
                                         LocationErrorCallback on_error) {
  location_callback_ = on_location;
  error_callback_ = on_error;
  int ret = location_manager_request_single_location(
      manager_for_current_location_, 5,
      [](location_error_e error, double latitude, double longitude,
         double altitude, time_t timestamp, double speed, double direction,
         double climb, void *user_data) {
        LocationManager *self = static_cast<LocationManager *>(user_data);

        if (error != LOCATIONS_ERROR_NONE && self->error_callback_) {
          self->error_callback_(LocationManagerError(error));
        } else if (self->location_callback_) {
          Position position;
          position.latitude = latitude;
          position.longitude = longitude;
          position.altitude = altitude;
          position.timestamp = timestamp;
          position.speed = speed;
          position.heading = direction;

          self->location_callback_(position);
        }
        self->location_callback_ = nullptr;
        self->error_callback_ = nullptr;
      },
      this);
  if (LOCATIONS_ERROR_NONE != ret) {
    error_callback_(LocationManagerError(ret));
    error_callback_ = nullptr;
  }
}

void LocationManager::StartListenServiceStatusUpdate(
    ServiceStatusCallback on_service_status_update) {
  service_status_update_callback_ = on_service_status_update;
  int ret = location_manager_set_service_state_changed_cb(
      manager_,
      [](location_service_state_e state, void *user_data) {
        LocationManager *self = static_cast<LocationManager *>(user_data);
        if (self->service_status_update_callback_) {
          ServiceStatus service_status = ServiceStatus::kDisabled;
          if (state == LOCATIONS_SERVICE_ENABLED) {
            service_status = ServiceStatus::kEnabled;
          }
          self->service_status_update_callback_(service_status);
        }
      },
      this);
  if (LOCATIONS_ERROR_NONE != ret) {
    throw LocationManagerError(ret);
  }
}

void LocationManager::StopListenServiceStatusUpdate() {
  int ret = location_manager_unset_service_state_changed_cb(manager_);
  if (LOCATIONS_ERROR_NONE != ret) {
    throw LocationManagerError(ret);
  }
}

void LocationManager::StartListenLocationUpdate(
    LocationCallback on_location_update) {
  location_update_callback_ = on_location_update;

  int ret = location_manager_set_position_updated_cb(
      manager_,
      [](double latitude, double longitude, double altitude, time_t timestamp,
         void *user_data) {
        LocationManager *self = static_cast<LocationManager *>(user_data);
        if (self->location_update_callback_) {
          Position position;
          position.longitude = longitude;
          position.latitude = latitude;
          position.timestamp = timestamp;
          position.altitude = altitude;
          self->location_update_callback_(position);
        }
      },
      2, this);
  if (LOCATIONS_ERROR_NONE != ret) {
    throw LocationManagerError(ret);
  }
  ret = location_manager_start(manager_);
  if (LOCATIONS_ERROR_NONE != ret) {
    throw LocationManagerError(ret);
  }
}

void LocationManager::StopListenLocationUpdate() {
  int ret = location_manager_unset_position_updated_cb(manager_);
  if (LOCATIONS_ERROR_NONE != ret) {
    throw LocationManagerError(ret);
  }
  ret = location_manager_stop(manager_);
  if (LOCATIONS_ERROR_NONE != ret) {
    throw LocationManagerError(ret);
  }
}
