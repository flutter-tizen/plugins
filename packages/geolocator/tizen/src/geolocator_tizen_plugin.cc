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
