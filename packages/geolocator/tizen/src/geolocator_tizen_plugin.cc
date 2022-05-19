// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "geolocator_tizen_plugin.h"

#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include "app_settings_manager.h"
#include "location_manager.h"
#include "log.h"
#include "permission_manager.h"

namespace {

typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;
typedef flutter::EventChannel<flutter::EncodableValue> FlEventChannel;
typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;

constexpr char kPrivilegeLocation[] = "http://tizen.org/privilege/location";

class GeolocatorTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel = std::make_unique<FlMethodChannel>(
        registrar->messenger(), "flutter.baseflow.com/geolocator",
        &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<GeolocatorTizenPlugin>();

    plugin->SetupGeolocatorServiceUpdatesChannel(registrar->messenger());
    plugin->SetupGeolocatorUpdatesChannel(registrar->messenger());

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  GeolocatorTizenPlugin()
      : permission_manager_(std::make_unique<PermissionManager>()),
        location_manager_(std::make_unique<LocationManager>()) {}

  virtual ~GeolocatorTizenPlugin() {}

 private:
  void SetupGeolocatorServiceUpdatesChannel(
      flutter::BinaryMessenger *messenger) {
    service_updates_channel_ = std::make_unique<FlEventChannel>(
        messenger, "flutter.baseflow.com/geolocator_service_updates",
        &flutter::StandardMethodCodec::GetInstance());
    auto handler = std::make_unique<flutter::StreamHandlerFunctions<>>(
        [this](const flutter::EncodableValue *arguments,
               std::unique_ptr<flutter::EventSink<>> &&events)
            -> std::unique_ptr<flutter::StreamHandlerError<>> {
          OnListenGeolocatorServiceUpdates(std::move(events));
          return nullptr;
        },
        [this](const flutter::EncodableValue *arguments)
            -> std::unique_ptr<flutter::StreamHandlerError<>> {
          OnCancelGeolocatorServiceUpdates();
          return nullptr;
        });
    service_updates_channel_->SetStreamHandler(std::move(handler));
  }

  void SetupGeolocatorUpdatesChannel(flutter::BinaryMessenger *messenger) {
    updates_channel_ = std::make_unique<FlEventChannel>(
        messenger, "flutter.baseflow.com/geolocator_updates",
        &flutter::StandardMethodCodec::GetInstance());
    auto handler = std::make_unique<flutter::StreamHandlerFunctions<>>(
        [this](const flutter::EncodableValue *arguments,
               std::unique_ptr<flutter::EventSink<>> &&events)
            -> std::unique_ptr<flutter::StreamHandlerError<>> {
          OnListenGeolocatorUpdates(std::move(events));
          return nullptr;
        },
        [this](const flutter::EncodableValue *arguments)
            -> std::unique_ptr<flutter::StreamHandlerError<>> {
          OnCancelGeolocatorUpdates();
          return nullptr;
        });
    updates_channel_->SetStreamHandler(std::move(handler));
  }

  void HandleMethodCall(const FlMethodCall &method_call,
                        std::unique_ptr<FlMethodResult> result) {
    std::string method_name = method_call.method_name();

    result_ = std::move(result);

    if (method_name == "checkPermission") {
      OnCheckPermission();
    } else if (method_name == "isLocationServiceEnabled") {
      OnIsLocationServiceEnabled();
    } else if (method_name == "requestPermission") {
      OnRequestPermission();
    } else if (method_name == "getLastKnownPosition") {
      OnGetLastKnownPosition();
    } else if (method_name == "getCurrentPosition") {
      OnGetCurrentPosition();
    } else if (method_name == "openAppSettings") {
      bool ret = app_settings_manager_->OpenAppSettings();
      result->Success(flutter::EncodableValue(ret));
    } else if (method_name == "openLocationSettings") {
      bool ret = app_settings_manager_->OpenLocationSetting();
      result->Success(flutter::EncodableValue(ret));
    } else {
      result->NotImplemented();
    }
  }

  void OnCheckPermission() {
    PermissionStatus result =
        permission_manager_->CheckPermission(kPrivilegeLocation);
    if (result == PermissionStatus::kError) {
      SendErrorResult("Operation failed", "Failed to request permission.");
      return;
    }
    LOG_INFO("permission_status is %d", result);
    SendResult(flutter::EncodableValue(static_cast<int>(result)));
  }

  void OnIsLocationServiceEnabled() {
    bool is_enabled = false;
    TizenResult ret = location_manager_->IsLocationServiceEnabled(&is_enabled);

    // TODO : add location service listener
    if (!ret) {
      SendErrorResult("Operation failed", ret.message());
    }
    SendResult(flutter::EncodableValue(is_enabled));
  }

  void OnRequestPermission() {
    PermissionStatus result =
        permission_manager_->RequestPermssion(kPrivilegeLocation);
    if (result == PermissionStatus::kDeniedForever ||
        result == PermissionStatus::kDenied) {
      SendErrorResult("Permission denied", "Permission denied by user.");
      return;
    } else if (result == PermissionStatus::kError) {
      SendErrorResult("Operation failed", "Failed to request permission.");
      return;
    }
    SendResult(flutter::EncodableValue(static_cast<int>(result)));
  }

  void OnGetLastKnownPosition() {
    Location location;
    TizenResult tizen_result =
        location_manager_->GetLastKnownLocation(&location);
    if (!tizen_result) {
      SendErrorResult("Operation failed", tizen_result.message());
      return;
    }
    SendResult(location.ToEncodableValue());
  }

  void OnGetCurrentPosition() {
    auto result_ptr = result_.get();
    TizenResult tizen_result = location_manager_->RequestCurrentLocationOnce(
        [result_ptr](Location location) {
          result_ptr->Success(location.ToEncodableValue());
        },
        [result_ptr](TizenResult error) {
          result_ptr->Error("Operation failed", error.message());
        });

    if (!tizen_result) {
      SendErrorResult("Operation failed", tizen_result.message());
    }
  }

  void OnListenGeolocatorServiceUpdates(
      std::unique_ptr<FlEventSink> &&event_sink) {
    service_updates_event_sink_ = std::move(event_sink);
    TizenResult tizen_result = location_manager_->SetOnServiceStateChanged(
        [this](ServiceState service_state) {
          flutter::EncodableValue value(static_cast<int>(service_state));
          service_updates_event_sink_->Success(value);
        });
    if (!tizen_result) {
      LOG_ERROR("Failed to set OnServiceStateChanged, %s.",
                tizen_result.message().c_str());
      service_updates_event_sink_ = nullptr;
    }
  }

  void OnCancelGeolocatorServiceUpdates() {
    service_updates_event_sink_ = nullptr;
    location_manager_->UnsetOnServiceStateChanged();
  }

  void OnListenGeolocatorUpdates(std::unique_ptr<FlEventSink> &&event_sink) {
    updates_event_sink_ = std::move(event_sink);
    TizenResult tizen_result =
        location_manager_->SetOnLocationUpdated([this](Location location) {
          updates_event_sink_->Success(location.ToEncodableValue());
        });
    if (!tizen_result) {
      LOG_ERROR("Failed to set OnLocationUpdated, %s.",
                tizen_result.message().c_str());
      updates_event_sink_ = nullptr;
    }
  }

  void OnCancelGeolocatorUpdates() {
    updates_event_sink_ = nullptr;
    location_manager_->UnsetOnLocationUpdated();
  }

  void SendResult(const flutter::EncodableValue &result) {
    if (!result_) {
      return;
    }
    result_->Success(result);
    result_ = nullptr;
  }

  void SendErrorResult(const std::string &error_code,
                       const std::string &error_message) {
    if (!result_) {
      return;
    }
    result_->Error(error_code, error_message);
    result_ = nullptr;
  }

  std::unique_ptr<FlMethodResult> result_;
  std::unique_ptr<PermissionManager> permission_manager_;
  std::unique_ptr<LocationManager> location_manager_;
  std::unique_ptr<AppSettingsManager> app_settings_manager_;
  std::unique_ptr<FlEventChannel> service_updates_channel_;
  std::unique_ptr<FlEventSink> service_updates_event_sink_;
  std::unique_ptr<FlEventChannel> updates_channel_;
  std::unique_ptr<FlEventSink> updates_event_sink_;
};

}  // namespace

void GeolocatorTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  GeolocatorTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
