// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "billing_service_proxy.h"

#include <dlfcn.h>

BillingWrapper::BillingWrapper() {
  handle_ = dlopen("libbilling_api.so", RTLD_LAZY);
}

BillingWrapper::~BillingWrapper() {
  if (handle_) {
    dlclose(handle_);
  }
}

bool BillingWrapper::Initialize() {
  if (!handle_) {
    return false;
  }

  get_products_list = reinterpret_cast<FuncGetProductslist>(
      dlsym(handle_, "service_billing_get_products_list"));
  get_purchase_list = reinterpret_cast<FuncGetpurchaselist>(
      dlsym(handle_, "service_billing_get_purchase_list"));
  is_service_available = reinterpret_cast<FuncIsServiceAvailable>(
      dlsym(handle_, "service_billing_is_service_available"));
  buyitem =
      reinterpret_cast<FuncBuyItem>(dlsym(handle_, "service_billing_buyitem"));
  set_buyitem_cb = reinterpret_cast<FuncSetBuyItemCb>(
      dlsym(handle_, "service_billing_set_buyitem_cb"));
  verify_invoice = reinterpret_cast<FuncVerifyInvoice>(
      dlsym(handle_, "service_billing_verify_invoice"));
  return get_products_list && get_purchase_list && is_service_available &&
         buyitem && set_buyitem_cb && verify_invoice;
}

bool BillingWrapper::service_billing_get_products_list(
    const char *app_id, const char *country_code, int page_size,
    int page_number, const char *check_value, billing_server_type server_type,
    billing_payment_api_cb callback, void *user_data) {
  if (get_products_list) {
    return get_products_list(app_id, country_code, page_size, page_number,
                             check_value, server_type, callback, user_data);
  }
  return false;
}

bool BillingWrapper::service_billing_get_purchase_list(
    const char *app_id, const char *custom_id, const char *country_code,
    int page_number, const char *check_value, billing_server_type server_type,
    billing_payment_api_cb callback, void *user_data) {
  if (get_purchase_list) {
    return get_purchase_list(app_id, custom_id, country_code, page_number,
                             check_value, server_type, callback, user_data);
  }
  return false;
}

bool BillingWrapper::service_billing_buyitem(const char *app_id,
                                             const char *server_type,
                                             const char *detail_info) {
  if (buyitem) {
    return buyitem(app_id, server_type, detail_info);
  }
  return false;
}

void BillingWrapper::service_billing_set_buyitem_cb(billing_buyitem_cb callback,
                                                    void *user_data) {
  if (set_buyitem_cb) {
    return set_buyitem_cb(callback, user_data);
  }
  return;
}

bool BillingWrapper::service_billing_is_service_available(
    billing_server_type server_type, billing_payment_api_cb callback,
    void *user_data) {
  if (is_service_available) {
    return is_service_available(server_type, callback, user_data);
  }
  return false;
}

bool BillingWrapper::service_billing_verify_invoice(
    const char *app_id, const char *custom_id, const char *invoice_id,
    const char *country_code, billing_server_type server_type,
    billing_payment_api_cb callback, void *user_data) {
  if (verify_invoice) {
    return verify_invoice(app_id, custom_id, invoice_id, country_code,
                          server_type, callback, user_data);
  }
  return false;
}
