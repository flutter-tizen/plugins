#ifndef FLUTTER_BLUE_TIZEN_BLUETOOTH_MANAGER_H
#define FLUTTER_BLUE_TIZEN_BLUETOOTH_MANAGER_H

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
#include "utils.h"

namespace flutter_blue_tizen {

class BluetoothManager {
 public:
  BluetoothManager(NotificationsHandler& notifications_handler);

  ~BluetoothManager() noexcept = default;

  void StartBluetoothDeviceScanLE(
      const proto::gen::ScanSettings& scan_settings);

  void StopBluetoothDeviceScanLE();

  void Connect(const proto::gen::ConnectRequest& conn_request);

  void Disconnect(const std::string& device_id);

  proto::gen::BluetoothState BluetoothState() const noexcept;

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

  BluetoothDeviceController* LocateDevice(const std::string& remote_id);

  btGatt::PrimaryService* LocatePrimaryService(const std::string& remote_id,
                                               const std::string& primary_uuid);

  btGatt::SecondaryService* LocateSecondaryService(
      const std::string& remote_id, const std::string& primary_uuid,
      const std::string& secondary_uuid);

  btGatt::BluetoothService* LocateService(const std::string& remote_id,
                                          const std::string& primary_uuid,
                                          const std::string& secondary_uuid);

  btGatt::BluetoothCharacteristic* LocateCharacteristic(
      const std::string& remote_id, const std::string& primary_uuid,
      const std::string& secondary_uuid,
      const std::string& characteristic_uuid);

  btGatt::BluetoothDescriptor* LocateDescriptor(
      const std::string& remote_id, const std::string& primary_uuid,
      const std::string& secondary_uuid, const std::string& characteristic_uuid,
      const std::string& descriptor_uuid);

  static bool IsBLEAvailable();

  static void ScanCallback(
      int result, bt_adapter_le_device_scan_result_info_s* discovery_info,
      void* user_data) noexcept;

 private:
  // Map key is device's mac address.
  SafeType<std::unordered_map<std::string,
                              std::unique_ptr<BluetoothDeviceController>>>
      bluetooth_devices_;

  NotificationsHandler& notifications_handler_;

  std::atomic<bool> scan_allow_duplicates_;
};

void DecodeAdvertisementData(const char* packets_data,
                             proto::gen::AdvertisementData& advertisement,
                             int data_len) noexcept;

}  // namespace flutter_blue_tizen
#endif  // FLUTTER_BLUE_TIZEN_BLUETOOTH_MANAGER_H