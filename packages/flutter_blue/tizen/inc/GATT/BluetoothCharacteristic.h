#ifndef FLUTTER_BLUE_TIZEN_GATT_BLEUTOOTH_CHARACTERISTIC_H
#define FLUTTER_BLUE_TIZEN_GATT_BLEUTOOTH_CHARACTERISTIC_H

#include <Utils.h>
#include <bluetooth.h>
#include <flutterblue.pb.h>

#include <map>
#include <memory>
#include <vector>

namespace flutter_blue_tizen {
namespace btGatt {
class BluetoothService;
class BluetoothDescriptor;

class BluetoothCharacteristic {
  using NotifyCallback = std::function<void(const BluetoothCharacteristic&)>;

  bt_gatt_h _handle;

  BluetoothService& _service;

  std::vector<std::unique_ptr<BluetoothDescriptor>> _descriptors;

  std::unique_ptr<NotifyCallback> _notifyCallback;

  /**
   * @brief used to validate whether the characteristic still exists in async
   * callback. key-uuid value-pointer of characteristic
   */
  static inline btu::SafeType<std::map<std::string, BluetoothCharacteristic*>>
      _activeCharacteristics;

 public:

  BluetoothCharacteristic(bt_gatt_h handle, BluetoothService& service);

  ~BluetoothCharacteristic() noexcept;

  proto::gen::BluetoothCharacteristic toProtoCharacteristic() const noexcept;

  BluetoothService const& cService() const noexcept;

  std::string UUID() const noexcept;

  std::string value() const noexcept;

  BluetoothDescriptor* getDescriptor(std::string const& uuid);

  void read(
      const std::function<void(const BluetoothCharacteristic&)>& callback);

  void write(
      const std::string value, bool withoutResponse,
      const std::function<void(bool success, const BluetoothCharacteristic&)>&
          callback);

  int properties() const noexcept;

  void setNotifyCallback(const NotifyCallback& callback);
  
  void unsetNotifyCallback();
};
}  // namespace btGatt
}  // namespace flutter_blue_tizen
#endif  // FLUTTER_BLUE_TIZEN_GATT_BLEUTOOTH_CHARACTERISTIC_H
