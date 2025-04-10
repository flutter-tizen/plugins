// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device_proxy.h"

#include <dlfcn.h>

#include "log.h"

typedef power_state_e (*FunDevicePowerGetState)(void);

DeviceProxy::DeviceProxy() {
  power_state_handle_ = dlopen("libdeviced.so.1", RTLD_LAZY);
  if (power_state_handle_ == nullptr) {
    LOG_ERROR("Failed to open libdeviced.so.1");
  }
}

DeviceProxy::~DeviceProxy() {
  if (power_state_handle_) {
    dlclose(power_state_handle_);
    power_state_handle_ = nullptr;
  }
}

power_state_e DeviceProxy::device_power_get_state(void) {
  if (!power_state_handle_) {
    LOG_ERROR("power_state_handle_ is invalid.");
    return POWER_STATE_ERROR;
  }
  FunDevicePowerGetState device_power_get_state =
      reinterpret_cast<FunDevicePowerGetState>(
          dlsym(power_state_handle_, "device_power_get_state"));
  if (!device_power_get_state) {
    LOG_ERROR("Failed to find device_power_get_state function.");
    return POWER_STATE_ERROR;
  }
  return device_power_get_state();
}
