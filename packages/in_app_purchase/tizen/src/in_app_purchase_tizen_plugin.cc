// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "in_app_purchase_tizen_plugin.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <string>

#include "billing_manager.h"
#include "log.h"

namespace {

const char *kInvalidArgument = "Invalid argument";

class InAppPurchaseTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *plugin_registrar);

  InAppPurchaseTizenPlugin(flutter::PluginRegistrar *plugin_registrar);
  virtual ~InAppPurchaseTizenPlugin() { Dispose(); }

 private:
  void Dispose();
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  flutter::PluginRegistrar *plugin_registrar_ = nullptr;
  std::unique_ptr<BillingManager> billing_ = nullptr;
};

void InAppPurchaseTizenPlugin::Dispose() {
  if (billing_) {
    billing_->Dispose();
  }
  billing_ = nullptr;
}

void InAppPurchaseTizenPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrar *plugin_registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          plugin_registrar->messenger(),
          "plugins.flutter.tizen.io/in_app_purchase",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<InAppPurchaseTizenPlugin>(plugin_registrar);

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  plugin_registrar->AddPlugin(std::move(plugin));
}

void InAppPurchaseTizenPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  const auto *encodables =
      std::get_if<flutter::EncodableMap>(method_call.arguments());
  try {
    const auto &method_name = method_call.method_name();
    if (method_name == "getProductList") {
      if (!encodables) {
        result->Error(kInvalidArgument, "No arguments provided");
        return;
      }
      if (!billing_->GetProductList(encodables, std::move(result))) {
        result->Error("getProductList failed");
        return;
      }
    } else if (method_name == "getPurchaseList") {
      if (!encodables) {
        result->Error(kInvalidArgument, "No arguments provided");
        return;
      }
      if (!billing_->GetPurchaseList(encodables, std::move(result))) {
        result->Error("getPurchaseList failed");
        return;
      }
    } else if (method_name == "buyItem") {
      if (!encodables) {
        result->Error(kInvalidArgument, "No arguments provided");
        return;
      }
      if (!billing_->BuyItem(encodables, std::move(result))) {
        result->Error("buyItem failed");
        return;
      }
    } else if (method_name == "isAvailable") {
      if (!billing_->BillingIsAvailable(std::move(result))) {
        result->Error("isAvailable failed");
        return;
      }
    } else if (method_name == "verifyInvoice") {
      if (!encodables) {
        result->Error(kInvalidArgument, "No arguments provided");
        return;
      }
      if (!billing_->VerifyInvoice(encodables, std::move(result))) {
        result->Error("verifyInvoice failed");
        return;
      }
    } else if (method_name == "GetCustomId") {
      result->Success(
          flutter::EncodableValue(std::string(billing_->GetCustomId())));
    } else if (method_name == "GetCountryCode") {
      result->Success(
          flutter::EncodableValue(std::string(billing_->GetCountryCode())));
    } else {
      result->NotImplemented();
    }
  } catch (const std::invalid_argument &error) {
    result->Error(kInvalidArgument, error.what());
  }
}

InAppPurchaseTizenPlugin::InAppPurchaseTizenPlugin(
    flutter::PluginRegistrar *plugin_registrar) {
  billing_ = std::make_unique<BillingManager>();
  if (!billing_->Init()) {
    Dispose();
  }
}

}  // namespace

void InAppPurchaseTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  InAppPurchaseTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
