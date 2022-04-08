#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <bluetooth.h>

#include <vector>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <unordered_set>

#include <Utils.h>
#include <NotificationsHandler.h>

namespace btGatt{
    class BluetoothDescriptor;
    class BluetoothCharacteristic;
}

namespace btu{
     class BluetoothDeviceController;
     class BluetoothManager{
          /**
          * @brief key - MAC address of the device
          */
          SafeType<std::unordered_map<std::string, std::shared_ptr<BluetoothDeviceController>>> _bluetoothDevices;

          NotificationsHandler& _notificationsHandler;
          std::atomic<bool> _scanAllowDuplicates;

     public:
          
          BluetoothManager(NotificationsHandler& notificationsHandler);
          virtual ~BluetoothManager() noexcept=default;
          BluetoothManager(const BluetoothManager& bluetoothManager)=delete;
          
          auto startBluetoothDeviceScanLE(const proto::gen::ScanSettings& scanSettings) -> void;
          auto stopBluetoothDeviceScanLE() -> void;
          auto connect(const proto::gen::ConnectRequest& connRequest) -> void;
          auto disconnect(const std::string& deviceID) -> void;
          auto bluetoothState() const noexcept -> proto::gen::BluetoothState;
          auto getConnectedProtoBluetoothDevices() noexcept -> std::vector<proto::gen::BluetoothDevice>;
          auto bluetoothDevices() noexcept -> decltype(_bluetoothDevices)&;
          auto readCharacteristic(const proto::gen::ReadCharacteristicRequest& request) -> void;
          auto readDescriptor(const proto::gen::ReadDescriptorRequest& request) -> void;
          auto writeCharacteristic(const proto::gen::WriteCharacteristicRequest& request) -> void;
          auto writeDescriptor(const proto::gen::WriteDescriptorRequest& request) -> void;

          auto setNotification(const proto::gen::SetNotificationRequest& request) -> void;

          auto getMtu(const std::string& deviceID) -> u_int32_t;
          auto requestMtu(const proto::gen::MtuSizeRequest& request) -> void;

          auto locateCharacteristic(const std::string& remoteID, const std::string& primaryUUID, const std::string& secondaryUUID, 
          const std::string& characteristicUUID) -> btGatt::BluetoothCharacteristic*;

          auto locateDescriptor(const std::string& remoteID, const std::string& primaryUUID, const std::string& secondaryUUID, 
          const std::string& characteristicUUID, const std::string& descriptorUUID) -> btGatt::BluetoothDescriptor*;

          static auto isBLEAvailable() -> bool;
          static auto scanCallback(int result, bt_adapter_le_device_scan_result_info_s* discovery_info, void* user_data) noexcept -> void;
     };
} // namespace btu

#endif //BLUETOOTH_MANAGER_H
