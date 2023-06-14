// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "billing_service_proxy.h"

#include <dlfcn.h>

FuncGetProductslist service_billing_get_products_list = nullptr;
FuncGetpurchaselist service_billing_get_purchase_list = nullptr;
FuncServiceBillingIsServiceAvailable service_billing_is_service_available =
    nullptr;
FuncBillingBuyItem service_billing_buyitem = nullptr;
FuncBillingSetBuyItemCb service_billing_set_buyitem_cb = nullptr;
FuncServiceBillingVerifyInvoice service_billing_verify_invoice = nullptr;
FuncSsoGetLoginInfo sso_get_login_info = nullptr;

void *OpenBillingApi() { return dlopen("libbilling_api.so", RTLD_LAZY); }
void *OpenSsoApi() { return dlopen("libsso_api.so", RTLD_LAZY); }

int InitBillingApi(void *handle) {
  if (!handle) {
    return 0;
  }

  service_billing_get_products_list = reinterpret_cast<FuncGetProductslist>(
      dlsym(handle, "service_billing_get_products_list"));
  if (!service_billing_get_products_list) {
    return 0;
  }

  service_billing_get_purchase_list = reinterpret_cast<FuncGetpurchaselist>(
      dlsym(handle, "service_billing_get_purchase_list"));
  if (!service_billing_get_purchase_list) {
    return 0;
  }

  service_billing_is_service_available =
      reinterpret_cast<FuncServiceBillingIsServiceAvailable>(
          dlsym(handle, "service_billing_is_service_available"));
  if (!service_billing_is_service_available) {
    return 0;
  }

  service_billing_buyitem = reinterpret_cast<FuncBillingBuyItem>(
      dlsym(handle, "service_billing_buyitem"));
  if (!service_billing_buyitem) {
    return 0;
  }

  service_billing_set_buyitem_cb = reinterpret_cast<FuncBillingSetBuyItemCb>(
      dlsym(handle, "service_billing_set_buyitem_cb"));
  if (!service_billing_set_buyitem_cb) {
    return 0;
  }

  service_billing_verify_invoice =
      reinterpret_cast<FuncServiceBillingVerifyInvoice>(
          dlsym(handle, "service_billing_verify_invoice"));
  if (!service_billing_verify_invoice) {
    return 0;
  }

  return 1;
}

int InitSsoApi(void *handle) {
  sso_get_login_info = reinterpret_cast<FuncSsoGetLoginInfo>(
      dlsym(handle, "sso_get_login_info"));
  if (!sso_get_login_info) {
    return 0;
  }
  return 1;
}

void CloseApi(void *handle) {
  if (handle) {
    dlclose(handle);
  }
}
