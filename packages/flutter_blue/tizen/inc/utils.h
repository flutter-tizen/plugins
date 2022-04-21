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
namespace btu {
class BluetoothDeviceController;

using MethodChannel = flutter::MethodChannel<flutter::EncodableValue>;

template <typename T>
struct SafeType {
  T var;

  std::mutex mut;

  SafeType(T const& t) : var(t) {}

  SafeType(T&& t) : var(std::move(t)) {}

  SafeType() : var(T()) {}
};

class BTException : public std::exception {
  std::string _mess;

 public:
  BTException(std::string const& mess);
  BTException(const int tizen_error, std::string const& mess);

  BTException(const int tizen_error);

  const char* what() const noexcept override;
};

std::vector<u_int8_t> messageToVector(
    google::protobuf::MessageLite const& messageLite) noexcept;

std::string getGattValue(bt_gatt_h handle);

std::string getGattUUID(bt_gatt_h handle);

bt_gatt_h getGattService(bt_gatt_client_h handle, const std::string& uuid);

std::string getGattClientAddress(bt_gatt_client_h handle);

proto::gen::DiscoverServicesResult getProtoServiceDiscoveryResult(
    BluetoothDeviceController const& device,
    std::vector<btGatt::PrimaryService*> const& services);

proto::gen::CharacteristicProperties getProtoCharacteristicProperties(
    int properties);
}  // namespace btu
}  // namespace flutter_blue_tizen
#endif  // FLUTTER_BLUE_TIZEN_UTILS_H
