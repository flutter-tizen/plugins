#include "utils.h"

#include "log.h"

namespace flutter_blue_tizen {


// Gets the value of descriptor or characteristic.
std::string GetGattValue(bt_gatt_h handle) {
  std::string result = "";
  char* value = nullptr;
  int length = 0;

  int ret = bt_gatt_get_value(handle, &value, &length);
  LOG_ERROR("bt_gatt_get_value", get_error_message(ret));
  if (!ret && value) {
    result = std::string(value, length);
    free(value);
  }

  return result;
}


// Gets the uuid of descriptor, characteristic or service.
std::string GetGattUuid(bt_gatt_h handle) {
  std::string result;
  char* uuid = nullptr;
  int ret = bt_gatt_get_uuid(handle, &uuid);
  LOG_ERROR("bt_gatt_get_uuid", get_error_message(ret));
  if (!ret && uuid) {
    result = std::string(uuid);
    free(uuid);
  }
  return result;
}

}  // namespace flutter_blue_tizen
