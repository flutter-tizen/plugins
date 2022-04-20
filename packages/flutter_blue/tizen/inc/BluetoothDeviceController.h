#ifndef FLUTTER_BLUE_TIZEN_BLUETOOTH_DEVICE_CONTROLLER_H
#define FLUTTER_BLUE_TIZEN_BLUETOOTH_DEVICE_CONTROLLER_H
#include <Utils.h>
#include <bluetooth.h>
#include <flutterblue.pb.h>

namespace flutter_blue_tizen {
namespace btGatt {
class PrimaryService;
class SecondaryService;
}  // namespace btGatt

namespace btu {
class NotificationsHandler;
class BluetoothService;
class BluetoothDeviceController {
  /**
   * @brief all attributes are depentent on this mutex
   */
  std::mutex operationM;

  std::vector<proto::gen::BluetoothDevice> _protoBluetoothDevices;

  std::vector<std::unique_ptr<btGatt::PrimaryService>> _services;

  std::string _address;

  std::atomic<bool> isConnecting = false;

  std::atomic<bool> isDisconnecting = false;

  NotificationsHandler& _notificationsHandler;

  using requestMtuCallback =
      std::function<void(bool, const BluetoothDeviceController&)>;

  static inline SafeType<std::map<std::string, BluetoothDeviceController*>>
      _activeDevices;

  static inline SafeType<std::unordered_map<std::string, bt_gatt_client_h>>
      gatt_clients;

 public:
 
  enum class State {
    CONNECTED,
    CONNECTING,
    DISCONNECTED,
    DISCONNECTING,
  };

  BluetoothDeviceController(
      const std::string& address,
      NotificationsHandler& notificationsHandler) noexcept;

  BluetoothDeviceController(
      const char* address, NotificationsHandler& notificationsHandler) noexcept;

  ~BluetoothDeviceController() noexcept;

  BluetoothDeviceController() = delete;

  BluetoothDeviceController( BluetoothDeviceController const& address) = delete;

  const std::string& cAddress() const noexcept;

  State state() const noexcept;

  std::vector<proto::gen::BluetoothDevice>& protoBluetoothDevices() noexcept;

  const std::vector<proto::gen::BluetoothDevice>& cProtoBluetoothDevices()
      const noexcept;

  void connect(bool autoConnect);

  void disconnect();

  static void connectionStateCallback(int result, bool connected,
                                      const char* remote_address,
                                      void* user_data) noexcept;

  static bt_gatt_client_h getGattClient(const std::string& address);

  static void destroyGattClientIfExists(const std::string& address) noexcept;
  
  static proto::gen::DeviceStateResponse_BluetoothDeviceState
  localToProtoDeviceState(const BluetoothDeviceController::State& s);

  void discoverServices();

  std::vector<btGatt::PrimaryService*> getServices() noexcept;

  btGatt::PrimaryService* getService(const std::string& uuid) noexcept;

  u_int32_t getMtu() const;

  void requestMtu(u_int32_t mtu, const requestMtuCallback& callback);

  void notifyDeviceState() const;
  
  const NotificationsHandler& cNotificationsHandler() const noexcept;
};
};  // namespace btu
}  // namespace flutter_blue_tizen
#endif  // FLUTTER_BLUE_TIZEN_BLUETOOTH_DEVICE_CONTROLLER_H
