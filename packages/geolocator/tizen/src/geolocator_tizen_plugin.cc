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

#include "location_manager.h"
#include "log.h"
#include "permission_manager.h"
#include "setting.h"

namespace {

typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;
typedef flutter::EventChannel<flutter::EncodableValue> FlEventChannel;
typedef flutter::MethodCall<flutter::EncodableValue> FlMethodCall;
typedef flutter::EventSink<flutter::EncodableValue> FlEventSink;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;

class GeolocatorTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<FlMethodChannel>(
            registrar->messenger(), "flutter.baseflow.com/geolocator",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<GeolocatorTizenPlugin>();

    plugin->SetupGeolocatorServiceUpdatesChannel(registrar->messenger());
    plugin->SetupGeolocatorUpdatesChannel(registrar->messenger());

    channel->SetMethodCallHandler(
        [plugin_ptr = plugin.get()](const auto &call, auto result) {
          plugin_ptr->HandleMethodCall(call, std::move(result));
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
    geolocator_service_updates_channel_ =
        std::make_unique<FlEventChannel>(
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
    geolocator_service_updates_channel_->SetStreamHandler(std::move(handler));
  }

  void SetupGeolocatorUpdatesChannel(flutter::BinaryMessenger *messenger) {
    geolocator_updates_channel_ =
        std::make_unique<FlEventChannel>(
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
    geolocator_updates_channel_->SetStreamHandler(std::move(handler));
  }


  void HandleMethodCall(
      const FlMethodCall &method_call,
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
      TizenResult ret = Setting::LaunchAppSetting();
      SendResult(flutter::EncodableValue(static_cast<bool>(ret)));
    } else if (method_name == "openLocationSettings") {
      TizenResult ret = Setting::LaunchLocationSetting();
      SendResult(flutter::EncodableValue(static_cast<bool>(ret)));
    } else {
      result->NotImplemented();
    }
  }

  void OnCheckPermission() {
    PermissionStatus permission_status;
    TizenResult ret =
        permission_manager_->CheckPermissionStatus(&permission_status);
    if (!ret) {
      SendErrorResult(ret.message(), "Failed to check permssion status.");
      return;
    }
    LOG_INFO("permission_status is %d", permission_status);
    SendResult(
        flutter::EncodableValue(static_cast<int>(permission_status)));
  }

  void OnIsLocationServiceEnabled() {
    bool is_enabled = false;
    TizenResult ret = location_manager_->IsLocationServiceEnabled(&is_enabled);

    // TODO : add location service listener
    if (!ret) {
      SendErrorResult(ret.message(), "Failed to check service enabled.");
    }
    SendResult(flutter::EncodableValue(is_enabled));
  }

  void OnRequestPermission() {
    auto result_ptr = result_.get();
    permission_manager_->RequestPermssion(
        [result_ptr](PermissionStatus permission_status) {
          result_ptr->Success(
              flutter::EncodableValue(static_cast<int>(permission_status)));
        },
        [result_ptr](TizenResult tizen_result) {
          result_ptr->Error(tizen_result.message(), "Failed to request permssion.");
        });
  }

  void OnGetLastKnownPosition() {
    Location location;
    TizenResult tizen_result =
        location_manager_->GetLastKnownLocation(&location);
    if (!tizen_result) {
      SendErrorResult(tizen_result.message(), "Failed to get last known position.");
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
          result_ptr->Error(error.message(),
              "An error occurred while requesting current location.");
        });

    if (!tizen_result) {
      SendErrorResult(tizen_result.message(), "Failed to call RequestCurrentLocationOnce.");
    }
  }

  void OnListenGeolocatorServiceUpdates(
      std::unique_ptr<FlEventSink>
          &&event_sink) {
    geolocator_service_updates_event_sink_ = std::move(event_sink);
    TizenResult tizen_result = location_manager_->SetOnServiceStateChanged(
        [this](ServiceState service_state) {
          flutter::EncodableValue value(static_cast<int>(service_state));
          geolocator_service_updates_event_sink_->Success(value);
        });
    if (!tizen_result) {
      LOG_ERROR("Failed to set OnServiceStateChanged, %s.",
                tizen_result.message().c_str());
      geolocator_service_updates_event_sink_ = nullptr;
    }
  }

  void OnCancelGeolocatorServiceUpdates() {
    geolocator_service_updates_event_sink_ = nullptr;
    location_manager_->UnsetOnServiceStateChanged();
  }

  void OnListenGeolocatorUpdates(
      std::unique_ptr<FlEventSink>
          &&event_sink) {
    geolocator_updates_event_sink_ = std::move(event_sink);
    TizenResult tizen_result =
        location_manager_->SetOnLocationUpdated([this](Location location) {
          geolocator_updates_event_sink_->Success(location.ToEncodableValue());
        });
    if (!tizen_result) {
      LOG_ERROR("Failed to set OnLocationUpdated, %s.",
                tizen_result.message().c_str());
      geolocator_updates_event_sink_ = nullptr;
    }
  }

  void OnCancelGeolocatorUpdates() {
    geolocator_updates_event_sink_ = nullptr;
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
  std::unique_ptr<FlEventChannel>
      geolocator_service_updates_channel_;
  std::unique_ptr<FlEventSink>
      geolocator_service_updates_event_sink_;
  std::unique_ptr<FlEventChannel>
      geolocator_updates_channel_;
  std::unique_ptr<FlEventSink>
      geolocator_updates_event_sink_;
};

}  // namespace

void GeolocatorTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  GeolocatorTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
