// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef FLUTTER_TIZEN_PLUGINS_WRAPPER_IN_APP_PURCHASE_H_
#define FLUTTER_TIZEN_PLUGINS_WRAPPER_IN_APP_PURCHASE_H_

#include <stdbool.h>
#include <stdint.h>

typedef void (*billing_payment_api_cb)(const char *detail_result,
                                       void *user_data);
typedef bool (*billing_buyitem_cb)(const char *pay_result,
                                   const char *detail_info, void *user_data);

typedef struct sso_login_info {
  char login_id[128];
  char login_pwd[128];
  char login_guid[128];
  char uid[128];
  char user_icon[128 * 8];
} sso_login_info_s;

#ifdef __cplusplus
extern "C" {
#endif

// Native libraries remain loaded for the process lifetime (callbacks may
// outlive calls).

bool ftpw_in_app_purchase_service_billing_get_products_list(
    const char *app_id, const char *country_code, int page_size,
    int page_number, const char *check_value, billing_payment_api_cb callback,
    void *user_data);
bool ftpw_in_app_purchase_service_billing_get_purchase_list(
    const char *app_id, const char *custom_id, const char *country_code,
    int page_number, const char *check_value, billing_payment_api_cb callback,
    void *user_data);
bool ftpw_in_app_purchase_service_billing_is_service_available(
    billing_payment_api_cb callback, void *user_data);
bool ftpw_in_app_purchase_service_billing_buyitem(const char *app_id,
                                                  const char *detail_info);
void ftpw_in_app_purchase_service_billing_set_buyitem_cb(
    billing_buyitem_cb callback, void *user_data);
bool ftpw_in_app_purchase_service_billing_verify_invoice(
    const char *app_id, const char *custom_id, const char *invoice_id,
    const char *country_code, billing_payment_api_cb callback, void *user_data);
bool ftpw_in_app_purchase_sso_get_login_info(sso_login_info_s *login_info);
char *ftpw_in_app_purchase_get_county_code();

#ifdef __cplusplus
}
#endif

#endif  // FLUTTER_TIZEN_PLUGINS_WRAPPER_IN_APP_PURCHASE_H_
