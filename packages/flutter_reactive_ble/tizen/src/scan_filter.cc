// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scan_filter.h"

#include <bluetooth.h>

#include "log.h"

ScanFilter::ScanFilter(const Uuid &service_id) {
  int ret = bt_adapter_le_scan_filter_create(&handle_);
  if (ret != BT_ERROR_NONE) {
    LOG_ERROR("Failed to create a scan filter handle.");
    return;
  }

  ret = bt_adapter_le_scan_filter_set_service_uuid(handle_, service_id.c_str());
  if (ret != BT_ERROR_NONE) {
    LOG_ERROR("Failed to set a service UUID for the scan filter.");
    return;
  }
};

ScanFilter::~ScanFilter() {
#ifdef TV_PROFILE
  // This call results in a crash on wearable and common devices.
  if (handle_) {
    bt_adapter_le_scan_filter_destroy(handle_);
  }
#endif
};

bool ScanFilter::Register() {
  int ret = bt_adapter_le_scan_filter_register(handle_);
  if (ret != BT_ERROR_NONE) {
    LOG_ERROR("Filter registration failed: %s", get_error_message(ret));
    return false;
  }
  return true;
};

bool ScanFilter::Unregister() {
  int ret = bt_adapter_le_scan_filter_unregister(handle_);
  if (ret != BT_ERROR_NONE) {
    LOG_ERROR("Filter unregistration failed: %s", get_error_message(ret));
    return false;
  }
  return true;
};

bool ScanFilter::UnregisterAll() {
  int ret = bt_adapter_le_scan_filter_unregister_all();
  if (ret != BT_ERROR_NONE) {
    LOG_ERROR("Filter unregistration failed: %s", get_error_message(ret));
    return false;
  }
  return true;
}
