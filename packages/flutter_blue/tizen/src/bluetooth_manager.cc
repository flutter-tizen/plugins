#include <GATT/bluetooth_characteristic.h>
#include <GATT/bluetooth_descriptor.h>
#include <GATT/bluetooth_service.h>
#include <bluetooth.h>
#include <bluetooth_device_controller.h>
#include <bluetooth_manager.h>
#include <log.h>
#include <system_info.h>
#include <tizen.h>

#include <mutex>

namespace flutter_blue_tizen {

using State = BluetoothDeviceController::State;

BluetoothManager::BluetoothManager(NotificationsHandler& notificationsHandler)
    : notifications_handler_(notificationsHandler) {
  int res;
  if (!(res = isBLEAvailable())) {
    LOG_ERROR("Bluetooth is not available on this device!",
              get_error_message(res));
    return;
  }
  if (res = bt_initialize()) {
    LOG_ERROR("[bt_initialize]", get_error_message(res));
    return;
  }
  if (res = bt_gatt_set_connection_state_changed_cb(
          &BluetoothDeviceController::connectionStateCallback, this)) {
    LOG_ERROR("[bt_gatt_set_connection_state_changed_cb]",
              get_error_message(res));
    return;
  }
  LOG_DEBUG("All callbacks successfully initialized.");
}

void BluetoothManager::setNotification(
    proto::gen::SetNotificationRequest const& request) {
  std::scoped_lock lock(bluetooth_devices_.mut);

  auto characteristic = locateCharacteristic(
      request.remote_id(), request.service_uuid(),
      request.secondary_service_uuid(), request.characteristic_uuid());

  if (request.enable())
    characteristic->setNotifyCallback([](auto& characteristic) {
      proto::gen::SetNotificationResponse response;
      response.set_remote_id(characteristic.cService().cDevice().cAddress());
      response.set_allocated_characteristic(
          new proto::gen::BluetoothCharacteristic(
              characteristic.toProtoCharacteristic()));
      characteristic.cService()
          .cDevice()
          .cNotificationsHandler()
          .notifyUIThread("SetNotificationResponse", response);
      LOG_DEBUG("notified UI thread - characteristic written by remote");
    });
  else
    characteristic->unsetNotifyCallback();
}

u_int32_t BluetoothManager::getMtu(const std::string& deviceID) {
  std::scoped_lock lock(bluetooth_devices_.mut);
  auto device = bluetooth_devices_.var.find(deviceID)->second;
  if (!device) throw BTException("could not find device of id=" + deviceID);

  return device->getMtu();
}

void BluetoothManager::requestMtu(const proto::gen::MtuSizeRequest& request) {
  auto device = bluetooth_devices_.var.find(request.remote_id())->second;
  if (!device)
    throw BTException("could not find device of id=" + request.remote_id());

  device->requestMtu(request.mtu(), [](auto status, auto& bluetoothDevice) {
    proto::gen::MtuSizeResponse res;
    res.set_remote_id(bluetoothDevice.cAddress());
    try {
      res.set_mtu(bluetoothDevice.getMtu());
    } catch (std::exception const& e) {
      // LOG_ERROR(e.what());
    }
    bluetoothDevice.cNotificationsHandler().notifyUIThread("MtuSize", res);
    LOG_DEBUG("mtu request callback sent response!");
  });
}

btGatt::BluetoothCharacteristic* BluetoothManager::locateCharacteristic(
    std::string const& remoteID, std::string const& primaryUUID,
    std::string const& secondaryUUID, std::string const& characteristicUUID) {
  auto it = bluetooth_devices_.var.find(remoteID);

  if (it != bluetooth_devices_.var.end()) {
    auto device = it->second;
    auto primary = device->getService(primaryUUID);
    btGatt::BluetoothService* service = primary;
    if (primary && !secondaryUUID.empty()) {
      service = primary->getSecondary(secondaryUUID);
    }
    if (service) {
      return service->getCharacteristic(characteristicUUID);
    }
  }
  throw BTException("could not locate characteristic " + characteristicUUID);
}

btGatt::BluetoothDescriptor* BluetoothManager::locateDescriptor(
    const std::string& remoteID, const std::string& primaryUUID,
    const std::string& secondaryUUID, const std::string& characteristicUUID,
    const std::string& descriptorUUID) {
  auto descriptor = locateCharacteristic(remoteID, primaryUUID, secondaryUUID,
                                         characteristicUUID)
                        ->getDescriptor(descriptorUUID);
  if (descriptor) return descriptor;

  throw BTException("could not locate descriptor " + descriptorUUID);
}

bool BluetoothManager::isBLEAvailable() {
  bool state = false;
  auto res = system_info_get_platform_bool(
      "http://tizen.org/feature/network.bluetooth.le", &state);
  if (res) throw BTException(res, "system_info_get_platform_bool");

  return state;
}

proto::gen::BluetoothState BluetoothManager::bluetoothState() const noexcept {
  /* Checks whether the Bluetooth adapter is enabled */
  bt_adapter_state_e adapter_state;
  int ret = bt_adapter_get_state(&adapter_state);
  proto::gen::BluetoothState bts;
  if (ret == BT_ERROR_NONE) {
    if (adapter_state == BT_ADAPTER_ENABLED)
      bts.set_state(proto::gen::BluetoothState_State_ON);
    else
      bts.set_state(proto::gen::BluetoothState_State_OFF);
  } else if (ret == BT_ERROR_NOT_INITIALIZED)
    bts.set_state(proto::gen::BluetoothState_State_UNAVAILABLE);
  else
    bts.set_state(proto::gen::BluetoothState_State_UNKNOWN);

  return bts;
}

void BluetoothManager::startBluetoothDeviceScanLE(
    proto::gen::ScanSettings const& scanSettings) {
  stopBluetoothDeviceScanLE();
  std::scoped_lock l(bluetooth_devices_.mut);
  bluetooth_devices_.var.clear();
  scan_allow_duplicates_ = scanSettings.allow_duplicates();
  auto res = bt_adapter_le_set_scan_mode(BT_ADAPTER_LE_SCAN_MODE_BALANCED);
  LOG_ERROR("bt_adapter_le_set_scan_mode", get_error_message(res));

  if (res) throw BTException(res, "bt_adapter_le_set_scan_mode");

  int uuidCount = scanSettings.service_uuids_size();
  std::vector<bt_scan_filter_h> filters(uuidCount);

  for (int i = 0; i < uuidCount; ++i) {
    const std::string& uuid = scanSettings.service_uuids()[i];
    res = bt_adapter_le_scan_filter_create(&filters[i]);

    LOG_ERROR("bt_adapter_le_scan_filter_create", get_error_message(res));
    if (res) throw BTException(res, "bt_adapter_le_scan_filter_create");

    res =
        bt_adapter_le_scan_filter_set_device_address(filters[i], uuid.c_str());

    LOG_ERROR("bt_adapter_le_scan_filter_set_device_address",
              get_error_message(res));

    if (res)
      throw BTException(res, "bt_adapter_le_scan_filter_set_device_address");
  }

  if (!res) {
    res = bt_adapter_le_start_scan(&BluetoothManager::scanCallback, this);
    LOG_ERROR("bt_adapter_le_start_scan", get_error_message(res));
    if (res) throw BTException(res, "bt_adapter_le_start_scan");
  } else {
    throw BTException(res, "cannot start scan");
  }

  for (auto& f : filters) {
    res = bt_adapter_le_scan_filter_destroy(&f);
    LOG_ERROR("bt_adapter_le_scan_filter_destroy", get_error_message(res));

    if (res) throw BTException(res, "bt_adapter_le_scan_filter_destroy");
  }
}

void BluetoothManager::scanCallback(
    int result, bt_adapter_le_device_scan_result_info_s* discovery_info,
    void* user_data) noexcept {
  BluetoothManager& bluetoothManager =
      *static_cast<BluetoothManager*>(user_data);
  if (!result) {
    std::string macAddress = discovery_info->remote_address;
    std::scoped_lock lock(bluetoothManager.bluetooth_devices_.mut);
    std::shared_ptr<BluetoothDeviceController> device;
    if (bluetoothManager.bluetooth_devices_.var.find(macAddress) ==
        bluetoothManager.bluetooth_devices_.var.end())
      device =
          bluetoothManager.bluetooth_devices_.var
              .insert({macAddress,
                       std::make_shared<BluetoothDeviceController>(
                           macAddress, bluetoothManager.notifications_handler_)})
              .first->second;
    else
      device = bluetoothManager.bluetooth_devices_.var.find(macAddress)->second;

    if (bluetoothManager.scan_allow_duplicates_ ||
        device->cProtoBluetoothDevices().empty()) {
      device->protoBluetoothDevices().emplace_back();

      auto& protoDev = device->protoBluetoothDevices().back();
      char* name;
      result = bt_adapter_le_get_scan_result_device_name(
          discovery_info, BT_ADAPTER_LE_PACKET_SCAN_RESPONSE, &name);
      if (!result) {
        protoDev.set_name(name);
        free(name);
      }
      protoDev.set_remote_id(macAddress);

      proto::gen::ScanResult scanResult;
      scanResult.set_rssi(discovery_info->rssi);
      proto::gen::AdvertisementData* advertisement_data =
          new proto::gen::AdvertisementData();
      decodeAdvertisementData(discovery_info->adv_data, *advertisement_data,
                              discovery_info->adv_data_len);

      scanResult.set_allocated_advertisement_data(advertisement_data);
      scanResult.set_allocated_device(
          new proto::gen::BluetoothDevice(protoDev));

      bluetoothManager.notifications_handler_.notifyUIThread("ScanResult",
                                                            scanResult);
    }
  }
}

void BluetoothManager::stopBluetoothDeviceScanLE() {
  static std::mutex m;
  std::scoped_lock lock(m);
  auto btState = bluetoothState().state();
  if (btState == proto::gen::BluetoothState_State_ON) {
    bool isDiscovering;
    auto res = bt_adapter_le_is_discovering(&isDiscovering);
    if (!res && isDiscovering) {
      res = bt_adapter_le_stop_scan();
      LOG_ERROR("bt_adapter_le_stop_scan", res);
    }

    LOG_ERROR("bt_adapter_le_is_discovering", res);
  }
}

void BluetoothManager::connect(const proto::gen::ConnectRequest& connRequest) {
  std::unique_lock lock(bluetooth_devices_.mut);
  auto device = bluetooth_devices_.var.find(connRequest.remote_id())->second;
  if (device)
    device->connect(connRequest.android_auto_connect());
  else
    throw BTException("device not found!");
}

void BluetoothManager::disconnect(const std::string& deviceID) {
  std::unique_lock lock(bluetooth_devices_.mut);
  auto device = bluetooth_devices_.var.find(deviceID)->second;
  if (device)
    device->disconnect();
  else
    throw BTException("device not found!");
}

std::vector<proto::gen::BluetoothDevice>
BluetoothManager::getConnectedProtoBluetoothDevices() noexcept {
  std::vector<proto::gen::BluetoothDevice> protoBD;
  std::scoped_lock lock(bluetooth_devices_.mut);
  for (const auto& e : bluetooth_devices_.var) {
    if (e.second->state() == State::CONNECTED) {
      auto& vec = e.second->cProtoBluetoothDevices();
      protoBD.insert(protoBD.end(), vec.cbegin(), vec.cend());
    }
  }
  return protoBD;
}

BluetoothManager::DevicesContainer&
BluetoothManager::bluetoothDevices() noexcept {
  return bluetooth_devices_;
}

void BluetoothManager::readCharacteristic(
    proto::gen::ReadCharacteristicRequest const& request) {
  std::scoped_lock lock(bluetooth_devices_.mut);
  auto characteristic = locateCharacteristic(
      request.remote_id(), request.service_uuid(),
      request.secondary_service_uuid(), request.characteristic_uuid());

  characteristic->read([](auto& characteristic) -> void {
    proto::gen::ReadCharacteristicResponse res;
    res.set_remote_id(characteristic.cService().cDevice().cAddress());
    res.set_allocated_characteristic(new proto::gen::BluetoothCharacteristic(
        characteristic.toProtoCharacteristic()));

    characteristic.cService().cDevice().cNotificationsHandler().notifyUIThread(
        "ReadCharacteristicResponse", res);
    LOG_DEBUG("finished characteristic read cb");
  });
}

void BluetoothManager::readDescriptor(
    proto::gen::ReadDescriptorRequest const& request) {
  std::scoped_lock lock(bluetooth_devices_.mut);
  auto descriptor = locateDescriptor(
      request.remote_id(), request.service_uuid(),
      request.secondary_service_uuid(), request.characteristic_uuid(),
      request.descriptor_uuid());

  descriptor->read([](auto& descriptor) -> void {
    proto::gen::ReadDescriptorRequest* request =
        new proto::gen::ReadDescriptorRequest();
    request->set_remote_id(
        descriptor.cCharacteristic().cService().cDevice().cAddress());
    request->set_characteristic_uuid(descriptor.cCharacteristic().UUID());
    request->set_descriptor_uuid(descriptor.UUID());

    if (descriptor.cCharacteristic().cService().getType() ==
        btGatt::ServiceType::PRIMARY) {
      request->set_service_uuid(descriptor.cCharacteristic().cService().UUID());
    } else {
      auto& secondary = dynamic_cast<const btGatt::SecondaryService&>(
          descriptor.cCharacteristic().cService());
      request->set_service_uuid(secondary.cPrimary().UUID());
      request->set_secondary_service_uuid(secondary.UUID());
    }

    proto::gen::ReadDescriptorResponse res;
    res.set_allocated_request(request);
    res.set_allocated_value(new std::string(descriptor.value()));
    descriptor.cCharacteristic()
        .cService()
        .cDevice()
        .cNotificationsHandler()
        .notifyUIThread("ReadDescriptorResponse", res);
  });
}

void BluetoothManager::writeCharacteristic(
    proto::gen::WriteCharacteristicRequest const& request) {
  std::scoped_lock lock(bluetooth_devices_.mut);
  auto characteristic = locateCharacteristic(
      request.remote_id(), request.service_uuid(),
      request.secondary_service_uuid(), request.characteristic_uuid());

  characteristic->write(
      request.value(), request.write_type(),
      [](bool success, auto& characteristic) {
        proto::gen::WriteCharacteristicResponse res;
        proto::gen::WriteCharacteristicRequest* request =
            new proto::gen::WriteCharacteristicRequest();
        request->set_remote_id(characteristic.cService().cDevice().cAddress());
        request->set_characteristic_uuid(characteristic.UUID());

        if (characteristic.cService().getType() ==
            btGatt::ServiceType::PRIMARY) {
          request->set_service_uuid(characteristic.cService().UUID());
        } else {
          auto& secondary = dynamic_cast<const btGatt::SecondaryService&>(
              characteristic.cService());
          request->set_service_uuid(secondary.cPrimary().UUID());
          request->set_secondary_service_uuid(secondary.UUID());
        }
        res.set_success(success);
        res.set_allocated_request(request);
        characteristic.cService()
            .cDevice()
            .cNotificationsHandler()
            .notifyUIThread("WriteCharacteristicResponse", res);
        LOG_DEBUG("finished characteristic write cb");
      });
}

void BluetoothManager::writeDescriptor(
    proto::gen::WriteDescriptorRequest const& request) {
  std::scoped_lock lock(bluetooth_devices_.mut);
  auto descriptor = locateDescriptor(
      request.remote_id(), request.service_uuid(),
      request.secondary_service_uuid(), request.characteristic_uuid(),
      request.descriptor_uuid());

  descriptor->write(
      request.value(), [](auto success, auto& descriptor) -> void {
        proto::gen::WriteDescriptorRequest* request =
            new proto::gen::WriteDescriptorRequest();

        if (descriptor.cCharacteristic().cService().getType() ==
            btGatt::ServiceType::PRIMARY) {
          request->set_service_uuid(
              descriptor.cCharacteristic().cService().UUID());
        } else {
          auto& secondary = dynamic_cast<const btGatt::SecondaryService&>(
              descriptor.cCharacteristic().cService());
          request->set_service_uuid(secondary.cPrimary().UUID());
          request->set_secondary_service_uuid(secondary.UUID());
        }
        request->set_descriptor_uuid(descriptor.UUID());
        request->set_remote_id(
            descriptor.cCharacteristic().cService().cDevice().cAddress());
        request->set_characteristic_uuid(descriptor.cCharacteristic().UUID());

        proto::gen::WriteDescriptorResponse res;
        res.set_success(success);
        res.set_allocated_request(request);

        descriptor.cCharacteristic()
            .cService()
            .cDevice()
            .cNotificationsHandler()
            .notifyUIThread("WriteDescriptorResponse", res);
      });
}

void decodeAdvertisementData(const char* packetsData,
                             proto::gen::AdvertisementData& adv,
                             int dataLen) noexcept {
  using byte = char;
  int start = 0;
  bool longNameSet = false;
  while (start < dataLen) {
    byte ad_len = packetsData[start] & 0xFFu;
    byte type = packetsData[start + 1] & 0xFFu;

    const byte* packet = packetsData + start + 2;
    switch (type) {
      case 0x09:
      case 0x08: {
        if (!longNameSet) {
          adv.set_local_name(packet, ad_len - 1);
        }

        if (type == 0x09) longNameSet = true;
        break;
      }
      case 0x01: {
        adv.set_connectable(*packet & 0x3);
        break;
      }
      case 0xFF: {
        break;
      }
      default:
        break;
    }
    start += ad_len + 1;
  }
}

}  // namespace flutter_blue_tizen