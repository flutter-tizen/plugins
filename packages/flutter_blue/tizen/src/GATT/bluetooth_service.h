#ifndef FLUTTER_BLUE_TIZEN_GATT_BLEUTOOTH_SERVICE_H
#define FLUTTER_BLUE_TIZEN_GATT_BLEUTOOTH_SERVICE_H

#include <GATT/bluetooth_characteristic.h>
#include <bluetooth.h>
#include <flutterblue.pb.h>

#include <vector>

namespace flutter_blue_tizen {

class BluetoothDeviceController;

namespace btGatt {

enum class ServiceType {
  PRIMARY,
  SECONDARY,
};

class SecondaryService;

class BluetoothService {

 public:

  virtual proto::gen::BluetoothService ToProtoService() const noexcept = 0;

  virtual const BluetoothDeviceController& cDevice() const noexcept = 0;

  virtual ServiceType GetType() const noexcept = 0;

  std::string Uuid() const noexcept;

  BluetoothCharacteristic* GetCharacteristic(const std::string& uuid);

 protected:

  bt_gatt_h handle_;

  std::vector<std::unique_ptr<BluetoothCharacteristic>> characteristics_;

  BluetoothService(bt_gatt_h handle);

  BluetoothService(const BluetoothService&) = default;

  virtual ~BluetoothService();
};


class PrimaryService : public BluetoothService {

public:

  PrimaryService(bt_gatt_h handle, BluetoothDeviceController& device);

  PrimaryService(const PrimaryService&) = default;

  ~PrimaryService();

  const BluetoothDeviceController& cDevice() const noexcept override;

  proto::gen::BluetoothService ToProtoService() const noexcept override;

  ServiceType GetType() const noexcept override;

  SecondaryService* GetSecondary(const std::string& uuid) noexcept;

private:

  BluetoothDeviceController& device_;

  std::vector<std::unique_ptr<SecondaryService>> secondary_services_;
};

class SecondaryService : public BluetoothService {

public:
  SecondaryService(bt_gatt_h service_handle, PrimaryService& primary_service);

  SecondaryService(const SecondaryService&) = default;

  ~SecondaryService();

  const BluetoothDeviceController& cDevice() const noexcept override;

  const PrimaryService& cPrimary() const noexcept;

  proto::gen::BluetoothService ToProtoService() const noexcept override;

  ServiceType GetType() const noexcept override;

  std::string PrimaryUuid() noexcept;
  
private:

  PrimaryService& primaryService_;
};

}  // namespace btGatt
}  // namespace flutter_blue_tizen
#endif  // FLUTTER_BLUE_TIZEN_GATT_BLEUTOOTH_SERVICE_H
