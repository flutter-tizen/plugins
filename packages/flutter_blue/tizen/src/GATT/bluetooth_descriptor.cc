#include <GATT/bluetooth_characteristic.h>
#include <GATT/bluetooth_descriptor.h>
#include <GATT/bluetooth_service.h>
#include <bluetooth_device_controller.h>
#include <log.h>
#include <utils.h>

namespace flutter_blue_tizen {
namespace btGatt {

BluetoothDescriptor::BluetoothDescriptor(
    bt_gatt_h handle, BluetoothCharacteristic& characteristic)
    : handle_(handle), characteristic_(characteristic) {
  std::scoped_lock lock(active_descriptors_.mutex_);
  active_descriptors_.var_[UUID()] = this;
}

proto::gen::BluetoothDescriptor BluetoothDescriptor::toProtoDescriptor()
    const noexcept {
  proto::gen::BluetoothDescriptor proto;
  proto.set_remote_id(characteristic_.cService().cDevice().cAddress());
  proto.set_serviceuuid(characteristic_.cService().UUID());
  proto.set_characteristicuuid(characteristic_.UUID());
  proto.set_uuid(UUID());
  return proto;
}

std::string BluetoothDescriptor::UUID() const noexcept {
  return getGattUUID(handle_);
}

std::string BluetoothDescriptor::value() const noexcept {
  return getGattValue(handle_);
}

void BluetoothDescriptor::read(
    const std::function<void(const BluetoothDescriptor&)>& func) {
  struct Scope {
    std::function<void(const BluetoothDescriptor&)> func;
    const std::string descriptor_uuid;
  };

  auto scope = new Scope{func, UUID()};  // unfortunately it requires raw ptr
  int res = bt_gatt_client_read_value(
      handle_,
      [](int result, bt_gatt_h request_handle, void* scope_ptr) {
        auto scope = static_cast<Scope*>(scope_ptr);
        std::scoped_lock lock(active_descriptors_.mutex_);
        auto it = active_descriptors_.var_.find(scope->descriptor_uuid);

        if (it != active_descriptors_.var_.end() && !result) {
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

  int res = bt_gatt_set_value(handle_, value.c_str(), value.size());
  LOG_ERROR("bt_gatt_set_value", get_error_message(res));

  if (res) throw BTException("could not set value");

  auto scope = new Scope{callback, UUID()};  // unfortunately it requires raw
                                             // ptr

  res = bt_gatt_client_write_value(
      handle_,
      [](int result, bt_gatt_h request_handle, void* scope_ptr) {
        LOG_ERROR("bt_gatt_client_request_completed_cb",
                  get_error_message(result));

        auto scope = static_cast<Scope*>(scope_ptr);
        std::scoped_lock lock(active_descriptors_.mutex_);
        auto it = active_descriptors_.var_.find(scope->descriptor_uuid);

        if (it != active_descriptors_.var_.end()) {
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
  return characteristic_;
}

BluetoothDescriptor::~BluetoothDescriptor() {
  std::scoped_lock lock(active_descriptors_.mutex_);
  active_descriptors_.var_.erase(UUID());
}

}  // namespace btGatt
}  // namespace flutter_blue_tizen