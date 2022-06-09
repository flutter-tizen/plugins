#ifndef FLUTTER_BLUE_TIZEN_BLUETOOTH_DEVICE_CONTROLLER_H
#define FLUTTER_BLUE_TIZEN_BLUETOOTH_DEVICE_CONTROLLER_H

#include <bluetooth.h>

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "GATT/bluetooth_service.h"
#include "notifications_handler.h"
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

  using requestMtuCallback =
      std::function<void(bool, const BluetoothDeviceController&)>;

  BluetoothDeviceController(
      const std::string& name, const std::string& address,
      NotificationsHandler& notifications_handler) noexcept;

  ~BluetoothDeviceController() noexcept;

  BluetoothDeviceController() = delete;

  BluetoothDeviceController(const BluetoothDeviceController& address) = delete;

  const std::string& cName() const noexcept;

  const std::string& cAddress() const noexcept;

  State GetState() const noexcept;

  void Connect(bool auto_connect);

  void Disconnect();

  static void ConnectionStateCallback(int result, bool connected,
                                      const char* remote_address,
                                      void* user_data) noexcept;

  static bt_gatt_client_h GetGattClient(const std::string& address);

  static void DestroyGattClientIfExists(const std::string& address) noexcept;

  void DiscoverServices();

  std::vector<btGatt::PrimaryService*> GetServices() noexcept;

  btGatt::PrimaryService* GetService(const std::string& uuid) noexcept;

  uint32_t GetMtu() const;

  void RequestMtu(uint32_t mtu, const requestMtuCallback& callback);

  void NotifyDeviceState() const;

  const NotificationsHandler& cNotificationsHandler() const noexcept;

 private:
  /**
   * @brief all attributes are depentent on this mutex
   */

  std::mutex operation_mutex_;

  std::vector<std::unique_ptr<btGatt::PrimaryService>> services_;

  std::string name_;

  std::string address_;

  std::atomic<bool> is_connecting_ = false;

  std::atomic<bool> is_disconnecting_ = false;

  NotificationsHandler& notifications_handler_;

  static inline SafeType<std::map<std::string, BluetoothDeviceController*>>
      active_devices_;

  static inline SafeType<std::unordered_map<std::string, bt_gatt_client_h>>
      gatt_clients_;
};

}  // namespace flutter_blue_tizen
#endif  // FLUTTER_BLUE_TIZEN_BLUETOOTH_DEVICE_CONTROLLER_H
