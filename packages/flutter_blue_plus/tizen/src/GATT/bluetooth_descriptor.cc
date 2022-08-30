#include "GATT/bluetooth_descriptor.h"

#include "log.h"

namespace flutter_blue_plus_tizen::btGatt {

BluetoothDescriptor::BluetoothDescriptor(bt_gatt_h handle) : handle_(handle) {
  std::scoped_lock lock(active_descriptors_.mutex_);
  active_descriptors_.var_[Uuid()] = this;
}

BluetoothDescriptor::~BluetoothDescriptor() {
  std::scoped_lock lock(active_descriptors_.mutex_);
  active_descriptors_.var_.erase(Uuid());
}

std::string BluetoothDescriptor::Uuid() const noexcept {
  return GetGattUuid(handle_);
}

std::string BluetoothDescriptor::Value() const noexcept {
  return GetGattValue(handle_);
}

void BluetoothDescriptor::Read(ReadCallback callback) const {
  struct Scope {
    ReadCallback callback;
    const std::string descriptor_uuid;
  };

  /* Requires raw pointer to be passed to bt_gatt_client_read_value. */
  auto scope = new Scope{std::move(callback), Uuid()};
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
        LOG_ERROR("bt_gatt_client_request_completed_cb %s",
                  get_error_message(result));
        delete scope;
      },
      scope);

  LOG_ERROR("bt_gatt_client_read_value %s", get_error_message(ret));
  if (ret) throw BtException("could not read descriptor");
}

void BluetoothDescriptor::Write(const std::string value,
                                WriteCallback callback) {
  struct Scope {
    WriteCallback callback;
    const std::string descriptor_uuid;
  };

  int ret = bt_gatt_set_value(handle_, value.c_str(), value.size());
  LOG_ERROR("bt_gatt_set_value %s", get_error_message(ret));

  if (ret) throw BtException("could not set value");

  /* Requires raw pointer to be passed to bt_gatt_client_write_value. */
  auto scope = new Scope{std::move(callback), Uuid()};
  ret = bt_gatt_client_write_value(
      handle_,
      [](int result, bt_gatt_h request_handle, void* scope_ptr) {
        LOG_ERROR("bt_gatt_client_request_completed_cb %s",
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

  LOG_ERROR("bt_gatt_client_write_value %s", get_error_message(ret));

  if (ret) throw BtException("could not write value to remote");
}

}  // namespace flutter_blue_plus_tizen::btGatt
