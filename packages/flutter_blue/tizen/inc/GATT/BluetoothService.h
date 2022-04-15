#ifndef BLEUTOOTH_SERVICE_H
#define BLEUTOOTH_SERVICE_H

#include <GATT/BluetoothCharacteristic.h>
#include <bluetooth.h>
#include <flutterblue.pb.h>

#include <vector>

namespace btu {
class BluetoothDeviceController;
};

namespace btGatt {
enum class ServiceType {
  PRIMARY,
  SECONDARY,
};

class SecondaryService;
class BluetoothService {
 protected:
  bt_gatt_h _handle;
  std::vector<std::unique_ptr<BluetoothCharacteristic>> _characteristics;

  BluetoothService(bt_gatt_h handle);
  BluetoothService(const BluetoothService&) = default;

  virtual ~BluetoothService();

 public:
  virtual auto toProtoService() const noexcept
      -> proto::gen::BluetoothService = 0;
  virtual auto cDevice() const noexcept
      -> const btu::BluetoothDeviceController& = 0;
  virtual auto getType() const noexcept -> ServiceType = 0;
  auto UUID() const noexcept -> std::string;
  auto getCharacteristic(const std::string& uuid) -> BluetoothCharacteristic*;
};

///////PRIMARY///////
class PrimaryService : public BluetoothService {
  btu::BluetoothDeviceController& _device;
  std::vector<std::unique_ptr<SecondaryService>> _secondaryServices;

 public:
  PrimaryService(bt_gatt_h handle, btu::BluetoothDeviceController& device);
  PrimaryService(const PrimaryService&) = default;
  ~PrimaryService();
  auto cDevice() const noexcept
      -> const btu::BluetoothDeviceController& override;
  auto toProtoService() const noexcept -> proto::gen::BluetoothService override;
  auto getType() const noexcept -> ServiceType override;
  auto getSecondary(const std::string& uuid) noexcept -> SecondaryService*;
};

///////SECONDARY///////
class SecondaryService : public BluetoothService {
  PrimaryService& _primaryService;

 public:
  SecondaryService(bt_gatt_h service_handle, PrimaryService& primaryService);
  SecondaryService(const SecondaryService&) = default;
  ~SecondaryService();
  auto cDevice() const noexcept
      -> const btu::BluetoothDeviceController& override;
  auto cPrimary() const noexcept -> const PrimaryService&;
  auto toProtoService() const noexcept -> proto::gen::BluetoothService override;
  auto getType() const noexcept -> ServiceType override;
  auto primaryUUID() noexcept -> std::string;
};
}  // namespace btGatt
#endif  // BLEUTOOTH_SERVICE_H
