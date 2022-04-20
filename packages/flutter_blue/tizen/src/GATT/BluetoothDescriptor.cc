#include <BluetoothDeviceController.h>
#include <GATT/BluetoothCharacteristic.h>
#include <GATT/BluetoothDescriptor.h>
#include <GATT/BluetoothService.h>
#include <Utils.h>
#include <log.h>

namespace flutter_blue_tizen {
namespace btGatt {

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
        auto scope = static_cast<Scope*>(scope_ptr);
        std::scoped_lock lock(_activeDescriptors.mut);
        auto it = _activeDescriptors.var.find(scope->descriptor_uuid);

        if (it != _activeDescriptors.var.end() && !result) {
          auto& descriptor = *it->second;
          scope->func(descriptor);
        }
        LOG_ERROR("bt_gatt_client_request_completed_cb",
                  get_error_message(result));
        delete scope;
      },
      scope);
  LOG_ERROR("bt_gatt_client_read_value", get_error_message(res));
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

  int res = bt_gatt_set_value(_handle, value.c_str(), value.size());
  LOG_ERROR("bt_gatt_set_value", get_error_message(res));

  if (res) throw BTException("could not set value");

  auto scope = new Scope{callback, UUID()};  // unfortunately it requires raw
                                             // ptr

  res = bt_gatt_client_write_value(
      _handle,
      [](int result, bt_gatt_h request_handle, void* scope_ptr) {
        LOG_ERROR("bt_gatt_client_request_completed_cb",
                  get_error_message(result));

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
  LOG_ERROR("bt_gatt_client_write_value", get_error_message(res));

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