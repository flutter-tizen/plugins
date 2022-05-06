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
  if (!(res = IsBLEAvailable())) {
    LOG_ERROR("Bluetooth is not available on this device!",
              get_error_message(res));
    return;
  }
  if (res = bt_initialize()) {
    LOG_ERROR("[bt_initialize]", get_error_message(res));
    return;
  }
  if (res = bt_gatt_set_connection_state_changed_cb(
          &BluetoothDeviceController::ConnectionStateCallback, this)) {
    LOG_ERROR("[bt_gatt_set_connection_state_changed_cb]",
              get_error_message(res));
    return;
  }
  LOG_DEBUG("All callbacks successfully initialized.");
}

void BluetoothManager::SetNotification(
    const proto::gen::SetNotificationRequest& request) {
  std::scoped_lock lock(bluetooth_devices_.mutex_);

  auto characteristic = LocateCharacteristic(
      request.remote_id(), request.service_uuid(),
      request.secondary_service_uuid(), request.characteristic_uuid());

  if (request.enable())
    characteristic->SetNotifyCallback([](auto& characteristic) {
      proto::gen::SetNotificationResponse response;
      response.set_remote_id(characteristic.cService().cDevice().cAddress());
      response.set_allocated_characteristic(
          new proto::gen::BluetoothCharacteristic(
              characteristic.ToProtoCharacteristic()));
      characteristic.cService()
          .cDevice()
          .cNotificationsHandler()
          .NotifyUIThread("SetNotificationResponse", response);
      LOG_DEBUG("notified UI thread - characteristic written by remote");
    });
  else
    characteristic->UnsetNotifyCallback();
}

u_int32_t BluetoothManager::GetMtu(const std::string& deviceID) {
  std::scoped_lock lock(bluetooth_devices_.mutex_);
  auto device = bluetooth_devices_.var_.find(deviceID)->second;
  if (!device) throw BTException("could not find device of id=" + deviceID);

  return device->GetMtu();
}

void BluetoothManager::RequestMtu(const proto::gen::MtuSizeRequest& request) {
  auto device = bluetooth_devices_.var_.find(request.remote_id())->second;
  if (!device)
    throw BTException("could not find device of id=" + request.remote_id());

  device->RequestMtu(request.mtu(), [](auto status, auto& bluetoothDevice) {
    proto::gen::MtuSizeResponse res;
    res.set_remote_id(bluetoothDevice.cAddress());
    try {
      res.set_mtu(bluetoothDevice.GetMtu());
    } catch (const std::exception& e) {
      // LOG_ERROR(e.what());
    }
    bluetoothDevice.cNotificationsHandler().NotifyUIThread("MtuSize", res);
    LOG_DEBUG("mtu request callback sent response!");
  });
}

btGatt::BluetoothCharacteristic* BluetoothManager::LocateCharacteristic(
    const std::string& remoteID, const std::string& primaryUUID,
    const std::string& secondaryUUID, const std::string& characteristicUUID) {
  auto it = bluetooth_devices_.var_.find(remoteID);

  if (it != bluetooth_devices_.var_.end()) {
    auto device = it->second;
    auto primary = device->GetService(primaryUUID);
    btGatt::BluetoothService* service = primary;
    if (primary && !secondaryUUID.empty()) {
      service = primary->GetSecondary(secondaryUUID);
    }
    if (service) {
      return service->GetCharacteristic(characteristicUUID);
    }
  }
  throw BTException("could not locate characteristic " + characteristicUUID);
}

btGatt::BluetoothDescriptor* BluetoothManager::LocateDescriptor(
    const std::string& remoteID, const std::string& primaryUUID,
    const std::string& secondaryUUID, const std::string& characteristicUUID,
    const std::string& descriptorUUID) {
  auto descriptor = LocateCharacteristic(remoteID, primaryUUID, secondaryUUID,
                                         characteristicUUID)
                        ->GetDescriptor(descriptorUUID);
  if (descriptor) return descriptor;

  throw BTException("could not locate descriptor " + descriptorUUID);
}

bool BluetoothManager::IsBLEAvailable() {
  bool state = false;
  auto res = system_info_get_platform_bool(
      "http://tizen.org/feature/network.bluetooth.le", &state);
  if (res) throw BTException(res, "system_info_get_platform_bool");

  return state;
}

proto::gen::BluetoothState BluetoothManager::BluetoothState() const noexcept {
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

void BluetoothManager::StartBluetoothDeviceScanLE(
    const proto::gen::ScanSettings& scanSettings) {
  StopBluetoothDeviceScanLE();
  std::scoped_lock l(bluetooth_devices_.mutex_);
  bluetooth_devices_.var_.clear();
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
    res = bt_adapter_le_start_scan(&BluetoothManager::ScanCallback, this);
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

void BluetoothManager::ScanCallback(
    int result, bt_adapter_le_device_scan_result_info_s* discovery_info,
    void* user_data) noexcept {
  BluetoothManager& bluetoothManager =
      *static_cast<BluetoothManager*>(user_data);
  if (!result) {
    std::string macAddress = discovery_info->remote_address;
    std::scoped_lock lock(bluetoothManager.bluetooth_devices_.mutex_);
    std::shared_ptr<BluetoothDeviceController> device;
    if (bluetoothManager.bluetooth_devices_.var_.find(macAddress) ==
        bluetoothManager.bluetooth_devices_.var_.end())
      device =
          bluetoothManager.bluetooth_devices_.var_
              .insert({macAddress,
                       std::make_shared<BluetoothDeviceController>(
                           macAddress, bluetoothManager.notifications_handler_)})
              .first->second;
    else
      device = bluetoothManager.bluetooth_devices_.var_.find(macAddress)->second;

    if (bluetoothManager.scan_allow_duplicates_ ||
        device->cProtoBluetoothDevices().empty()) {
      device->ProtoBluetoothDevices().emplace_back();

      auto& protoDev = device->ProtoBluetoothDevices().back();
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
      DecodeAdvertisementData(discovery_info->adv_data, *advertisement_data,
                              discovery_info->adv_data_len);

      scanResult.set_allocated_advertisement_data(advertisement_data);
      scanResult.set_allocated_device(
          new proto::gen::BluetoothDevice(protoDev));

      bluetoothManager.notifications_handler_.NotifyUIThread("ScanResult",
                                                            scanResult);
    }
  }
}

void BluetoothManager::StopBluetoothDeviceScanLE() {
  static std::mutex m;
  std::scoped_lock lock(m);
  auto btState = BluetoothState().state();
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

void BluetoothManager::Connect(const proto::gen::ConnectRequest& connRequest) {
  std::unique_lock lock(bluetooth_devices_.mutex_);
  auto device = bluetooth_devices_.var_.find(connRequest.remote_id())->second;
  if (device)
    device->Connect(connRequest.android_auto_connect());
  else
    throw BTException("device not found!");
}

void BluetoothManager::Disconnect(const std::string& deviceID) {
  std::unique_lock lock(bluetooth_devices_.mutex_);
  auto device = bluetooth_devices_.var_.find(deviceID)->second;
  if (device)
    device->Disconnect();
  else
    throw BTException("device not found!");
}

std::vector<proto::gen::BluetoothDevice>
BluetoothManager::GetConnectedProtoBluetoothDevices() noexcept {
  std::vector<proto::gen::BluetoothDevice> protoBD;
  std::scoped_lock lock(bluetooth_devices_.mutex_);
  for (const auto& e : bluetooth_devices_.var_) {
    if (e.second->GetState() == State::CONNECTED) {
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

void BluetoothManager::ReadCharacteristic(
    const proto::gen::ReadCharacteristicRequest& request) {
  std::scoped_lock lock(bluetooth_devices_.mutex_);
  auto characteristic = LocateCharacteristic(
      request.remote_id(), request.service_uuid(),
      request.secondary_service_uuid(), request.characteristic_uuid());

  characteristic->Read([](auto& characteristic) -> void {
    proto::gen::ReadCharacteristicResponse res;
    res.set_remote_id(characteristic.cService().cDevice().cAddress());
    res.set_allocated_characteristic(new proto::gen::BluetoothCharacteristic(
        characteristic.ToProtoCharacteristic()));

    characteristic.cService().cDevice().cNotificationsHandler().NotifyUIThread(
        "ReadCharacteristicResponse", res);
    LOG_DEBUG("finished characteristic read cb");
  });
}

void BluetoothManager::ReadDescriptor(
    const proto::gen::ReadDescriptorRequest& request) {
  std::scoped_lock lock(bluetooth_devices_.mutex_);
  auto descriptor = LocateDescriptor(
      request.remote_id(), request.service_uuid(),
      request.secondary_service_uuid(), request.characteristic_uuid(),
      request.descriptor_uuid());

  descriptor->Read([](auto& descriptor) -> void {
    proto::gen::ReadDescriptorRequest* request =
        new proto::gen::ReadDescriptorRequest();
    request->set_remote_id(
        descriptor.cCharacteristic().cService().cDevice().cAddress());
    request->set_characteristic_uuid(descriptor.cCharacteristic().Uuid());
    request->set_descriptor_uuid(descriptor.Uuid());

    if (descriptor.cCharacteristic().cService().GetType() ==
        btGatt::ServiceType::PRIMARY) {
      request->set_service_uuid(descriptor.cCharacteristic().cService().Uuid());
    } else {
      auto& secondary = dynamic_cast<const btGatt::SecondaryService&>(
          descriptor.cCharacteristic().cService());
      request->set_service_uuid(secondary.cPrimary().Uuid());
      request->set_secondary_service_uuid(secondary.Uuid());
    }

    proto::gen::ReadDescriptorResponse res;
    res.set_allocated_request(request);
    res.set_allocated_value(new std::string(descriptor.value()));
    descriptor.cCharacteristic()
        .cService()
        .cDevice()
        .cNotificationsHandler()
        .NotifyUIThread("ReadDescriptorResponse", res);
  });
}

void BluetoothManager::WriteCharacteristic(
    const proto::gen::WriteCharacteristicRequest& request) {
  std::scoped_lock lock(bluetooth_devices_.mutex_);
  auto characteristic = LocateCharacteristic(
      request.remote_id(), request.service_uuid(),
      request.secondary_service_uuid(), request.characteristic_uuid());

  characteristic->Write(
      request.value(), request.write_type(),
      [](bool success, auto& characteristic) {
        proto::gen::WriteCharacteristicResponse res;
        proto::gen::WriteCharacteristicRequest* request =
            new proto::gen::WriteCharacteristicRequest();
        request->set_remote_id(characteristic.cService().cDevice().cAddress());
        request->set_characteristic_uuid(characteristic.Uuid());

        if (characteristic.cService().GetType() ==
            btGatt::ServiceType::PRIMARY) {
          request->set_service_uuid(characteristic.cService().Uuid());
        } else {
          auto& secondary = dynamic_cast<const btGatt::SecondaryService&>(
              characteristic.cService());
          request->set_service_uuid(secondary.cPrimary().Uuid());
          request->set_secondary_service_uuid(secondary.Uuid());
        }
        res.set_success(success);
        res.set_allocated_request(request);
        characteristic.cService()
            .cDevice()
            .cNotificationsHandler()
            .NotifyUIThread("WriteCharacteristicResponse", res);
        LOG_DEBUG("finished characteristic write cb");
      });
}

void BluetoothManager::writeDescriptor(
    const proto::gen::WriteDescriptorRequest& request) {
  std::scoped_lock lock(bluetooth_devices_.mutex_);
  auto descriptor = LocateDescriptor(
      request.remote_id(), request.service_uuid(),
      request.secondary_service_uuid(), request.characteristic_uuid(),
      request.descriptor_uuid());

  descriptor->Write(
      request.value(), [](auto success, auto& descriptor) -> void {
        proto::gen::WriteDescriptorRequest* request =
            new proto::gen::WriteDescriptorRequest();

        if (descriptor.cCharacteristic().cService().GetType() ==
            btGatt::ServiceType::PRIMARY) {
          request->set_service_uuid(
              descriptor.cCharacteristic().cService().Uuid());
        } else {
          auto& secondary = dynamic_cast<const btGatt::SecondaryService&>(
              descriptor.cCharacteristic().cService());
          request->set_service_uuid(secondary.cPrimary().Uuid());
          request->set_secondary_service_uuid(secondary.Uuid());
        }
        request->set_descriptor_uuid(descriptor.Uuid());
        request->set_remote_id(
            descriptor.cCharacteristic().cService().cDevice().cAddress());
        request->set_characteristic_uuid(descriptor.cCharacteristic().Uuid());

        proto::gen::WriteDescriptorResponse res;
        res.set_success(success);
        res.set_allocated_request(request);

        descriptor.cCharacteristic()
            .cService()
            .cDevice()
            .cNotificationsHandler()
            .NotifyUIThread("WriteDescriptorResponse", res);
      });
}

void DecodeAdvertisementData(const char* packetsData,
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