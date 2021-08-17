// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LOCATON_MANAGER_H_
#define LOCATON_MANAGER_H_

#include <locations.h>

#include <functional>

#include "location.h"
#include "tizen_result.h"

using OnLocationUpdate = std::function<void(Location)>;
using OnError = std::function<void(TizenResult)>;

class LocationManager {
 public:
  LocationManager();
  ~LocationManager();

  TizenResult IsLocationServiceEnabled(bool *is_enabled);

  TizenResult RequestCurrentLocationOnce(OnLocationUpdate on_position_update,
                                         OnError on_error);

 private:
  TizenResult CreateLocationManager();

  TizenResult DestroyLocationManager();

  location_manager_h manager_ = nullptr;

  OnLocationUpdate on_position_update_;
  OnError on_error_;
};
#endif  // LOCATON_MANAGER_H_
