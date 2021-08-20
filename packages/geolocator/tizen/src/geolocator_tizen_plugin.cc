// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "geolocator_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "locaton_manager.h"
#include "log.h"
#include "permission_manager.h"

namespace {

class GeolocatorTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "flutter.baseflow.com/geolocator",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<GeolocatorTizenPlugin>();

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
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    LOG_DEBUG("method call [%s]", method_call.method_name().c_str());

    std::string method_name = method_call.method_name();
    if (method_name == "checkPermission") {
      onCheckPermission(std::move(result));
    } else if (method_name == "isLocationServiceEnabled") {
      onIsLocationServiceEnabled(std::move(result));
    } else if (method_name == "requestPermission") {
      onRequestPermission(std::move(result));
    } else if (method_name == "getLastKnownPosition") {
      onGetLastKnownPosition(std::move(result));
    } else if (method_name == "getCurrentPosition") {
      onGetCurrentPosition(std::move(result));
    } else {
      result->NotImplemented();
    }
  }

  void onCheckPermission(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    PermissionStatus permission_status;
    TizenResult ret =
        permission_manager_->CheckPermissionStatus(&permission_status);
    if (!ret) {
      result->Error("Failed to check permssion status", ret.message());
      return;
    }
    LOG_INFO("permission_status is %d", permission_status);
    result->Success(
        flutter::EncodableValue(static_cast<int>(permission_status)));
  }

  void onIsLocationServiceEnabled(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    bool is_enabled = false;
    TizenResult ret = location_manager_->IsLocationServiceEnabled(&is_enabled);

    // TODO : add location service listener
    if (!ret) {
      result->Error("Failed to check service enabled", ret.message());
    }
    result->Success(flutter::EncodableValue(is_enabled));
  }

  void onRequestPermission(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    auto result_ptr = result.release();
    permission_manager_->RequestPermssion(
        [result_ptr](PermissionStatus permission_status) {
          result_ptr->Success(
              flutter::EncodableValue(static_cast<int>(permission_status)));
          delete result_ptr;
        },
        [result_ptr](TizenResult tizen_result) {
          result_ptr->Error("Failed to request permssion",
                            tizen_result.message());
          delete result_ptr;
        });
  }

  void onGetLastKnownPosition(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    Location location;
    TizenResult tizen_result =
        location_manager_->GetLastKnownLocation(&location);
    if (!tizen_result) {
      result->Error("Failed to get last known position",
                    tizen_result.message());
      return;
    }
    result->Success(location.ToEncodableValue());
  }

  void onGetCurrentPosition(
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    auto result_ptr = result.release();
    TizenResult tizen_result = location_manager_->RequestCurrentLocationOnce(
        [result_ptr](Location location) {
          result_ptr->Success(location.ToEncodableValue());
          delete result_ptr;
        },
        [result_ptr](TizenResult error) {
          result_ptr->Error(
              "An error occurred while requesting current location",
              error.message());
          delete result_ptr;
        });

    if (!tizen_result) {
      result_ptr->Error("Failed to call RequestCurrentLocationOnce",
                        tizen_result.message());
      delete result_ptr;
    }
  }

  std::unique_ptr<PermissionManager> permission_manager_;
  std::unique_ptr<LocationManager> location_manager_;
};

}  // namespace

void GeolocatorTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  GeolocatorTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
