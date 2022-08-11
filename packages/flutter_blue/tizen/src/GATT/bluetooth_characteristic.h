#ifndef FLUTTER_BLUE_TIZEN_GATT_BLEUTOOTH_CHARACTERISTIC_H
#define FLUTTER_BLUE_TIZEN_GATT_BLEUTOOTH_CHARACTERISTIC_H

#include <bluetooth.h>

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "GATT/bluetooth_descriptor.h"
#include "utils.h"

namespace flutter_blue_tizen {
namespace btGatt {

class BluetoothCharacteristic {
 public:
  using NotifyCallback = std::function<void(const BluetoothCharacteristic&)>;
  using ReadCallback = std::function<void(const BluetoothCharacteristic&)>;
  using WriteCallback =
      std::function<void(bool success, const BluetoothCharacteristic&)>;

  BluetoothCharacteristic(bt_gatt_h handle);

  BluetoothCharacteristic(const BluetoothCharacteristic&) = delete;

  BluetoothCharacteristic(BluetoothCharacteristic&&) = default;

  ~BluetoothCharacteristic() noexcept;

  std::string Uuid() const noexcept;

  std::string Value() const noexcept;

  BluetoothDescriptor* GetDescriptor(const std::string& uuid) const;

  std::vector<BluetoothDescriptor*> GetDescriptors() const;

  void Read(ReadCallback callback) const;

  void Write(const std::string value, bool without_response,
             WriteCallback callback);

  int Properties() const noexcept;

  void SetNotifyCallback(const NotifyCallback& callback);

  void UnsetNotifyCallback();

 private:
  bt_gatt_h handle_;

  std::vector<std::unique_ptr<BluetoothDescriptor>> descriptors_;

  std::unique_ptr<NotifyCallback> notify_callback_;

  /**
   * @brief used to validate whether the characteristic still exists in async
   * callback. key-uuid value-pointer of characteristic
   */
  static inline SafeType<std::map<std::string, BluetoothCharacteristic*>>
      active_characteristics_;
};

}  // namespace btGatt
}  // namespace flutter_blue_tizen
#endif  // FLUTTER_BLUE_TIZEN_GATT_BLEUTOOTH_CHARACTERISTIC_H
