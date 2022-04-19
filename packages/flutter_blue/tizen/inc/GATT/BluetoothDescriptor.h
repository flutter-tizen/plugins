#ifndef FLUTTER_BLUE_TIZEN_GATT_BLUETOOTH_DESCRIPTOR_H
#define FLUTTER_BLUE_TIZEN_GATT_BLUETOOTH_DESCRIPTOR_H

#include <bluetooth.h>
#include <flutterblue.pb.h>

#include <map>
#include <memory>
#include <string>

namespace btGatt {
class BluetoothCharacteristic;
class BluetoothDescriptor {
  bt_gatt_h _handle;
  BluetoothCharacteristic& _characteristic;

  /**
   * @brief used to validate whether the descriptor still exists in async
   * callback. key-uuid value-pointer of descriptor
   */
  static inline btu::SafeType<std::map<std::string, BluetoothDescriptor*>>
      _activeDescriptors;

 public:
  BluetoothDescriptor(bt_gatt_h handle,
                      BluetoothCharacteristic& characteristic);
  ~BluetoothDescriptor();
  proto::gen::BluetoothDescriptor toProtoDescriptor() const noexcept;
  std::string UUID() const noexcept;
  std::string value() const noexcept;
  void read(const std::function<void(const BluetoothDescriptor&)>& callback);
  void write(const std::string value,
             const std::function<void(bool success,
                                      const BluetoothDescriptor&)>& callback);
  BluetoothCharacteristic const& cCharacteristic() const noexcept;
};
}  // namespace btGatt
#endif  // FLUTTER_BLUE_TIZEN_GATT_BLUETOOTH_DESCRIPTOR_H
