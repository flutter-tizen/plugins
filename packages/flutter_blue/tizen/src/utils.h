#ifndef FLUTTER_BLUE_TIZEN_UTILS_H
#define FLUTTER_BLUE_TIZEN_UTILS_H

#include <bluetooth.h>
#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutterblue.pb.h>

#include <exception>
#include <mutex>

namespace flutter_blue_tizen {

namespace btGatt {

class PrimaryService;
class SecondaryService;

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

class BTException : public std::exception {
 public:
  BTException(const std::string& message);
  BTException(const int tizen_error, const std::string& message);

  BTException(const int tizen_error);

  const char* what() const noexcept override;

private:

  std::string message_;
};

std::vector<u_int8_t> messageToVector(
    const google::protobuf::MessageLite& message_lite) noexcept;

std::string getGattValue(bt_gatt_h handle);

std::string getGattUUID(bt_gatt_h handle);

bt_gatt_h getGattService(bt_gatt_client_h handle, const std::string& uuid);

std::string getGattClientAddress(bt_gatt_client_h handle);

proto::gen::DiscoverServicesResult getProtoServiceDiscoveryResult(
    const BluetoothDeviceController& device,
    const std::vector<btGatt::PrimaryService*>& services);

proto::gen::CharacteristicProperties getProtoCharacteristicProperties(
    int properties);

}  // namespace flutter_blue_tizen

#endif  // FLUTTER_BLUE_TIZEN_UTILS_H
