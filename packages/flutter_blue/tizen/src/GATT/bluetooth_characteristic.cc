#include <GATT/bluetooth_characteristic.h>
#include <GATT/bluetooth_descriptor.h>
#include <GATT/bluetooth_service.h>
#include <bluetooth_device_controller.h>
#include <bluetooth_manager.h>
#include <log.h>
#include <notifications_handler.h>
#include <utils.h>

#include <exception>

namespace flutter_blue_tizen {
namespace btGatt {

BluetoothCharacteristic::BluetoothCharacteristic(bt_gatt_h handle,
                                                 BluetoothService& service)
    : handle_(handle), service_(service) {
  int res = bt_gatt_characteristic_foreach_descriptors(
      handle,
      [](int total, int index, bt_gatt_h descriptor_handle,
         void* scope_ptr) -> bool {
        auto& characteristic =
            *static_cast<BluetoothCharacteristic*>(scope_ptr);
        characteristic.descriptors_.emplace_back(
            std::make_unique<BluetoothDescriptor>(descriptor_handle,
                                                  characteristic));
        return true;
      },
      this);
  if (res) throw BTException(res, "bt_gatt_characteristic_foreach_descriptors");
  std::scoped_lock lock(active_characteristics_.mutex_);
  active_characteristics_.var_[UUID()] = this;
}

proto::gen::BluetoothCharacteristic
BluetoothCharacteristic::toProtoCharacteristic() const noexcept {
  proto::gen::BluetoothCharacteristic proto;
  proto.set_remote_id(service_.cDevice().cAddress());
  proto.set_uuid(UUID());
  proto.set_allocated_properties(new proto::gen::CharacteristicProperties(
      getProtoCharacteristicProperties(properties())));
  proto.set_value(value());
  if (service_.getType() == ServiceType::PRIMARY)
    proto.set_serviceuuid(service_.UUID());
  else {
    SecondaryService& sec = dynamic_cast<SecondaryService&>(service_);
    proto.set_serviceuuid(sec.UUID());
    proto.set_secondaryserviceuuid(sec.primaryUUID());
  }
  for (const auto& descriptor : descriptors_) {
    *proto.add_descriptors() = descriptor->toProtoDescriptor();
  }
  return proto;
}

const BluetoothService& BluetoothCharacteristic::cService() const noexcept {
  return service_;
}

std::string BluetoothCharacteristic::UUID() const noexcept {
  return getGattUUID(handle_);
}

std::string BluetoothCharacteristic::value() const noexcept {
  return getGattValue(handle_);
}

BluetoothDescriptor* BluetoothCharacteristic::getDescriptor(
    const std::string& uuid) {
  for (auto& s : descriptors_)
    if (s->UUID() == uuid) return s.get();
  return nullptr;
}

void BluetoothCharacteristic::read(
    const std::function<void(const BluetoothCharacteristic&)>& func) {
  struct Scope {
    std::function<void(const BluetoothCharacteristic&)> func;
    const std::string characteristic_uuid;
  };

  Scope* scope = new Scope{func, UUID()};
  int res = bt_gatt_client_read_value(
      handle_,
      [](int result, bt_gatt_h request_handle, void* scope_ptr) {
        auto scope = static_cast<Scope*>(scope_ptr);
        std::scoped_lock lock(active_characteristics_.mutex_);
        auto it = active_characteristics_.var_.find(scope->characteristic_uuid);
        if (it != active_characteristics_.var_.end()) {
          auto& characteristic = *it->second;
          scope->func(characteristic);
          LOG_ERROR("bt_gatt_client_request_completed_cb",
                    get_error_message(result));
        }

        delete scope;
      },
      scope);

  LOG_ERROR("bt_gatt_client_read_value", get_error_message(res));
  if (res) throw BTException(res, "could not read characteristic");
}

void BluetoothCharacteristic::write(
    const std::string value, bool withoutResponse,
    const std::function<void(bool success, const BluetoothCharacteristic&)>&
        callback) {
  struct Scope {
    std::function<void(bool success, const BluetoothCharacteristic&)> func;
    const std::string characteristic_uuid;
  };

  int res = bt_gatt_characteristic_set_write_type(
      handle_, withoutResponse ? BT_GATT_WRITE_TYPE_WRITE_NO_RESPONSE
                               : BT_GATT_WRITE_TYPE_WRITE);
  LOG_ERROR("bt_gatt_characteristic_set_write_type", get_error_message(res));

  if (res)
    throw BTException(res,
                      "could not set write type to characteristic " + UUID());

  res = bt_gatt_set_value(handle_, value.c_str(), value.size());
  LOG_ERROR("bt_gatt_set_value", get_error_message(res));

  if (res) throw BTException(res, "could not set value");

  Scope* scope = new Scope{callback, UUID()};

  res = bt_gatt_client_write_value(
      handle_,
      [](int result, bt_gatt_h request_handle, void* scope_ptr) {
        LOG_ERROR("bt_gatt_client_request_completed_cb",
                  get_error_message(result));

        auto scope = static_cast<Scope*>(scope_ptr);
        std::scoped_lock lock(active_characteristics_.mutex_);
        auto it = active_characteristics_.var_.find(scope->characteristic_uuid);

        if (it != active_characteristics_.var_.end()) {
          auto& characteristic = *it->second;
          scope->func(!result, characteristic);
        }

        delete scope;
      },
      scope);
  LOG_ERROR("bt_gatt_client_write_value", get_error_message(res));

  if (res) throw BTException("could not write value to remote");
}

int BluetoothCharacteristic::properties() const noexcept {
  auto prop = 0;
  auto res = bt_gatt_characteristic_get_properties(handle_, &prop);
  LOG_ERROR("bt_gatt_characteristic_get_properties", get_error_message(res));
  return prop;
}

void BluetoothCharacteristic::setNotifyCallback(
    const NotifyCallback& callback) {
  auto p = properties();
  if (!(p & 0x30))
    throw BTException("cannot set callback! notify=0 && indicate=0");

  unsetNotifyCallback();
  notify_callback_ = std::make_unique<NotifyCallback>(callback);

  std::string* uuid = new std::string(UUID());

  auto res = bt_gatt_client_set_characteristic_value_changed_cb(
      handle_,
      [](bt_gatt_h ch_handle, char* value, int len, void* scope_ptr) {
        std::string* uuid = static_cast<std::string*>(scope_ptr);

        std::scoped_lock lock(active_characteristics_.mutex_);
        auto it = active_characteristics_.var_.find(*uuid);

        if (it != active_characteristics_.var_.end()) {
          auto& characteristic = *it->second;
          characteristic.notify_callback_->operator()(characteristic);
        }

        delete uuid;
      },
      uuid);
  LOG_ERROR("bt_gatt_client_set_characteristic_value_changed_cb",
            get_error_message(res));
  if (res)
    throw BTException(res,
                      "bt_gatt_client_set_characteristic_value_changed_cb");
}

void BluetoothCharacteristic::unsetNotifyCallback() {
  if (cService().cDevice().state() ==
          BluetoothDeviceController::State::CONNECTED &&
      notify_callback_) {
    auto res = bt_gatt_client_unset_characteristic_value_changed_cb(handle_);
    LOG_ERROR("bt_gatt_client_unset_characteristic_value_changed_cb",
              get_error_message(res));
  }
  notify_callback_ = nullptr;
}

BluetoothCharacteristic::~BluetoothCharacteristic() noexcept {
  std::scoped_lock lock(active_characteristics_.mutex_);
  active_characteristics_.var_.erase(UUID());
  unsetNotifyCallback();
  descriptors_.clear();
}

}  // namespace btGatt
}  // namespace flutter_blue_tizen