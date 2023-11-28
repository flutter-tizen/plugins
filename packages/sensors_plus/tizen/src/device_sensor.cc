// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device_sensor.h"

#include "log.h"

namespace {

sensor_type_e ToTizenSensorType(const SensorType &sensor_type) {
  switch (sensor_type) {
    case SensorType::kAccelerometer:
      return SENSOR_ACCELEROMETER;
    case SensorType::kGyroscope:
      return SENSOR_GYROSCOPE;
    case SensorType::kMagnetometer:
      return SENSOR_MAGNETIC;
    case SensorType::kUserAccel:
    default:
      return SENSOR_LINEAR_ACCELERATION;
  }
}

}  // namespace

DeviceSensor::DeviceSensor(SensorType sensor_type) : sensor_type_(sensor_type) {
  sensor_h sensor;
  int ret = sensor_get_default_sensor(ToTizenSensorType(sensor_type_), &sensor);
  if (ret != SENSOR_ERROR_NONE) {
    LOG_ERROR("Failed to get default sensor: %s", get_error_message(ret));
    last_error_ = ret;
    return;
  }

  ret = sensor_create_listener(sensor, &listener_);
  if (ret != SENSOR_ERROR_NONE) {
    LOG_ERROR("Failed to create listener: %s", get_error_message(ret));
    last_error_ = ret;
  }
}

DeviceSensor::~DeviceSensor() {
  if (is_listening_) {
    StopListen();
  }

  if (listener_) {
    sensor_destroy_listener(listener_);
    listener_ = nullptr;
  }
}

bool DeviceSensor::StartListen(SensorEventCallback callback) {
  if (sensor_type_ == SensorType::kMagnetometer) {
    LOG_ERROR("Not supported sensor type.");
    last_error_ = SENSOR_ERROR_NOT_SUPPORTED;
    return false;
  }

  if (is_listening_) {
    LOG_WARN("Already listening.");
    last_error_ = SENSOR_ERROR_OPERATION_FAILED;
    return false;
  }

  int ret = sensor_listener_set_event_cb(
      listener_, interval_ms_,
      [](sensor_h sensor, sensor_event_s *event, void *user_data) {
        auto *self = static_cast<DeviceSensor *>(user_data);
        SensorEvent sensor_event;
        for (int i = 0; i < event->value_count; i++) {
          sensor_event.push_back(event->values[i]);
        }
        self->callback_(sensor_event);
      },
      this);
  if (ret != SENSOR_ERROR_NONE) {
    LOG_ERROR("Failed to set event callback: %s", get_error_message(ret));
    last_error_ = ret;
    return false;
  }

  ret = sensor_listener_start(listener_);
  if (ret != SENSOR_ERROR_NONE) {
    LOG_ERROR("Failed to start listener: %s", get_error_message(ret));
    last_error_ = ret;
    return false;
  }

  callback_ = callback;
  is_listening_ = true;

  return true;
}

void DeviceSensor::StopListen() {
  if (!is_listening_) {
    LOG_WARN("Already canceled.");
    return;
  }

  int ret = sensor_listener_stop(listener_);
  if (ret != SENSOR_ERROR_NONE) {
    LOG_ERROR("Failed to stop listener: %s", get_error_message(ret));
    last_error_ = ret;
    return;
  }

  is_listening_ = false;

  ret = sensor_listener_unset_event_cb(listener_);
  if (ret != SENSOR_ERROR_NONE) {
    LOG_ERROR("Failed to unset event callback: %s", get_error_message(ret));
    last_error_ = ret;
    return;
  }
}

void DeviceSensor::SetInterval(int interval_ms) {
  interval_ms_ = interval_ms;

  if (listener_) {
    sensor_listener_set_interval(listener_, interval_ms_);
  }
}
