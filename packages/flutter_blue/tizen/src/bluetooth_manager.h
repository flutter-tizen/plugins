#ifndef FLUTTER_BLUE_TIZEN_BLUETOOTH_MANAGER_H
#define FLUTTER_BLUE_TIZEN_BLUETOOTH_MANAGER_H

#include <bluetooth.h>
#include <notifications_handler.h>
#include <utils.h>

#include <atomic>
#include <memory>
#include <unordered_map>
#include <vector>

namespace flutter_blue_tizen {

namespace btGatt {

class BluetoothDescriptor;
class BluetoothCharacteristic;

}  // namespace btGatt

class BluetoothDeviceController;

class BluetoothManager {
  /**
   * @brief key - MAC address of the device
   */
  using DevicesContainer =
      SafeType<std::unordered_map<std::string,
                                  std::shared_ptr<BluetoothDeviceController>>>;

  DevicesContainer _bluetoothDevices;

  NotificationsHandler& _notificationsHandler;

  std::atomic<bool> _scanAllowDuplicates;

 public:
  BluetoothManager(NotificationsHandler& notificationsHandler);

  virtual ~BluetoothManager() noexcept = default;

  BluetoothManager(const BluetoothManager& bluetoothManager) = delete;

  void startBluetoothDeviceScanLE(const proto::gen::ScanSettings& scanSettings);

  void stopBluetoothDeviceScanLE();

  void connect(const proto::gen::ConnectRequest& connRequest);

  void disconnect(const std::string& deviceID);

  proto::gen::BluetoothState bluetoothState() const noexcept;

  std::vector<proto::gen::BluetoothDevice>
  getConnectedProtoBluetoothDevices() noexcept;

  DevicesContainer& bluetoothDevices() noexcept;

  void readCharacteristic(const proto::gen::ReadCharacteristicRequest& request);

  void readDescriptor(const proto::gen::ReadDescriptorRequest& request);

  void writeCharacteristic(
      proto::gen::WriteCharacteristicRequest const& request);

  void writeDescriptor(const proto::gen::WriteDescriptorRequest& request);

  void setNotification(const proto::gen::SetNotificationRequest& request);

  u_int32_t getMtu(const std::string& deviceID);

  void requestMtu(const proto::gen::MtuSizeRequest& request);

  btGatt::BluetoothCharacteristic* locateCharacteristic(
      const std::string& remoteID, const std::string& primaryUUID,
      const std::string& secondaryUUID, const std::string& characteristicUUID);

  btGatt::BluetoothDescriptor* locateDescriptor(
      const std::string& remoteID, const std::string& primaryUUID,
      const std::string& secondaryUUID, const std::string& characteristicUUID,
      const std::string& descriptorUUID);

  static bool isBLEAvailable();

  static void scanCallback(
      int result, bt_adapter_le_device_scan_result_info_s* discovery_info,
      void* user_data) noexcept;
};

void decodeAdvertisementData(const char* packetsData,
                             proto::gen::AdvertisementData& adv,
                             int dataLen) noexcept;

}  // namespace flutter_blue_tizen
#endif  // FLUTTER_BLUE_TIZEN_BLUETOOTH_MANAGER_H
