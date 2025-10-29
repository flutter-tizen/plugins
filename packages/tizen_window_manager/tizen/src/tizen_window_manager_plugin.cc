// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_window_manager_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <flutter_tizen.h>

#include <memory>

#include "log.h"
#include "tizen_window_manager.h"

namespace {

class TizenWindowManagerPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(
      FlutterDesktopPluginRegistrarRef registrar_ref,
      flutter::PluginRegistrar *plugin_registrar) {
    auto channel =
        std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
            plugin_registrar->messenger(), "tizen/window_manager",
            &flutter::StandardMethodCodec::GetInstance());

    auto plugin = std::make_unique<TizenWindowManagerPlugin>(registrar_ref);

    channel->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    plugin_registrar->AddPlugin(std::move(plugin));
  }

  TizenWindowManagerPlugin(FlutterDesktopPluginRegistrarRef registrar_ref) {
    FlutterDesktopViewRef flutter_view =
        FlutterDesktopPluginRegistrarGetView(registrar_ref);
    void *handle = FlutterDesktopViewGetNativeHandle(flutter_view);
    if (!handle) {
      LOG_ERROR("Fail to get native window handle.");
      return;
    }
    window_manager_ = std::make_unique<TizenWindowManager>(handle);
  }

  virtual ~TizenWindowManagerPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
    const auto &method_name = method_call.method_name();

    if (!window_manager_) {
      result->Error("No window handle.");
      return;
    }

    if (method_name == "activate") {
      window_manager_->Activate();
      result->Success();
    } else if (method_name == "lower") {
      window_manager_->Lower();
      result->Success();
    } else if (method_name == "getGeometry") {
      result->Success(flutter::EncodableValue(window_manager_->GetGeometry()));
    } else {
      result->NotImplemented();
    }
  }

  std::unique_ptr<TizenWindowManager> window_manager_ = nullptr;
};

}  // namespace

void TizenWindowManagerPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  TizenWindowManagerPlugin::RegisterWithRegistrar(
      registrar, flutter::PluginRegistrarManager::GetInstance()
                     ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
