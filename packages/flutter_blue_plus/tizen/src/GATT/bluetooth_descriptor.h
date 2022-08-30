#ifndef FLUTTER_BLUE_PLUS_TIZEN_GATT_BLUETOOTH_DESCRIPTOR_H
#define FLUTTER_BLUE_PLUS_TIZEN_GATT_BLUETOOTH_DESCRIPTOR_H

#include <bluetooth.h>

#include <functional>
#include <map>
#include <string>

#include "utils.h"

namespace flutter_blue_plus_tizen::btGatt {

class BluetoothDescriptor {
 public:
  using ReadCallback = std::function<void(const BluetoothDescriptor&)>;
  using WriteCallback =
      std::function<void(bool success, const BluetoothDescriptor&)>;

  BluetoothDescriptor(bt_gatt_h handle);

  BluetoothDescriptor(const BluetoothDescriptor&) = delete;

  BluetoothDescriptor(BluetoothDescriptor&&) = default;

  ~BluetoothDescriptor();

  std::string Uuid() const noexcept;

  std::string Value() const noexcept;

  void Read(ReadCallback callback) const;

  void Write(const std::string value, WriteCallback callback);

 private:
  bt_gatt_h handle_;

  /**
   * @brief used to validate whether the descriptor still exists in async
   * callback. key-uuid value-pointer of descriptor
   */
  static inline SafeType<std::map<std::string, BluetoothDescriptor*>>
      active_descriptors_;
};

}  // namespace flutter_blue_plus_tizen::btGatt
#endif  // FLUTTER_BLUE_PLUS_TIZEN_GATT_BLUETOOTH_DESCRIPTOR_H
