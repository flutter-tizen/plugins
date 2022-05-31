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
  int ret = IsBLEAvailable();
  if (ret == 0) {
    LOG_ERROR("Bluetooth is not available on this device!",
              get_error_message(ret));
    return;
  }
  ret = bt_initialize();
  if (ret != 0) {
    LOG_ERROR("[bt_initialize]", get_error_message(ret));
    return;
  }
  ret = bt_gatt_set_connection_state_changed_cb(
      &BluetoothDeviceController::ConnectionStateCallback, this);
  if (ret != 0) {
    LOG_ERROR("[bt_gatt_set_connection_state_changed_cb]",
              get_error_message(ret));
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

  if (request.enable()) {
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
  } else {
    characteristic->UnsetNotifyCallback();
  }
}

uint32_t BluetoothManager::GetMtu(const std::string& device_id) {
  std::scoped_lock lock(bluetooth_devices_.mutex_);
  auto device = bluetooth_devices_.var_.find(device_id)->second;
  if (!device) throw BtException("could not find device of id=" + device_id);

  return device->GetMtu();
}

void BluetoothManager::RequestMtu(const proto::gen::MtuSizeRequest& request) {
  auto device = bluetooth_devices_.var_.find(request.remote_id())->second;
  if (!device)
    throw BtException("could not find device of id=" + request.remote_id());

  device->RequestMtu(request.mtu(), [](auto status, auto& bluetooth_device) {
    proto::gen::MtuSizeResponse mtu_size_response;
    mtu_size_response.set_remote_id(bluetooth_device.cAddress());
    try {
      mtu_size_response.set_mtu(bluetooth_device.GetMtu());
    } catch (const std::exception& e) {
      // LOG_ERROR(e.what());
    }
    bluetooth_device.cNotificationsHandler().NotifyUIThread("MtuSize",
                                                            mtu_size_response);
    LOG_DEBUG("mtu request callback sent response!");
  });
}

BluetoothDeviceController* BluetoothManager::LocateDevice(
    const std::string& remote_id) {
  auto it = bluetooth_devices_.var_.find(remote_id);
  return (it == bluetooth_devices_.var_.end() ? nullptr : it->second.get());
}

btGatt::PrimaryService* BluetoothManager::LocatePrimaryService(
    const std::string& remote_id, const std::string& primary_uuid) {
  auto device = LocateDevice(remote_id);
  return device->GetService(primary_uuid);
}

btGatt::SecondaryService* BluetoothManager::LocateSecondaryService(
    const std::string& remote_id, const std::string& primary_uuid,
    const std::string& secondary_uuid) {
  auto device = LocateDevice(remote_id);
  auto primary = device->GetService(primary_uuid);
  return primary->GetSecondary(secondary_uuid);
}

btGatt::BluetoothCharacteristic* BluetoothManager::LocateCharacteristic(
    const std::string& remote_id, const std::string& primary_uuid,
    const std::string& secondary_uuid, const std::string& characteristic_uuid) {
  auto device = LocateDevice(remote_id);

  if (device) {
    auto primary = device->GetService(primary_uuid);
    btGatt::BluetoothService* service = primary;
    if (primary && !secondary_uuid.empty()) {
      service = primary->GetSecondary(secondary_uuid);
    }
    if (service) {
      return service->GetCharacteristic(characteristic_uuid);
    }
  }
  throw BtException("could not locate characteristic " + characteristic_uuid);
}

btGatt::BluetoothDescriptor* BluetoothManager::LocateDescriptor(
    const std::string& remote_id, const std::string& primary_uuid,
    const std::string& secondary_uuid, const std::string& characteristic_uuid,
    const std::string& descriptor_uuid) {
  auto descriptor = LocateCharacteristic(remote_id, primary_uuid,
                                         secondary_uuid, characteristic_uuid)
                        ->GetDescriptor(descriptor_uuid);
  if (descriptor) return descriptor;

  throw BtException("could not locate descriptor " + descriptor_uuid);
}

bool BluetoothManager::IsBLEAvailable() {
  bool state = false;
  auto ret = system_info_get_platform_bool(
      "http://tizen.org/feature/network.bluetooth.le", &state);
  if (ret) throw BtException(ret, "system_info_get_platform_bool");

  return state;
}

proto::gen::BluetoothState BluetoothManager::BluetoothState() const noexcept {
  /* Checks whether the Bluetooth adapter is enabled */
  bt_adapter_state_e adapter_state;
  int ret = bt_adapter_get_state(&adapter_state);
  proto::gen::BluetoothState state;
  if (ret == BT_ERROR_NONE) {
    if (adapter_state == BT_ADAPTER_ENABLED) {
      state.set_state(proto::gen::BluetoothState_State_ON);
    } else {
      state.set_state(proto::gen::BluetoothState_State_OFF);
    }
  } else if (ret == BT_ERROR_NOT_INITIALIZED) {
    state.set_state(proto::gen::BluetoothState_State_UNAVAILABLE);
  } else {
    state.set_state(proto::gen::BluetoothState_State_UNKNOWN);
  }

  return state;
}

void BluetoothManager::StartBluetoothDeviceScanLE(
    const proto::gen::ScanSettings& scan_settings) {
  StopBluetoothDeviceScanLE();
  std::scoped_lock lock(bluetooth_devices_.mutex_);
  bluetooth_devices_.var_.clear();
  scan_allow_duplicates_ = scan_settings.allow_duplicates();
  auto ret = bt_adapter_le_set_scan_mode(BT_ADAPTER_LE_SCAN_MODE_BALANCED);
  LOG_ERROR("bt_adapter_le_set_scan_mode", get_error_message(ret));

  if (ret) throw BtException(ret, "bt_adapter_le_set_scan_mode");

  int uuid_count = scan_settings.service_uuids_size();
  std::vector<bt_scan_filter_h> filters(uuid_count);

  for (int i = 0; i < uuid_count; ++i) {
    const std::string& uuid = scan_settings.service_uuids()[i];
    ret = bt_adapter_le_scan_filter_create(&filters[i]);

    LOG_ERROR("bt_adapter_le_scan_filter_create", get_error_message(ret));
    if (ret) throw BtException(ret, "bt_adapter_le_scan_filter_create");

    ret =
        bt_adapter_le_scan_filter_set_device_address(filters[i], uuid.c_str());

    LOG_ERROR("bt_adapter_le_scan_filter_set_device_address",
              get_error_message(ret));

    if (ret)
      throw BtException(ret, "bt_adapter_le_scan_filter_set_device_address");
  }

  if (!ret) {
    ret = bt_adapter_le_start_scan(&BluetoothManager::ScanCallback, this);
    LOG_ERROR("bt_adapter_le_start_scan", get_error_message(ret));
    if (ret) throw BtException(ret, "bt_adapter_le_start_scan");
  } else {
    throw BtException(ret, "cannot start scan");
  }

  for (auto& filter : filters) {
    ret = bt_adapter_le_scan_filter_destroy(&filter);
    LOG_ERROR("bt_adapter_le_scan_filter_destroy", get_error_message(ret));

    if (ret) throw BtException(ret, "bt_adapter_le_scan_filter_destroy");
  }
}

void BluetoothManager::ScanCallback(
    int result, bt_adapter_le_device_scan_result_info_s* discovery_info,
    void* user_data) noexcept {
  BluetoothManager& bluetooth_manager =
      *static_cast<BluetoothManager*>(user_data);
  if (!result) {
    std::string mac_address = discovery_info->remote_address;
    std::scoped_lock lock(bluetooth_manager.bluetooth_devices_.mutex_);
    std::shared_ptr<BluetoothDeviceController> device;
    if (bluetooth_manager.bluetooth_devices_.var_.find(mac_address) ==
        bluetooth_manager.bluetooth_devices_.var_.end()) {
      device = bluetooth_manager.bluetooth_devices_.var_
                   .insert({mac_address,
                            std::make_shared<BluetoothDeviceController>(
                                mac_address,
                                bluetooth_manager.notifications_handler_)})
                   .first->second;
    } else {
      device =
          bluetooth_manager.bluetooth_devices_.var_.find(mac_address)->second;
    }

    if (bluetooth_manager.scan_allow_duplicates_ ||
        device->cProtoBluetoothDevices().empty()) {
      device->ProtoBluetoothDevices().emplace_back();

      auto& proto_device = device->ProtoBluetoothDevices().back();
      char* name;
      result = bt_adapter_le_get_scan_result_device_name(
          discovery_info, BT_ADAPTER_LE_PACKET_SCAN_RESPONSE, &name);
      if (!result) {
        proto_device.set_name(name);
        free(name);
      }
      proto_device.set_remote_id(mac_address);

      proto::gen::ScanResult scan_result;
      scan_result.set_rssi(discovery_info->rssi);
      proto::gen::AdvertisementData* advertisement_data =
          new proto::gen::AdvertisementData();
      DecodeAdvertisementData(discovery_info->adv_data, *advertisement_data,
                              discovery_info->adv_data_len);

      scan_result.set_allocated_advertisement_data(advertisement_data);
      scan_result.set_allocated_device(
          new proto::gen::BluetoothDevice(proto_device));

      bluetooth_manager.notifications_handler_.NotifyUIThread("ScanResult",
                                                              scan_result);
    }
  }
}

void BluetoothManager::StopBluetoothDeviceScanLE() {
  static std::mutex mutex;
  std::scoped_lock lock(mutex);
  auto bt_state = BluetoothState().state();
  if (bt_state == proto::gen::BluetoothState_State_ON) {
    bool is_discovering;
    auto ret = bt_adapter_le_is_discovering(&is_discovering);
    if (!ret && is_discovering) {
      ret = bt_adapter_le_stop_scan();
      LOG_ERROR("bt_adapter_le_stop_scan", ret);
    }

    LOG_ERROR("bt_adapter_le_is_discovering", ret);
  }
}

void BluetoothManager::Connect(const proto::gen::ConnectRequest& conn_request) {
  std::unique_lock lock(bluetooth_devices_.mutex_);
  auto device = bluetooth_devices_.var_.find(conn_request.remote_id())->second;
  if (device) {
    device->Connect(conn_request.android_auto_connect());
  } else {
    throw BtException("device not found!");
  }
}

void BluetoothManager::Disconnect(const std::string& device_id) {
  std::unique_lock lock(bluetooth_devices_.mutex_);
  auto device = bluetooth_devices_.var_.find(device_id)->second;
  if (device) {
    device->Disconnect();
  } else {
    throw BtException("device not found!");
  }
}

std::vector<proto::gen::BluetoothDevice>
BluetoothManager::GetConnectedProtoBluetoothDevices() noexcept {
  std::vector<proto::gen::BluetoothDevice> proto_bluetooth_device;
  std::scoped_lock lock(bluetooth_devices_.mutex_);
  for (const auto& e : bluetooth_devices_.var_) {
    if (e.second->GetState() == State::kConnected) {
      auto& vec = e.second->cProtoBluetoothDevices();
      proto_bluetooth_device.insert(proto_bluetooth_device.end(), vec.cbegin(),
                                    vec.cend());
    }
  }
  return proto_bluetooth_device;
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
    proto::gen::ReadCharacteristicResponse read_characteristic_result;
    read_characteristic_result.set_remote_id(
        characteristic.cService().cDevice().cAddress());
    read_characteristic_result.set_allocated_characteristic(
        new proto::gen::BluetoothCharacteristic(
            characteristic.ToProtoCharacteristic()));

    characteristic.cService().cDevice().cNotificationsHandler().NotifyUIThread(
        "ReadCharacteristicResponse", read_characteristic_result);
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
        btGatt::ServiceType::kPrimary) {
      request->set_service_uuid(descriptor.cCharacteristic().cService().Uuid());
    } else {
      auto& secondary = dynamic_cast<const btGatt::SecondaryService&>(
          descriptor.cCharacteristic().cService());
      request->set_service_uuid(secondary.cPrimary().Uuid());
      request->set_secondary_service_uuid(secondary.Uuid());
    }

    proto::gen::ReadDescriptorResponse read_descriptor_response;
    read_descriptor_response.set_allocated_request(request);
    read_descriptor_response.set_allocated_value(
        new std::string(descriptor.Value()));
    descriptor.cCharacteristic()
        .cService()
        .cDevice()
        .cNotificationsHandler()
        .NotifyUIThread("ReadDescriptorResponse", read_descriptor_response);
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
        proto::gen::WriteCharacteristicResponse write_characteristic_response;
        proto::gen::WriteCharacteristicRequest* write_characteristic_request =
            new proto::gen::WriteCharacteristicRequest();
        write_characteristic_request->set_remote_id(
            characteristic.cService().cDevice().cAddress());
        write_characteristic_request->set_characteristic_uuid(
            characteristic.Uuid());

        if (characteristic.cService().GetType() ==
            btGatt::ServiceType::kPrimary) {
          write_characteristic_request->set_service_uuid(
              characteristic.cService().Uuid());
        } else {
          auto& secondary = dynamic_cast<const btGatt::SecondaryService&>(
              characteristic.cService());
          write_characteristic_request->set_service_uuid(
              secondary.cPrimary().Uuid());
          write_characteristic_request->set_secondary_service_uuid(
              secondary.Uuid());
        }
        write_characteristic_response.set_success(success);
        write_characteristic_response.set_allocated_request(
            write_characteristic_request);
        characteristic.cService()
            .cDevice()
            .cNotificationsHandler()
            .NotifyUIThread("WriteCharacteristicResponse",
                            write_characteristic_response);
        LOG_DEBUG("finished characteristic write cb");
      });
}

void BluetoothManager::WriteDescriptor(
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
            btGatt::ServiceType::kPrimary) {
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

        proto::gen::WriteDescriptorResponse write_descriptor_response;
        write_descriptor_response.set_success(success);
        write_descriptor_response.set_allocated_request(request);

        descriptor.cCharacteristic()
            .cService()
            .cDevice()
            .cNotificationsHandler()
            .NotifyUIThread("WriteDescriptorResponse",
                            write_descriptor_response);
      });
}

void DecodeAdvertisementData(const char* packets_data,
                             proto::gen::AdvertisementData& advertisement_data,
                             int dataLen) noexcept {
  using byte = char;
  int start = 0;
  bool long_name_set = false;
  while (start < dataLen) {
    byte advertisement_data_len = packets_data[start] & 0xFFu;
    byte type = packets_data[start + 1] & 0xFFu;

    const byte* packet = packets_data + start + 2;
    switch (type) {
      case 0x09:
      case 0x08: {
        if (!long_name_set)
          advertisement_data.set_local_name(packet, advertisement_data_len - 1);

        if (type == 0x09) long_name_set = true;

        break;
      }
      case 0x01: {
        advertisement_data.set_connectable(*packet & 0x3);
        break;
      }
      case 0xFF: {
        break;
      }
      default:
        break;
    }
    start += advertisement_data_len + 1;
  }
}

}  // namespace flutter_blue_tizen