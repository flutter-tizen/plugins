// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_BILLING_SERVICE_PROXY_H_
#define FLUTTER_PLUGIN_BILLING_SERVICE_PROXY_H_

#define SSO_API_MAX_STRING_LEN 128

typedef enum server_type {
  SERVERTYPE_OPERATE = 10005,
  SERVERTYPE_DEV,
  SERVERTYPE_WORKING,
  SERVERTYPE_DUMMY,
  SERVERTYPE_NONE
} SERVERTYPE;

typedef struct sso_login_info {
  char login_id[SSO_API_MAX_STRING_LEN];
  char login_pwd[SSO_API_MAX_STRING_LEN];
  char login_guid[SSO_API_MAX_STRING_LEN];
  char uid[SSO_API_MAX_STRING_LEN];
  char user_icon[SSO_API_MAX_STRING_LEN * 8];
} sso_login_info_s;

typedef void (*billing_payment_api_cb)(const char *detail_result,
                                       void *user_data);
typedef bool (*billing_buyitem_cb)(const char *pay_result,
                                   const char *detail_info, void *user_data);
typedef bool (*FuncGetProductslist)(const char *app_id,
                                    const char *country_code, int page_size,
                                    int page_number, const char *check_value,
                                    SERVERTYPE server_type,
                                    billing_payment_api_cb callback,
                                    void *user_data);
typedef bool (*FuncGetpurchaselist)(const char *app_id, const char *custom_id,
                                    const char *country_code, int page_number,
                                    const char *check_value,
                                    SERVERTYPE server_type,
                                    billing_payment_api_cb callback,
                                    void *user_data);
typedef bool (*FuncBillingBuyItem)(const char *app_id, const char *server_type,
                                   const char *detail_info);
typedef void (*FuncBillingSetBuyItemCb)(billing_buyitem_cb callback,
                                        void *user_data);
typedef bool (*FuncServiceBillingIsServiceAvailable)(
    SERVERTYPE server_type, billing_payment_api_cb callback, void *user_data);
typedef bool (*FuncServiceBillingVerifyInvoice)(
    const char *app_id, const char *custom_id, const char *invoice_id,
    const char *country_code, SERVERTYPE server_type,
    billing_payment_api_cb callback, void *user_data);
typedef bool (*FuncSsoGetLoginInfo)(sso_login_info_s *login_info);

void *OpenBillingApi();
void *OpenSsoApi();

int InitBillingApi(void *handle);
int InitSsoApi(void *handle);
void CloseApi(void *handle);

extern FuncGetProductslist service_billing_get_products_list;
extern FuncGetpurchaselist service_billing_get_purchase_list;
extern FuncBillingBuyItem service_billing_buyitem;
extern FuncBillingSetBuyItemCb service_billing_set_buyitem_cb;
extern FuncServiceBillingIsServiceAvailable
    service_billing_is_service_available;
extern FuncServiceBillingVerifyInvoice service_billing_verify_invoice;
extern FuncSsoGetLoginInfo sso_get_login_info;

#endif  // FLUTTER_PLUGIN_BILLING_SERVICE_PROXY_H_
