// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_DEVICE_BATTERY_H_
#define FLUTTER_PLUGIN_DEVICE_BATTERY_H_

#include <device/callback.h>
#include <tizen.h>

#include <functional>
#include <string>

enum class BatteryStatus { kFull, kCharging, kDischarging, kUnknown, kError };

typedef std::function<void(BatteryStatus)> BatteryStatusCallback;

class DeviceBattery {
 public:
  DeviceBattery() {}
  ~DeviceBattery() {}

  int GetLastError() { return last_error_; }

  std::string GetLastErrorString() { return get_error_message(last_error_); }

  bool StartListen(BatteryStatusCallback callback);

  void StopListen();

  int32_t GetLevel();

  BatteryStatus GetStatus();

 private:
  static void OnBatteryStatusChanged(device_callback_e type, void *value,
                                     void *user_data);

  int last_error_ = TIZEN_ERROR_NONE;
  BatteryStatusCallback callback_ = nullptr;
  bool is_full_ = false;
};

#endif  // FLUTTER_PLUGIN_DEVICE_BATTERY_H_
