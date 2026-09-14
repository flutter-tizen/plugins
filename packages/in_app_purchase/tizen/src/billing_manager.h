// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_BILLING_MANAGER_H
#define FLUTTER_PLUGIN_BILLING_MANAGER_H

#include <tizen_error.h>

#include <cassert>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <variant>

#include "billing_service_proxy.h"
#include "messages.h"
#include "rapidjson/document.h"

#define SSO_API_MAX_STRING_LEN 128

namespace in_app_purchase_tizen {

typedef struct sso_login_info {
  char login_id[SSO_API_MAX_STRING_LEN];
  char login_pwd[SSO_API_MAX_STRING_LEN];
  char login_guid[SSO_API_MAX_STRING_LEN];
  char uid[SSO_API_MAX_STRING_LEN];
  char user_icon[SSO_API_MAX_STRING_LEN * 8];
} sso_login_info_s;

typedef enum {
  PRD = 0,
  DEV,
} tv_server_type;

typedef enum {
  SYSTEM_INFO_ERROR_NONE = TIZEN_ERROR_NONE, /**< Successful */
  SYSTEM_INFO_ERROR_INVALID_PARAMETER =
      TIZEN_ERROR_INVALID_PARAMETER, /**< Invalid parameter */
  SYSTEM_INFO_ERROR_OUT_OF_MEMORY =
      TIZEN_ERROR_OUT_OF_MEMORY, /**< Out of memory */
  SYSTEM_INFO_ERROR_IO_ERROR =
      TIZEN_ERROR_IO_ERROR, /**< An input/output error occurred when reading
                               value from system */
  SYSTEM_INFO_ERROR_PERMISSION_DENIED =
      TIZEN_ERROR_PERMISSION_DENIED, /**< No permission to use the API */
  SYSTEM_INFO_ERROR_NOT_SUPPORTED =
      TIZEN_ERROR_NOT_SUPPORTED, /**< Not supported parameter (Since 3.0) */
} system_info_error_e;

typedef enum {
  SYSTEM_INFO_KEY_INFO_LINK_SERVER_TYPE = 126,
} system_info_key_e;

// Returns SSORESULT: 0 on success, -1 on failure.
typedef int (*FuncSsoGetLoginInfo)(sso_login_info_s *login_info);
typedef char *(*FuncVconfGetStr)(const char *in_key);
typedef int (*FuncSystemInfGetValueInt)(system_info_key_e key, int *value);

template <typename T>
using FunctionResult = std::function<void(ErrorOr<T>)>;

template <typename T>
struct AlwaysFalseType : std::false_type {};

template <typename T>
T GetJsonValue(const rapidjson::Value &doc, const char *key,
               const T &default_val = T()) {
  auto itr = doc.FindMember(key);
  if (itr == doc.MemberEnd()) {
    return default_val;  // when key is not exist, return default_val
  }

  const rapidjson::Value &value = itr->value;

  if constexpr (std::is_same_v<T, std::string>) {
    return value.IsString() ? value.GetString() : default_val;
  } else if constexpr (std::is_same_v<T, int64_t>) {
    return value.IsInt() ? value.GetInt() : default_val;
  } else if constexpr (std::is_same_v<T, bool>) {
    return value.IsBool() ? value.GetBool() : default_val;
  } else if constexpr (std::is_same_v<T, double>) {
    return value.IsDouble() ? value.GetDouble() : default_val;
  } else if constexpr (std::is_same_v<T, const rapidjson::Value &>) {
    return value.IsArray() ? value : default_val;
  } else if constexpr (std::is_same_v<T, std::variant<int, double>>) {
    if (value.IsInt())
      return value.GetInt();
    else if (value.IsDouble())
      return value.GetDouble();
    else
      return default_val;
  } else {
    static_assert(AlwaysFalseType<T>::value, "Unsupported type");
  }
}

class BillingManager {
 public:
  explicit BillingManager() {}
  ~BillingManager(){};

  bool Init();
  void Dispose();
  bool IsAvailable(FunctionResult<bool> result);
  bool BuyItem(const char *app_id, const char *detail_info,
               FunctionResult<BillingBuyData> result);
  bool GetProductList(const char *app_id, const char *country_code,
                      int page_size, int page_number, const char *check_value,
                      FunctionResult<ProductsListApiResult> result);
  bool GetPurchaseList(const char *app_id, const char *custom_id,
                       const char *country_code, int page_number,
                       const char *check_value,
                       FunctionResult<GetUserPurchaseListAPIResult> result);
  bool VerifyInvoice(const char *app_id, const char *custom_id,
                     const char *invoice_id, const char *country_code,
                     FunctionResult<VerifyInvoiceAPIResult> result);
  std::optional<std::string> GetCustomId();
  std::string GetCountryCode();

 private:
  static void OnProducts(const char *detail_result, void *user_data);
  static void OnPurchase(const char *detail_result, void *user_data);
  static bool OnBuyItem(const char *pay_result, const char *detail_info,
                        void *user_data);
  static void OnAvailable(const char *detail_result, void *user_data);
  static void OnVerify(const char *detail_result, void *user_data);

  billing_server_type billing_server_type_;

  FunctionResult<bool> is_available_callback_;
  FunctionResult<BillingBuyData> buy_item_callback_;
  FunctionResult<ProductsListApiResult> get_product_list_callback_;
  FunctionResult<GetUserPurchaseListAPIResult> get_purchase_list_callback_;
  FunctionResult<VerifyInvoiceAPIResult> verify_invoice_callback_;
  std::mutex mutex_;
};

}  // namespace in_app_purchase_tizen

#endif  // FLUTTER_PLUGIN_BILLING_MANAGER_H
