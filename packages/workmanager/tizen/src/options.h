// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_WORKMANAGER_OPTIONS_H_
#define FLUTTER_PLUGIN_WORKMANAGER_OPTIONS_H_

enum class ExistingWorkPolicy { kReplace, kKeep, kAppend, kUpdate };

enum class NetworkType {
  kConnected,
  kMetered,
  kNotRequired,
  kNotRoaming,
  kUnmetered,
  kTemporarilyUnmetered,
};

struct Constraints {
  const NetworkType network_type;

  const bool battery_not_low;

  const bool charging;

  const bool device_idle;  // Not supported

  const bool storage_not_low;  // Not supported

  Constraints(NetworkType network_type, bool battery_not_low = false,
              bool charging = false, bool device_idle = false,
              bool storage_not_low = false)
      : network_type(network_type),
        battery_not_low(battery_not_low),
        charging(charging),
        device_idle(device_idle),
        storage_not_low(storage_not_low) {}
};

#endif  // FLUTTER_PLUGIN_WORKMANAGER_OPTIONS_H_
