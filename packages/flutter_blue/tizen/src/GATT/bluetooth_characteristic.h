#ifndef FLUTTER_BLUE_TIZEN_GATT_BLEUTOOTH_CHARACTERISTIC_H
#define FLUTTER_BLUE_TIZEN_GATT_BLEUTOOTH_CHARACTERISTIC_H

#include <GATT/bluetooth_descriptor.h>
#include <bluetooth.h>
#include <flutterblue.pb.h>
#include <utils.h>

#include <map>
#include <memory>
#include <vector>

namespace flutter_blue_tizen {
namespace btGatt {

class BluetoothService;

class BluetoothCharacteristic {
 public:
  using NotifyCallback = std::function<void(const BluetoothCharacteristic&)>;

  BluetoothCharacteristic(bt_gatt_h handle, BluetoothService& service);

  ~BluetoothCharacteristic() noexcept;

  proto::gen::BluetoothCharacteristic ToProtoCharacteristic() const noexcept;

  const BluetoothService& cService() const noexcept;

  std::string Uuid() const noexcept;

  std::string Value() const noexcept;

  BluetoothDescriptor* GetDescriptor(const std::string& uuid) const;

  std::vector<BluetoothDescriptor*> GetDescriptors() const;

  void Read(
      const std::function<void(const BluetoothCharacteristic&)>& callback);

  void Write(
      const std::string value, bool without_response,
      const std::function<void(bool success, const BluetoothCharacteristic&)>&
          callback);

  int Properties() const noexcept;

  void SetNotifyCallback(const NotifyCallback& callback);

  void UnsetNotifyCallback();

 private:
  bt_gatt_h handle_;

  BluetoothService& service_;

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
