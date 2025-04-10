// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_DEVICE_PROXY_H_
#define FLUTTER_PLUGIN_DEVICE_PROXY_H_

typedef enum {
  POWER_STATE_NORMAL,          /**< Normal state */
  POWER_STATE_PICTUREOFF,      /**< Picture off state */
  POWER_STATE_STANDBY,         /**< Background Standby state */
  POWER_STATE_OFF,             /**< Cold Standby state */
  POWER_STATE_SUSPEND_STANDBY, /**< Suspend Standby state */
  POWER_STATE_ERROR            /**< Error state */
} power_state_e;

class DeviceProxy {
 public:
  DeviceProxy();
  ~DeviceProxy();
  power_state_e device_power_get_state(void);

 private:
  void* power_state_handle_ = nullptr;
};

#endif  // FLUTTER_PLUGIN_DEVICE_PROXY_H_
