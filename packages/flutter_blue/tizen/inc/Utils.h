#ifndef UTILS_H
#define UTILS_H

#include <bluetooth.h>
#include <flutter/encodable_value.h>
#include <flutter/method_channel.h>
#include <flutterblue.pb.h>

#include <exception>
#include <mutex>

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

  SafeType(const T& t) : var(t) {}
  SafeType(T&& t) : var(std::move(t)) {}
  SafeType() : var(T()) {}
};
class BTException : public std::exception {
  std::string _m;

 public:
  BTException(const std::string& m) : _m(m) {}
  BTException(const int tizen_error, std::string const& m)
      : _m(std::string(get_error_message(tizen_error)) + ": " + m) {}

  BTException(const int tizen_error) : _m(get_error_message(tizen_error)) {}

  const char* what() const noexcept override { return _m.c_str(); };
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
#endif  // UTILS_H
