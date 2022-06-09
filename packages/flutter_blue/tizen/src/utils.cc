#include "utils.h"

#include "log.h"

namespace flutter_blue_tizen {

/**
 * @brief Get the value of Gatt descriptor, characteristic
 *
 * @param handle
 * @return std::string
 */
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

/**
 * @brief Get the uuid of Gatt descriptor, characteristic or service
 *
 * @param handle
 * @return std::string
 */
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
