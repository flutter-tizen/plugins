// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ble_tizen.h"

#include <bluetooth.h>

#include "log.h"

bool BleTizen::Initialize() {
  last_error_ = bt_initialize();
  if (last_error_ != BT_ERROR_NONE) {
    return false;
  }
  return true;
}

bool BleTizen::Deinitialize() {
  last_error_ = bt_deinitialize();
  if (last_error_ != BT_ERROR_NONE) {
    return false;
  }
  return true;
}

BleStatus BleTizen::GetBleStatus() {
  bt_adapter_state_e adapter_state;
  last_error_ = bt_adapter_get_state(&adapter_state);
  if (last_error_ == BT_ERROR_NONE) {
    return adapter_state == BT_ADAPTER_ENABLED ? BleStatus::kEnabled
                                               : BleStatus::kDisabled;
  } else if (last_error_ == BT_ERROR_NOT_SUPPORTED) {
    return BleStatus::kNotSupported;
  }
  return BleStatus::kUnknown;
}

bool BleTizen::SetBleStatusChangeCallback(BleStatusChangeCallback callback) {
  last_error_ = bt_adapter_set_state_changed_cb(
      [](int result, bt_adapter_state_e adapter_state, void *user_data) {
        if (result != BT_ERROR_NONE) {
          LOG_ERROR("The operation failed unexpectedly: %s",
                    get_error_message(result));
          return;
        }
        BleTizen *self = static_cast<BleTizen *>(user_data);
        if (self->ble_status_change_callback_) {
          BleStatus status = adapter_state == BT_ADAPTER_ENABLED
                                 ? BleStatus::kEnabled
                                 : BleStatus::kDisabled;
          self->ble_status_change_callback_(status);
        }
      },
      this);
  if (last_error_ != BT_ERROR_NONE) {
    return false;
  }
  ble_status_change_callback_ = std::move(callback);
  return true;
}

bool BleTizen::UnsetBleStatusChangeCallback() {
  last_error_ = bt_adapter_unset_state_changed_cb();
  if (last_error_ != BT_ERROR_NONE) {
    return false;
  }
  ble_status_change_callback_ = nullptr;
  return true;
}

bool BleTizen::Scan(const std::vector<Uuid> &service_ids) {
  if (is_scanning_) {
    StopScan();
  }

  for (const Uuid &service_id : service_ids) {
    ScanFilter filter = ScanFilter(service_id);
    if (filter.Register()) {
      filters_.emplace_back(std::move(filter));
    } else {
      LOG_ERROR("Ignoring scan filter: %s", service_id.c_str());
    }
  }

  last_error_ = bt_adapter_le_start_scan(
      [](int result, bt_adapter_le_device_scan_result_info_s *info,
         void *user_data) {
        BleTizen *self = static_cast<BleTizen *>(user_data);
        if (result != BT_ERROR_NONE) {
          LOG_ERROR("The scan operation resulted in an error: %s",
                    get_error_message(result));
          if (self->device_scan_error_callback_) {
            self->device_scan_error_callback_(result,
                                              get_error_message(result));
          }
          return;
        }

        DiscoveredDevice device = {};
        device.device_id = std::string(info->remote_address);
        device.rssi = info->rssi;

        auto iter = self->cached_device_names_.find(device.device_id);
        if (iter != self->cached_device_names_.end()) {
          device.name = iter->second;
        } else {
          char *name = nullptr;
          int ret = bt_adapter_le_get_scan_result_device_name(
              info, BT_ADAPTER_LE_PACKET_SCAN_RESPONSE, &name);
          if (ret == BT_ERROR_NO_DATA) {
            // Retrying with a different packet type.
            ret = bt_adapter_le_get_scan_result_device_name(
                info, BT_ADAPTER_LE_PACKET_ADVERTISING, &name);
          }
          if (ret == BT_ERROR_NONE) {
            device.name = std::string(name);
            free(name);
          } else if (ret != BT_ERROR_NO_DATA) {
            LOG_ERROR("Could not obtain name info from the device %s.",
                      info->remote_address);
            if (self->device_scan_error_callback_) {
              self->device_scan_error_callback_(ret, get_error_message(ret));
            }
            return;
          }
          if (!device.name.empty()) {
            self->cached_device_names_[device.device_id] = device.name;
          }
        }

        int manufacturer_id = 0;
        char *manufacturer_data = nullptr;
        int manufacturer_data_len = 0;
        int ret = bt_adapter_le_get_scan_result_manufacturer_data(
            info, BT_ADAPTER_LE_PACKET_SCAN_RESPONSE, &manufacturer_id,
            &manufacturer_data, &manufacturer_data_len);
        if (ret == BT_ERROR_NONE) {
          device.manufacturer_data = std::vector<uint8_t>(
              manufacturer_data, manufacturer_data + manufacturer_data_len);
          free(manufacturer_data);
        } else if (ret != BT_ERROR_NO_DATA) {
          LOG_ERROR("Could not obtain manufacturer info from the device %s.",
                    info->remote_address);
          if (self->device_scan_error_callback_) {
            self->device_scan_error_callback_(ret, get_error_message(ret));
          }
          return;
        }

        char **service_ids = nullptr;
        int service_count = 0;
        ret = bt_adapter_le_get_scan_result_service_uuids(
            info, BT_ADAPTER_LE_PACKET_SCAN_RESPONSE, &service_ids,
            &service_count);
        if (ret == BT_ERROR_NONE) {
          for (int i = 0; i < service_count; ++i) {
            device.service_ids.emplace_back(std::string(service_ids[i]));
            free(service_ids[i]);
          }
          free(service_ids);
        } else if (ret != BT_ERROR_NO_DATA) {
          LOG_ERROR("Could not obtain service UUIDs from the device %s.",
                    info->remote_address);
          if (self->device_scan_error_callback_) {
            self->device_scan_error_callback_(ret, get_error_message(ret));
          }
          return;
        }

        bt_adapter_le_service_data_s *service_data = nullptr;
        int service_data_count = 0;
        ret = bt_adapter_le_get_scan_result_service_data_list(
            info, BT_ADAPTER_LE_PACKET_SCAN_RESPONSE, &service_data,
            &service_data_count);
        if (ret == BT_ERROR_NONE) {
          for (int i = 0; i < service_data_count; ++i) {
            std::string service_id = service_data[i].service_uuid;
            device.service_data[service_id] =
                std::vector<uint8_t>(service_data[i].service_data,
                                     service_data[i].service_data +
                                         service_data[i].service_data_len);
          }
          bt_adapter_le_free_service_data_list(service_data,
                                               service_data_count);
        } else if (ret != BT_ERROR_NO_DATA) {
          LOG_ERROR("Could not obtain service data from the device %s.",
                    info->remote_address);
          if (self->device_scan_error_callback_) {
            self->device_scan_error_callback_(ret, get_error_message(ret));
          }
          return;
        }

        if (self->device_scan_callback_) {
          self->device_scan_callback_(device);
        }
      },
      this);
  if (last_error_ != BT_ERROR_NONE) {
    return false;
  }

  is_scanning_ = true;
  return true;
}

bool BleTizen::StopScan() {
  if (!is_scanning_) {
    return true;
  }

  last_error_ = bt_adapter_le_stop_scan();
  if (last_error_ != BT_ERROR_NONE) {
    return false;
  }
  ScanFilter::UnregisterAll();
  filters_.clear();

  is_scanning_ = false;
  return true;
}

bool BleTizen::ConnectToDevice(const std::string &device_id) {
  last_error_ = bt_gatt_connect(device_id.c_str(), false);
  if (last_error_ != BT_ERROR_NONE) {
    return false;
  }
  if (connection_state_change_callback_) {
    connection_state_change_callback_(device_id, ConnectionState::kConnecting);
  }
  return true;
}

bool BleTizen::DisconnectFromDevice(const std::string &device_id) {
  last_error_ = bt_gatt_disconnect(device_id.c_str());
  if (last_error_ != BT_ERROR_NONE) {
    return false;
  }
  if (connection_state_change_callback_) {
    connection_state_change_callback_(device_id,
                                      ConnectionState::kDisconnecting);
  }
  return true;
}

bool BleTizen::SetConnectionStateChangeCallback(
    ConnectionStateChangeCallback on_change, ErrorCallback on_error) {
  last_error_ = bt_gatt_set_connection_state_changed_cb(
      [](int result, bool connected, const char *remote_address,
         void *user_data) {
        LOG_DEBUG(
            "Connection state changed: device[%s], connected[%d], result[%s]",
            remote_address, connected, get_error_message(result));
        BleTizen *self = static_cast<BleTizen *>(user_data);
        if (remote_address) {
          if (result != BT_ERROR_NONE) {
            LOG_ERROR("The connection request for device %s failed: %s",
                      remote_address, get_error_message(result));
          }
          std::string device_id = std::string(remote_address);
          bool success =
              result == BT_ERROR_NONE || result == BT_ERROR_ALREADY_DONE;
          ConnectionState state = success && connected
                                      ? ConnectionState::kConnected
                                      : ConnectionState::kDisconnected;
          if (self->connection_state_change_callback_) {
            self->connection_state_change_callback_(device_id, state);
          }
        } else {
          LOG_ERROR("The connection request failed: %s",
                    get_error_message(result));
          if (self->connection_error_callback_) {
            self->connection_error_callback_(result, get_error_message(result));
          }
        }
      },
      this);
  if (last_error_ != BT_ERROR_NONE) {
    return false;
  }
  connection_state_change_callback_ = std::move(on_change);
  connection_error_callback_ = std::move(on_error);
  return true;
}

bool BleTizen::UnsetConnectionStateChangeCallback() {
  last_error_ = bt_gatt_unset_connection_state_changed_cb();
  if (last_error_ != BT_ERROR_NONE) {
    return false;
  }
  connection_state_change_callback_ = nullptr;
  connection_error_callback_ = nullptr;
  return true;
}
