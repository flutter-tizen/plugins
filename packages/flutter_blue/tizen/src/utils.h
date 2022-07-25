#ifndef FLUTTER_BLUE_TIZEN_UTILS_H
#define FLUTTER_BLUE_TIZEN_UTILS_H

#include <bluetooth.h>

#include <exception>
#include <mutex>
#include <vector>
#include <string>


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


struct BleScanSettings{
	bool allow_duplicates_;
	bool clear_devices_;
	std::vector<std::string> device_ids_filters_;
	std::vector<std::string> service_uuids_filters_;
};

struct AdvertisementData{
	bool connectable_;
	std::string local_name_;
};

}  // namespace flutter_blue_tizen

#endif  // FLUTTER_BLUE_TIZEN_UTILS_H
