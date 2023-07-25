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

template <typename T>
static T GetRequiredArg(const flutter::EncodableMap *arguments,
                        const char *key) {
  T value;
  if (GetValueFromEncodableMap(arguments, key, value)) {
    return value;
  }
  std::string message =
      "No " + std::string(key) + " provided or has invalid type or value.";
  throw std::invalid_argument(message);
}

class InAppPurchaseTizenPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *plugin_registrar);

  InAppPurchaseTizenPlugin();
  virtual ~InAppPurchaseTizenPlugin() { Dispose(); }

 private:
  void Dispose();
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  void GetProductList(
      const flutter::EncodableMap *encodables,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  void GetPurchaseList(
      const flutter::EncodableMap *encodables,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  void BuyItem(
      const flutter::EncodableMap *encodables,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  void VerifyInvoice(
      const flutter::EncodableMap *encodables,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

  std::unique_ptr<BillingManager> billing_ = nullptr;
};

InAppPurchaseTizenPlugin::InAppPurchaseTizenPlugin() {
  billing_ = std::make_unique<BillingManager>();
  if (!billing_->Init()) {
    Dispose();
  }
}

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

  auto plugin = std::make_unique<InAppPurchaseTizenPlugin>();

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
  const auto &method_name = method_call.method_name();

  try {
    if (method_name == "getProductList") {
      if (!encodables) {
        result->Error(kInvalidArgument, "No arguments provided");
        return;
      }
      GetProductList(encodables, std::move(result));
    } else if (method_name == "getPurchaseList") {
      if (!encodables) {
        result->Error(kInvalidArgument, "No arguments provided");
        return;
      }
      GetPurchaseList(encodables, std::move(result));
    } else if (method_name == "buyItem") {
      if (!encodables) {
        result->Error(kInvalidArgument, "No arguments provided");
        return;
      }
      BuyItem(encodables, std::move(result));
    } else if (method_name == "verifyInvoice") {
      if (!encodables) {
        result->Error(kInvalidArgument, "No arguments provided");
        return;
      }
      VerifyInvoice(encodables, std::move(result));
    } else if (method_name == "isAvailable") {
      if (!billing_->BillingIsAvailable(std::move(result))) {
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

void InAppPurchaseTizenPlugin::GetProductList(
    const flutter::EncodableMap *encodables,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  std::string app_id = GetRequiredArg<std::string>(encodables, "appId");
  std::string country_code =
      GetRequiredArg<std::string>(encodables, "countryCode");
  int64_t page_size = GetRequiredArg<int>(encodables, "pageSize");
  int64_t page_num = GetRequiredArg<int>(encodables, "pageNum");
  std::string check_value =
      GetRequiredArg<std::string>(encodables, "checkValue");

  if (!billing_->GetProductList(app_id.c_str(), country_code.c_str(), page_size,
                                page_num, check_value.c_str(),
                                std::move(result))) {
    return;
  }
}

void InAppPurchaseTizenPlugin::GetPurchaseList(
    const flutter::EncodableMap *encodables,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  std::string app_id = GetRequiredArg<std::string>(encodables, "appId");
  std::string custom_id = GetRequiredArg<std::string>(encodables, "customId");
  std::string country_code =
      GetRequiredArg<std::string>(encodables, "countryCode");
  int64_t page_num = GetRequiredArg<int>(encodables, "pageNum");
  std::string check_value =
      GetRequiredArg<std::string>(encodables, "checkValue");

  if (!billing_->GetPurchaseList(app_id.c_str(), custom_id.c_str(),
                                 country_code.c_str(), page_num,
                                 check_value.c_str(), std::move(result))) {
    return;
  }
}

void InAppPurchaseTizenPlugin::BuyItem(
    const flutter::EncodableMap *encodables,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  std::string pay_details =
      GetRequiredArg<std::string>(encodables, "payDetails");
  std::string app_id = GetRequiredArg<std::string>(encodables, "appId");

  if (!billing_->BuyItem(app_id.c_str(), pay_details.c_str(),
                         std::move(result))) {
    return;
  }
}

void InAppPurchaseTizenPlugin::VerifyInvoice(
    const flutter::EncodableMap *encodables,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  std::string app_id = GetRequiredArg<std::string>(encodables, "appId");
  std::string custom_id = GetRequiredArg<std::string>(encodables, "customId");
  std::string invoice_id = GetRequiredArg<std::string>(encodables, "invoiceId");
  std::string country_code =
      GetRequiredArg<std::string>(encodables, "countryCode");

  if (!billing_->VerifyInvoice(app_id.c_str(), custom_id.c_str(),
                               invoice_id.c_str(), country_code.c_str(),
                               std::move(result))) {
    return;
  }
}

}  // namespace

void InAppPurchaseTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  InAppPurchaseTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
