// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ble_device.h"

#include <bluetooth.h>

#include "log.h"

namespace {

Uuid GetUuid(const bt_gatt_h &gatt_handle) {
  char *uuid = nullptr;
  int ret = bt_gatt_get_uuid(gatt_handle, &uuid);
  if (ret != BT_ERROR_NONE) {
    LOG_ERROR("Failed to retrieve a UUID: %s", get_error_message(ret));
    return std::string();
  }
  std::string result = std::string(uuid);
  free(uuid);
  return result;
}

void AddDiscoveredService(bt_gatt_h service_handle,
                          std::vector<DiscoveredService> *discovered_services) {
  DiscoveredService discovered_service = {};
  discovered_service.service_id = GetUuid(service_handle);

  bt_gatt_service_foreach_included_services(
      service_handle,
      [](int total, int index, bt_gatt_h included_service_handle,
         void *user_data) -> bool {
        auto *discovered_service = static_cast<DiscoveredService *>(user_data);
        AddDiscoveredService(included_service_handle,
                             &discovered_service->included_services);
        return true;
      },
      &discovered_service);
  bt_gatt_service_foreach_characteristics(
      service_handle,
      [](int total, int index, bt_gatt_h characteristic_handle,
         void *user_data) -> bool {
        int properties = 0;
        int ret = bt_gatt_characteristic_get_properties(characteristic_handle,
                                                        &properties);
        if (ret != BT_ERROR_NONE) {
          LOG_ERROR("Unable to get characteristic properties: %s",
                    get_error_message(ret));
          return true;
        }

        DiscoveredCharacteristic characteristic = {};
        characteristic.characteristic_id = GetUuid(characteristic_handle);
        characteristic.is_readable = properties & BT_GATT_PROPERTY_READ;
        characteristic.is_writable_with_response =
            properties & BT_GATT_PROPERTY_WRITE;
        characteristic.is_writable_without_response =
            properties & BT_GATT_PROPERTY_WRITE_WITHOUT_RESPONSE;
        characteristic.is_notifiable = properties & BT_GATT_PROPERTY_NOTIFY;
        characteristic.is_indicatable = properties & BT_GATT_PROPERTY_INDICATE;

        auto *discovered_service = static_cast<DiscoveredService *>(user_data);
        characteristic.service_id = discovered_service->service_id;
        discovered_service->characteristic_ids.emplace_back(
            characteristic.characteristic_id);
        discovered_service->characteristics.emplace_back(characteristic);
        return true;
      },
      &discovered_service);

  discovered_services->emplace_back(discovered_service);
}

}  // namespace

BleDevice::BleDevice(const std::string &device_id) : device_id_(device_id) {
  last_error_ = bt_gatt_client_create(device_id.c_str(), &handle_);
  if (last_error_ != BT_ERROR_NONE) {
    LOG_ERROR("Failed to create a client handle: %s",
              get_error_message(last_error_));
    return;
  }
}

BleDevice::~BleDevice() {
  if (handle_) {
    bt_gatt_client_destroy(handle_);
  }
}

std::vector<DiscoveredService> BleDevice::DiscoverServices() {
  std::vector<DiscoveredService> discovered_services;
  last_error_ = bt_gatt_client_foreach_services(
      handle_,
      [](int total, int index, bt_gatt_h service_handle, void *user_data) {
        auto *discovered_services =
            static_cast<std::vector<DiscoveredService> *>(user_data);
        AddDiscoveredService(service_handle, discovered_services);
        return true;
      },
      &discovered_services);
  if (last_error_ != BT_ERROR_NONE) {
    LOG_ERROR("Unable to look up services: %s", get_error_message(last_error_));
  }
  return discovered_services;
}

bool BleDevice::NegotiateMtuSize(int32_t request_mtu,
                                 MtuNegotiationCallback on_done,
                                 ErrorCallback on_error) {
  struct UserData {
    MtuNegotiationCallback on_done;
    ErrorCallback on_error;
  } *params = new UserData;
  params->on_done = std::move(on_done);
  params->on_error = std::move(on_error);

  // Unset any existing callback just before setting a new callback, because
  // calling the unset function inside the callback will result in an internal
  // memory corruption.
  bt_gatt_client_unset_att_mtu_changed_cb(handle_);

  last_error_ = bt_gatt_client_set_att_mtu_changed_cb(
      handle_,
      [](bt_gatt_client_h client, const bt_gatt_client_att_mtu_info_s *mtu_info,
         void *user_data) {
        UserData *params = static_cast<UserData *>(user_data);
        if (mtu_info->status == 0) {
          params->on_done(mtu_info->mtu);
        } else {
          // TODO(swift-kim): What does the status value mean?
          params->on_error(mtu_info->status, "Failed to update the MTU value.");
        }
        delete params;
      },
      params);
  if (last_error_ != BT_ERROR_NONE) {
    LOG_ERROR("Could not set an MTU change callback: %s",
              get_error_message(last_error_));
    delete params;
    return false;
  }

  last_error_ = bt_gatt_client_request_att_mtu_change(handle_, request_mtu);
  if (last_error_ != BT_ERROR_NONE) {
    LOG_ERROR("Could not request an MTU change: %s",
              get_error_message(last_error_));
    delete params;
    return false;
  }
  return true;
}

bool BleDevice::ReadCharacteristic(
    const QualifiedCharacteristic &characteristic, VoidCallback on_done,
    ErrorCallback on_error) {
  struct UserData {
    BleDevice *self;
    VoidCallback on_done;
    ErrorCallback on_error;
  } *params = new UserData;
  params->self = this;
  params->on_done = std::move(on_done);
  params->on_error = std::move(on_error);

  last_error_ = bt_gatt_client_read_value(
      characteristic.handle(),
      [](int result, bt_gatt_h request_handle, void *user_data) {
        UserData *params = static_cast<UserData *>(user_data);
        if (result != BT_ERROR_NONE) {
          LOG_ERROR("The read operation resulted in an error: %s",
                    get_error_message(result));
          if (params->on_error) {
            params->on_error(result, get_error_message(result));
          }
          delete params;
          return;
        }

        std::shared_ptr<QualifiedCharacteristic> characteristic =
            params->self->FindCharacteristic(request_handle);
        if (!characteristic) {
          LOG_ERROR("The characteristic is unexpectedly not found.");
          if (params->on_error) {
            params->on_error(BT_ERROR_NONE, "Something went wrong.");
          }
          delete params;
          return;
        }

        char *value = nullptr;
        int length = 0;
        int ret = bt_gatt_get_value(request_handle, &value, &length);
        if (ret != BT_ERROR_NONE) {
          LOG_ERROR("Failed to retrieve a value: %s", get_error_message(ret));
          if (params->on_error) {
            params->on_error(ret, get_error_message(ret));
          }
          delete params;
          return;
        }
        std::vector<uint8_t> bytes(value, value + length);
        free(value);

        // The done callback must be called before the notification is sent.
        if (params->on_done) {
          params->on_done();
        }
        if (params->self->notification_callback_) {
          params->self->notification_callback_(*characteristic, bytes);
        }
        delete params;
      },
      params);
  if (last_error_ != BT_ERROR_NONE) {
    LOG_ERROR("Could not read a value from the characteristic: %s",
              get_error_message(last_error_));
    return false;
  }
  return true;
}

bool BleDevice::WriteCharacteristic(
    const QualifiedCharacteristic &characteristic,
    const std::vector<uint8_t> &value, VoidCallback on_done,
    ErrorCallback on_error) {
  last_error_ = bt_gatt_set_value(characteristic.handle(),
                                  reinterpret_cast<const char *>(value.data()),
                                  value.size());
  if (last_error_ != BT_ERROR_NONE) {
    LOG_ERROR("Unable to set a value to the characteristic: %s",
              get_error_message(last_error_));
    return false;
  }

  struct UserData {
    VoidCallback on_done;
    ErrorCallback on_error;
  } *params = new UserData;
  params->on_done = std::move(on_done);
  params->on_error = std::move(on_error);

  last_error_ = bt_gatt_client_write_value(
      characteristic.handle(),
      [](int result, bt_gatt_h request_handle, void *user_data) {
        UserData *params = static_cast<UserData *>(user_data);
        if (result != BT_ERROR_NONE) {
          LOG_ERROR("The write operation resulted in an error: %s",
                    get_error_message(result));
          if (params->on_error) {
            params->on_error(result, get_error_message(result));
          }
        } else if (params->on_done) {
          params->on_done();
        }
        delete params;
      },
      params);
  if (last_error_ != BT_ERROR_NONE) {
    LOG_ERROR("Could not write to the characteristic: %s",
              get_error_message(last_error_));
    delete params;
    return false;
  }

  return true;
}

bool BleDevice::ListenNotifications(
    const QualifiedCharacteristic &characteristic) {
  last_error_ = bt_gatt_client_set_characteristic_value_changed_cb(
      characteristic.handle(),
      [](bt_gatt_h handle, char *value, int len, void *user_data) {
        BleDevice *self = static_cast<BleDevice *>(user_data);
        std::shared_ptr<QualifiedCharacteristic> characteristic =
            self->FindCharacteristic(handle);
        if (self->notification_callback_ && characteristic) {
          std::vector<uint8_t> bytes(value, value + len);
          self->notification_callback_(*characteristic, bytes);
        }
      },
      this);
  if (last_error_ != BT_ERROR_NONE) {
    return false;
  }
  return true;
}

bool BleDevice::StopNotifications(
    const QualifiedCharacteristic &characteristic) {
  last_error_ = bt_gatt_client_unset_characteristic_value_changed_cb(
      characteristic.handle());
  if (last_error_ != BT_ERROR_NONE) {
    return false;
  }
  return true;
}
