// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LOCATON_MANAGER_H_
#define LOCATON_MANAGER_H_

#include <locations.h>

#include <functional>

#include "location.h"
#include "tizen_result.h"

// Defined in:
// https://github.com/Baseflow/flutter-geolocator/blob/master/geolocator_platform_interface/lib/src/enums/location_service.dart
enum class ServiceState { kDisabled, kEnabled };

using OnLocationUpdated = std::function<void(Location)>;
using OnError = std::function<void(TizenResult)>;
using OnServiceStateChanged = std::function<void(ServiceState)>;

class LocationManager {
 public:
  LocationManager();
  ~LocationManager();

  TizenResult IsLocationServiceEnabled(bool* is_enabled);

  TizenResult RequestCurrentLocationOnce(OnLocationUpdated on_success,
                                         OnError on_error);

  TizenResult GetLastKnownLocation(Location* locaton);

  TizenResult SetOnServiceStateChanged(
      OnServiceStateChanged on_service_state_changed);

  TizenResult UnsetOnServiceStateChanged();

  TizenResult SetOnLocationUpdated(OnLocationUpdated on_location_updated);

  TizenResult UnsetOnLocationUpdated();

 private:
  TizenResult CreateLocationManager(location_manager_h* manager);

  TizenResult DestroyLocationManager(location_manager_h manager);

  location_manager_h manager_ = nullptr;

  // According to the document, the handler to request current location once
  // must not be the same as a handler to listen position updated
  location_manager_h manager_for_current_location_ = nullptr;

  OnLocationUpdated on_success_;
  OnError on_error_;
  OnServiceStateChanged on_service_state_changed_;
  OnLocationUpdated on_location_updated_;
};
#endif  // LOCATON_MANAGER_H_
