#include "GATT/bluetooth_service.h"

#include "GATT/bluetooth_characteristic.h"
#include "bluetooth.h"
#include "bluetooth_device_controller.h"
#include "log.h"

namespace flutter_blue_tizen {
namespace btGatt {

BluetoothService::BluetoothService(bt_gatt_h handle) : handle_(handle) {
  int ret = bt_gatt_service_foreach_characteristics(
      handle,
      [](int total, int index, bt_gatt_h handle, void* scope_ptr) -> bool {
        auto& service = *static_cast<BluetoothService*>(scope_ptr);
        service.characteristics_.emplace_back(
            std::make_unique<BluetoothCharacteristic>(handle));
        return true;
      },
      this);

  LOG_ERROR("bt_gatt_service_foreach_characteristics", get_error_message(ret));
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
  LOG_ERROR("bt_gatt_service_foreach_included_services",
            get_error_message(ret));
}

SecondaryService::SecondaryService(bt_gatt_h service_handle,
                                   PrimaryService& primary_service)
    : BluetoothService(service_handle), primary_service_(primary_service) {}

ServiceType PrimaryService::GetType() const noexcept {
  return ServiceType::kPrimary;
}

const PrimaryService& SecondaryService::cPrimary() const noexcept {
  return primary_service_;
}

ServiceType SecondaryService::GetType() const noexcept {
  return ServiceType::kSecondary;
}

std::string SecondaryService::PrimaryUuid() const noexcept {
  return primary_service_.Uuid();
}

std::string BluetoothService::Uuid() const noexcept {
  return GetGattUuid(handle_);
}

BluetoothCharacteristic* BluetoothService::GetCharacteristic(
    const std::string& uuid) const {
  auto it = std::find_if(characteristics_.begin(), characteristics_.end(),
                         [&uuid](const auto& characteristic) -> bool {
                           return characteristic->Uuid() == uuid;
                         });
  return (it != characteristics_.end() ? it->get() : nullptr);
}

std::vector<BluetoothCharacteristic*> BluetoothService::GetCharacteristics()
    const {
  std::vector<BluetoothCharacteristic*> characteristics;
  std::transform(characteristics_.begin(), characteristics_.end(),
                 std::back_inserter(characteristics),
                 [](auto& characteristic) -> BluetoothCharacteristic* {
                   return characteristic.get();
                 });

  return characteristics;
}

SecondaryService* PrimaryService::GetSecondary(
    const std::string& uuid) const noexcept {
  auto it = std::find_if(
      secondary_services_.begin(), secondary_services_.end(),
      [&uuid](const auto& service) -> bool { return service->Uuid() == uuid; });
  return (it != secondary_services_.end() ? it->get() : nullptr);
}

std::vector<SecondaryService*> PrimaryService::getSecondaryServices()
    const noexcept {
  std::vector<SecondaryService*> services;
  std::transform(
      secondary_services_.begin(), secondary_services_.end(),
      std::back_inserter(services),
      [](auto& service) -> SecondaryService* { return service.get(); });

  return services;
}

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