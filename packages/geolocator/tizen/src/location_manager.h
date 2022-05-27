// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_LOCATION_MANAGER_H_
#define FLUTTER_PLUGIN_LOCATION_MANAGER_H_

#include <locations.h>
#include <tizen_error.h>

#include <functional>
#include <string>

#include "position.h"

// Defined in:
// https://github.com/Baseflow/flutter-geolocator/blob/master/geolocator_platform_interface/lib/src/enums/location_service.dart
enum class ServiceStatus { kDisabled, kEnabled };

class LocationManagerError {
 public:
  LocationManagerError(int error_code) : error_code_(error_code) {}

  int GetErrorCode() const { return error_code_; }

  std::string GetErrorString() const { return get_error_message(error_code_); }

 private:
  int error_code_;
};

typedef std::function<void(Position)> LocationCallback;
typedef std::function<void(ServiceStatus)> ServiceStatusCallback;
typedef std::function<void(LocationManagerError)> LocationErrorCallback;

class LocationManager {
 public:
  LocationManager();
  ~LocationManager();

  bool IsLocationServiceEnabled();

  Position GetLastKnownPosition();

  void GetCurrentPosition(LocationCallback on_location_callback,
                          LocationErrorCallback on_error_callback);

  void StartListenServiceStatusUpdate(
      ServiceStatusCallback on_service_status_update);

  void StopListenServiceStatusUpdate();

  void StartListenLocationUpdate(LocationCallback on_location_update);

  void StopListenLocationUpdate();

 private:
  // According to the document, the handler to request current location once
  // must not be the same as a handler to listen position updates.
  location_manager_h manager_for_current_location_ = nullptr;
  location_manager_h manager_ = nullptr;

  ServiceStatusCallback service_status_update_callback_;
  LocationCallback location_update_callback_;
  LocationCallback location_callback_;
  LocationErrorCallback error_callback_;
};

#endif  // FLUTTER_PLUGIN_LOCATION_MANAGER_H_
