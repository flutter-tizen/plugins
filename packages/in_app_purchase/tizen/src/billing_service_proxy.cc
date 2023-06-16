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

  service_billing_get_products_list = reinterpret_cast<FuncGetProductslist>(
      dlsym(handle_, "service_billing_get_products_list"));
  service_billing_get_purchase_list = reinterpret_cast<FuncGetpurchaselist>(
      dlsym(handle_, "service_billing_get_purchase_list"));
  service_billing_is_service_available =
      reinterpret_cast<FuncServiceBillingIsServiceAvailable>(
          dlsym(handle_, "service_billing_is_service_available"));
  service_billing_buyitem = reinterpret_cast<FuncBillingBuyItem>(
      dlsym(handle_, "service_billing_buyitem"));
  service_billing_set_buyitem_cb = reinterpret_cast<FuncBillingSetBuyItemCb>(
      dlsym(handle_, "service_billing_set_buyitem_cb"));
  service_billing_verify_invoice =
      reinterpret_cast<FuncServiceBillingVerifyInvoice>(
          dlsym(handle_, "service_billing_verify_invoice"));
  return service_billing_get_products_list &&
         service_billing_get_purchase_list &&
         service_billing_is_service_available && service_billing_buyitem &&
         service_billing_set_buyitem_cb && service_billing_verify_invoice;
}
