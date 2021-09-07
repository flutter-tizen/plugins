// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "locaton_manager.h"

#include "log.h"

LocationManager::LocationManager() {
  CreateLocationManager(&manager_);
  CreateLocationManager(&manager_for_current_location_);
}

LocationManager::~LocationManager() {
  DestroyLocationManager(manager_);
  DestroyLocationManager(manager_for_current_location_);
}

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
    OnLocationUpdated on_success, OnError on_error) {
  on_success_ = on_success;
  on_error_ = on_error;
  TizenResult ret = location_manager_request_single_location(
      manager_for_current_location_, 5,
      [](location_error_e error, double latitude, double longitude,
         double altitude, time_t timestamp, double speed, double direction,
         double climb, void *user_data) {
        LocationManager *self = static_cast<LocationManager *>(user_data);

        if (error != LOCATIONS_ERROR_NONE && self->on_error_) {
          self->on_error_(error);
        } else if (self->on_success_) {
          Location location;
          location.latitude = latitude;
          location.longitude = longitude;
          location.altitude = altitude;
          location.timestamp = timestamp;
          location.speed = speed;
          location.heading = direction;

          self->on_success_(location);
        }

        self->on_error_ = nullptr;
        self->on_success_ = nullptr;
      },
      this);
  return ret;
}

TizenResult LocationManager::GetLastKnownLocation(Location *location) {
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

  TizenResult ret = location_manager_get_last_location(
      manager_, &altitude, &latitude, &longitude, &climb, &direction, &speed,
      &level, &horizontal, &vertical, &timestamp);
  if (!ret) {
    return ret;
  }

  location->longitude = longitude;
  location->latitude = latitude;
  location->timestamp = timestamp;
  location->accuracy = horizontal;
  location->altitude = altitude;
  location->heading = direction;
  location->speed = speed;

  return ret;
}

TizenResult LocationManager::SetOnServiceStateChanged(
    OnServiceStateChanged on_service_state_changed) {
  on_service_state_changed_ = on_service_state_changed;
  return location_manager_set_service_state_changed_cb(
      manager_,
      [](location_service_state_e state, void *user_data) {
        LocationManager *self = static_cast<LocationManager *>(user_data);
        if (self->on_service_state_changed_) {
          ServiceState service_state = ServiceState::kDisabled;
          if (state == LOCATIONS_SERVICE_ENABLED) {
            service_state = ServiceState::kEnabled;
          }
          self->on_service_state_changed_(service_state);
        }
      },
      this);
}

TizenResult LocationManager::UnsetOnServiceStateChanged() {
  return location_manager_unset_service_state_changed_cb(manager_);
}

TizenResult LocationManager::SetOnLocationUpdated(
    OnLocationUpdated on_location_updated) {
  on_location_updated_ = on_location_updated;
  TizenResult result = location_manager_set_position_updated_cb(
      manager_,
      [](double latitude, double longitude, double altitude, time_t timestamp,
         void *user_data) {
        LocationManager *self = static_cast<LocationManager *>(user_data);
        if (self->on_location_updated_) {
          Location location;
          location.longitude = longitude;
          location.latitude = latitude;
          location.timestamp = timestamp;
          location.altitude = altitude;
          self->on_location_updated_(location);
        }
      },
      2, this);
  if (!result) {
    return result;
  }
  return location_manager_start(manager_);
}

TizenResult LocationManager::UnsetOnLocationUpdated() {
  location_manager_unset_position_updated_cb(manager_);
  return location_manager_stop(manager_);
}

TizenResult LocationManager::CreateLocationManager(
    location_manager_h *manager) {
  return location_manager_create(LOCATIONS_METHOD_HYBRID, manager);
}

TizenResult LocationManager::DestroyLocationManager(
    location_manager_h manager) {
  return location_manager_destroy(manager);
}
