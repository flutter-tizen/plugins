// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_BLUE_PLUS_TIZEN_BLUETOOTH_MANAGER_H
#define FLUTTER_BLUE_PLUS_TIZEN_BLUETOOTH_MANAGER_H

#include <app_event.h>
#include <bluetooth.h>

#include <atomic>
#include <memory>
#include <unordered_map>
#include <vector>

#include "GATT/bluetooth_characteristic.h"
#include "GATT/bluetooth_descriptor.h"
#include "GATT/bluetooth_service.h"
#include "bluetooth_device_controller.h"
#include "flutterblue.pb.h"
#include "notifications_handler.h"
#include "system_event_handler.h"
#include "utils.h"

namespace flutter_blue_plus_tizen {

class BluetoothManager {
 public:
  enum class BluetoothState {
    kAdapterOn,
    kAdapterOff,
    kUnavailable,
    kUnknown,
  };

  explicit BluetoothManager(
      std::shared_ptr<NotificationsHandler> notifications_handler);

  using ScanCallback = std::function<void(const std::string& address,
                                          const std::string& device_name,
                                          int rssi, const AdvertisementData&)>;

  /**
   * @brief this member function already contains a callback redirecting output
   * to flutter.
   */
  void StartBluetoothDeviceScanLE(const BleScanSettings& scan_settings);

  /**
   * @brief this static member is designed to use from other places such as
   * bluetooth_device_controller for rssi fetch.
   * It does not clear all the current devices.
   *
   */
  static void StartBluetoothDeviceScanLE(const BleScanSettings& scan_settings,
                                         ScanCallback callback);

  static void StopBluetoothDeviceScanLE();

  void Connect(const proto::gen::ConnectRequest& conn_request);

  void Disconnect(const std::string& device_id);

  static BluetoothState GetBluetoothState() noexcept;

  std::vector<proto::gen::BluetoothDevice>
  GetConnectedProtoBluetoothDevices() noexcept;

  void ReadCharacteristic(const proto::gen::ReadCharacteristicRequest& request);

  void ReadDescriptor(const proto::gen::ReadDescriptorRequest& request);

  void WriteCharacteristic(
      const proto::gen::WriteCharacteristicRequest& request);

  void WriteDescriptor(const proto::gen::WriteDescriptorRequest& request);

  void SetNotification(const proto::gen::SetNotificationRequest& request);

  uint32_t GetMtu(const std::string& device_id);

  void RequestMtu(const proto::gen::MtuSizeRequest& request);

  void ReadRssi(const std::string& device_id);

  void Pair(const std::string& device_id);

  std::vector<BluetoothDeviceController*> GetBondedDevices();

  BluetoothDeviceController& LocateDevice(const std::string& remote_id);

  btGatt::PrimaryService& LocatePrimaryService(const std::string& remote_id,
                                               const std::string& primary_uuid);

  btGatt::SecondaryService& LocateSecondaryService(
      const std::string& remote_id, const std::string& primary_uuid,
      const std::string& secondary_uuid);

  btGatt::BluetoothService& LocateService(const std::string& remote_id,
                                          const std::string& primary_uuid,
                                          const std::string& secondary_uuid);

  btGatt::BluetoothCharacteristic& LocateCharacteristic(
      const std::string& remote_id, const std::string& primary_uuid,
      const std::string& secondary_uuid,
      const std::string& characteristic_uuid);

  btGatt::BluetoothDescriptor& LocateDescriptor(
      const std::string& remote_id, const std::string& primary_uuid,
      const std::string& secondary_uuid, const std::string& characteristic_uuid,
      const std::string& descriptor_uuid);

  static bool IsBLEAvailable();

 private:
  /* Map key is device's mac address. */
  SafeType<std::unordered_map<std::string,
                              std::unique_ptr<BluetoothDeviceController>>>
      bluetooth_devices_;

  std::shared_ptr<NotificationsHandler> notifications_handler_;
};

}  // namespace flutter_blue_plus_tizen
#endif  // FLUTTER_BLUE_PLUS_TIZEN_BLUETOOTH_MANAGER_H
