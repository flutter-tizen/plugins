#ifndef FLUTTER_BLUE_TIZEN_UTILS_H
#define FLUTTER_BLUE_TIZEN_UTILS_H

#include <bluetooth.h>

#include <exception>
#include <mutex>

namespace flutter_blue_tizen {

template <typename T>
struct SafeType {
  T var_;

  std::mutex mutex_;

  SafeType(const T& t) : var_(t) {}

  SafeType(T&& t) : var_(std::move(t)) {}

  SafeType() : var_(T()) {}
};

class BtException : public std::exception {
 public:
  BtException(const std::string& message) : message_(message){};
  BtException(const int tizen_error, const std::string& message)
      : message_(std::string(get_error_message(tizen_error)) + ": " +
                 message){};

  BtException(const int tizen_error)
      : message_(get_error_message(tizen_error)){};

  const char* what() const noexcept override { return message_.c_str(); }

 private:
  std::string message_;
};

std::string GetGattValue(bt_gatt_h handle);

std::string GetGattUuid(bt_gatt_h handle);

bt_gatt_h GetGattService(bt_gatt_client_h handle, const std::string& uuid);

std::string GetGattClientAddress(bt_gatt_client_h handle);

}  // namespace flutter_blue_tizen

#endif  // FLUTTER_BLUE_TIZEN_UTILS_H
