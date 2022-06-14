#ifndef FLUTTER_BLUE_TIZEN_GATT_BLUETOOTH_DESCRIPTOR_H
#define FLUTTER_BLUE_TIZEN_GATT_BLUETOOTH_DESCRIPTOR_H

#include <bluetooth.h>

#include <functional>
#include <map>
#include <string>

#include "utils.h"

namespace flutter_blue_tizen {
namespace btGatt {

class BluetoothDescriptor {
 public:
  BluetoothDescriptor(bt_gatt_h handle);

  ~BluetoothDescriptor();

  std::string Uuid() const noexcept;

  std::string Value() const noexcept;

  void Read(const std::function<void(const BluetoothDescriptor&)>& callback);

  void Write(const std::string value,
             const std::function<void(bool success,
                                      const BluetoothDescriptor&)>& callback);

 private:
  bt_gatt_h handle_;

  /**
   * @brief used to validate whether the descriptor still exists in async
   * callback. key-uuid value-pointer of descriptor
   */
  static inline SafeType<std::map<std::string, BluetoothDescriptor*>>
      active_descriptors_;
};

}  // namespace btGatt
}  // namespace flutter_blue_tizen
#endif  // FLUTTER_BLUE_TIZEN_GATT_BLUETOOTH_DESCRIPTOR_H