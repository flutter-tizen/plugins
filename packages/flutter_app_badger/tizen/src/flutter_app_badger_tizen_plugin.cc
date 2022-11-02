// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_app_badger_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>
#include <system_info.h>

#include <memory>
#include <string>

#include "log.h"
#include "tizen_badge.h"

namespace {

typedef flutter::MethodChannel<flutter::EncodableValue> FlMethodChannel;
typedef flutter::MethodResult<flutter::EncodableValue> FlMethodResult;

template <typename T>
static bool GetValueFromEncodableMap(const flutter::EncodableMap *map,
                                     const char *key, T &out) {
  auto iter = map->find(flutter::EncodableValue(key));
  if (iter != map->end() && !iter->second.IsNull()) {
    if (auto *value = std::get_if<T>(&iter->second)) {
      out = *value;
      return true;
    }
  }
  return false;
}

class FlutterAppBadgerTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto plugin = std::make_unique<FlutterAppBadgerTizenPlugin>(registrar);
    registrar->AddPlugin(std::move(plugin));
  }

  FlutterAppBadgerTizenPlugin(flutter::PluginRegistrar *registrar) {
    // FlutterAppBadgerPlugin : https://pub.dev/packages/flutter_app_badger
    channel_ = std::make_unique<FlMethodChannel>(
        registrar->messenger(), "g123k/flutter_app_badger",
        &flutter::StandardMethodCodec::GetInstance());
    channel_->SetMethodCallHandler([this](const auto &call, auto result) {
      this->HandleMethodCall(call, std::move(result));
    });

    badge_ = std::make_unique<TizenBadge>();
    if (!badge_->Initialize()) {
      badge_ = nullptr;
    }
  }

  virtual ~FlutterAppBadgerTizenPlugin() {}

 private:
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<FlMethodResult> result) {
    const auto &method_name = method_call.method_name();

    if (!badge_) {
      result->Error("Operation failed", "Badge initialization failed.");
      return;
    }

    if (method_name == "isAppBadgeSupported") {
      if (badge_->IsSupported()) {
        result->Success(flutter::EncodableValue(true));
      } else {
        result->Success(flutter::EncodableValue(false));
      }
    } else if (method_name == "updateBadgeCount") {
      const auto *arguments =
          std::get_if<flutter::EncodableMap>(method_call.arguments());
      if (!arguments) {
        result->Error("Invalid arguments", "Invalid argument type.");
        return;
      }
      int count = 0;
      if (GetValueFromEncodableMap(arguments, "count", count)) {
        if (badge_->UpdateBadgeCount(count)) {
          result->Success(flutter::EncodableValue(true));
        } else {
          result->Error("Operation failed", "updateBadgeCount() falied.");
        }
      } else {
        result->Error("Invalid arguments", "Invalid argument type.");
      }
    } else if (method_name == "removeBadge") {
      if (badge_->RemoveBadge()) {
        result->Success(flutter::EncodableValue(true));
      } else {
        result->Error("Operation failed", "removeBadge() failed.");
      }
    } else {
      result->NotImplemented();
    }
  }

  std::unique_ptr<TizenBadge> badge_;
  std::unique_ptr<FlMethodChannel> channel_;
};

}  // namespace

void FlutterAppBadgerTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  FlutterAppBadgerTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
