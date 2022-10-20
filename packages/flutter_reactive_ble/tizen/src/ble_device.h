// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_BLE_DEVICE_H_
#define FLUTTER_PLUGIN_BLE_DEVICE_H_

#include <bluetooth_type.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "qualified_characteristic.h"

struct DiscoveredCharacteristic {
  Uuid characteristic_id;
  Uuid service_id;
  bool is_readable = false;
  bool is_writable_with_response = false;
  bool is_writable_without_response = false;
  bool is_notifiable = false;
  bool is_indicatable = false;
};

struct DiscoveredService {
  Uuid service_id;
  std::vector<Uuid> characteristic_ids;
  std::vector<DiscoveredCharacteristic> characteristics;
  std::vector<DiscoveredService> included_services;
};

typedef std::function<void()> VoidCallback;

typedef std::function<void(int error_code, const std::string& error_message)>
    ErrorCallback;

typedef std::function<void(int32_t result_mtu)> MtuNegotiationCallback;

typedef std::function<void(const QualifiedCharacteristic& characteristic,
                           const std::vector<uint8_t>& bytes)>
    NotificationCallback;

// A wrapper around the bt_gatt_client module.
class BleDevice {
 public:
  explicit BleDevice(const std::string& device_id);
  ~BleDevice();

  // Discovers services adverstised by this device.
  std::vector<DiscoveredService> DiscoverServices();

  // Requests a specific MTU for this device.
  bool NegotiateMtuSize(int32_t request_mtu, MtuNegotiationCallback on_done,
                        ErrorCallback on_error);

  // Reads a value from the characteristic.
  bool ReadCharacteristic(const QualifiedCharacteristic& characteristic,
                          VoidCallback on_done, ErrorCallback on_error);

  // Writes a value to the characteristic.
  bool WriteCharacteristic(const QualifiedCharacteristic& characteristic,
                           const std::vector<uint8_t>& value,
                           VoidCallback on_done, ErrorCallback on_error);

  // Subscribes to updates from the characteristic.
  bool ListenNotifications(const QualifiedCharacteristic& characteristic);

  // Unsubscribes from the characteristic.
  bool StopNotifications(const QualifiedCharacteristic& characteristic);

  void SetNotificationCallback(NotificationCallback on_event) {
    notification_callback_ = std::move(on_event);
  }

  std::shared_ptr<QualifiedCharacteristic> FindCharacteristicById(
      const Uuid& service_id, const Uuid& characteristic_id) {
    for (const auto& characteristic : characteristics_) {
      if (service_id == characteristic->service_id() &&
          characteristic_id == characteristic->characteristic_id()) {
        return characteristic;
      }
    }
    return RegisterCharacteristic(service_id, characteristic_id);
  }

  std::string device_id() const { return device_id_; }

  bt_gatt_client_h handle() const { return handle_; }

  int GetLastError() { return last_error_; }

  std::string GetLastErrorString() { return get_error_message(last_error_); }

 private:
  std::shared_ptr<QualifiedCharacteristic> RegisterCharacteristic(
      const Uuid& service_id, const Uuid& characteristic_id) {
    auto characteristic = std::make_shared<QualifiedCharacteristic>(
        this, service_id, characteristic_id);
    if (!characteristic->handle()) {
      return nullptr;
    }
    return characteristics_.emplace_back(std::move(characteristic));
  }

  std::shared_ptr<QualifiedCharacteristic> FindCharacteristic(
      bt_gatt_h handle) {
    for (const auto& characteristic : characteristics_) {
      if (handle == characteristic->handle()) {
        return characteristic;
      }
    }
    return nullptr;
  }

  std::string device_id_;
  bt_gatt_client_h handle_ = nullptr;

  std::vector<std::shared_ptr<QualifiedCharacteristic>> characteristics_;
  NotificationCallback notification_callback_;

  int last_error_ = BT_ERROR_NONE;
};

#endif  // FLUTTER_PLUGIN_BLE_DEVICE_H_
