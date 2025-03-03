// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "billing_manager.h"

#include <dlfcn.h>
#include <flutter/standard_method_codec.h>

#include <cassert>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "log.h"

static std::string ServerTypeToString(billing_server_type server_type) {
  switch (server_type) {
    case SERVERTYPE_OPERATE:
      return "PRD";
      break;
    case SERVERTYPE_DEV:
      return "DEV";
      break;
    default:
      return "NONE";
      break;
  }
}

// std::function<void(ErrorOr<T>) <--> void *
template <typename T>
struct FunctionBox {
  std::function<void(ErrorOr<T>)> handler;
};

// template <typename T>
// static std::unique_ptr<void, void (*)(void *)> pack_function(
//     std::function<void(ErrorOr<T>)> func) {
//   auto deleter = [](void *ptr) { delete static_cast<FunctionBox<T> *>(ptr);
//   }; return {new FunctionBox<T>{std::move(func)}, deleter};
// }

template <typename T>
static void *pack_function(std::function<void(ErrorOr<T>)> func) {
  return new FunctionBox<T>{std::move(func)};
}

template <typename T>
static std::function<void(ErrorOr<T>)> unpack_function(void *boxed) {
  auto *wrapper = static_cast<FunctionBox<T> *>(boxed);
  return wrapper ? wrapper->handler : nullptr;
}
// std::function<void(ErrorOr<T>) <--> void *

bool BillingManager::Init() {
  LOG_INFO("[BillingManager] Init billing api.");

  if (!BillingWrapper::GetInstance().Initialize()) {
    LOG_ERROR("[BillingManager] Fail to initialize billing APIs.");
    return false;
  }
  return true;
}

std::string BillingManager::GetCustomId() {
  void *handle = dlopen("libsso_api.so", RTLD_LAZY);
  std::string custom_id = "";
  if (!handle) {
    LOG_ERROR("[BillingManager] Fail to open sso APIs.");
  } else {
    FuncSsoGetLoginInfo sso_get_login_info =
        reinterpret_cast<FuncSsoGetLoginInfo>(
            dlsym(handle, "sso_get_login_info"));
    if (sso_get_login_info) {
      sso_login_info_s login_info;
      if (!sso_get_login_info(&login_info)) {
        custom_id = login_info.uid;
      }
    }
    dlclose(handle);
  }
  return custom_id;
}

std::string BillingManager::GetCountryCode() {
  void *handle = dlopen("libvconf.so.0.3.1", RTLD_LAZY);
  char *country_code = "";
  if (!handle) {
    LOG_ERROR("[BillingManager] Fail to open vconf APIs.");
  } else {
    FuncVconfGetStr vconf_get_str =
        reinterpret_cast<FuncVconfGetStr>(dlsym(handle, "vconf_get_str"));
    if (vconf_get_str) {
      country_code = vconf_get_str("db/comss/countrycode");
    }
    dlclose(handle);
  }
  return country_code;
}

bool BillingManager::IsAvailable(
    std::function<void(ErrorOr<bool> reply)> result) {
  LOG_INFO("[BillingManager] Check billing server is available.");

  void *handle = dlopen("libcapi-system-info.so.0.2.1", RTLD_LAZY);
  if (!handle) {
    LOG_ERROR("[BillingManager] Fail to open system APIs.");
  } else {
    FuncSystemInfGetValueInt system_info_get_value_int =
        reinterpret_cast<FuncSystemInfGetValueInt>(
            dlsym(handle, "system_info_get_value_int"));
    if (system_info_get_value_int) {
      int tv_server_type = 0;
      int ret = system_info_get_value_int(SYSTEM_INFO_KEY_INFO_LINK_SERVER_TYPE,
                                          &tv_server_type);
      if (ret == SYSTEM_INFO_ERROR_NONE) {
        switch (tv_server_type) {
          case PRD:
            billing_server_type_ = SERVERTYPE_OPERATE;
            break;
          case DEV:
            billing_server_type_ = SERVERTYPE_DEV;
            break;
          default:
            billing_server_type_ = SERVERTYPE_NONE;
            break;
        }
        LOG_INFO("[BillingManager] Billing Server Type is %d",
                 billing_server_type_);
      } else {
        LOG_ERROR("[BillingManager] Fail to get TV server type.");
        dlclose(handle);
        return false;
      }
    }
    dlclose(handle);
  }

  auto result_ptr = pack_function<bool>(result);
  bool ret = BillingWrapper::GetInstance().service_billing_is_service_available(
      billing_server_type_, OnAvailable, result_ptr);
  delete (result_ptr);
  if (!ret) {
    LOG_ERROR("[BillingManager] service_billing_is_service_available failed.");
    return false;
  }
  return true;
}

bool BillingManager::GetProductList(
    const char *app_id, const char *country_code, int page_size,
    int page_number, const char *check_value,
    std::function<void(ErrorOr<ProductsListApiResult> reply)> result) {
  LOG_INFO("[BillingManager] Start get product list.");

  auto result_ptr = pack_function<ProductsListApiResult>(result);
  bool ret = BillingWrapper::GetInstance().service_billing_get_products_list(
      app_id, country_code, page_size, page_number, check_value,
      billing_server_type_, OnProducts, result_ptr);
  delete (result_ptr);
  if (!ret) {
    LOG_ERROR("[BillingManager] service_billing_get_products_list failed.");
    return false;
  }
  return true;
}

bool BillingManager::GetPurchaseList(
    const char *app_id, const char *custom_id, const char *country_code,
    int page_number, const char *check_value,
    std::function<void(ErrorOr<GetUserPurchaseListAPIResult> reply)> result) {
  LOG_INFO("[BillingManager] Start get purchase list.");

  auto result_ptr = pack_function<GetUserPurchaseListAPIResult>(result);
  bool ret = BillingWrapper::GetInstance().service_billing_get_purchase_list(
      app_id, custom_id, country_code, page_number, check_value,
      billing_server_type_, OnPurchase, result_ptr);
  delete (result_ptr);
  if (!ret) {
    LOG_ERROR("[BillingManager] service_billing_get_purchase_list failed.");
    return false;
  }
  return true;
}

bool BillingManager::BuyItem(
    const char *app_id, const char *detail_info,
    std::function<void(ErrorOr<BillingBuyData> reply)> result) {
  LOG_INFO("[BillingManager] Start buy item");

  auto result_ptr = pack_function<BillingBuyData>(result);
  bool ret = BillingWrapper::GetInstance().service_billing_buyitem(
      app_id, ServerTypeToString(billing_server_type_).c_str(), detail_info);
  BillingWrapper::GetInstance().service_billing_set_buyitem_cb(OnBuyItem,
                                                               result_ptr);
  delete (result_ptr);
  if (!ret) {
    LOG_ERROR("[BillingManager] service_billing_buyitem failed.");
    return false;
  }
  return true;
}

bool BillingManager::VerifyInvoice(
    const char *app_id, const char *custom_id, const char *invoice_id,
    const char *country_code,
    std::function<void(ErrorOr<VerifyInvoiceAPIResult> reply)> result) {
  LOG_INFO("[BillingManager] Start verify invoice");

  auto result_ptr = pack_function<VerifyInvoiceAPIResult>(result);
  bool ret = BillingWrapper::GetInstance().service_billing_verify_invoice(
      app_id, custom_id, invoice_id, country_code, billing_server_type_,
      OnVerify, result_ptr);
  delete (result_ptr);
  if (!ret) {
    LOG_ERROR("[BillingManager] service_billing_verify_invoice failed.");
    return false;
  }
  return true;
}

void BillingManager::OnAvailable(const char *detail_result, void *user_data) {
  LOG_INFO("[BillingManager] Billing server detail_result: %s", detail_result);

  std::function<void(ErrorOr<bool> reply)> result =
      unpack_function<bool>(user_data);

  if (result) {
    bool res = false;

    JsonParser *parser = json_parser_new();
    GError *error = nullptr;
    if (!json_parser_load_from_data(parser, detail_result, -1, &error)) {
      g_object_unref(parser);
      throw std::runtime_error(error->message);
    }

    JsonNode *root = json_parser_get_root(parser);
    JsonObject *obj = json_node_get_object(root);

    std::string status = g_strdup(json_object_get_string_member(obj, "status"));
    g_object_unref(parser);

    if (status == "100000") res = true;
    result(res);
  } else {
    result(FlutterError("OnAvailable Failed", "method result is null !"));
  }
}

void BillingManager::OnProducts(const char *detail_result, void *user_data) {
  LOG_INFO("[BillingManager] Productlist: %s", detail_result);

  std::function<void(ErrorOr<ProductsListApiResult> reply)> result =
      unpack_function<ProductsListApiResult>(user_data);

  if (result) {
    JsonParser *parser = json_parser_new();
    GError *error = nullptr;
    if (!json_parser_load_from_data(parser, detail_result, -1, &error)) {
      g_object_unref(parser);
      throw std::runtime_error(error->message);
    }

    JsonNode *root = json_parser_get_root(parser);
    JsonObject *obj = json_node_get_object(root);

    std::string cp_status =
        g_strdup(json_object_get_string_member(obj, "CPStatus"));
    std::string cp_result =
        g_strdup(json_object_get_string_member(obj, "CPResult"));
    int64_t total_count = json_object_get_int_member(obj, "TotalCount");
    std::string check_value =
        g_strdup(json_object_get_string_member(obj, "CheckValue"));
    flutter::EncodableList item_details;
    {
      JsonArray *jarray = json_object_get_array_member(obj, "ItemDetails");
      for (guint i = 0; i < json_array_get_length(jarray); ++i) {
        JsonObject *item = json_array_get_object_element(jarray, i);

        int64_t seq = json_object_get_int_member(item, "Seq");
        std::string item_id =
            g_strdup(json_object_get_string_member(item, "ItemID"));
        std::string item_title =
            g_strdup(json_object_get_string_member(item, "ItemTitle"));
        std::string item_desc =
            g_strdup(json_object_get_string_member(item, "ItemDesc"));
        int64_t item_type = json_object_get_int_member(item, "ItemType");
        double price = json_object_get_double_member(item, "Price");
        std::string currency_id =
            g_strdup(json_object_get_string_member(item, "CurrencyID"));

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
                 flutter::EncodableValue(price)},
                {flutter::EncodableValue("CurrencyID"),
                 flutter::EncodableValue(currency_id)},
            });
        item_details.push_back(detail);
      }
    }
    g_object_unref(parser);
    ProductsListApiResult products_list(cp_status, &cp_result, total_count,
                                        check_value, item_details);
    result(products_list);
  } else {
    result(FlutterError("OnProducts Failed", "method result is null !"));
  }
}

void BillingManager::OnPurchase(const char *detail_result, void *user_data) {
  LOG_INFO("[BillingManager] Purchaselist: %s", detail_result);

  std::function<void(ErrorOr<GetUserPurchaseListAPIResult> reply)> result =
      unpack_function<GetUserPurchaseListAPIResult>(user_data);
  if (result) {
    JsonParser *parser = json_parser_new();
    GError *error = nullptr;
    if (!json_parser_load_from_data(parser, detail_result, -1, &error)) {
      g_object_unref(parser);
      throw std::runtime_error(error->message);
    }

    JsonNode *root = json_parser_get_root(parser);
    JsonObject *obj = json_node_get_object(root);

    std::string cp_status =
        g_strdup(json_object_get_string_member(obj, "CPStatus"));
    std::string cp_result =
        g_strdup(json_object_get_string_member(obj, "CPResult"));
    int64_t total_count = json_object_get_int_member(obj, "TotalCount");
    std::string check_value =
        g_strdup(json_object_get_string_member(obj, "CheckValue"));
    flutter::EncodableList invoice_details;
    {
      JsonArray *jarray = json_object_get_array_member(obj, "InvoiceDetails");
      for (guint i = 0; i < json_array_get_length(jarray); ++i) {
        JsonObject *item = json_array_get_object_element(jarray, i);

        int64_t seq = json_object_get_int_member(item, "Seq");
        std::string invoice_id =
            g_strdup(json_object_get_string_member(item, "InvoiceID"));
        std::string item_id =
            g_strdup(json_object_get_string_member(item, "ItemID"));
        std::string item_title =
            g_strdup(json_object_get_string_member(item, "ItemTitle"));
        int64_t item_type = json_object_get_int_member(item, "ItemType");
        std::string order_time =
            g_strdup(json_object_get_string_member(item, "OrderTime"));
        int64_t period = json_object_get_int_member(item, "Period");
        double price = json_object_get_double_member(item, "Price");
        std::string order_currency_id =
            g_strdup(json_object_get_string_member(item, "OrderCurrencyID"));
        bool cancel_status =
            json_object_get_boolean_member(item, "CancelStatus");
        bool applied_status =
            json_object_get_boolean_member(item, "AppliedStatus");
        std::string applied_time =
            g_strdup(json_object_get_string_member(item, "AppliedTime"));
        std::string limit_end_time =
            g_strdup(json_object_get_string_member(item, "LimitEndTime"));
        std::string remain_time =
            g_strdup(json_object_get_string_member(item, "RemainTime"));

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
                 flutter::EncodableValue(price)},
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
    g_object_unref(parser);
    GetUserPurchaseListAPIResult purchase_list(
        cp_status, &cp_result, &total_count, &check_value, invoice_details);
    result(purchase_list);
  } else {
    result(FlutterError("OnPurchase Failed", "method result is null !"));
  }
}

bool BillingManager::OnBuyItem(const char *pay_result, const char *detail_info,
                               void *user_data) {
  LOG_INFO("[BillingManager] Buy items result: %s, result details: %s",
           pay_result, detail_info);

  std::function<void(ErrorOr<BillingBuyData> reply)> result =
      unpack_function<BillingBuyData>(user_data);

  if (result) {
    size_t len = strlen(pay_result);
    std::string pay_res(pay_result, len);

    flutter::EncodableMap pay_details;
    JsonParser *parser = json_parser_new();
    GError *error = nullptr;
    if (!json_parser_load_from_data(parser, detail_info, -1, &error)) {
      g_object_unref(parser);
      throw std::runtime_error(error->message);
    }
    JsonNode *root = json_parser_get_root(parser);
    JsonObject *obj = json_node_get_object(root);
    GList *members = json_object_get_members(obj);
    for (GList *iter = members; iter != NULL; iter = iter->next) {
      const gchar *key = (const gchar *)iter->data;
      std::string value = g_strdup(json_object_get_string_member(obj, key));
      pay_details[flutter::EncodableValue(key)] =
          flutter::EncodableValue(value);
    }

    g_object_unref(parser);
    BillingBuyData buy_data(pay_res, pay_details);
    result(buy_data);
  } else {
    result(FlutterError("OnBuyItem Failed", "method result is null !"));
  }
  return true;
}

void BillingManager::OnVerify(const char *detail_result, void *user_data) {
  LOG_INFO("[BillingManager] Verify details: %s", detail_result);

  std::function<void(ErrorOr<VerifyInvoiceAPIResult> reply)> result =
      unpack_function<VerifyInvoiceAPIResult>(user_data);
  if (result) {
    JsonParser *parser = json_parser_new();
    GError *error = nullptr;
    if (!json_parser_load_from_data(parser, detail_result, -1, &error)) {
      g_object_unref(parser);
      throw std::runtime_error(error->message);
    }

    JsonNode *root = json_parser_get_root(parser);
    JsonObject *obj = json_node_get_object(root);

    std::string cp_status =
        g_strdup(json_object_get_string_member(obj, "CPStatus"));
    std::string cp_result =
        g_strdup(json_object_get_string_member(obj, "CPResult"));
    std::string app_id = g_strdup(json_object_get_string_member(obj, "AppID"));
    std::string invoice_id =
        g_strdup(json_object_get_string_member(obj, "InvoiceID"));

    g_object_unref(parser);
    VerifyInvoiceAPIResult verify_invoice(cp_status, &cp_result, app_id,
                                          invoice_id);
    result(verify_invoice);
  } else {
    result(FlutterError("OnVerify Failed", "method result is null !"));
  }
}

void BillingManager::Dispose() {
  LOG_INFO("[BillingManager] Dispose billing.");

  basic_method_channel_->SetMessageHandler(nullptr);
}
