#ifndef BLEUTOOTH_CHARACTERISTIC_H
#define BLEUTOOTH_CHARACTERISTIC_H
#include <Utils.h>
#include <bluetooth.h>
#include <flutterblue.pb.h>

#include <atomic>
#include <map>
#include <memory>
#include <vector>

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
  auto toProtoCharacteristic() const noexcept
      -> proto::gen::BluetoothCharacteristic;
  auto cService() const noexcept -> const BluetoothService&;
  auto UUID() const noexcept -> std::string;
  auto value() const noexcept -> std::string;
  auto getDescriptor(const std::string& uuid) -> BluetoothDescriptor*;
  auto read(const std::function<void(const BluetoothCharacteristic&)>& callback)
      -> void;
  auto write(
      const std::string value, bool withoutResponse,
      const std::function<void(bool success, const BluetoothCharacteristic&)>&
          callback) -> void;
  auto properties() const noexcept -> int;

  auto setNotifyCallback(const NotifyCallback& callback) -> void;
  void unsetNotifyCallback();
};
}  // namespace btGatt
#endif  // BLEUTOOTH_CHARACTERISTIC_H
