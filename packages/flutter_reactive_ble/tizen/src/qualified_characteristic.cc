// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "qualified_characteristic.h"

#include <bluetooth.h>

#include "ble_device.h"
#include "log.h"

QualifiedCharacteristic::QualifiedCharacteristic(const BleDevice *device,
                                                 const Uuid &service_id,
                                                 const Uuid &characteristic_id)
    : device_id_(device->device_id()),
      service_id_(service_id),
      characteristic_id_(characteristic_id) {
  bt_gatt_h service_handle = nullptr;
  int ret = bt_gatt_client_get_service(device->handle(), service_id.c_str(),
                                       &service_handle);
  if (ret != BT_ERROR_NONE) {
    LOG_ERROR("Failed to create a service handle: %s", get_error_message(ret));
    return;
  }

  ret = bt_gatt_service_get_characteristic(service_handle,
                                           characteristic_id.c_str(), &handle_);
  if (ret != BT_ERROR_NONE) {
    LOG_ERROR("Failed to create a characteristic handle: %s",
              get_error_message(ret));
  }
}

int QualifiedCharacteristic::GetProperties() const {
  int properties = 0;
  int ret = bt_gatt_characteristic_get_properties(handle_, &properties);
  if (ret != BT_ERROR_NONE) {
    LOG_ERROR("Unable to get characteristic properties: %s",
              get_error_message(ret));
  }
  return properties;
}

void QualifiedCharacteristic::SetWriteType(bool no_response) {
  int ret = bt_gatt_characteristic_set_write_type(
      handle_, no_response ? BT_GATT_WRITE_TYPE_WRITE_NO_RESPONSE
                           : BT_GATT_WRITE_TYPE_WRITE);
  if (ret != BT_ERROR_NONE) {
    LOG_ERROR("Failed to set write type: %s", get_error_message(ret));
  }
}
