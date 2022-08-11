#include "GATT/bluetooth_characteristic.h"

#include "log.h"

namespace flutter_blue_tizen {
namespace btGatt {

BluetoothCharacteristic::BluetoothCharacteristic(bt_gatt_h handle)
    : handle_(handle) {
  int ret = bt_gatt_characteristic_foreach_descriptors(
      handle,
      [](int total, int index, bt_gatt_h descriptor_handle,
         void* data) -> bool {
        auto& characteristic = *static_cast<BluetoothCharacteristic*>(data);
        characteristic.descriptors_.emplace_back(
            std::make_unique<BluetoothDescriptor>(descriptor_handle));
        return true;
      },
      this);
  if (ret) throw BtException(ret, "bt_gatt_characteristic_foreach_descriptors");
  std::scoped_lock lock(active_characteristics_.mutex_);
  active_characteristics_.var_[Uuid()] = this;
}

BluetoothCharacteristic::~BluetoothCharacteristic() noexcept {
  std::scoped_lock lock(active_characteristics_.mutex_);
  UnsetNotifyCallback();
  descriptors_.clear();
  active_characteristics_.var_.erase(Uuid());
}

std::string BluetoothCharacteristic::Uuid() const noexcept {
  return GetGattUuid(handle_);
}

std::string BluetoothCharacteristic::Value() const noexcept {
  return GetGattValue(handle_);
}

BluetoothDescriptor* BluetoothCharacteristic::GetDescriptor(
    const std::string& uuid) const {
  for (const auto& descriptor : descriptors_)
    if (descriptor->Uuid() == uuid) return descriptor.get();
  return nullptr;
}

std::vector<BluetoothDescriptor*> BluetoothCharacteristic::GetDescriptors()
    const {
  std::vector<BluetoothDescriptor*> descriptors;
  for (const auto& descriptor : descriptors_) {
    descriptors.emplace_back(descriptor.get());
  }
  return descriptors;
}

void BluetoothCharacteristic::Read(ReadCallback callback) const {
  struct Scope {
    ReadCallback callback;
    const std::string uuid;
  };

  // Requires raw pointer to be passed to bt_gatt_client_read_value.
  Scope* scope = new Scope{std::move(callback), Uuid()};
  int ret = bt_gatt_client_read_value(
      handle_,
      [](int result, bt_gatt_h request_handle, void* scope_ptr) {
        auto scope = static_cast<Scope*>(scope_ptr);
        std::scoped_lock lock(active_characteristics_.mutex_);
        auto it = active_characteristics_.var_.find(scope->uuid);
        if (it != active_characteristics_.var_.end()) {
          auto& characteristic = *it->second;
          scope->callback(characteristic);
          LOG_ERROR("bt_gatt_client_request_completed_cb %s",
                    get_error_message(result));
        }

        delete scope;
      },
      scope);

  LOG_ERROR("bt_gatt_client_read_value %s", get_error_message(ret));
  if (ret) throw BtException(ret, "could not read characteristic");
}

void BluetoothCharacteristic::Write(const std::string value,
                                    bool without_response,
                                    WriteCallback callback) {
  struct Scope {
    WriteCallback callback;
    const std::string uuid;
  };

  int ret = bt_gatt_characteristic_set_write_type(
      handle_, without_response ? BT_GATT_WRITE_TYPE_WRITE_NO_RESPONSE
                                : BT_GATT_WRITE_TYPE_WRITE);
  LOG_ERROR("bt_gatt_characteristic_set_write_type %s", get_error_message(ret));

  if (ret)
    throw BtException(ret,
                      "could not set write type to characteristic " + Uuid());

  ret = bt_gatt_set_value(handle_, value.c_str(), value.size());
  LOG_ERROR("bt_gatt_set_value %s", get_error_message(ret));

  if (ret) throw BtException(ret, "could not set value");

  // Requires raw pointer to be passed to bt_gatt_client_write_value.
  Scope* scope = new Scope{std::move(callback), Uuid()};
  ret = bt_gatt_client_write_value(
      handle_,
      [](int result, bt_gatt_h request_handle, void* scope_ptr) {
        LOG_ERROR("bt_gatt_client_request_completed_cb %s",
                  get_error_message(result));

        auto scope = static_cast<Scope*>(scope_ptr);
        std::scoped_lock lock(active_characteristics_.mutex_);
        auto it = active_characteristics_.var_.find(scope->uuid);

        if (it != active_characteristics_.var_.end()) {
          auto& characteristic = *it->second;
          scope->callback(!result, characteristic);
        }

        delete scope;
      },
      scope);
  LOG_ERROR("bt_gatt_client_write_value %s", get_error_message(ret));

  if (ret) throw BtException("could not write value to remote");
}

int BluetoothCharacteristic::Properties() const noexcept {
  int properties = 0;
  auto ret = bt_gatt_characteristic_get_properties(handle_, &properties);
  LOG_ERROR("bt_gatt_characteristic_get_properties %s", get_error_message(ret));
  return properties;
}

void BluetoothCharacteristic::SetNotifyCallback(
    const NotifyCallback& callback) {
  auto properties = Properties();
  if ((properties & 0x30) == 0)
    throw BtException("cannot set callback! notify=0 && indicate=0");

  UnsetNotifyCallback();
  notify_callback_ = std::make_unique<NotifyCallback>(callback);

  auto ret = bt_gatt_client_set_characteristic_value_changed_cb(
      handle_,
      [](bt_gatt_h characteristics_handle, char* value, int length,
         void* data) {
        BluetoothCharacteristic& characteristic =
            *static_cast<BluetoothCharacteristic*>(data);

        std::scoped_lock lock(active_characteristics_.mutex_);
        auto it = active_characteristics_.var_.find(characteristic.Uuid());
        if (it != active_characteristics_.var_.end()) {
          characteristic.notify_callback_->operator()(characteristic);
        }
      },
      this);

  LOG_ERROR("bt_gatt_client_set_characteristic_value_changed_cb %s",
            get_error_message(ret));
  if (ret)
    throw BtException(ret,
                      "bt_gatt_client_set_characteristic_value_changed_cb");
}

void BluetoothCharacteristic::UnsetNotifyCallback() {
  if (notify_callback_) {
    int ret = bt_gatt_client_unset_characteristic_value_changed_cb(handle_);
    LOG_ERROR("bt_gatt_client_unset_characteristic_value_changed_cb %s",
              get_error_message(ret));
  }
  notify_callback_ = nullptr;
}

}  // namespace btGatt
}  // namespace flutter_blue_tizen
