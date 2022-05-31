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
  int ret = bt_gatt_characteristic_foreach_descriptors(
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
  if (ret) throw BtException(ret, "bt_gatt_characteristic_foreach_descriptors");
  std::scoped_lock lock(active_characteristics_.mutex_);
  active_characteristics_.var_[Uuid()] = this;
}

proto::gen::BluetoothCharacteristic
BluetoothCharacteristic::ToProtoCharacteristic() const noexcept {
  proto::gen::BluetoothCharacteristic proto;
  proto.set_remote_id(service_.cDevice().cAddress());
  proto.set_uuid(Uuid());
  proto.set_allocated_properties(new proto::gen::CharacteristicProperties(
      GetProtoCharacteristicProperties(Properties())));
  proto.set_value(Value());
  if (service_.GetType() == ServiceType::kPrimary) {
    proto.set_serviceuuid(service_.Uuid());
  } else {
    SecondaryService& secondary = dynamic_cast<SecondaryService&>(service_);
    proto.set_serviceuuid(secondary.PrimaryUuid());
    proto.set_secondaryserviceuuid(secondary.Uuid());
  }
  for (const auto& descriptor : descriptors_) {
    *proto.add_descriptors() = descriptor->ToProtoDescriptor();
  }
  return proto;
}

const BluetoothService& BluetoothCharacteristic::cService() const noexcept {
  return service_;
}

std::string BluetoothCharacteristic::Uuid() const noexcept {
  return GetGattUuid(handle_);
}

std::string BluetoothCharacteristic::Value() const noexcept {
  return GetGattValue(handle_);
}

BluetoothDescriptor* BluetoothCharacteristic::GetDescriptor(
    const std::string& uuid) {
  for (auto& service : descriptors_)
    if (service->Uuid() == uuid) return service.get();
  return nullptr;
}

void BluetoothCharacteristic::Read(
    const std::function<void(const BluetoothCharacteristic&)>& callback) {
  struct Scope {
    std::function<void(const BluetoothCharacteristic&)> callback;
    const std::string characteristic_uuid;
  };

  Scope* scope = new Scope{callback, Uuid()};
  int ret = bt_gatt_client_read_value(
      handle_,
      [](int result, bt_gatt_h request_handle, void* scope_ptr) {
        auto scope = static_cast<Scope*>(scope_ptr);
        std::scoped_lock lock(active_characteristics_.mutex_);
        auto it = active_characteristics_.var_.find(scope->characteristic_uuid);
        if (it != active_characteristics_.var_.end()) {
          auto& characteristic = *it->second;
          scope->callback(characteristic);
          LOG_ERROR("bt_gatt_client_request_completed_cb",
                    get_error_message(result));
        }

        delete scope;
      },
      scope);

  LOG_ERROR("bt_gatt_client_read_value", get_error_message(ret));
  if (ret) throw BtException(ret, "could not read characteristic");
}

void BluetoothCharacteristic::Write(
    const std::string value, bool without_response,
    const std::function<void(bool success, const BluetoothCharacteristic&)>&
        callback) {
  struct Scope {
    std::function<void(bool success, const BluetoothCharacteristic&)> callback;
    const std::string characteristic_uuid;
  };

  int ret = bt_gatt_characteristic_set_write_type(
      handle_, without_response ? BT_GATT_WRITE_TYPE_WRITE_NO_RESPONSE
                                : BT_GATT_WRITE_TYPE_WRITE);
  LOG_ERROR("bt_gatt_characteristic_set_write_type", get_error_message(ret));

  if (ret)
    throw BtException(ret,
                      "could not set write type to characteristic " + Uuid());

  ret = bt_gatt_set_value(handle_, value.c_str(), value.size());
  LOG_ERROR("bt_gatt_set_value", get_error_message(ret));

  if (ret) throw BtException(ret, "could not set value");

  Scope* scope = new Scope{callback, Uuid()};

  ret = bt_gatt_client_write_value(
      handle_,
      [](int result, bt_gatt_h request_handle, void* scope_ptr) {
        LOG_ERROR("bt_gatt_client_request_completed_cb",
                  get_error_message(result));

        auto scope = static_cast<Scope*>(scope_ptr);
        std::scoped_lock lock(active_characteristics_.mutex_);
        auto it = active_characteristics_.var_.find(scope->characteristic_uuid);

        if (it != active_characteristics_.var_.end()) {
          auto& characteristic = *it->second;
          scope->callback(!result, characteristic);
        }

        delete scope;
      },
      scope);
  LOG_ERROR("bt_gatt_client_write_value", get_error_message(ret));

  if (ret) throw BtException("could not write value to remote");
}

int BluetoothCharacteristic::Properties() const noexcept {
  auto properties = 0;
  auto ret = bt_gatt_characteristic_get_properties(handle_, &properties);
  LOG_ERROR("bt_gatt_characteristic_get_properties", get_error_message(ret));
  return properties;
}

void BluetoothCharacteristic::SetNotifyCallback(
    const NotifyCallback& callback) {
  auto properties = Properties();
  if ((properties & 0x30) == 0)
    throw BtException("cannot set callback! notify=0 && indicate=0");

  UnsetNotifyCallback();
  notify_callback_ = std::make_unique<NotifyCallback>(callback);

  std::string* uuid = new std::string(Uuid());

  auto ret = bt_gatt_client_set_characteristic_value_changed_cb(
      handle_,
      [](bt_gatt_h characteristics_handle, char* value, int length,
         void* scope_ptr) {
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
            get_error_message(ret));
  if (ret)
    throw BtException(ret,
                      "bt_gatt_client_set_characteristic_value_changed_cb");
}

void BluetoothCharacteristic::UnsetNotifyCallback() {
  if (cService().cDevice().GetState() ==
          BluetoothDeviceController::State::kConnected &&
      notify_callback_) {
    auto ret = bt_gatt_client_unset_characteristic_value_changed_cb(handle_);
    LOG_ERROR("bt_gatt_client_unset_characteristic_value_changed_cb",
              get_error_message(ret));
  }
  notify_callback_ = nullptr;
}

BluetoothCharacteristic::~BluetoothCharacteristic() noexcept {
  std::scoped_lock lock(active_characteristics_.mutex_);
  active_characteristics_.var_.erase(Uuid());
  UnsetNotifyCallback();
  descriptors_.clear();
}

}  // namespace btGatt
}  // namespace flutter_blue_tizen