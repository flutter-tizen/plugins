// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_reactive_ble_tizen_plugin.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "ble_tizen.h"
#include "qualified_characteristic.h"

namespace {

typedef flutter::EventChannel<flutter::EncodableValue> FlEventChannel;
typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;
typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;
typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;
typedef flutter::StreamHandler<flutter::EncodableValue> FlStreamHandler;
typedef flutter::StreamHandlerError<flutter::EncodableValue>
    FlStreamHandlerError;

// String key constants for method call arguments and results.
// Keep these in sync with those in `models.dart`.
constexpr char kDeviceId[] = "device_id";
constexpr char kDeviceName[] = "device_name";
constexpr char kServiceIds[] = "service_ids";
constexpr char kServiceData[] = "service_data";
constexpr char kManufacturerData[] = "manufacturer_data";
constexpr char kRssi[] = "rssi";
constexpr char kConnectionState[] = "connection_state";
constexpr char kCharacteristicId[] = "characteristic_id";
constexpr char kServiceId[] = "service_id";
constexpr char kIsReadable[] = "is_readable";
constexpr char kIsWritableWithResponse[] = "is_writable_with_response";
constexpr char kIsWritableWithoutResponse[] = "is_writable_without_response";
constexpr char kIsNotifiable[] = "is_notifiable";
constexpr char kIsIndicatable[] = "is_indicatable";
constexpr char kIncludedServices[] = "included_services";
constexpr char kCharacteristicIds[] = "characteristic_ids";
constexpr char kCharacteristics[] = "characteristics";
constexpr char kQualifiedCharacteristic[] = "qualified_characteristic";
constexpr char kResult[] = "result";
constexpr char kMtu[] = "mtu";
constexpr char kValue[] = "value";

std::string MissingArgumentError(const char *name) {
  std::stringstream stream;
  stream << "Argument " << name << " not provided, or not of expected type.";
  return stream.str();
}

template <typename T>
bool GetValueFromEncodableMap(const flutter::EncodableMap *map, const char *key,
                              T &out) {
  auto iter = map->find(flutter::EncodableValue(key));
  if (iter != map->end() && !iter->second.IsNull()) {
    if (auto *value = std::get_if<T>(&iter->second)) {
      out = *value;
      return true;
    }
  }
  return false;
}

// Packs a DiscoveredService into an EncodableValue recursively.
flutter::EncodableValue ServiceToEncodableValue(
    const DiscoveredService &service) {
  flutter::EncodableList characteristic_ids;
  for (const Uuid &characteristic_id : service.characteristic_ids) {
    characteristic_ids.emplace_back(flutter::EncodableValue(characteristic_id));
  }
  flutter::EncodableList included_services;
  for (const DiscoveredService &included_service : service.included_services) {
    included_services.emplace_back(ServiceToEncodableValue(included_service));
  }
  flutter::EncodableList characteristics;
  for (const DiscoveredCharacteristic &characteristic :
       service.characteristics) {
    flutter::EncodableMap map = {
        {flutter::EncodableValue(kCharacteristicId),
         flutter::EncodableValue(characteristic.characteristic_id)},
        {flutter::EncodableValue(kServiceId),
         flutter::EncodableValue(characteristic.service_id)},
        {flutter::EncodableValue(kIsReadable),
         flutter::EncodableValue(characteristic.is_readable)},
        {flutter::EncodableValue(kIsWritableWithResponse),
         flutter::EncodableValue(characteristic.is_writable_with_response)},
        {flutter::EncodableValue(kIsWritableWithoutResponse),
         flutter::EncodableValue(characteristic.is_writable_without_response)},
        {flutter::EncodableValue(kIsNotifiable),
         flutter::EncodableValue(characteristic.is_notifiable)},
        {flutter::EncodableValue(kIsIndicatable),
         flutter::EncodableValue(characteristic.is_indicatable)},
    };
    characteristics.emplace_back(flutter::EncodableValue(map));
  }
  flutter::EncodableMap map = {
      {flutter::EncodableValue(kServiceId),
       flutter::EncodableValue(service.service_id)},
      {flutter::EncodableValue(kCharacteristicIds),
       flutter::EncodableValue(characteristic_ids)},
      {flutter::EncodableValue(kIncludedServices),
       flutter::EncodableValue(included_services)},
      {flutter::EncodableValue(kCharacteristics),
       flutter::EncodableValue(characteristics)},
  };
  return flutter::EncodableValue(map);
}

// A callback shared by subscribeToNotifications and readCharacteristic.
NotificationCallback notification_callback_;

class BleCharUpdateStreamHandler : public FlStreamHandler {
 public:
  explicit BleCharUpdateStreamHandler(std::shared_ptr<BleTizen> ble_tizen)
      : ble_tizen_(ble_tizen){};

 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    events_ = std::move(events);
    notification_callback_ = [this](
                                 const QualifiedCharacteristic &characteristic,
                                 const std::vector<uint8_t> &data) {
      flutter::EncodableMap characteristic_map = {
          {flutter::EncodableValue(kDeviceId),
           flutter::EncodableValue(characteristic.device_id())},
          {flutter::EncodableValue(kServiceId),
           flutter::EncodableValue(characteristic.service_id())},
          {flutter::EncodableValue(kCharacteristicId),
           flutter::EncodableValue(characteristic.characteristic_id())},
      };
      flutter::EncodableMap map = {
          {flutter::EncodableValue(kQualifiedCharacteristic),
           flutter::EncodableValue(characteristic_map)},
          {flutter::EncodableValue(kResult),
           flutter::EncodableValue(flutter::EncodableValue(data))},
      };
      events_->Success(flutter::EncodableValue(map));
    };
    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    notification_callback_ = nullptr;
    events_.reset();
    return nullptr;
  }

 private:
  std::shared_ptr<BleTizen> ble_tizen_;
  std::unique_ptr<FlEventSink> events_;
};

class ConnectionUpdateStreamHandler : public FlStreamHandler {
 public:
  explicit ConnectionUpdateStreamHandler(std::shared_ptr<BleTizen> ble_tizen)
      : ble_tizen_(ble_tizen){};

 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    events_ = std::move(events);
    ble_tizen_->SetConnectionStateChangeCallback(
        [this](const std::string &device_id, ConnectionState state) {
          std::string connection_state = "disconnected";
          if (state == ConnectionState::kConnecting) {
            connection_state = "connecting";
          } else if (state == ConnectionState::kConnected) {
            connection_state = "connected";
          } else if (state == ConnectionState::kDisconnecting) {
            connection_state = "disconnecting";
          }
          flutter::EncodableMap map = {
              {flutter::EncodableValue(kDeviceId),
               flutter::EncodableValue(device_id)},
              {flutter::EncodableValue(kConnectionState),
               flutter::EncodableValue(connection_state)},
          };
          events_->Success(flutter::EncodableValue(map));
        },
        [this](int error_code, const std::string &error_message) {
          events_->Error(std::to_string(error_code), error_message);
        });
    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    ble_tizen_->UnsetConnectionStateChangeCallback();
    events_.reset();
    return nullptr;
  }

 private:
  std::shared_ptr<BleTizen> ble_tizen_;
  std::unique_ptr<FlEventSink> events_;
};

class BleScanStreamHandler : public FlStreamHandler {
 public:
  explicit BleScanStreamHandler(std::shared_ptr<BleTizen> ble_tizen)
      : ble_tizen_(ble_tizen){};

 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    events_ = std::move(events);
    ble_tizen_->SetDeviceScanCallback(
        [this](const DiscoveredDevice &device) {
          flutter::EncodableList service_ids;
          for (const Uuid &service_id : device.service_ids) {
            service_ids.emplace_back(flutter::EncodableValue(service_id));
          }
          flutter::EncodableMap service_data;
          for (const auto &[service_id, data] : device.service_data) {
            service_data[flutter::EncodableValue(service_id)] =
                flutter::EncodableValue(data);
          }
          flutter::EncodableMap map = {
              {flutter::EncodableValue(kDeviceId),
               flutter::EncodableValue(device.device_id)},
              {flutter::EncodableValue(kDeviceName),
               flutter::EncodableValue(device.name)},
              {flutter::EncodableValue(kManufacturerData),
               flutter::EncodableValue(device.manufacturer_data)},
              {flutter::EncodableValue(kServiceIds),
               flutter::EncodableValue(service_ids)},
              {flutter::EncodableValue(kServiceData),
               flutter::EncodableValue(service_data)},
              {flutter::EncodableValue(kRssi),
               flutter::EncodableValue(device.rssi)},
          };
          events_->Success(flutter::EncodableValue(map));
        },
        [this](int error_code, const std::string &error_message) {
          events_->Error(std::to_string(error_code), error_message);
        });
    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    ble_tizen_->SetDeviceScanCallback(nullptr, nullptr);
    ble_tizen_->StopScan();
    events_.reset();
    return nullptr;
  }

 private:
  std::shared_ptr<BleTizen> ble_tizen_;
  std::unique_ptr<FlEventSink> events_;
};

class BleStatusStreamHandler : public FlStreamHandler {
 public:
  explicit BleStatusStreamHandler(std::shared_ptr<BleTizen> ble_tizen)
      : ble_tizen_(ble_tizen){};

 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    events_ = std::move(events);

    BleStatus status = ble_tizen_->GetBleStatus();
    std::string ble_status = "unknown";
    if (status == BleStatus::kEnabled) {
      ble_status = "ready";
    } else if (status == BleStatus::kDisabled) {
      ble_status = "poweredOff";
    } else if (status == BleStatus::kNotSupported) {
      ble_status = "unsupported";
    }
    events_->Success(flutter::EncodableValue(ble_status));

    ble_tizen_->SetBleStatusChangeCallback([this](BleStatus status) {
      std::string ble_status =
          status == BleStatus::kEnabled ? "ready" : "poweredOff";
      events_->Success(flutter::EncodableValue(ble_status));
    });
    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    ble_tizen_->UnsetBleStatusChangeCallback();
    events_.reset();
    return nullptr;
  }

 private:
  std::shared_ptr<BleTizen> ble_tizen_;
  std::unique_ptr<FlEventSink> events_;
};

class FlutterReactiveBleTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<FlutterReactiveBleTizenPlugin>();

    auto method_channel = std::make_unique<FlMethodChannel>(
        registrar->messenger(), "flutter_reactive_ble_method",
        &flutter::StandardMethodCodec::GetInstance());
    method_channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    plugin->SetUpEventChannels(registrar);

    registrar->AddPlugin(std::move(plugin));
  }

  FlutterReactiveBleTizenPlugin() : ble_tizen_(std::make_shared<BleTizen>()) {}

  virtual ~FlutterReactiveBleTizenPlugin() {}

 private:
  void SetUpEventChannels(flutter::PluginRegistrar *registrar) {
    ble_status_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "flutter_reactive_ble_status",
        &flutter::StandardMethodCodec::GetInstance());
    ble_status_channel_->SetStreamHandler(
        std::make_unique<BleStatusStreamHandler>(ble_tizen_));

    scan_event_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "flutter_reactive_ble_scan",
        &flutter::StandardMethodCodec::GetInstance());
    scan_event_channel_->SetStreamHandler(
        std::make_unique<BleScanStreamHandler>(ble_tizen_));

    connected_device_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "flutter_reactive_ble_connected_device",
        &flutter::StandardMethodCodec::GetInstance());
    connected_device_channel_->SetStreamHandler(
        std::make_unique<ConnectionUpdateStreamHandler>(ble_tizen_));

    char_event_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "flutter_reactive_ble_char_update",
        &flutter::StandardMethodCodec::GetInstance());
    char_event_channel_->SetStreamHandler(
        std::make_unique<BleCharUpdateStreamHandler>(ble_tizen_));
  }

  void HandleMethodCall(const FlMethodCall &method_call,
                        std::unique_ptr<FlMethodResult> result) {
    const auto &method_name = method_call.method_name();

    if (method_name == "initialize") {
      if (!ble_tizen_->Initialize()) {
        result->Error(std::to_string(ble_tizen_->GetLastError()),
                      ble_tizen_->GetLastErrorString());
        return;
      };
      result->Success();
    } else if (method_name == "deinitialize") {
      if (!ble_tizen_->Deinitialize()) {
        result->Error(std::to_string(ble_tizen_->GetLastError()),
                      ble_tizen_->GetLastErrorString());
        return;
      };
      result->Success();
    } else if (method_name == "scanForDevices") {
      const auto *arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      if (!arguments) {
        result->Error("Invalid arguments", "No arguments provided.");
        return;
      }

      flutter::EncodableList service_id_list;
      if (!GetValueFromEncodableMap(arguments, kServiceIds, service_id_list)) {
        result->Error("Invalid arguments", MissingArgumentError(kServiceIds));
        return;
      }
      std::vector<Uuid> service_ids;
      for (const flutter::EncodableValue &service_id : service_id_list) {
        if (std::holds_alternative<std::string>(service_id)) {
          service_ids.emplace_back(std::get<std::string>(service_id));
        }
      }

      if (!ble_tizen_->Scan(service_ids)) {
        result->Error(std::to_string(ble_tizen_->GetLastError()),
                      ble_tizen_->GetLastErrorString());
        return;
      };
      result->Success();
    } else if (method_name == "connectToDevice" ||
               method_name == "disconnectDevice" ||
               method_name == "discoverServices" ||
               method_name == "requestMtuSize") {
      HandleDeviceOperation(method_call, std::move(result));
    } else if (method_name == "readCharacteristic" ||
               method_name == "writeCharacteristicWithResponse" ||
               method_name == "writeCharacteristicWithoutResponse" ||
               method_name == "subscribeToNotifications" ||
               method_name == "stopSubscribingToNotifications") {
      HandleCharacteristicOperation(method_call, std::move(result));
    } else {
      result->NotImplemented();
    }
  }

  void HandleDeviceOperation(const FlMethodCall &method_call,
                             std::unique_ptr<FlMethodResult> result) {
    const auto &method_name = method_call.method_name();
    const auto *arguments =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("Invalid arguments", "No arguments provided.");
      return;
    }

    std::string device_id;
    if (!GetValueFromEncodableMap(arguments, kDeviceId, device_id)) {
      result->Error("Invalid arguments", MissingArgumentError(kDeviceId));
      return;
    }

    if (method_name == "connectToDevice") {
      if (!ble_tizen_->ConnectToDevice(device_id)) {
        result->Error(std::to_string(ble_tizen_->GetLastError()),
                      ble_tizen_->GetLastErrorString());
        return;
      };
      result->Success();
    } else if (method_name == "disconnectDevice") {
      if (!ble_tizen_->DisconnectFromDevice(device_id)) {
        result->Error(std::to_string(ble_tizen_->GetLastError()),
                      ble_tizen_->GetLastErrorString());
        return;
      };
      result->Success();
    } else if (method_name == "discoverServices") {
      std::shared_ptr<BleDevice> device = ble_tizen_->FindDeviceById(device_id);
      if (!device) {
        result->Error("Operation failed", "Failed to register a device.");
        return;
      }

      flutter::EncodableList services;
      for (const DiscoveredService &service : device->DiscoverServices()) {
        services.emplace_back(ServiceToEncodableValue(service));
      }
      result->Success(flutter::EncodableValue(services));
    } else if (method_name == "requestMtuSize") {
      std::shared_ptr<BleDevice> device = ble_tizen_->FindDeviceById(device_id);
      if (!device) {
        result->Error("Operation failed", "Failed to register a device.");
        return;
      }

      int32_t request_mtu;
      if (!GetValueFromEncodableMap(arguments, kMtu, request_mtu)) {
        result->Error("Invalid arguments", MissingArgumentError(kMtu));
        return;
      }
      if (request_mtu < 23) {
        result->Error("Invalid arguments", "The MTU cannot be less than 23.");
        return;
      }

      FlMethodResult *result_ptr = result.release();
      if (!device->NegotiateMtuSize(
              request_mtu,
              [result_ptr](int32_t result_mtu) {
                result_ptr->Success(flutter::EncodableValue(result_mtu));
                delete result_ptr;
              },
              [result_ptr](int error_code, const std::string &error_message) {
                result_ptr->Error(std::to_string(error_code), error_message);
                delete result_ptr;
              })) {
        result_ptr->Error(std::to_string(device->GetLastError()),
                          device->GetLastErrorString());
        delete result_ptr;
      }
    }
  }

  void HandleCharacteristicOperation(const FlMethodCall &method_call,
                                     std::unique_ptr<FlMethodResult> result) {
    const auto &method_name = method_call.method_name();
    const auto *arguments =
        std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("Invalid arguments", "No arguments provided.");
      return;
    }

    flutter::EncodableMap qualified;
    if (!GetValueFromEncodableMap(arguments, kQualifiedCharacteristic,
                                  qualified)) {
      result->Error("Invalid arguments",
                    MissingArgumentError(kQualifiedCharacteristic));
      return;
    }
    std::string device_id;
    if (!GetValueFromEncodableMap(&qualified, kDeviceId, device_id)) {
      result->Error("Invalid arguments", MissingArgumentError(kDeviceId));
      return;
    }
    std::string service_id;
    if (!GetValueFromEncodableMap(&qualified, kServiceId, service_id)) {
      result->Error("Invalid arguments", MissingArgumentError(kServiceId));
      return;
    }
    std::string characteristic_id;
    if (!GetValueFromEncodableMap(&qualified, kCharacteristicId,
                                  characteristic_id)) {
      result->Error("Invalid arguments",
                    MissingArgumentError(kCharacteristicId));
      return;
    }

    std::shared_ptr<BleDevice> device = ble_tizen_->FindDeviceById(device_id);
    std::shared_ptr<QualifiedCharacteristic> characteristic =
        device->FindCharacteristicById(service_id, characteristic_id);
    if (!characteristic) {
      result->Error("Operation failed", "Failed to register a characteristic.");
      return;
    }

    if (method_name == "readCharacteristic") {
      if (!characteristic->IsReadable()) {
        result->Error("Operation failed",
                      "Operation not allowed on this characteristic.");
        return;
      }
      device->SetNotificationCallback(notification_callback_);

      FlMethodResult *result_ptr = result.release();
      if (!device->ReadCharacteristic(
              *characteristic,
              [result_ptr]() {
                result_ptr->Success();
                delete result_ptr;
              },
              [result_ptr](int error_code, const std::string &error_message) {
                result_ptr->Error(std::to_string(error_code), error_message);
                delete result_ptr;
              })) {
        result_ptr->Error(std::to_string(device->GetLastError()),
                          device->GetLastErrorString());
        delete result_ptr;
      }
    } else if (method_name == "writeCharacteristicWithResponse" ||
               method_name == "writeCharacteristicWithoutResponse") {
      bool no_response = method_name == "writeCharacteristicWithoutResponse";
      if ((no_response && !characteristic->IsWritableWithoutResponse()) ||
          (!no_response && !characteristic->IsWritable())) {
        result->Error("Operation failed",
                      "Operation not allowed on this characteristic.");
        return;
      }
      characteristic->SetWriteType(no_response);

      std::vector<uint8_t> value;
      if (!GetValueFromEncodableMap(arguments, kValue, value)) {
        result->Error("Invalid arguments", MissingArgumentError(kValue));
        return;
      }

      FlMethodResult *result_ptr = result.release();
      if (!device->WriteCharacteristic(
              *characteristic, value,
              [result_ptr, qualified]() {
                flutter::EncodableMap map = {
                    {flutter::EncodableValue(kQualifiedCharacteristic),
                     flutter::EncodableValue(qualified)},
                };
                result_ptr->Success(flutter::EncodableValue(map));
                delete result_ptr;
              },
              [result_ptr](int error_code, const std::string &error_message) {
                result_ptr->Error(std::to_string(error_code), error_message);
                delete result_ptr;
              })) {
        result_ptr->Error(std::to_string(device->GetLastError()),
                          device->GetLastErrorString());
        delete result_ptr;
      }
    } else if (method_name == "subscribeToNotifications") {
      device->SetNotificationCallback(notification_callback_);
      if (!device->ListenNotifications(*characteristic)) {
        result->Error(std::to_string(device->GetLastError()),
                      device->GetLastErrorString());
        return;
      }
      result->Success();
    } else if (method_name == "stopSubscribingToNotifications") {
      device->SetNotificationCallback(nullptr);
      if (!device->StopNotifications(*characteristic)) {
        result->Error(std::to_string(device->GetLastError()),
                      device->GetLastErrorString());
        return;
      }
      result->Success();
    }
  }

  std::shared_ptr<BleTizen> ble_tizen_;
  std::unique_ptr<FlEventChannel> ble_status_channel_;
  std::unique_ptr<FlEventChannel> scan_event_channel_;
  std::unique_ptr<FlEventChannel> connected_device_channel_;
  std::unique_ptr<FlEventChannel> char_event_channel_;
};

}  // namespace

void FlutterReactiveBleTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterReactiveBleTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
