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

const char *kInvalidArgument = "Invalid argument";

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

bool BillingManager::BillingIsAvailable(
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
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

  bool ret = BillingWrapper::GetInstance().service_billing_is_service_available(
      billing_server_type_, OnAvailable, result.release());
  if (!ret) {
    LOG_ERROR("[BillingManager] service_billing_is_service_available failed.");
    return false;
  }
  return true;
}

bool BillingManager::GetProductList(
    const char *app_id, const char *country_code, int page_size,
    int page_number, const char *check_value,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  LOG_INFO("[BillingManager] Start get product list.");

  bool ret = BillingWrapper::GetInstance().service_billing_get_products_list(
      app_id, country_code, page_size, page_number, check_value,
      billing_server_type_, OnProducts, result.release());
  if (!ret) {
    LOG_ERROR("[BillingManager] service_billing_get_products_list failed.");
    return false;
  }
  return true;
}

bool BillingManager::GetPurchaseList(
    const char *app_id, const char *custom_id, const char *country_code,
    int page_number, const char *check_value,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  LOG_INFO("[BillingManager] Start get purchase list.");

  bool ret = BillingWrapper::GetInstance().service_billing_get_purchase_list(
      app_id, custom_id, country_code, page_number, check_value,
      billing_server_type_, OnPurchase, result.release());
  if (!ret) {
    LOG_ERROR("[BillingManager] service_billing_get_purchase_list failed.");
    return false;
  }
  return true;
}

bool BillingManager::BuyItem(
    const char *app_id, const char *detail_info,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  LOG_INFO("[BillingManager] Start buy item");

  bool ret = BillingWrapper::GetInstance().service_billing_buyitem(
      app_id, ServerTypeToString(billing_server_type_).c_str(), detail_info);
  BillingWrapper::GetInstance().service_billing_set_buyitem_cb(
      OnBuyItem, result.release());
  if (!ret) {
    LOG_ERROR("[BillingManager] service_billing_buyitem failed.");
    return false;
  }
  return true;
}

bool BillingManager::VerifyInvoice(
    const char *app_id, const char *custom_id, const char *invoice_id,
    const char *country_code,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  LOG_INFO("[BillingManager] Start verify invoice");

  bool ret = BillingWrapper::GetInstance().service_billing_verify_invoice(
      app_id, custom_id, invoice_id, country_code, billing_server_type_,
      OnVerify, result.release());
  if (!ret) {
    LOG_ERROR("[BillingManager] service_billing_verify_invoice failed.");
    return false;
  }
  return true;
}

void BillingManager::OnAvailable(const char *detail_result, void *user_data) {
  LOG_INFO("[BillingManager] Billing server detail_result: %s", detail_result);

  flutter::MethodResult<flutter::EncodableValue> *result =
      reinterpret_cast<flutter::MethodResult<flutter::EncodableValue> *>(
          user_data);
  if (result) {
    result->Success(flutter::EncodableValue(std::string(detail_result)));
  } else {
    result->Error("OnAvailable Failed", "method result is null !");
  }
  delete (result);
}

void BillingManager::OnProducts(const char *detail_result, void *user_data) {
  LOG_INFO("[BillingManager] Productlist: %s", detail_result);

  flutter::MethodResult<flutter::EncodableValue> *result =
      reinterpret_cast<flutter::MethodResult<flutter::EncodableValue> *>(
          user_data);
  if (result) {
    result->Success(flutter::EncodableValue(std::string(detail_result)));
  } else {
    result->Error("OnProducts Failed", "method result is null !");
  }
  delete (result);
}

void BillingManager::OnPurchase(const char *detail_result, void *user_data) {
  LOG_INFO("[BillingManager] Purchaselist: %s", detail_result);

  flutter::MethodResult<flutter::EncodableValue> *result =
      reinterpret_cast<flutter::MethodResult<flutter::EncodableValue> *>(
          user_data);
  if (result) {
    result->Success(flutter::EncodableValue(std::string(detail_result)));
  } else {
    result->Error("OnPurchase Failed", "method result is null !");
  }
  delete (result);
}

bool BillingManager::OnBuyItem(const char *pay_result, const char *detail_info,
                               void *user_data) {
  LOG_INFO("[BillingManager] Buy items result: %s, result details: %s",
           pay_result, detail_info);

  flutter::EncodableMap result_map = {
      {flutter::EncodableValue("PayResult"),
       flutter::EncodableValue(pay_result)},
  };

  flutter::MethodResult<flutter::EncodableValue> *result =
      reinterpret_cast<flutter::MethodResult<flutter::EncodableValue> *>(
          user_data);
  if (result) {
    result->Success(flutter::EncodableValue(result_map));
  } else {
    result->Error("OnBuyItem Failed", "method result is null !");
  }
  delete (result);
}

void BillingManager::OnVerify(const char *detail_result, void *user_data) {
  LOG_INFO("[BillingManager] Verify details: %s", detail_result);

  flutter::MethodResult<flutter::EncodableValue> *result =
      reinterpret_cast<flutter::MethodResult<flutter::EncodableValue> *>(
          user_data);
  if (result) {
    result->Success(flutter::EncodableValue(std::string(detail_result)));
  } else {
    result->Error("OnVerify Failed", "method result is null !");
  }
  delete (result);
}

void BillingManager::Dispose() {
  LOG_INFO("[BillingManager] Dispose billing.");

  method_channel_->SetMethodCallHandler(nullptr);
}
