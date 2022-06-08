#include "GATT/bluetooth_descriptor.h"

#include "GATT/bluetooth_characteristic.h"
#include "GATT/bluetooth_service.h"
#include "bluetooth_device_controller.h"
#include "log.h"
#include "utils.h"

namespace flutter_blue_tizen {
namespace btGatt {

BluetoothDescriptor::BluetoothDescriptor(bt_gatt_h handle) : handle_(handle) {
  std::scoped_lock lock(active_descriptors_.mutex_);
  active_descriptors_.var_[Uuid()] = this;
}

std::string BluetoothDescriptor::Uuid() const noexcept {
  return GetGattUuid(handle_);
}

std::string BluetoothDescriptor::Value() const noexcept {
  return GetGattValue(handle_);
}

void BluetoothDescriptor::Read(
    const std::function<void(const BluetoothDescriptor&)>& callback) {
  struct Scope {
    std::function<void(const BluetoothDescriptor&)> callback;
    const std::string descriptor_uuid;
  };

  auto scope =
      new Scope{callback, Uuid()};  // unfortunately it requires raw ptr
  int ret = bt_gatt_client_read_value(
      handle_,
      [](int result, bt_gatt_h request_handle, void* scope_ptr) {
        auto scope = static_cast<Scope*>(scope_ptr);
        std::scoped_lock lock(active_descriptors_.mutex_);
        auto it = active_descriptors_.var_.find(scope->descriptor_uuid);

        if (it != active_descriptors_.var_.end() && !result) {
          auto& descriptor = *it->second;
          scope->callback(descriptor);
        }
        LOG_ERROR("bt_gatt_client_request_completed_cb",
                  get_error_message(result));
        delete scope;
      },
      scope);

  LOG_ERROR("bt_gatt_client_read_value", get_error_message(ret));
  if (ret) throw BtException("could not read descriptor");
}

void BluetoothDescriptor::Write(
    const std::string value,
    const std::function<void(bool success, const BluetoothDescriptor&)>&
        callback) {
  struct Scope {
    std::function<void(bool success, const BluetoothDescriptor&)> callback;
    const std::string descriptor_uuid;
  };

  int ret = bt_gatt_set_value(handle_, value.c_str(), value.size());
  LOG_ERROR("bt_gatt_set_value", get_error_message(ret));

  if (ret) throw BtException("could not set value");

  auto scope = new Scope{callback, Uuid()};  // unfortunately it requires raw
                                             // ptr

  ret = bt_gatt_client_write_value(
      handle_,
      [](int result, bt_gatt_h request_handle, void* scope_ptr) {
        LOG_ERROR("bt_gatt_client_request_completed_cb",
                  get_error_message(result));

        auto scope = static_cast<Scope*>(scope_ptr);
        std::scoped_lock lock(active_descriptors_.mutex_);
        auto it = active_descriptors_.var_.find(scope->descriptor_uuid);

        if (it != active_descriptors_.var_.end()) {
          auto& descriptor = *it->second;
          scope->callback(!result, descriptor);
        }

        delete scope;
      },
      scope);

  LOG_ERROR("bt_gatt_client_write_value", get_error_message(ret));

  if (ret) throw BtException("could not write value to remote");
}

BluetoothDescriptor::~BluetoothDescriptor() {
  std::scoped_lock lock(active_descriptors_.mutex_);
  active_descriptors_.var_.erase(Uuid());
}

}  // namespace btGatt
}  // namespace flutter_blue_tizen