#include <BluetoothDeviceController.h>
#include <GATT/BluetoothCharacteristic.h>
#include <GATT/BluetoothDescriptor.h>
#include <GATT/BluetoothService.h>
#include <Logger.h>
#include <Utils.h>

namespace flutter_blue_tizen {
namespace btGatt {
using btlog::Logger;
using btlog::LogLevel;
using btu::BTException;
BluetoothDescriptor::BluetoothDescriptor(
    bt_gatt_h handle, BluetoothCharacteristic& characteristic)
    : _handle(handle), _characteristic(characteristic) {
  std::scoped_lock lock(_activeDescriptors.mut);
  _activeDescriptors.var[UUID()] = this;
}

proto::gen::BluetoothDescriptor BluetoothDescriptor::toProtoDescriptor()
    const noexcept {
  proto::gen::BluetoothDescriptor proto;
  proto.set_remote_id(_characteristic.cService().cDevice().cAddress());
  proto.set_serviceuuid(_characteristic.cService().UUID());
  proto.set_characteristicuuid(_characteristic.UUID());
  proto.set_uuid(UUID());
  return proto;
}
std::string BluetoothDescriptor::UUID() const noexcept {
  return btu::getGattUUID(_handle);
}
std::string BluetoothDescriptor::value() const noexcept {
  return btu::getGattValue(_handle);
}
void BluetoothDescriptor::read(
    const std::function<void(const BluetoothDescriptor&)>& func) {
  struct Scope {
    std::function<void(const BluetoothDescriptor&)> func;
    const std::string descriptor_uuid;
  };
  auto scope = new Scope{func, UUID()};  // unfortunately it requires raw ptr
  int res = bt_gatt_client_read_value(
      _handle,
      [](int result, bt_gatt_h request_handle, void* scope_ptr) {
        Logger::log(LogLevel::DEBUG, "called native descriptor read cb");
        auto scope = static_cast<Scope*>(scope_ptr);
        std::scoped_lock lock(_activeDescriptors.mut);
        auto it = _activeDescriptors.var.find(scope->descriptor_uuid);

        if (it != _activeDescriptors.var.end() && !result) {
          auto& descriptor = *it->second;
          scope->func(descriptor);
        }
        Logger::showResultError("bt_gatt_client_request_completed_cb", result);

        delete scope;
      },
      scope);
  Logger::showResultError("bt_gatt_client_read_value", res);
  if (res) throw BTException("could not read descriptor");
}
void BluetoothDescriptor::write(
    const std::string value,
    const std::function<void(bool success, const BluetoothDescriptor&)>&
        callback) {
  struct Scope {
    std::function<void(bool success, const BluetoothDescriptor&)> func;
    const std::string descriptor_uuid;
  };
  Logger::log(LogLevel::DEBUG,
              "setting descriptor to value=" + value +
                  ", with size=" + std::to_string(value.size()));
  int res = bt_gatt_set_value(_handle, value.c_str(), value.size());
  Logger::showResultError("bt_gatt_set_value", res);

  if (res) throw BTException("could not set value");

  auto scope = new Scope{callback, UUID()};  // unfortunately it requires raw
                                             // ptr

  res = bt_gatt_client_write_value(
      _handle,
      [](int result, bt_gatt_h request_handle, void* scope_ptr) {
        Logger::showResultError("bt_gatt_client_request_completed_cb", result);
        Logger::log(LogLevel::DEBUG, "descriptor native write cb native");

        auto scope = static_cast<Scope*>(scope_ptr);
        std::scoped_lock lock(_activeDescriptors.mut);
        auto it = _activeDescriptors.var.find(scope->descriptor_uuid);

        if (it != _activeDescriptors.var.end()) {
          auto& descriptor = *it->second;
          scope->func(!result, descriptor);
        }

        delete scope;
      },
      scope);
  Logger::showResultError("bt_gatt_client_write_value", res);

  if (res) throw BTException("could not write value to remote");
}
BluetoothCharacteristic const& BluetoothDescriptor::cCharacteristic()
    const noexcept {
  return _characteristic;
}
BluetoothDescriptor::~BluetoothDescriptor() {
  std::scoped_lock lock(_activeDescriptors.mut);
  _activeDescriptors.var.erase(UUID());
}
}  // namespace btGatt
}  // namespace flutter_blue_tizen