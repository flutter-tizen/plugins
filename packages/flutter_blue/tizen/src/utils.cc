#include <GATT/bluetooth_service.h>
#include <bluetooth_device_controller.h>
#include <log.h>
#include <utils.h>

namespace flutter_blue_tizen {


std::vector<u_int8_t> messageToVector(
    google::protobuf::MessageLite const& messageLite) noexcept {
  std::vector<u_int8_t> encoded(messageLite.ByteSizeLong());
  messageLite.SerializeToArray(encoded.data(), messageLite.ByteSizeLong());
  return encoded;
}


proto::gen::CharacteristicProperties getProtoCharacteristicProperties(
    int properties) {
  proto::gen::CharacteristicProperties p;
  p.set_broadcast((properties & 0x01) != 0);
  p.set_read((properties & 0x02) != 0);
  p.set_write_without_response((properties & 0x04) != 0);
  p.set_write((properties & 0x08) != 0);
  p.set_notify((properties & 0x10) != 0);
  p.set_indicate((properties & 0x20) != 0);
  p.set_authenticated_signed_writes((properties & 0x40) != 0);
  p.set_extended_properties((properties & 0x80) != 0);
  // p.set_notify_encryption_required((properties & 256) != 0);
  // p.set_indicate_encryption_required((properties & 512) != 0);
  return p;
}


/**
 * @brief Get the value of Gatt descriptor, characteristic
 *
 * @param handle
 * @return std::string
 */
std::string getGattValue(bt_gatt_h handle) {
  std::string result = "";
  char* value = nullptr;
  int length = 0;

  int res = bt_gatt_get_value(handle, &value, &length);
  LOG_ERROR("bt_gatt_get_value", get_error_message(res));
  if (!res && value) {
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
std::string getGattUUID(bt_gatt_h handle) {
  std::string result;
  char* uuid = nullptr;
  int res = bt_gatt_get_uuid(handle, &uuid);
  LOG_ERROR("bt_gatt_get_uuid", get_error_message(res));
  if (!res && uuid) {
    result = std::string(uuid);
    free(uuid);
  }
  return result;
}


std::string getGattClientAddress(bt_gatt_client_h handle) {
  std::string result;
  char* address = nullptr;
  int res = bt_gatt_client_get_remote_address(handle, &address);
  LOG_ERROR("bt_gatt_client_get_remote_address", get_error_message(res));
  if (!res && address) {
    result = std::string(address);
    free(address);
  }
  return result;
}


proto::gen::DiscoverServicesResult getProtoServiceDiscoveryResult(
    BluetoothDeviceController const& device,
    std::vector<btGatt::PrimaryService*> const& services) {
  proto::gen::DiscoverServicesResult res;
  for (const auto& s : services) {
    *res.add_services() = s->toProtoService();
  }
  res.set_remote_id(device.cAddress());
  return res;
}


bt_gatt_h getGattService(bt_gatt_client_h handle, const std::string& uuid) {
  bt_gatt_h result;
  int res = bt_gatt_client_get_service(handle, uuid.c_str(), &result);
  LOG_ERROR("bt_gatt_client_get_service", get_error_message(res));

  return result;
}


BTException::BTException(std::string const& mess) : _mess(mess) {}


BTException::BTException(const int tizen_error, std::string const& mess)
    : _mess(std::string(get_error_message(tizen_error)) + ": " + mess) {}


BTException::BTException(const int tizen_error)
    : _mess(get_error_message(tizen_error)) {}


const char* BTException::what() const noexcept { return _mess.c_str(); }


}  // namespace flutter_blue_tizen