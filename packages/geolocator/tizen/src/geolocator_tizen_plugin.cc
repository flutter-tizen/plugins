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

#include <memory>
#include <string>

#include "locaton_manager.h"
#include "log.h"
#include "permission_manager.h"
#include "setting.h"

namespace {

class GeolocatorTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
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
    geolocator_service_updates_channel_ =
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
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
        std::make_unique<flutter::EventChannel<flutter::EncodableValue>>(
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
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    std::string method_name = method_call.method_name();
    if (method_name == "checkPermission") {
      OnCheckPermission(std::move(result));
    } else if (method_name == "isLocationServiceEnabled") {
      OnIsLocationServiceEnabled(std::move(result));
    } else if (method_name == "requestPermission") {
      OnRequestPermission(std::move(result));
    } else if (method_name == "getLastKnownPosition") {
      OnGetLastKnownPosition(std::move(result));
    } else if (method_name == "getCurrentPosition") {
      OnGetCurrentPosition(std::move(result));
    } else if (method_name == "openAppSettings") {
      TizenResult ret = Setting::LaunchAppSetting();
      result->Success(flutter::EncodableValue(static_cast<bool>(ret)));
    } else if (method_name == "openLocationSettings") {
      TizenResult ret = Setting::LaunchLocationSetting();
      result->Success(flutter::EncodableValue(static_cast<bool>(ret)));
    } else {
      result->NotImplemented();
    }
  }

  void OnCheckPermission(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    PermissionStatus permission_status;
    TizenResult ret =
        permission_manager_->CheckPermissionStatus(&permission_status);
    if (!ret) {
      result->Error("Failed to check permssion status.", ret.message());
      return;
    }
    LOG_INFO("permission_status is %d", permission_status);
    result->Success(
        flutter::EncodableValue(static_cast<int>(permission_status)));
  }

  void OnIsLocationServiceEnabled(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    bool is_enabled = false;
    TizenResult ret = location_manager_->IsLocationServiceEnabled(&is_enabled);

    // TODO : add location service listener
    if (!ret) {
      result->Error("Failed to check service enabled.", ret.message());
    }
    result->Success(flutter::EncodableValue(is_enabled));
  }

  void OnRequestPermission(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    auto result_ptr = result.release();
    permission_manager_->RequestPermssion(
        [result_ptr](PermissionStatus permission_status) {
          result_ptr->Success(
              flutter::EncodableValue(static_cast<int>(permission_status)));
          delete result_ptr;
        },
        [result_ptr](TizenResult tizen_result) {
          result_ptr->Error("Failed to request permssion.",
                            tizen_result.message());
          delete result_ptr;
        });
  }

  void OnGetLastKnownPosition(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    Location location;
    TizenResult tizen_result =
        location_manager_->GetLastKnownLocation(&location);
    if (!tizen_result) {
      result->Error("Failed to get last known position.",
                    tizen_result.message());
      return;
    }
    result->Success(location.ToEncodableValue());
  }

  void OnGetCurrentPosition(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    auto result_ptr = result.release();
    TizenResult tizen_result = location_manager_->RequestCurrentLocationOnce(
        [result_ptr](Location location) {
          result_ptr->Success(location.ToEncodableValue());
          delete result_ptr;
        },
        [result_ptr](TizenResult error) {
          result_ptr->Error(
              "An error occurred while requesting current location.",
              error.message());
          delete result_ptr;
        });

    if (!tizen_result) {
      result_ptr->Error("Failed to call RequestCurrentLocationOnce.",
                        tizen_result.message());
      delete result_ptr;
    }
  }

  void OnListenGeolocatorServiceUpdates(
      std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>
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
      std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>
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

  std::unique_ptr<PermissionManager> permission_manager_;
  std::unique_ptr<LocationManager> location_manager_;
  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>
      geolocator_service_updates_channel_;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>
      geolocator_service_updates_event_sink_;
  std::unique_ptr<flutter::EventChannel<flutter::EncodableValue>>
      geolocator_updates_channel_;
  std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>
      geolocator_updates_event_sink_;
};

}  // namespace

void GeolocatorTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  GeolocatorTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
