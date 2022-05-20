// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_LOCATION_MANAGER_H_
#define FLUTTER_PLUGIN_LOCATION_MANAGER_H_

#include <locations.h>

#include <functional>

#include "position.h"

// Defined in:
// https://github.com/Baseflow/flutter-geolocator/blob/master/geolocator_platform_interface/lib/src/enums/location_service.dart
enum class ServiceStatus { kDisabled, kEnabled };

class LocationManagerError {
 public:
  LocationManagerError(int error_code) : error_code_(error_code) {}
  std::string GetErrorString() const { return get_error_message(error_code_); }

 private:
  int error_code_;
};

typedef std::function<void(Position)> LocationCallback;
typedef std::function<void(ServiceStatus)> ServiceStatusCallback;
typedef std::function<void(LocationManagerError)> LocationErrorListener;

class LocationManager {
 public:
  LocationManager();
  ~LocationManager();

  bool IsLocationServiceEnabled();

  Position GetLastKnownPosition();

  void GetCurrentPosition(LocationCallback location_updated_callback,
                          LocationErrorListener location_error_listener);

  bool StartServiceUpdatedListen(
      ServiceStatusCallback service_status_updated_callback);

  bool StopServiceUpdatedListen();

  bool StartLocationUpdatesListen(LocationCallback location_updated_callback);

  bool StopLocationUpdatesListen();

  int GetLastError() { return last_error_; }

  std::string GetLastErrorString() { return get_error_message(last_error_); }

 private:
  // According to the document, the handler to request current location once
  // must not be the same as a handler to listen position updated.
  location_manager_h manager_for_current_location_ = nullptr;
  location_manager_h manager_ = nullptr;

  int last_error_ = TIZEN_ERROR_NONE;
  ServiceStatusCallback service_status_updated_callback_;
  LocationCallback location_updated_callback_;
  LocationCallback location_callback_;
  LocationErrorListener location_error_listener_;
};
#endif  // FLUTTER_PLUGIN_LOCATION_MANAGER_H_
