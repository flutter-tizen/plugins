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
  int ret_gps =
      location_manager_is_enabled_method(LOCATIONS_METHOD_GPS, &gps_enabled);
  if (LOCATIONS_ERROR_NONE != ret_gps) {
    throw LocationManagerError(ret_gps);
  }

  bool wps_enabled = false;
  int ret_wps =
      location_manager_is_enabled_method(LOCATIONS_METHOD_WPS, &wps_enabled);
  if (LOCATIONS_ERROR_NONE != ret_wps) {
    throw LocationManagerError(ret_wps);
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

void LocationManager::GetCurrentPosition(
    LocationCallback location_callback,
    LocationErrorListener location_error_listener) {
  location_callback_ = location_callback;
  location_error_listener_ = location_error_listener;
  int ret = location_manager_request_single_location(
      manager_for_current_location_, 5,
      [](location_error_e error, double latitude, double longitude,
         double altitude, time_t timestamp, double speed, double direction,
         double climb, void *user_data) {
        LocationManager *self = static_cast<LocationManager *>(user_data);

        if (error != LOCATIONS_ERROR_NONE && self->location_error_listener_) {
          self->location_error_listener_(LocationManagerError(error));
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
        self->location_error_listener_ = nullptr;
      },
      this);
  if (LOCATIONS_ERROR_NONE != ret) {
    throw LocationManagerError(ret);
  }
}

bool LocationManager::StartServiceUpdatedListen(
    ServiceStatusCallback service_status_updated_callback) {
  service_status_updated_callback_ = service_status_updated_callback;
  int ret = location_manager_set_service_state_changed_cb(
      manager_,
      [](location_service_state_e state, void *user_data) {
        LocationManager *self = static_cast<LocationManager *>(user_data);
        if (self->service_status_updated_callback_) {
          ServiceStatus service_status = ServiceStatus::kDisabled;
          if (state == LOCATIONS_SERVICE_ENABLED) {
            service_status = ServiceStatus::kEnabled;
          }
          self->service_status_updated_callback_(service_status);
        }
      },
      this);
  if (LOCATIONS_ERROR_NONE != ret) {
    last_error_ = ret;
    return false;
  }
  return true;
}

bool LocationManager::StopServiceUpdatedListen() {
  int ret = location_manager_unset_service_state_changed_cb(manager_);
  if (LOCATIONS_ERROR_NONE != ret) {
    last_error_ = ret;
    return false;
  }
  return true;
}

bool LocationManager::StartLocationUpdatesListen(
    LocationCallback location_updated_callback) {
  location_updated_callback_ = location_updated_callback;

  int ret = location_manager_set_position_updated_cb(
      manager_,
      [](double latitude, double longitude, double altitude, time_t timestamp,
         void *user_data) {
        LocationManager *self = static_cast<LocationManager *>(user_data);
        if (self->location_updated_callback_) {
          Position position;
          position.longitude = longitude;
          position.latitude = latitude;
          position.timestamp = timestamp;
          position.altitude = altitude;
          self->location_updated_callback_(position);
        }
      },
      2, this);
  if (LOCATIONS_ERROR_NONE != ret) {
    last_error_ = ret;
    return false;
  }

  ret = location_manager_start(manager_);
  if (LOCATIONS_ERROR_NONE != ret) {
    last_error_ = ret;
    return false;
  }
  return true;
}

bool LocationManager::StopLocationUpdatesListen() {
  int ret = location_manager_unset_position_updated_cb(manager_);
  if (LOCATIONS_ERROR_NONE != ret) {
    last_error_ = ret;
    return false;
  }
  ret = location_manager_stop(manager_);
  if (LOCATIONS_ERROR_NONE != ret) {
    last_error_ = ret;
    return false;
  }
  return true;
}
