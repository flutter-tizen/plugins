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
typedef flutter::StreamHandler<flutter::EncodableValue> FlStreamHandler;
typedef flutter::StreamHandlerError<flutter::EncodableValue>
    FlStreamHandlerError;

constexpr char kPrivilegeLocation[] = "http://tizen.org/privilege/location";

class LocationStreamHandler : public FlStreamHandler {
 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    events_ = std::move(events);
    LocationCallback callback = [this](Position position) -> void {
      events_->Success(position.ToEncodableValue());
    };
    try {
      location_manager_.StartListenLocationUpdate(callback);
    } catch (const LocationManagerError &error) {
      return std::make_unique<FlStreamHandlerError>(
          std::to_string(error.GetErrorCode()), error.GetErrorString(),
          nullptr);
    }
    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    try {
      location_manager_.StopListenLocationUpdate();
    } catch (const LocationManagerError &error) {
      return std::make_unique<FlStreamHandlerError>(
          std::to_string(error.GetErrorCode()), error.GetErrorString(),
          nullptr);
    }
    events_.reset();
    return nullptr;
  }

 private:
  LocationManager location_manager_;
  std::unique_ptr<FlEventSink> events_;
};

class ServiceStatusStreamHandler : public FlStreamHandler {
 protected:
  std::unique_ptr<FlStreamHandlerError> OnListenInternal(
      const flutter::EncodableValue *arguments,
      std::unique_ptr<FlEventSink> &&events) override {
    events_ = std::move(events);
    ServiceStatusCallback callback = [this](ServiceStatus status) -> void {
      events_->Success(flutter::EncodableValue(static_cast<int>(status)));
    };
    try {
      location_manager_.StartListenServiceStatusUpdate(callback);
    } catch (const LocationManagerError &error) {
      return std::make_unique<FlStreamHandlerError>(
          std::to_string(error.GetErrorCode()), error.GetErrorString(),
          nullptr);
    }
    return nullptr;
  }

  std::unique_ptr<FlStreamHandlerError> OnCancelInternal(
      const flutter::EncodableValue *arguments) override {
    try {
      location_manager_.StopListenServiceStatusUpdate();
    } catch (const LocationManagerError &error) {
      return std::make_unique<FlStreamHandlerError>(
          std::to_string(error.GetErrorCode()), error.GetErrorString(),
          nullptr);
    }
    events_.reset();
    return nullptr;
  }

 private:
  LocationManager location_manager_;
  std::unique_ptr<FlEventSink> events_;
};

class GeolocatorTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel = std::make_unique<FlMethodChannel>(
        registrar->messenger(), "flutter.baseflow.com/geolocator",
        &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<GeolocatorTizenPlugin>();

    plugin->service_updates_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(),
        "flutter.baseflow.com/geolocator_service_updates",
        &flutter::StandardMethodCodec::GetInstance());
    plugin->service_updates_channel_->SetStreamHandler(
        std::make_unique<ServiceStatusStreamHandler>());

    plugin->updates_channel_ = std::make_unique<FlEventChannel>(
        registrar->messenger(), "flutter.baseflow.com/geolocator_updates",
        &flutter::StandardMethodCodec::GetInstance());
    plugin->updates_channel_->SetStreamHandler(
        std::make_unique<LocationStreamHandler>());

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
      bool opened = app_settings_manager_->OpenAppSettings();
      SendResult(flutter::EncodableValue(opened));
    } else if (method_name == "openLocationSettings") {
      bool opened = app_settings_manager_->OpenLocationSettings();
      SendResult(flutter::EncodableValue(opened));
    } else {
      result->NotImplemented();
    }
  }

  void OnCheckPermission() {
    PermissionStatus result =
        permission_manager_->CheckPermission(kPrivilegeLocation);
    if (result == PermissionStatus::kError) {
      SendErrorResult("Operation failed", "Permission request failed.");
      return;
    }
    LOG_INFO("permission_status is %d", result);
    SendResult(flutter::EncodableValue(static_cast<int>(result)));
  }

  void OnIsLocationServiceEnabled() {
    try {
      bool is_enabled = location_manager_->IsLocationServiceEnabled();
      SendResult(flutter::EncodableValue(is_enabled));
    } catch (const LocationManagerError &error) {
      SendErrorResult("Operation failed", error.GetErrorString());
    }
  }

  void OnRequestPermission() {
    PermissionStatus result =
        permission_manager_->RequestPermission(kPrivilegeLocation);

    if (result == PermissionStatus::kError) {
      SendErrorResult("Operation failed", "Permission request failed.");
      return;
    } else if (result == PermissionStatus::kDeniedForever ||
               result == PermissionStatus::kDenied) {
      SendErrorResult("Permission denied", "Permission denied by user.");
      return;
    }
    SendResult(flutter::EncodableValue(static_cast<int>(result)));
  }

  void OnGetLastKnownPosition() {
    try {
      Position position = location_manager_->GetLastKnownPosition();
      SendResult(position.ToEncodableValue());
    } catch (const LocationManagerError &error) {
      SendErrorResult("Operation failed", error.GetErrorString());
    }
  }

  void OnGetCurrentPosition() {
    auto result_ptr = result_.release();
    try {
      location_manager_->GetCurrentPosition(
          [result_ptr](Position position) {
            if (result_ptr) {
              result_ptr->Success(position.ToEncodableValue());
              delete result_ptr;
            }
          },
          [result_ptr](LocationManagerError error) {
            if (result_ptr) {
              result_ptr->Error("Operation failed", error.GetErrorString());
              delete result_ptr;
            }
          });
    } catch (const LocationManagerError &error) {
      if (result_ptr) {
        result_ptr->Error("Operation failed", error.GetErrorString());
        delete result_ptr;
      }
    }
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
