// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_DEVICE_SENSOR_H_
#define FLUTTER_PLUGIN_DEVICE_SENSOR_H_

#include <sensor.h>
#include <tizen.h>

#include <functional>
#include <string>
#include <vector>

enum class SensorType { kAccelerometer, kGyroscope, kUserAccel };

typedef std::vector<double> SensorEvent;
typedef std::function<void(SensorEvent)> SensorEventCallback;

class DeviceSensor {
 public:
  DeviceSensor(SensorType sensor_type);
  ~DeviceSensor();

  int GetLastError() { return last_error_; }

  std::string GetLastErrorString() { return get_error_message(last_error_); }

  bool StartListen(SensorEventCallback callback);

  void StopListen();

 private:
  SensorType sensor_type_;
  sensor_listener_h listener_ = nullptr;
  bool is_listening_ = false;
  int last_error_ = TIZEN_ERROR_NONE;

  SensorEventCallback callback_ = nullptr;
};

#endif  // FLUTTER_PLUGIN_DEVICE_SENSOR_H_
