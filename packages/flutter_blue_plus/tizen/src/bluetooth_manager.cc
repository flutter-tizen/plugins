#include "bluetooth_manager.h"

#include <system_info.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <mutex>

#include "bluetooth.h"
#include "bluetooth_device_controller.h"
#include "bluetooth_type.h"
#include "flutter/encodable_value.h"
#include "flutterblue.pb.h"
#include "log.h"
#include "proto_helper.h"
#include "system_event_handler.h"
#include "tizen_error.h"
#include "utils.h"

namespace flutter_blue_tizen {

using State = BluetoothDeviceController::State;

BluetoothManager::BluetoothManager(NotificationsHandler& notificationsHandler)
    : notifications_handler_(notificationsHandler) {
  int ret = IsBLEAvailable();
  if (ret == 0) {
    LOG_ERROR("Bluetooth is not available on this device! %s",
              get_error_message(ret));
    return;
  }
  ret = bt_initialize();
  if (ret != 0) {
    LOG_ERROR("[bt_initialize] %s", get_error_message(ret));
    return;
  }

  BluetoothDeviceController::SetConnectionStateChangedCallback(
      [&notifications_handler = notifications_handler_](
          BluetoothDeviceController::State state,
          const BluetoothDeviceController& device) {
        proto::gen::DeviceStateResponse device_state;
        device_state.set_remote_id(device.address());
        device_state.set_state(ToProtoDeviceState(state));
        notifications_handler.NotifyUIThread("DeviceState", device_state);
      });

  LOG_DEBUG("All callbacks successfully initialized.");
}

void BluetoothManager::SetNotification(
    const proto::gen::SetNotificationRequest& request) {
  const auto& device = LocateDevice(request.remote_id());

  const auto& service =
      LocateService(request.remote_id(), request.service_uuid(),
                    request.secondary_service_uuid());

  auto& characteristic = LocateCharacteristic(
      request.remote_id(), request.service_uuid(),
      request.secondary_service_uuid(), request.characteristic_uuid());

  if (request.enable()) {
    characteristic.SetNotifyCallback(
        [&device, &service, &notifications_handler = notifications_handler_](
            auto& characteristic) {
          proto::gen::SetNotificationResponse response;
          response.set_remote_id(device.address());
          response.set_allocated_characteristic(
              new proto::gen::BluetoothCharacteristic(
                  ToProtoCharacteristic(device, service, characteristic)));

          notifications_handler.NotifyUIThread("SetNotificationResponse",
                                               response);
          LOG_DEBUG("notified UI thread - characteristic written by remote");
        });
  } else {
    characteristic.UnsetNotifyCallback();
  }
}

uint32_t BluetoothManager::GetMtu(const std::string& device_id) {
  const auto& device = LocateDevice(device_id);

  return device.GetMtu();
}

void BluetoothManager::RequestMtu(const proto::gen::MtuSizeRequest& request) {
  auto& device = LocateDevice(request.remote_id());
  device.RequestMtu(
      request.mtu(), [&notifications_handler = notifications_handler_](
                         auto status, auto& bluetooth_device) {
        proto::gen::MtuSizeResponse mtu_size_response;
        mtu_size_response.set_remote_id(bluetooth_device.address());
        try {
          mtu_size_response.set_mtu(bluetooth_device.GetMtu());
        } catch (const std::exception& e) {
          // LOG_ERROR(e.what());
        }
        notifications_handler.NotifyUIThread("MtuSize", mtu_size_response);
        LOG_DEBUG("mtu request callback sent response!");
      });
}

void BluetoothManager::ReadRssi(const std::string& device_id) {
  const auto& device = LocateDevice(device_id);

  device.ReadRssi([&notifications_handler = notifications_handler_](
                      auto& bluetoothDevice, int rssi) {
    LOG_DEBUG("read_rssi_callback called");

    proto::gen::ReadRssiResult result;
    result.set_rssi(rssi);
    result.set_remote_id(bluetoothDevice.address());
    notifications_handler.NotifyUIThread("ReadRssiResult", result);
  });
}

void BluetoothManager::Pair(const std::string& device_id) {
  auto& device = LocateDevice(device_id);

  device.Pair([](auto&&...) { LOG_DEBUG("pair_callback called"); });
}

std::vector<BluetoothDeviceController*> BluetoothManager::GetBondedDevices() {
  std::scoped_lock lock(bluetooth_devices_.mutex_);
  std::vector<BluetoothDeviceController*> ret;
  for (auto& device : bluetooth_devices_.var_) {
    ret.push_back(device.second.get());
  }
  return ret;
}

BluetoothDeviceController& BluetoothManager::LocateDevice(
    const std::string& remote_id) {
  std::scoped_lock lock(bluetooth_devices_.mutex_);
  auto it = bluetooth_devices_.var_.find(remote_id);

  if (it == bluetooth_devices_.var_.end())
    throw BtException("could not locate device " + remote_id);

  return *it->second;
}

btGatt::PrimaryService& BluetoothManager::LocatePrimaryService(
    const std::string& remote_id, const std::string& primary_uuid) {
  auto& device = LocateDevice(remote_id);

  auto primary = device.GetService(primary_uuid);

  if (!primary)
    throw BtException("could not locate primary service " + primary_uuid);

  return *primary;
}

btGatt::SecondaryService& BluetoothManager::LocateSecondaryService(
    const std::string& remote_id, const std::string& primary_uuid,
    const std::string& secondary_uuid) {
  const auto& primary = LocatePrimaryService(remote_id, primary_uuid);

  auto secondary = primary.GetSecondary(secondary_uuid);

  if (!secondary)
    throw BtException("could not locate secondary service " + secondary_uuid);
  return *secondary;
}

btGatt::BluetoothService& BluetoothManager::LocateService(
    const std::string& remote_id, const std::string& primary_uuid,
    const std::string& secondary_uuid) {
  if (secondary_uuid.empty())
    return LocatePrimaryService(remote_id, primary_uuid);

  return LocateSecondaryService(remote_id, primary_uuid, secondary_uuid);
}

btGatt::BluetoothCharacteristic& BluetoothManager::LocateCharacteristic(
    const std::string& remote_id, const std::string& primary_uuid,
    const std::string& secondary_uuid, const std::string& characteristic_uuid) {
  const auto& service = LocateService(remote_id, primary_uuid, secondary_uuid);

  auto characteristic = service.GetCharacteristic(characteristic_uuid);

  if (!characteristic)
    throw BtException("could not locate characteristic " + characteristic_uuid);

  return *characteristic;
}

btGatt::BluetoothDescriptor& BluetoothManager::LocateDescriptor(
    const std::string& remote_id, const std::string& primary_uuid,
    const std::string& secondary_uuid, const std::string& characteristic_uuid,
    const std::string& descriptor_uuid) {
  auto descriptor = LocateCharacteristic(remote_id, primary_uuid,
                                         secondary_uuid, characteristic_uuid)
                        .GetDescriptor(descriptor_uuid);
  if (descriptor) return *descriptor;

  throw BtException("could not locate descriptor " + descriptor_uuid);
}

bool BluetoothManager::IsBLEAvailable() {
  bool state = false;
  auto ret = system_info_get_platform_bool(
      "http://tizen.org/feature/network.bluetooth.le", &state);
  if (ret) throw BtException(ret, "system_info_get_platform_bool");

  return state;
}

enum BluetoothManager::BluetoothState
BluetoothManager::GetBluetoothState() noexcept {
  using NativeState = bt_adapter_state_e;

  NativeState adapter_state;
  int ret = bt_adapter_get_state(&adapter_state);

  if (ret == BT_ERROR_NONE) {
    if (adapter_state == NativeState::BT_ADAPTER_ENABLED) {
      return BluetoothState::kAdapterOn;
    } else {
      return BluetoothState::kAdapterOff;
    }
  } else if (ret == BT_ERROR_NOT_INITIALIZED) {
    return BluetoothState::kUnavailable;
  } else {
    return BluetoothState::kUnknown;
  }
}

void BluetoothManager::StartBluetoothDeviceScanLE(
    const BleScanSettings& scan_settings, ScanCallback callback) {
  auto ret = bt_adapter_le_set_scan_mode(BT_ADAPTER_LE_SCAN_MODE_BALANCED);
  LOG_ERROR("bt_adapter_le_set_scan_mode %s", get_error_message(ret));

  if (ret) throw BtException(ret, "bt_adapter_le_set_scan_mode");

  StopBluetoothDeviceScanLE();

  struct Scope {
    ScanCallback scan_callback;
    std::vector<bt_scan_filter_h> filters;
  };

  static SafeType<std::optional<Scope>>
      scope;  // there's only one scan available per time.

  std::scoped_lock lock(scope.mutex_);

  if (scope.var_.has_value()) {
    auto& filters = scope.var_->filters;
    ret = bt_adapter_le_scan_filter_unregister_all();
    LOG_ERROR("bt_adapter_le_scan_filter_unregister_all %s",
              get_error_message(ret));
    LOG_DEBUG("bt_adapter_le_scan_filter_unregister_all");

    for (auto filter : filters) {
      ret = bt_adapter_le_scan_filter_destroy(filter);
      LOG_ERROR("bt_adapter_le_scan_filter_destroy %s", get_error_message(ret));
      LOG_DEBUG("bt_adapter_le_scan_filter_destroy");
    }

    scope.var_->filters.clear();
  }

  scope.var_.emplace(Scope{std::move(callback)});

  auto& filters = scope.var_->filters;
  std::transform(scan_settings.service_uuids_filters_.begin(),
                 scan_settings.service_uuids_filters_.end(),
                 std::back_inserter(filters), [](const std::string& uuid) {
                   bt_scan_filter_h filter;
                   auto ret = bt_adapter_le_scan_filter_create(&filter);
                   LOG_ERROR("bt_adapter_le_scan_filter_create %s",
                             get_error_message(ret));

                   ret = bt_adapter_le_scan_filter_set_service_uuid(
                       filter, uuid.c_str());
                   LOG_ERROR("bt_adapter_le_scan_filter_set_service_uuid%s ",
                             get_error_message(ret));

                   ret = bt_adapter_le_scan_filter_register(filter);

                   return filter;
                 });

  std::transform(scan_settings.device_ids_filters_.begin(),
                 scan_settings.device_ids_filters_.end(),
                 std::back_inserter(filters), [](const std::string& uuid) {
                   bt_scan_filter_h filter;
                   auto ret = bt_adapter_le_scan_filter_create(&filter);
                   LOG_ERROR("bt_adapter_le_scan_filter_create %s",
                             get_error_message(ret));

                   ret = bt_adapter_le_scan_filter_set_device_address(
                       filter, uuid.c_str());
                   LOG_ERROR("bt_adapter_le_scan_filter_set_device_address%s",
                             get_error_message(ret));

                   ret = bt_adapter_le_scan_filter_register(filter);

                   return filter;
                 });

  if (!ret) {
    ret = bt_adapter_le_start_scan(
        [](int result, bt_adapter_le_device_scan_result_info_s* info,
           void* scope_ptr) {
          if (!result) {
            auto& scope =
                *static_cast<SafeType<std::optional<Scope>>*>(scope_ptr);
            std::scoped_lock scope_lock(scope.mutex_);

            LOG_DEBUG("native scan callback: %s", get_error_message(result));
            auto rssi = info->rssi;
            std::string address = info->remote_address;
            AdvertisementData advertisement_data =
                DecodeAdvertisementData(info->adv_data, info->adv_data_len);

            std::string device_name;
            char* name_cstr;
            int ret = bt_adapter_le_get_scan_result_device_name(
                info, BT_ADAPTER_LE_PACKET_SCAN_RESPONSE, &name_cstr);

            if (!ret) {
              device_name = std::string(name_cstr);
              free(name_cstr);
            }

            scope.var_->scan_callback(address, device_name, rssi,
                                      advertisement_data);
          }
        },
        static_cast<void*>(&scope));  // note that it's a static variable

    LOG_ERROR("bt_adapter_le_start_scan %s", get_error_message(ret));
  }
}

void BluetoothManager::StartBluetoothDeviceScanLE(
    const BleScanSettings& scan_settings) {
  std::scoped_lock lock(bluetooth_devices_.mutex_);
  bluetooth_devices_.var_.clear();

  StartBluetoothDeviceScanLE(
      scan_settings,
      [&bluetooth_manager = *this,
       &notifications_handler = notifications_handler_](
          const std::string& address, const std::string& device_name, int rssi,
          const flutter_blue_tizen::AdvertisementData& advertisement_data) {
        std::scoped_lock lock(bluetooth_manager.bluetooth_devices_.mutex_);

        auto& devices_container = bluetooth_manager.bluetooth_devices_.var_;

        auto it = devices_container.find(address);

        BluetoothDeviceController* device;
        if (it == devices_container.end()) {
          device =
              bluetooth_manager.bluetooth_devices_.var_
                  .insert({address, std::make_unique<BluetoothDeviceController>(
                                        device_name, address)})
                  .first->second.get();
        } else {
          device = it->second.get();
        }

        proto::gen::ScanResult scan_result;
        scan_result.set_rssi(rssi);

        auto proto_advertisement_data = new proto::gen::AdvertisementData();

        flutter_blue_tizen::ToProtoAdvertisementData(advertisement_data,
                                                     *proto_advertisement_data);

        scan_result.set_allocated_advertisement_data(proto_advertisement_data);

        auto proto_device =
            new proto::gen::BluetoothDevice(ToProtoDevice(*device));

        scan_result.set_allocated_device(proto_device);

        notifications_handler.NotifyUIThread("ScanResult", scan_result);
      });
}

void BluetoothManager::StopBluetoothDeviceScanLE() {
  static std::mutex mutex;
  std::scoped_lock lock(mutex);
  auto bt_state = GetBluetoothState();
  if (bt_state == BluetoothState::kAdapterOn) {
    auto ret = bt_adapter_le_stop_scan();
    LOG_ERROR("bt_adapter_le_is_discovering %s", get_error_message(ret));
  }
}

void BluetoothManager::Connect(const proto::gen::ConnectRequest& conn_request) {
  auto& device = LocateDevice(conn_request.remote_id());
  device.Connect(conn_request.android_auto_connect());
}

void BluetoothManager::Disconnect(const std::string& device_id) {
  auto& device = LocateDevice(device_id);
  device.Disconnect();
}

std::vector<proto::gen::BluetoothDevice>
BluetoothManager::GetConnectedProtoBluetoothDevices() noexcept {
  std::vector<proto::gen::BluetoothDevice> proto_bluetooth_device;
  std::scoped_lock lock(bluetooth_devices_.mutex_);
  for (const auto& e : bluetooth_devices_.var_) {
    if (e.second->GetState() == State::kConnected) {
      proto_bluetooth_device.emplace_back(ToProtoDevice(*e.second));
    }
  }
  return proto_bluetooth_device;
}

void BluetoothManager::ReadCharacteristic(
    const proto::gen::ReadCharacteristicRequest& request) {
  const auto& device = LocateDevice(request.remote_id());

  const auto& characteristic = LocateCharacteristic(
      request.remote_id(), request.service_uuid(),
      request.secondary_service_uuid(), request.characteristic_uuid());

  const auto& service =
      LocateService(request.remote_id(), request.service_uuid(),
                    request.secondary_service_uuid());

  characteristic.Read(
      [&device, &service, &notifications_handler = notifications_handler_](
          auto& characteristic) -> void {
        proto::gen::ReadCharacteristicResponse read_characteristic_result;
        read_characteristic_result.set_remote_id(device.address());
        read_characteristic_result.set_allocated_characteristic(
            new proto::gen::BluetoothCharacteristic(
                ToProtoCharacteristic(device, service, characteristic)));

        notifications_handler.NotifyUIThread("ReadCharacteristicResponse",
                                             read_characteristic_result);
        LOG_DEBUG("finished characteristic read cb");
      });
}

void BluetoothManager::ReadDescriptor(
    const proto::gen::ReadDescriptorRequest& request) {
  const auto& descriptor = LocateDescriptor(
      request.remote_id(), request.service_uuid(),
      request.secondary_service_uuid(), request.characteristic_uuid(),
      request.descriptor_uuid());

  descriptor.Read([request, &notifications_handler = notifications_handler_](
                      auto& descriptor) -> void {
    proto::gen::ReadDescriptorResponse read_descriptor_response;
    read_descriptor_response.set_allocated_request(
        new proto::gen::ReadDescriptorRequest(request));
    read_descriptor_response.set_allocated_value(
        new std::string(descriptor.Value()));
    notifications_handler.NotifyUIThread("ReadDescriptorResponse",
                                         read_descriptor_response);
  });
}

void BluetoothManager::WriteCharacteristic(
    const proto::gen::WriteCharacteristicRequest& request) {
  auto& characteristic = LocateCharacteristic(
      request.remote_id(), request.service_uuid(),
      request.secondary_service_uuid(), request.characteristic_uuid());

  characteristic.Write(
      request.value(), request.write_type(),
      [request, &notifications_handler = notifications_handler_](
          bool success, auto& characteristic) {
        proto::gen::WriteCharacteristicResponse write_characteristic_response;

        write_characteristic_response.set_success(success);
        write_characteristic_response.set_allocated_request(
            new proto::gen::WriteCharacteristicRequest(request));
        notifications_handler.NotifyUIThread("WriteCharacteristicResponse",
                                             write_characteristic_response);
        LOG_DEBUG("finished characteristic write cb");
      });
}

void BluetoothManager::WriteDescriptor(
    const proto::gen::WriteDescriptorRequest& request) {
  auto& descriptor = LocateDescriptor(
      request.remote_id(), request.service_uuid(),
      request.secondary_service_uuid(), request.characteristic_uuid(),
      request.descriptor_uuid());

  descriptor.Write(
      request.value(),
      [request, &notifications_handler = notifications_handler_](
          auto success, auto& descriptor) -> void {
        proto::gen::WriteDescriptorResponse write_descriptor_response;
        write_descriptor_response.set_success(success);
        write_descriptor_response.set_allocated_request(
            new proto::gen::WriteDescriptorRequest(request));

        notifications_handler.NotifyUIThread("WriteDescriptorResponse",
                                             write_descriptor_response);
      });
}

}  // namespace flutter_blue_tizen
