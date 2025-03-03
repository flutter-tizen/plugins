// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_BILLING_SERVICE_PROXY_H_
#define FLUTTER_PLUGIN_BILLING_SERVICE_PROXY_H_

#include <any>

typedef enum {
  SERVERTYPE_OPERATE = 10005,
  SERVERTYPE_DEV,
  SERVERTYPE_WORKING,
  SERVERTYPE_DUMMY,
  SERVERTYPE_NONE
} billing_server_type;

typedef void (*billing_payment_api_cb)(const char *detail_result,
                                       void *user_data);
typedef bool (*billing_buyitem_cb)(const char *pay_result,
                                   const char *detail_info, void *user_data);
typedef bool (*FuncGetProductslist)(const char *app_id,
                                    const char *country_code, int page_size,
                                    int page_number, const char *check_value,
                                    billing_server_type server_type,
                                    billing_payment_api_cb callback,
                                    void *user_data);
typedef bool (*FuncGetpurchaselist)(const char *app_id, const char *custom_id,
                                    const char *country_code, int page_number,
                                    const char *check_value,
                                    billing_server_type server_type,
                                    billing_payment_api_cb callback,
                                    void *user_data);
typedef bool (*FuncBuyItem)(const char *app_id, const char *server_type,
                            const char *detail_info);
typedef void (*FuncSetBuyItemCb)(billing_buyitem_cb callback, void *user_data);
typedef bool (*FuncIsServiceAvailable)(billing_server_type server_type,
                                       billing_payment_api_cb callback,
                                       void *user_data);
typedef bool (*FuncVerifyInvoice)(const char *app_id, const char *custom_id,
                                  const char *invoice_id,
                                  const char *country_code,
                                  billing_server_type server_type,
                                  billing_payment_api_cb callback,
                                  void *user_data);

class BillingWrapper {
 public:
  static BillingWrapper &GetInstance() {
    static BillingWrapper instance = BillingWrapper();
    return instance;
  }

  ~BillingWrapper();

  BillingWrapper(const BillingWrapper &) = delete;
  BillingWrapper &operator=(const BillingWrapper &) = delete;

  bool Initialize();

  bool service_billing_get_products_list(
      const char *app_id, const char *country_code, int page_size,
      int page_number, const char *check_value, billing_server_type server_type,
      billing_payment_api_cb callback, void *user_data);
  bool service_billing_get_purchase_list(
      const char *app_id, const char *custom_id, const char *country_code,
      int page_number, const char *check_value, billing_server_type server_type,
      billing_payment_api_cb callback, void *user_data);
  bool service_billing_buyitem(const char *app_id, const char *server_type,
                               const char *detail_info);
  void service_billing_set_buyitem_cb(billing_buyitem_cb callback,
                                      void *user_data);
  bool service_billing_is_service_available(billing_server_type server_type,
                                            billing_payment_api_cb callback,
                                            void *user_data);
  bool service_billing_verify_invoice(const char *app_id, const char *custom_id,
                                      const char *invoice_id,
                                      const char *country_code,
                                      billing_server_type server_type,
                                      billing_payment_api_cb callback,
                                      void *user_data);

 private:
  BillingWrapper();

  FuncGetProductslist get_products_list = nullptr;
  FuncGetpurchaselist get_purchase_list = nullptr;
  FuncBuyItem buyitem = nullptr;
  FuncSetBuyItemCb set_buyitem_cb = nullptr;
  FuncIsServiceAvailable is_service_available = nullptr;
  FuncVerifyInvoice verify_invoice = nullptr;

  void *handle_ = nullptr;
};

#endif  // FLUTTER_PLUGIN_BILLING_SERVICE_PROXY_H_
