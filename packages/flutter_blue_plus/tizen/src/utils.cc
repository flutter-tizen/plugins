// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "utils.h"

#include <algorithm>

#include "log.h"

namespace flutter_blue_plus_tizen {

// Gets the value of descriptor or characteristic.
std::string GetGattValue(bt_gatt_h handle) {
  std::string result = "";
  char* value = nullptr;
  int length = 0;

  int ret = bt_gatt_get_value(handle, &value, &length);
  LOG_ERROR("bt_gatt_get_value %s", get_error_message(ret));
  if (!ret && value) {
    result = std::string(value, length);
    free(value);
  }

  return result;
}

// Gets the uuid of descriptor, characteristic or service.
std::string GetGattUuid(bt_gatt_h handle) {
  std::string result;
  char* uuid = nullptr;
  int ret = bt_gatt_get_uuid(handle, &uuid);
  LOG_ERROR("bt_gatt_get_uuid %s", get_error_message(ret));
  if (!ret && uuid) {
    result = std::string(uuid);
    free(uuid);
  }
  return result;
}

AdvertisementData DecodeAdvertisementData(const char* packets_data,
                                          int data_len) noexcept {
  AdvertisementData advertisement_data;
  using byte = char;
  int start = 0;
  bool long_name_set = false;

  // TODO(JRazek): fix. read, write properies not checked!

  while (start < data_len) {
    byte advertisement_data_len = packets_data[start] & 0xFFu;
    byte type = packets_data[start + 1] & 0xFFu;

    const byte* packet = packets_data + start + 2;
    switch (type) {
      case 0x09:
      case 0x08: {
        if (!long_name_set)
          std::copy_n(packet, advertisement_data_len - 1,
                      std::back_inserter(advertisement_data.local_name_));

        if (type == 0x09) long_name_set = true;

        break;
      }
      case 0x01: {
        advertisement_data.connectable_ = *packet & 0x3;
        break;
      }
      case 0xFF: {
        break;
      }
      default:
        break;
    }
    start += advertisement_data_len + 1;
  }
  return advertisement_data;
}

}  // namespace flutter_blue_plus_tizen
