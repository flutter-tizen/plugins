// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
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
#include "messages.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace {

class InAppPurchaseTizenPlugin : public flutter::Plugin,
                                 public InAppPurchaseApi {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *plugin_registrar);

  InAppPurchaseTizenPlugin(flutter::PluginRegistrar *plugin_registrar);
  virtual ~InAppPurchaseTizenPlugin() { Dispose(); }

  void GetProductsList(const ProductMessage &product,
                       std::function<void(ErrorOr<ProductsListApiResult> reply)>
                           result) override;
  void GetUserPurchaseList(
      const PurchaseMessage &purchase,
      std::function<void(ErrorOr<GetUserPurchaseListAPIResult> reply)> result)
      override;
  void BuyItem(
      const BuyInfoMessage &buy_info,
      std::function<void(ErrorOr<BillingBuyData> reply)> result) override;
  void VerifyInvoice(const InvoiceMessage &invoice,
                     std::function<void(ErrorOr<VerifyInvoiceAPIResult> reply)>
                         result) override;
  void IsServiceAvailable(
      std::function<void(ErrorOr<bool> reply)> result) override;
  ErrorOr<std::string> GetCustomId() override;
  ErrorOr<std::string> GetCountryCode() override;

 private:
  void Dispose();

  std::unique_ptr<BillingManager> billing_ = nullptr;
};

InAppPurchaseTizenPlugin::InAppPurchaseTizenPlugin(
    flutter::PluginRegistrar *plugin_registrar) {
  billing_ = std::make_unique<BillingManager>();
  if (!billing_->Init()) {
    Dispose();
  }
  InAppPurchaseTizenPlugin::SetUp(plugin_registrar->messenger(), this);
}

void InAppPurchaseTizenPlugin::Dispose() {
  if (billing_) {
    billing_->Dispose();
  }
  billing_ = nullptr;
}

void InAppPurchaseTizenPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrar *plugin_registrar) {
  auto plugin = std::make_unique<InAppPurchaseTizenPlugin>(plugin_registrar);
  plugin_registrar->AddPlugin(std::move(plugin));
}

void InAppPurchaseTizenPlugin::GetProductsList(
    const ProductMessage &product,
    std::function<void(ErrorOr<ProductsListApiResult> reply)> result) {
  std::string app_id = product.app_id();
  std::string country_code = product.country_code();
  int64_t page_size = product.page_size();
  int64_t page_num = product.page_num();
  std::string check_value = product.check_value();

  if (!billing_->GetProductList(app_id.c_str(), country_code.c_str(), page_size,
                                page_num, check_value.c_str(),
                                std::move(result))) {
    result(FlutterError("GetProductList", "get product list failed"));
    return;
  }
}

void InAppPurchaseTizenPlugin::GetUserPurchaseList(
    const PurchaseMessage &purchase,
    std::function<void(ErrorOr<GetUserPurchaseListAPIResult> reply)> result) {
  std::string app_id = purchase.app_id();
  std::string custom_id = purchase.custom_id();
  std::string country_code = purchase.country_code();
  int64_t page_num = purchase.page_num();
  std::string check_value = purchase.check_value();

  if (!billing_->GetPurchaseList(app_id.c_str(), custom_id.c_str(),
                                 country_code.c_str(), page_num,
                                 check_value.c_str(), std::move(result))) {
    result(FlutterError("GetPurchaseList", "get purchase list failed"));
    return;
  }
}

void InAppPurchaseTizenPlugin::BuyItem(
    const BuyInfoMessage &buy_info,
    std::function<void(ErrorOr<BillingBuyData> reply)> result) {
  OrderDetails pay_details = buy_info.pay_detials();
  std::string app_id = buy_info.app_id();
  std::string pay_details_json;
  {
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
    rapidjson::Value order_item_id, order_title, order_total, order_curenncy_id,
        order_custom_id;
    order_item_id.SetString(pay_details.order_item_id().c_str(),
                            strlen(pay_details.order_item_id().c_str()),
                            allocator);
    order_title.SetString(pay_details.order_title().c_str(),
                          strlen(pay_details.order_title().c_str()), allocator);
    order_total.SetString(pay_details.order_total().c_str(),
                          strlen(pay_details.order_total().c_str()), allocator);
    order_curenncy_id.SetString(pay_details.order_currency_id().c_str(),
                                strlen(pay_details.order_currency_id().c_str()),
                                allocator);
    order_custom_id.SetString(pay_details.order_custom_id().c_str(),
                              strlen(pay_details.order_custom_id().c_str()),
                              allocator);
    doc.AddMember("OrderItemID", order_item_id, allocator);
    doc.AddMember("OrderTitle", order_title, allocator);
    doc.AddMember("OrderTotal", order_total, allocator);
    doc.AddMember("OrderCurrencyID", order_curenncy_id, allocator);
    doc.AddMember("OrderCustomID", order_custom_id, allocator);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    pay_details_json = buffer.GetString();
  }

  if (!billing_->BuyItem(app_id.c_str(), pay_details_json.c_str(),
                         std::move(result))) {
    result(FlutterError("BuyItem", "buy item failed"));
    return;
  }
}

void InAppPurchaseTizenPlugin::VerifyInvoice(
    const InvoiceMessage &invoice,
    std::function<void(ErrorOr<VerifyInvoiceAPIResult> reply)> result) {
  std::string app_id = invoice.app_id();
  std::string custom_id = invoice.custom_id();
  std::string invoice_id = invoice.invoice_id();
  std::string country_code = invoice.country_code();

  if (!billing_->VerifyInvoice(app_id.c_str(), custom_id.c_str(),
                               invoice_id.c_str(), country_code.c_str(),
                               std::move(result))) {
    result(FlutterError("VerifyInvoice", "invoice verify failed"));
    return;
  }
}

void InAppPurchaseTizenPlugin::IsServiceAvailable(
    std::function<void(ErrorOr<bool> reply)> result) {
  if (!billing_->IsAvailable(std::move(result))) {
    result(FlutterError("IsAvailable", "billing is not available"));
    return;
  }
}

ErrorOr<std::string> InAppPurchaseTizenPlugin::GetCustomId() {
  return billing_->GetCustomId();
}

ErrorOr<std::string> InAppPurchaseTizenPlugin::GetCountryCode() {
  return billing_->GetCountryCode();
}

}  // namespace

void InAppPurchaseTizenPluginRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  InAppPurchaseTizenPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar));
}
