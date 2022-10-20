// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_BLE_TIZEN_H_
#define FLUTTER_PLUGIN_BLE_TIZEN_H_

#include <bluetooth_type.h>

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ble_device.h"
#include "scan_filter.h"

struct DiscoveredDevice {
  std::string device_id;
  std::string name;
  std::vector<uint8_t> manufacturer_data;
  std::vector<Uuid> service_ids;
  std::map<Uuid, std::vector<std::uint8_t>> service_data;
  int32_t rssi = 0;
};

enum class BleStatus {
  kUnknown,
  kEnabled,
  kDisabled,
  kNotSupported,
};

enum class ConnectionState {
  kConnecting,
  kConnected,
  kDisconnecting,
  kDisconnected,
};

typedef std::function<void(BleStatus status)> BleStatusChangeCallback;

typedef std::function<void(const DiscoveredDevice& device)> DeviceScanCallback;

typedef std::function<void(const std::string& device_id, ConnectionState state)>
    ConnectionStateChangeCallback;

// A wrapper around the bt_gatt module.
class BleTizen {
 public:
  explicit BleTizen() {}
  ~BleTizen() = default;

  // Initializes the BLE service.
  bool Initialize();

  // Deinitializes the BLE service.
  bool Deinitialize();

  // Returns the current BLE adapter status.
  BleStatus GetBleStatus();

  bool SetBleStatusChangeCallback(BleStatusChangeCallback callback);

  bool UnsetBleStatusChangeCallback();

  // Starts scanning for devices with optional |service_ids|.
  bool Scan(const std::vector<Uuid>& service_ids);

  // Stops scanning for devices.
  bool StopScan();

  void SetDeviceScanCallback(DeviceScanCallback on_done,
                             ErrorCallback on_error) {
    device_scan_callback_ = std::move(on_done);
    device_scan_error_callback_ = std::move(on_error);
  }

  // Connects to a device identified by |device_id|.
  bool ConnectToDevice(const std::string& device_id);

  // Disconnects from the device.
  bool DisconnectFromDevice(const std::string& device_id);

  bool SetConnectionStateChangeCallback(ConnectionStateChangeCallback on_change,
                                        ErrorCallback on_error);

  bool UnsetConnectionStateChangeCallback();

  std::shared_ptr<BleDevice> FindDeviceById(const std::string& device_id) {
    auto iter = devices_.find(device_id);
    if (iter != devices_.end()) {
      return iter->second;
    }
    return RegisterDevice(device_id);
  }

  int GetLastError() { return last_error_; }

  std::string GetLastErrorString() { return get_error_message(last_error_); }

 private:
  std::shared_ptr<BleDevice> RegisterDevice(const std::string& device_id) {
    auto device = std::make_shared<BleDevice>(device_id);
    if (!device->handle()) {
      return nullptr;
    }
    devices_[device_id] = std::move(device);
    return devices_[device_id];
  }

  bool is_scanning_ = false;

  BleStatusChangeCallback ble_status_change_callback_;
  DeviceScanCallback device_scan_callback_;
  ErrorCallback device_scan_error_callback_;
  ConnectionStateChangeCallback connection_state_change_callback_;
  ErrorCallback connection_error_callback_;

  std::unordered_map<std::string, std::shared_ptr<BleDevice>> devices_;
  std::unordered_map<std::string, std::string> cached_device_names_;
  std::vector<ScanFilter> filters_;

  int last_error_ = BT_ERROR_NONE;
};

#endif  // FLUTTER_PLUGIN_BLE_TIZEN_H_
