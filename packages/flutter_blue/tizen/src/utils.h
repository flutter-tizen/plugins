#ifndef FLUTTER_BLUE_TIZEN_UTILS_H
#define FLUTTER_BLUE_TIZEN_UTILS_H

#include <bluetooth.h>
#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>

#include <exception>
#include <mutex>

#include "flutterblue.pb.h"

namespace flutter_blue_tizen {

namespace btGatt {

class BluetoothService;
class PrimaryService;
class SecondaryService;
class BluetoothCharacteristic;
class BluetoothDescriptor;

}  // namespace btGatt

class BluetoothDeviceController;

using MethodChannel = flutter::MethodChannel<flutter::EncodableValue>;

template <typename T>
struct SafeType {
  T var_;

  std::mutex mutex_;

  SafeType(const T& t) : var_(t) {}

  SafeType(T&& t) : var_(std::move(t)) {}

  SafeType() : var_(T()) {}
};

class BtException : public std::exception {
 public:
  BtException(const std::string& message);
  BtException(const int tizen_error, const std::string& message);

  BtException(const int tizen_error);

  const char* what() const noexcept override;

 private:
  std::string message_;
};

std::vector<uint8_t> MessageToVector(
    const google::protobuf::MessageLite& message_lite) noexcept;

std::string GetGattValue(bt_gatt_h handle);

std::string GetGattUuid(bt_gatt_h handle);

bt_gatt_h GetGattService(bt_gatt_client_h handle, const std::string& uuid);

std::string GetGattClientAddress(bt_gatt_client_h handle);

proto::gen::DiscoverServicesResult GetProtoServiceDiscoveryResult(
    const BluetoothDeviceController& device,
    const std::vector<btGatt::PrimaryService*>& services);

proto::gen::CharacteristicProperties GetProtoCharacteristicProperties(
    int properties);

proto::gen::BluetoothService ToProtoService(
    const BluetoothDeviceController& device,
    const btGatt::BluetoothService& service) noexcept;

proto::gen::BluetoothCharacteristic ToProtoCharacteristic(
    const BluetoothDeviceController& device,
    const btGatt::BluetoothService& service,
    const btGatt::BluetoothCharacteristic& characteristic) noexcept;

proto::gen::BluetoothDescriptor ToProtoDescriptor(
    const BluetoothDeviceController& device,
    const btGatt::BluetoothService& service,
    const btGatt::BluetoothCharacteristic& characteristic,
    const btGatt::BluetoothDescriptor& descriptor) noexcept;

}  // namespace flutter_blue_tizen

#endif  // FLUTTER_BLUE_TIZEN_UTILS_H
