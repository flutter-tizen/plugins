// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device_battery.h"

#include <device/battery.h>

#include "log.h"

bool DeviceBattery::StartListen(BatteryStatusCallback callback) {
  // DEVICE_CALLBACK_BATTERY_CHARGING callback is called in only two cases
  // like charging and discharing. When the charging status becomes "Full",
  // discharging status event is called. So if it is full and disconnected
  // from USB or AC charger, then any callbacks will not be called because the
  // status already is the discharging status. To resolve this issue,
  // DEVICE_CALLBACK_BATTERY_LEVEL callback is added. This callback can check
  // whether it is disconnected in "Full" status. That is, when the battery
  // status is full and disconnected, the level status will be changed from
  // DEVICE_BATTERY_LEVEL_FULL to DEVICE_BATTERY_LEVEL_HIGH.
  int ret = device_add_callback(DEVICE_CALLBACK_BATTERY_CHARGING,
                                OnBatteryStatusChanged, this);
  if (ret != DEVICE_ERROR_NONE) {
    LOG_ERROR("Failed to add callback: %s", get_error_message(ret));
    return false;
  }

  ret = device_add_callback(DEVICE_CALLBACK_BATTERY_LEVEL,
                            OnBatteryStatusChanged, this);
  if (ret != DEVICE_ERROR_NONE) {
    LOG_ERROR("Failed to add callback: %s", get_error_message(ret));
    return false;
  }

  callback_ = callback;
  return true;
}

void DeviceBattery::StopListen() {
  int ret = device_remove_callback(DEVICE_CALLBACK_BATTERY_CHARGING,
                                   OnBatteryStatusChanged);
  if (ret != DEVICE_ERROR_NONE) {
    LOG_ERROR("Failed to remove callback: %s", get_error_message(ret));
  }

  ret = device_remove_callback(DEVICE_CALLBACK_BATTERY_LEVEL,
                               OnBatteryStatusChanged);
  if (ret != DEVICE_ERROR_NONE) {
    LOG_ERROR("Failed to remove callback: %s", get_error_message(ret));
  }
}

int32_t DeviceBattery::GetLevel() {
  int32_t level;
  int ret = device_battery_get_percent(&level);
  if (ret != DEVICE_ERROR_NONE) {
    LOG_ERROR("Failed to get battery level: %s", get_error_message(ret));
    return -1;
  }
  return level;
}

BatteryStatus DeviceBattery::GetStatus() {
  device_battery_status_e status;
  int ret = device_battery_get_status(&status);
  if (ret != DEVICE_ERROR_NONE) {
    LOG_ERROR("Failed to get battery status: %s", get_error_message(ret));
    return BatteryStatus::kError;
  }

  switch (status) {
    case DEVICE_BATTERY_STATUS_CHARGING:
      return BatteryStatus::kCharging;
    case DEVICE_BATTERY_STATUS_FULL:
      return BatteryStatus::kFull;
    case DEVICE_BATTERY_STATUS_DISCHARGING:
    case DEVICE_BATTERY_STATUS_NOT_CHARGING:
      return BatteryStatus::kDischarging;
    default:
      return BatteryStatus::kUnknown;
  }
}

void DeviceBattery::OnBatteryStatusChanged(device_callback_e type, void *value,
                                           void *user_data) {
  auto *self = static_cast<DeviceBattery *>(user_data);

  // DEVICE_CALLBACK_BATTERY_LEVEL callback is used only for checking whether
  // the battery became a discharging status while it is full.
  if (type == DEVICE_CALLBACK_BATTERY_LEVEL &&
      intptr_t(value) < DEVICE_BATTERY_LEVEL_HIGH) {
    return;
  }

  BatteryStatus status = self->GetStatus();
  if (status == BatteryStatus::kFull && self->is_full_) {
    // This function is called twice by registered callbacks when battery
    // status is full. So, it needs to avoid the unnecessary second call.
    return;
  }
  self->is_full_ = status == BatteryStatus::kFull;

  self->callback_(status);
}
