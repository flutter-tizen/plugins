#include "GATT/bluetooth_service.h"

#include "log.h"

namespace flutter_blue_tizen::btGatt {

// BluetoothService

BluetoothService::BluetoothService(bt_gatt_h handle) : handle_(handle) {
  int ret = bt_gatt_service_foreach_characteristics(
      handle,
      [](int total, int index, bt_gatt_h handle, void* data) -> bool {
        auto& service = *static_cast<BluetoothService*>(data);
        service.characteristics_.emplace_back(
            std::make_unique<BluetoothCharacteristic>(handle));
        return true;
      },
      this);

  LOG_ERROR("bt_gatt_service_foreach_characteristics %s",
            get_error_message(ret));
}

std::string BluetoothService::Uuid() const noexcept {
  return GetGattUuid(handle_);
}

BluetoothCharacteristic* BluetoothService::GetCharacteristic(
    const std::string& uuid) const noexcept {
  for (const auto& characteristic : characteristics_) {
    if (characteristic->Uuid() == uuid) return characteristic.get();
  }
  return nullptr;
}

std::vector<BluetoothCharacteristic*> BluetoothService::GetCharacteristics()
    const noexcept {
  std::vector<BluetoothCharacteristic*> characteristics;
  for (const auto& characteristic : characteristics_) {
    characteristics.emplace_back(characteristic.get());
  }
  return characteristics;
}

PrimaryService::PrimaryService(bt_gatt_h handle) : BluetoothService(handle) {
  int ret = bt_gatt_service_foreach_included_services(
      handle,
      [](int total, int index, bt_gatt_h handle, void* scope_ptr) -> bool {
        auto& service = *static_cast<PrimaryService*>(scope_ptr);
        service.secondary_services_.emplace_back(
            std::make_unique<SecondaryService>(handle, service));
        return true;
      },
      this);
  LOG_ERROR("bt_gatt_service_foreach_included_services %s",
            get_error_message(ret));
}

/* This must not be in a virtual destructor. Characteristic references abstract
 method, when derived objects are already destroyed otherwise. */
PrimaryService::~PrimaryService() { characteristics_.clear(); }

bool PrimaryService::IsPrimary() const noexcept { return true; }

SecondaryService* PrimaryService::GetSecondary(
    const std::string& uuid) const noexcept {
  for (const auto& service : secondary_services_) {
    if (service->Uuid() == uuid) return service.get();
  }
  return nullptr;
}

std::vector<SecondaryService*> PrimaryService::getSecondaryServices()
    const noexcept {
  std::vector<SecondaryService*> services;
  for (const auto& service : secondary_services_) {
    services.emplace_back(service.get());
  }
  return services;
}

SecondaryService::SecondaryService(bt_gatt_h service_handle,
                                   const PrimaryService& primary_service)
    : BluetoothService(service_handle), primary_service_(primary_service) {}

/* This must not be in a virtual destructor. Characteristic references abstract
 method, when derived objects are already destroyed otherwise. */
SecondaryService::~SecondaryService() { characteristics_.clear(); }

bool SecondaryService::IsPrimary() const noexcept { return false; }

std::string SecondaryService::PrimaryUuid() const noexcept {
  return primary_service_.Uuid();
}

}  // namespace flutter_blue_tizen::btGatt
