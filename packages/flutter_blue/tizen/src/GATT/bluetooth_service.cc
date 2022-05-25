#include <GATT/bluetooth_characteristic.h>
#include <GATT/bluetooth_service.h>
#include <bluetooth.h>
#include <bluetooth_device_controller.h>
#include <log.h>

namespace flutter_blue_tizen {
namespace btGatt {

BluetoothService::BluetoothService(bt_gatt_h handle) : handle_(handle) {
  int res = bt_gatt_service_foreach_characteristics(
      handle,
      [](int total, int index, bt_gatt_h handle, void* scope_ptr) -> bool {
        auto& service = *static_cast<BluetoothService*>(scope_ptr);
        service.characteristics_.emplace_back(
            std::make_unique<BluetoothCharacteristic>(handle, service));
        return true;
      },
      this);

  LOG_ERROR("bt_gatt_service_foreach_characteristics", get_error_message(res));
}

PrimaryService::PrimaryService(bt_gatt_h handle,
                               BluetoothDeviceController& device)
    : BluetoothService(handle), device_(device) {
  int res = bt_gatt_service_foreach_included_services(
      handle,
      [](int total, int index, bt_gatt_h handle, void* scope_ptr) -> bool {
        auto& service = *static_cast<PrimaryService*>(scope_ptr);
        service.secondary_services_.emplace_back(
            std::make_unique<SecondaryService>(handle, service));
        return true;
      },
      this);
  LOG_ERROR("bt_gatt_service_foreach_included_services",
            get_error_message(res));
}

SecondaryService::SecondaryService(bt_gatt_h service_handle,
                                   PrimaryService& primary_service)
    : BluetoothService(service_handle), primary_service_(primary_service) {}

const BluetoothDeviceController& PrimaryService::cDevice() const noexcept {
  return device_;
}

proto::gen::BluetoothService PrimaryService::ToProtoService() const noexcept {
  proto::gen::BluetoothService proto;
  proto.set_remote_id(device_.cAddress());
  proto.set_uuid(Uuid());
  proto.set_is_primary(true);
  for (const auto& characteristic : characteristics_) {
    *proto.add_characteristics() = characteristic->ToProtoCharacteristic();
  }
  for (const auto& secondary : secondary_services_) {
    *proto.add_included_services() = secondary->ToProtoService();
  }
  return proto;
}

ServiceType PrimaryService::GetType() const noexcept {
  return ServiceType::kPrimary;
}

const BluetoothDeviceController& SecondaryService::cDevice() const noexcept {
  return primary_service_.cDevice();
}

const PrimaryService& SecondaryService::cPrimary() const noexcept {
  return primary_service_;
}

proto::gen::BluetoothService SecondaryService::ToProtoService() const noexcept {
  proto::gen::BluetoothService proto;
  proto.set_remote_id(primary_service_.cDevice().cAddress());
  proto.set_uuid(Uuid());
  proto.set_is_primary(false);
  for (const auto& characteristic : characteristics_) {
    *proto.add_characteristics() = characteristic->ToProtoCharacteristic();
  }
  return proto;
}

ServiceType SecondaryService::GetType() const noexcept {
  return ServiceType::kSecondary;
}

std::string SecondaryService::PrimaryUuid() noexcept {
  return primary_service_.Uuid();
}

std::string BluetoothService::Uuid() const noexcept {
  return GetGattUuid(handle_);
}

BluetoothCharacteristic* BluetoothService::GetCharacteristic(
    const std::string& uuid) {
  for (auto& characteristic : characteristics_) {
    if (characteristic->Uuid() == uuid) return characteristic.get();
  }
  return nullptr;
}

SecondaryService* PrimaryService::GetSecondary(
    const std::string& uuid) noexcept {
  for (auto& service : secondary_services_) {
    if (service->Uuid() == uuid) return service.get();
  }
  return nullptr;
}

BluetoothService::~BluetoothService() {}

/**
 *
 * these must not be in a virtual destructor. Characteristic references abstract
 * method, when derived objects are already destroyed otherwise.
 *
 */
PrimaryService::~PrimaryService() { characteristics_.clear(); }

SecondaryService::~SecondaryService() { characteristics_.clear(); }

}  // namespace btGatt

}  // namespace flutter_blue_tizen