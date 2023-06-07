// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wakelock_tizen_plugin.h"

#include <device/power.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <string>
#include <variant>

namespace {

class WakelockTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            registrar->messenger(), "tizen/wakelock_plugin",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<WakelockTizenPlugin>();

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  WakelockTizenPlugin() {}

  virtual ~WakelockTizenPlugin() = default;

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const auto &method_name = method_call.method_name();

    if (method_name == "toggle") {
      const auto &arguments = *method_call.arguments();
      if (std::holds_alternative<bool>(arguments)) {
        bool enable = std::get<bool>(arguments);
        int ret = DEVICE_ERROR_NONE;
        if (enable) {
          ret = device_power_request_lock(POWER_LOCK_DISPLAY, 0);
        } else {
          ret = device_power_release_lock(POWER_LOCK_DISPLAY);
        }
        if (ret == DEVICE_ERROR_NONE) {
          enabled_ = enable;
          result->Success();
        } else {
          result->Error(std::to_string(ret), get_error_message(ret));
        }
      } else {
        result->Error("Invalid argument",
                      "The argument must be a boolean value.");
      }
    } else if (method_name == "isEnabled") {
      result->Success(flutter::EncodableValue(enabled_));
    } else {
      result->NotImplemented();
    }
  }

  bool enabled_ = false;
};

}  // namespace

void WakelockTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  WakelockTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
