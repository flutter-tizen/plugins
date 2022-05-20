// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_POSITION_H_
#define FLUTTER_PLUGIN_POSITION_H_

#include <flutter/encodable_value.h>
#include <time.h>

#include <optional>

// Defined in:
// https://github.com/Baseflow/flutter-geolocator/blob/master/geolocator_platform_interface/lib/src/models/position.dart

struct Position {
  flutter::EncodableValue ToEncodableValue() {
    flutter::EncodableMap values = {
        {flutter::EncodableValue("longitude"),
         flutter::EncodableValue(longitude)},
        {flutter::EncodableValue("latitude"),
         flutter::EncodableValue(latitude)},
        {flutter::EncodableValue("timestamp"),
         flutter::EncodableValue(static_cast<int64_t>(timestamp))},
        {flutter::EncodableValue("altitude"),
         flutter::EncodableValue(altitude)},
    };
    if (accuracy) {
      values[flutter::EncodableValue("accuracy")] =
          flutter::EncodableValue(*accuracy);
    }
    if (heading) {
      values[flutter::EncodableValue("heading")] =
          flutter::EncodableValue(*heading);
    }
    if (speed) {
      values[flutter::EncodableValue("speed")] =
          flutter::EncodableValue(*speed);
    }
    if (speedAccuracy) {
      values[flutter::EncodableValue("speedAccuracy")] =
          flutter::EncodableValue(*speedAccuracy);
    }
    return flutter::EncodableValue(values);
  }

  double longitude;
  double latitude;
  time_t timestamp;
  std::optional<double> accuracy;
  double altitude;
  std::optional<double> heading;
  std::optional<double> speed;
  std::optional<double> speedAccuracy;
};

#endif  // FLUTTER_PLUGIN_POSITION_H_
