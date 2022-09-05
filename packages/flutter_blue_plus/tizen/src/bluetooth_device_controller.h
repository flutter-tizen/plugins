// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_BLUE_PLUS_TIZEN_BLUETOOTH_DEVICE_CONTROLLER_H
#define FLUTTER_BLUE_PLUS_TIZEN_BLUETOOTH_DEVICE_CONTROLLER_H

#include <bluetooth.h>

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "GATT/bluetooth_service.h"
#include "utils.h"

namespace flutter_blue_plus_tizen {

class BluetoothDeviceController {
 public:
  enum class State {
    kConnected,
    kConnecting,
    kDisconnected,
    kDisconnecting,
  };

  enum class Bond {
    kUnknown,
    kCreated,
    kNotCreated,
  };

  using RequestMtuCallback =
      std::function<void(bool, const BluetoothDeviceController&)>;

  using ConnectionStateChangedCallback =
      std::function<void(State state, const BluetoothDeviceController& device)>;

  using ReadRssiCallback =
      std::function<void(const BluetoothDeviceController& device, int rssi)>;

  using PairCallback = std::function<void(
      const BluetoothDeviceController& device, const Bond bond)>;

  BluetoothDeviceController(std::string name, std::string address) noexcept;

  ~BluetoothDeviceController() noexcept;

  std::string name() const noexcept;

  std::string address() const noexcept;

  State GetState() const noexcept;

  void Connect(bool auto_connect);

  void Disconnect();

  void DiscoverServices();

  std::vector<btGatt::PrimaryService*> GetServices() noexcept;

  btGatt::PrimaryService* GetService(const std::string& uuid) noexcept;

  uint32_t GetMtu() const;

  void RequestMtu(uint32_t mtu, const RequestMtuCallback& callback);

  void ReadRssi(ReadRssiCallback callback) const;

  void Pair(PairCallback callback);

  static void SetConnectionStateChangedCallback(
      ConnectionStateChangedCallback connection_changed_callback);

 private:
  std::mutex operation_mutex_;

  std::vector<std::unique_ptr<btGatt::PrimaryService>> services_;

  std::string name_;

  std::string address_;

  std::atomic<Bond> bond_state_{Bond::kUnknown};

  std::atomic<bool> is_connecting_ = false;

  std::atomic<bool> is_disconnecting_ = false;

  static inline ConnectionStateChangedCallback
      connection_state_changed_callback_;

  static inline SafeType<std::map<std::string, BluetoothDeviceController*>>
      active_devices_;

  static inline SafeType<std::unordered_map<std::string, bt_gatt_client_h>>
      gatt_clients_;

  static bt_gatt_client_h GetGattClient(const std::string& address);

  static void DestroyGattClientIfExists(const std::string& address) noexcept;
};

}  // namespace flutter_blue_plus_tizen
#endif  // FLUTTER_BLUE_PLUS_TIZEN_BLUETOOTH_DEVICE_CONTROLLER_H
