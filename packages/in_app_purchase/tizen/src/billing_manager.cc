// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "billing_manager.h"

#include <flutter/encodable_value.h>
#include <flutter/standard_method_codec.h>

#include <cstddef>
#include <cstdlib>
#include <mutex>
#include <string>
#include <variant>

#include "ftpw_in_app_purchase.h"
#include "log.h"
#include "messages.h"
#include "rapidjson/document.h"

namespace in_app_purchase_tizen {

static flutter::EncodableValue ConvertVariantToEncodable(
    std::variant<int, double> var) {
  if (std::holds_alternative<int>(var)) {
    return flutter::EncodableValue(std::get<int>(var));
  } else if (std::holds_alternative<double>(var)) {
    return flutter::EncodableValue(std::get<double>(var));
  }
  return flutter::EncodableValue();
}

std::optional<std::string> BillingManager::GetCustomId() {
  std::optional<std::string> custom_id;
  sso_login_info_s login_info = {};
  if (!ftpw_in_app_purchase_sso_get_login_info(&login_info)) {
    custom_id = login_info.uid;
  } else {
    LOG_ERROR("[BillingManager] Fail to get the login info.");
  }
  // NOTE: written through volatile because a plain memset on a struct that
  // is dead afterwards is dropped by the optimizer.
  auto *bytes = reinterpret_cast<volatile unsigned char *>(&login_info);
  for (std::size_t i = 0; i < sizeof(login_info); ++i) {
    bytes[i] = 0;
  }
  return custom_id;
}

std::optional<std::string> BillingManager::GetCountryCode() {
  std::optional<std::string> country_code;
  char *value = ftpw_in_app_purchase_get_county_code();
  if (value) {
    country_code = value;
    free(value);
  } else {
    LOG_ERROR("[BillingManager] Fail to get country code.");
  }
  return country_code;
}

bool BillingManager::IsAvailable(FunctionResult<bool> result) {
  LOG_INFO("[BillingManager] Check billing server is available.");

  if (is_available_callback_) {
    LOG_ERROR("[BillingManager] IsAvailable is already called.");
    return false;
  }

  is_available_callback_ = std::move(result);
  bool ret = ftpw_in_app_purchase_service_billing_is_service_available(
      OnAvailable, this);
  if (!ret) {
    is_available_callback_ = nullptr;
    LOG_ERROR("[BillingManager] service_billing_is_service_available failed.");
    return false;
  }
  return true;
}

bool BillingManager::GetProductList(
    const char *app_id, const char *country_code, int page_size,
    int page_number, const char *check_value,
    FunctionResult<ProductsListApiResult> result) {
  LOG_INFO("[BillingManager] Start get product list.");

  if (get_product_list_callback_) {
    LOG_ERROR("[BillingManager] GetProductList is already called.");
    return false;
  }

  get_product_list_callback_ = std::move(result);
  bool ret = ftpw_in_app_purchase_service_billing_get_products_list(
      app_id, country_code, page_size, page_number, check_value, OnProducts,
      this);
  if (!ret) {
    get_product_list_callback_ = nullptr;
    LOG_ERROR("[BillingManager] service_billing_get_products_list failed.");
    return false;
  }
  return true;
}

bool BillingManager::GetPurchaseList(
    const char *app_id, const char *custom_id, const char *country_code,
    int page_number, const char *check_value,
    FunctionResult<GetUserPurchaseListAPIResult> result) {
  LOG_INFO("[BillingManager] Start get purchase list.");

  if (get_purchase_list_callback_) {
    LOG_ERROR("[BillingManager] GetPurchaseList is already called.");
    return false;
  }

  get_purchase_list_callback_ = std::move(result);
  bool ret = ftpw_in_app_purchase_service_billing_get_purchase_list(
      app_id, custom_id, country_code, page_number, check_value, OnPurchase,
      this);
  if (!ret) {
    get_purchase_list_callback_ = nullptr;
    LOG_ERROR("[BillingManager] service_billing_get_purchase_list failed.");
    return false;
  }
  return true;
}

bool BillingManager::BuyItem(const char *app_id, const char *detail_info,
                             FunctionResult<BillingBuyData> result) {
  LOG_INFO("[BillingManager] Start buy item");

  std::lock_guard<std::mutex> lock(mutex_);
  if (buy_item_callback_) {
    LOG_ERROR("[BillingManager] BuyItem is already called.");
    return false;
  }

  buy_item_callback_ = std::move(result);
  bool ret = ftpw_in_app_purchase_service_billing_buyitem(app_id, detail_info);
  ftpw_in_app_purchase_service_billing_set_buyitem_cb(OnBuyItem, this);
  if (!ret) {
    buy_item_callback_ = nullptr;
    LOG_ERROR("[BillingManager] service_billing_buyitem failed.");
    return false;
  }
  return true;
}

bool BillingManager::VerifyInvoice(
    const char *app_id, const char *custom_id, const char *invoice_id,
    const char *country_code, FunctionResult<VerifyInvoiceAPIResult> result) {
  LOG_INFO("[BillingManager] Start verify invoice");

  if (verify_invoice_callback_) {
    LOG_ERROR("[BillingManager] VerifyInvoice is already called.");
    return false;
  }

  verify_invoice_callback_ = std::move(result);
  bool ret = ftpw_in_app_purchase_service_billing_verify_invoice(
      app_id, custom_id, invoice_id, country_code, OnVerify, this);
  if (!ret) {
    verify_invoice_callback_ = nullptr;
    LOG_ERROR("[BillingManager] service_billing_verify_invoice failed.");
    return false;
  }
  return true;
}

void BillingManager::OnAvailable(const char *detail_result, void *user_data) {
  LOG_INFO("[BillingManager] Billing server detail_result: %s", detail_result);

  BillingManager *self = reinterpret_cast<BillingManager *>(user_data);

  if (self->is_available_callback_) {
    rapidjson::Document doc;
    doc.Parse(detail_result);
    if (doc.HasParseError()) {
      self->is_available_callback_(
          FlutterError("Operation failed", "OnAvailable parse error."));
      self->is_available_callback_ = nullptr;
      return;
    }

    std::string status = GetJsonValue<std::string>(doc, "status", "Unknown");
    if (status == "100000")
      self->is_available_callback_(true);
    else
      self->is_available_callback_(false);
  } else {
    self->is_available_callback_(
        FlutterError("Invalid argument", "is_available_callback_ is null !"));
  }
  self->is_available_callback_ = nullptr;
}

void BillingManager::OnProducts(const char *detail_result, void *user_data) {
  LOG_INFO("[BillingManager] Productlist: %s", detail_result);

  BillingManager *self = reinterpret_cast<BillingManager *>(user_data);

  if (self->get_product_list_callback_) {
    rapidjson::Document doc;
    doc.Parse(detail_result);
    if (doc.HasParseError()) {
      self->get_product_list_callback_(
          FlutterError("Operation failed", "OnProducts parse error."));
      self->get_product_list_callback_ = nullptr;
      return;
    }
    std::string cp_status =
        GetJsonValue<std::string>(doc, "CPStatus", "Unknown");
    std::string cp_result =
        GetJsonValue<std::string>(doc, "CPResult", "Unknown");
    int64_t total_count = GetJsonValue<int64_t>(doc, "TotalCount", -1);
    std::string check_value =
        GetJsonValue<std::string>(doc, "CheckValue", "Unknown");
    flutter::EncodableList item_details;
    {
      rapidjson::Document empty_doc;
      empty_doc.SetArray();
      const rapidjson::Value &default_array = empty_doc;
      const rapidjson::Value &jarray = GetJsonValue<const rapidjson::Value &>(
          doc, "ItemDetails", default_array);
      for (rapidjson::SizeType i = 0; i < jarray.Size(); ++i) {
        int64_t seq = GetJsonValue<int64_t>(jarray[i], "Seq", -1);
        std::string item_id =
            GetJsonValue<std::string>(jarray[i], "ItemID", "Unknown");
        std::string item_title =
            GetJsonValue<std::string>(jarray[i], "ItemTitle", "Unknown");
        std::string item_desc =
            GetJsonValue<std::string>(jarray[i], "ItemDesc", "Unknown");
        int64_t item_type = GetJsonValue<int64_t>(jarray[i], "ItemType", -1);
        std::variant<int, double> price =
            GetJsonValue<std::variant<int, double>>(jarray[i], "Price", -1);
        std::string currency_id =
            GetJsonValue<std::string>(jarray[i], "CurrencyID", "Unknown");

        flutter::EncodableValue detail =
            flutter::EncodableValue(flutter::EncodableMap{
                {flutter::EncodableValue("Seq"), flutter::EncodableValue(seq)},
                {flutter::EncodableValue("ItemID"),
                 flutter::EncodableValue(item_id)},
                {flutter::EncodableValue("ItemTitle"),
                 flutter::EncodableValue(item_title)},
                {flutter::EncodableValue("ItemDesc"),
                 flutter::EncodableValue(item_desc)},
                {flutter::EncodableValue("ItemType"),
                 flutter::EncodableValue(item_type)},
                {flutter::EncodableValue("Price"),
                 ConvertVariantToEncodable(price)},
                {flutter::EncodableValue("CurrencyID"),
                 flutter::EncodableValue(currency_id)},
            });
        item_details.push_back(detail);
      }
    }
    ProductsListApiResult products_list(cp_status, &cp_result, total_count,
                                        check_value, item_details);
    self->get_product_list_callback_(products_list);
  } else {
    self->get_product_list_callback_(FlutterError(
        "Invalid argument", "get_product_list_callback_ is null !"));
  }
  self->get_product_list_callback_ = nullptr;
}

void BillingManager::OnPurchase(const char *detail_result, void *user_data) {
  LOG_INFO("[BillingManager] Purchaselist: %s", detail_result);

  BillingManager *self = reinterpret_cast<BillingManager *>(user_data);

  if (self->get_purchase_list_callback_) {
    rapidjson::Document doc;
    doc.Parse(detail_result);
    if (doc.HasParseError()) {
      self->get_purchase_list_callback_(
          FlutterError("Operation failed", "OnPurchase parse error."));
      self->get_purchase_list_callback_ = nullptr;
      return;
    }
    std::string cp_status =
        GetJsonValue<std::string>(doc, "CPStatus", "Unknown");
    std::string cp_result =
        GetJsonValue<std::string>(doc, "CPResult", "Unknown");
    int64_t total_count = GetJsonValue<int64_t>(doc, "TotalCount", -1);
    std::string check_value =
        GetJsonValue<std::string>(doc, "CheckValue", "Unknown");
    flutter::EncodableList invoice_details;
    {
      rapidjson::Document empty_doc;
      empty_doc.SetArray();
      const rapidjson::Value &default_array = empty_doc;
      const rapidjson::Value &jarray = GetJsonValue<const rapidjson::Value &>(
          doc, "InvoiceDetails", default_array);
      for (rapidjson::SizeType i = 0; i < jarray.Size(); ++i) {
        int64_t seq = GetJsonValue<int64_t>(jarray[i], "Seq", -1);
        std::string invoice_id =
            GetJsonValue<std::string>(jarray[i], "InvoiceID", "Unknown");
        std::string item_id =
            GetJsonValue<std::string>(jarray[i], "ItemID", "Unknown");
        std::string item_title =
            GetJsonValue<std::string>(jarray[i], "ItemTitle", "Unknown");
        int64_t item_type = GetJsonValue<int64_t>(jarray[i], "ItemType" - 1);
        std::string order_time =
            GetJsonValue<std::string>(jarray[i], "OrderTime", "Unknown");
        int64_t period = GetJsonValue<int64_t>(jarray[i], "Period", -1);
        std::variant<int, double> price =
            GetJsonValue<std::variant<int, double>>(jarray[i], "Price", -1);
        std::string order_currency_id =
            GetJsonValue<std::string>(jarray[i], "OrderCurrencyID", "Unknown");
        bool cancel_status =
            GetJsonValue<bool>(jarray[i], "CancelStatus", false);
        bool applied_status =
            GetJsonValue<bool>(jarray[i], "AppliedStatus", false);
        std::string applied_time =
            GetJsonValue<std::string>(jarray[i], "AppliedTime", "Unknown");
        std::string limit_end_time =
            GetJsonValue<std::string>(jarray[i], "LimitEndTime", "Unknown");
        std::string remain_time =
            GetJsonValue<std::string>(jarray[i], "RemainTime", "Unknown");

        flutter::EncodableValue detail =
            flutter::EncodableValue(flutter::EncodableMap{
                {flutter::EncodableValue("Seq"), flutter::EncodableValue(seq)},
                {flutter::EncodableValue("InvoiceID"),
                 flutter::EncodableValue(invoice_id)},
                {flutter::EncodableValue("ItemID"),
                 flutter::EncodableValue(item_id)},
                {flutter::EncodableValue("ItemTitle"),
                 flutter::EncodableValue(item_title)},
                {flutter::EncodableValue("ItemType"),
                 flutter::EncodableValue(item_type)},
                {flutter::EncodableValue("OrderTime"),
                 flutter::EncodableValue(order_time)},
                {flutter::EncodableValue("Period"),
                 flutter::EncodableValue(period)},
                {flutter::EncodableValue("Price"),
                 ConvertVariantToEncodable(price)},
                {flutter::EncodableValue("OrderCurrencyID"),
                 flutter::EncodableValue(order_currency_id)},
                {flutter::EncodableValue("CancelStatus"),
                 flutter::EncodableValue(cancel_status)},
                {flutter::EncodableValue("AppliedStatus"),
                 flutter::EncodableValue(applied_status)},
                {flutter::EncodableValue("AppliedTime"),
                 flutter::EncodableValue(applied_time)},
                {flutter::EncodableValue("LimitEndTime"),
                 flutter::EncodableValue(limit_end_time)},
                {flutter::EncodableValue("RemainTime"),
                 flutter::EncodableValue(remain_time)},
            });
        invoice_details.push_back(detail);
      }
    }
    GetUserPurchaseListAPIResult purchase_list(
        cp_status, &cp_result, total_count, check_value, invoice_details);
    self->get_purchase_list_callback_(purchase_list);
  } else {
    self->get_purchase_list_callback_(FlutterError(
        "Invalid argument", "get_purchase_list_callback_ is null !"));
  }
  self->get_purchase_list_callback_ = nullptr;
}

bool BillingManager::OnBuyItem(const char *pay_result, const char *detail_info,
                               void *user_data) {
  LOG_INFO("[BillingManager] Buy items result: %s, result details: %s",
           pay_result, detail_info);

  BillingManager *self = reinterpret_cast<BillingManager *>(user_data);

  if (self->buy_item_callback_) {
    size_t len = strlen(pay_result);
    std::string pay_res(pay_result, len);

    flutter::EncodableMap pay_details;
    rapidjson::Document doc;
    doc.Parse(detail_info);
    if (doc.HasParseError()) {
      self->buy_item_callback_(
          FlutterError("Operation failed", "OnBuyItem parse error."));
      self->buy_item_callback_ = nullptr;
      return false;
    }
    for (auto it = doc.MemberBegin(); it != doc.MemberEnd(); ++it) {
      const std::string key = it->name.GetString();
      if (it->value.IsString()) {  // Only string now.
        pay_details[flutter::EncodableValue(key)] =
            flutter::EncodableValue(it->value.GetString());
      } else if (it->value.IsInt()) {
        pay_details[flutter::EncodableValue(key)] =
            flutter::EncodableValue(it->value.GetInt());
      } else if (it->value.IsDouble()) {
        pay_details[flutter::EncodableValue(key)] =
            flutter::EncodableValue(it->value.GetDouble());
      } else if (it->value.IsBool()) {
        pay_details[flutter::EncodableValue(key)] =
            flutter::EncodableValue(it->value.GetBool());
      }
    }
    BillingBuyData buy_data(pay_res, pay_details);
    self->buy_item_callback_(buy_data);
  } else {
    self->buy_item_callback_(
        FlutterError("Invalid argument", "buy_item_callback_ is null !"));
  }
  self->buy_item_callback_ = nullptr;
  return true;
}

void BillingManager::OnVerify(const char *detail_result, void *user_data) {
  LOG_INFO("[BillingManager] Verify details: %s", detail_result);

  BillingManager *self = reinterpret_cast<BillingManager *>(user_data);

  if (self->verify_invoice_callback_) {
    rapidjson::Document doc;
    doc.Parse(detail_result);
    if (doc.HasParseError()) {
      self->verify_invoice_callback_(
          FlutterError("Operation failed", "OnVerify parse error."));
      self->verify_invoice_callback_ = nullptr;
      return;
    }
    std::string cp_status =
        GetJsonValue<std::string>(doc, "CPStatus", "Unknown");
    std::string cp_result =
        GetJsonValue<std::string>(doc, "CPResult", "Unknown");
    std::string app_id = GetJsonValue<std::string>(doc, "AppID", "Unknown");
    std::string invoice_id =
        GetJsonValue<std::string>(doc, "InvoiceID", "Unknown");
    VerifyInvoiceAPIResult verify_invoice(cp_status, &cp_result, app_id,
                                          invoice_id);
    self->verify_invoice_callback_(verify_invoice);
  } else {
    self->verify_invoice_callback_(
        FlutterError("Invalid argument", "verify_invoice_callback_ is null !"));
  }
  self->verify_invoice_callback_ = nullptr;
}

}  // namespace in_app_purchase_tizen
