#ifndef FLUTTER_BLUE_TIZEN_BLUETOOTH_DEVICE_CONTROLLER_H
#define FLUTTER_BLUE_TIZEN_BLUETOOTH_DEVICE_CONTROLLER_H

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

namespace flutter_blue_tizen {

class BluetoothDeviceController {
 public:
  enum class State {
    kConnected,
    kConnecting,
    kDisconnected,
    kDisconnecting,
  };

  enum class Bond {
    unknown,
    created,
    not_created,
  };

  using RequestMtuCallback =
      std::function<void(bool, const BluetoothDeviceController&)>;

  using ConnectionStateChangedCallback =
      std::function<void(State state, const BluetoothDeviceController*
                                          device)>;  // TODO should be ref

  using ReadRssiCallback =
      std::function<void(const BluetoothDeviceController& device, int rssi)>;

  using PairCallback = std::function<void(
      const BluetoothDeviceController& device, const Bond bond)>;

  BluetoothDeviceController(const std::string& name,
                            const std::string& address) noexcept;

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

  void ReadRssi(ReadRssiCallback callback);

  void Pair(PairCallback callback);

  static void SetConnectionStateChangedCallback(
      ConnectionStateChangedCallback connection_changed_callback);

 private:
  std::mutex operation_mutex_;

  std::vector<std::unique_ptr<btGatt::PrimaryService>> services_;

  std::string name_;

  std::string address_;

  std::atomic<Bond> bond_state_{Bond::unknown};

  std::atomic<bool> is_connecting_ = false;

  std::atomic<bool> is_disconnecting_ = false;

  static inline std::function<void(State state,
                                   const BluetoothDeviceController* device)>
      connection_changed_callback_;

  static inline SafeType<std::map<std::string, BluetoothDeviceController*>>
      active_devices_;

  static inline SafeType<std::unordered_map<std::string, bt_gatt_client_h>>
      gatt_clients_;

  static bt_gatt_client_h GetGattClient(const std::string& address);

  static void DestroyGattClientIfExists(const std::string& address) noexcept;
};

}  // namespace flutter_blue_tizen
#endif  // FLUTTER_BLUE_TIZEN_BLUETOOTH_DEVICE_CONTROLLER_H
