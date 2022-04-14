#include <BluetoothDeviceController.h>
#include <GATT/BluetoothCharacteristic.h>
#include <GATT/BluetoothService.h>
#include <Logger.h>
#include <bluetooth.h>

namespace btGatt {
using namespace btu;
using namespace btlog;

BluetoothService::BluetoothService(bt_gatt_h handle) : _handle(handle) {
  int res = bt_gatt_service_foreach_characteristics(
      handle,
      [](int total, int index, bt_gatt_h handle, void* scope_ptr) -> bool {
        auto& service = *static_cast<BluetoothService*>(scope_ptr);
        service._characteristics.emplace_back(
            std::make_unique<BluetoothCharacteristic>(handle, service));
        return true;
      },
      this);

  Logger::showResultError("bt_gatt_service_foreach_characteristics", res);
}

PrimaryService::PrimaryService(bt_gatt_h handle,
                               BluetoothDeviceController& device)
    : BluetoothService(handle), _device(device) {
  int res = bt_gatt_service_foreach_included_services(
      handle,
      [](int total, int index, bt_gatt_h handle, void* scope_ptr) -> bool {
        auto& service = *static_cast<PrimaryService*>(scope_ptr);
        service._secondaryServices.emplace_back(
            std::make_unique<SecondaryService>(handle, service));
        return true;
      },
      this);
  Logger::showResultError("bt_gatt_service_foreach_included_services", res);
}

SecondaryService::SecondaryService(bt_gatt_h service_handle,
                                   PrimaryService& primaryService)
    : BluetoothService(service_handle), _primaryService(primaryService) {}

auto PrimaryService::cDevice() const noexcept
    -> const btu::BluetoothDeviceController& {
  return _device;
}
auto PrimaryService::toProtoService() const noexcept
    -> proto::gen::BluetoothService {
  proto::gen::BluetoothService proto;
  proto.set_remote_id(_device.cAddress());
  proto.set_uuid(UUID());
  proto.set_is_primary(true);
  for (const auto& characteristic : _characteristics) {
    *proto.add_characteristics() = characteristic->toProtoCharacteristic();
  }
  for (const auto& secondary : _secondaryServices) {
    *proto.add_included_services() = secondary->toProtoService();
  }
  return proto;
}
auto PrimaryService::getType() const noexcept -> ServiceType {
  return ServiceType::PRIMARY;
}

auto SecondaryService::cDevice() const noexcept
    -> const btu::BluetoothDeviceController& {
  return _primaryService.cDevice();
}
auto SecondaryService::cPrimary() const noexcept -> const PrimaryService& {
  return _primaryService;
}
auto SecondaryService::toProtoService() const noexcept
    -> proto::gen::BluetoothService {
  proto::gen::BluetoothService proto;
  proto.set_remote_id(_primaryService.cDevice().cAddress());
  proto.set_uuid(UUID());
  proto.set_is_primary(false);
  for (const auto& characteristic : _characteristics) {
    *proto.add_characteristics() = characteristic->toProtoCharacteristic();
  }
  return proto;
}

auto SecondaryService::getType() const noexcept -> ServiceType {
  return ServiceType::SECONDARY;
}
auto SecondaryService::primaryUUID() noexcept -> std::string {
  return _primaryService.UUID();
}
auto BluetoothService::UUID() const noexcept -> std::string {
  return getGattUUID(_handle);
}
auto BluetoothService::getCharacteristic(const std::string& uuid)
    -> BluetoothCharacteristic* {
  for (auto& c : _characteristics) {
    if (c->UUID() == uuid) return c.get();
  }
  return nullptr;
}
auto PrimaryService::getSecondary(const std::string& uuid) noexcept
    -> SecondaryService* {
  for (auto& s : _secondaryServices) {
    if (s->UUID() == uuid) return s.get();
  }
  return nullptr;
}

BluetoothService::~BluetoothService() {
  btlog::Logger::log(btlog::LogLevel::DEBUG,
                     "calling destroy for bluetoothService!");
}

/**
 * these must not be in a virtual destructor. Characteristic references abstract
 * method, when derived objects are already destroyed otherwise.
 *
 */
PrimaryService::~PrimaryService() { _characteristics.clear(); }
SecondaryService::~SecondaryService() { _characteristics.clear(); }
}  // namespace btGatt
