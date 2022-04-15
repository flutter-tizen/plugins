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
  virtual proto::gen::BluetoothService toProtoService() const noexcept = 0;
  virtual btu::BluetoothDeviceController const& cDevice() const noexcept = 0;
  virtual ServiceType getType() const noexcept = 0;
  std::string UUID() const noexcept;
  BluetoothCharacteristic* getCharacteristic(const std::string& uuid);
};

///////PRIMARY///////
class PrimaryService : public BluetoothService {
  btu::BluetoothDeviceController& _device;
  std::vector<std::unique_ptr<SecondaryService>> _secondaryServices;

 public:
  PrimaryService(bt_gatt_h handle, btu::BluetoothDeviceController& device);
  PrimaryService(const PrimaryService&) = default;
  ~PrimaryService();
  btu::BluetoothDeviceController const& cDevice() const noexcept override;
  proto::gen::BluetoothService toProtoService() const noexcept override;
  ServiceType getType() const noexcept override;
  SecondaryService* getSecondary(const std::string& uuid) noexcept;
};

///////SECONDARY///////
class SecondaryService : public BluetoothService {
  PrimaryService& _primaryService;

 public:
  SecondaryService(bt_gatt_h service_handle, PrimaryService& primaryService);
  SecondaryService(const SecondaryService&) = default;
  ~SecondaryService();
  btu::BluetoothDeviceController const& cDevice() const noexcept override;
  PrimaryService const& cPrimary() const noexcept;
  proto::gen::BluetoothService toProtoService() const noexcept override;
  ServiceType getType() const noexcept override;
  std::string primaryUUID() noexcept;
};
}  // namespace btGatt
#endif  // BLEUTOOTH_SERVICE_H
