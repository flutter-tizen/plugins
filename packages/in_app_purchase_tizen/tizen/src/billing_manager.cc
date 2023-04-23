#include "billing_manager.h"

#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>

#include <cassert>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "log.h"

static server_type ConvertServerType(const char *server_type_string) {
  if (strcasecmp("DEV", server_type_string) == 0) {
    return SERVERTYPE_DEV;
  } else if (strcasecmp("OPERATE", server_type_string) == 0) {
    return SERVERTYPE_OPERATE;
  } else if (strcasecmp("WORKING", server_type_string) == 0) {
    return SERVERTYPE_WORKING;
  } else if (strcasecmp("DUMMY", server_type_string) == 0) {
    return SERVERTYPE_DUMMY;
  } else
    return SERVERTYPE_NONE;
}

void BillingManager::Init() {
  billing_api_handle_ = OpenBillingApi();
  if (billing_api_handle_ == nullptr) {
    LOG_ERROR("Fail to open billing api");
  }

  int init_billing_api = InitBillingApi(billing_api_handle_);
  LOG_INFO("init_billing_api:%d", init_billing_api);
  if (init_billing_api == 0) {
    LOG_ERROR(" Fail to init billing api");
  }
}

bool BillingManager::GetProductList(const flutter::EncodableValue *args) {
  LOG_INFO("start get product list");
  std::string app_id;
  std::string country_code;
  std::string item_type;
  int page_size;
  int page_num;
  std::string check_value;
  std::string server_type;
  flutter::EncodableMap encodables = std::get<flutter::EncodableMap>(*args);
  flutter::EncodableValue &app_id_value =
      encodables[flutter::EncodableValue("appId")];
  app_id = std::get<std::string>(app_id_value);
  flutter::EncodableValue &country_code_value =
      encodables[flutter::EncodableValue("countryCode")];
  country_code = std::get<std::string>(country_code_value);
  flutter::EncodableValue &item_type_value =
      encodables[flutter::EncodableValue("itemType")];
  item_type = std::get<std::string>(item_type_value);
  flutter::EncodableValue &page_size_value =
      encodables[flutter::EncodableValue("pageSize")];
  page_size = std::get<int>(page_size_value);
  flutter::EncodableValue &page_num_value =
      encodables[flutter::EncodableValue("pageNum")];
  page_num = std::get<int>(page_num_value);
  flutter::EncodableValue &check_value_value =
      encodables[flutter::EncodableValue("checkValue")];
  check_value = std::get<std::string>(check_value_value);
  flutter::EncodableValue &server_type_value =
      encodables[flutter::EncodableValue("serverType")];
  server_type = std::get<std::string>(server_type_value);

  bool bRet = service_billing_get_products_list(
      app_id.c_str(), country_code.c_str(), page_size, page_num,
      check_value.c_str(), ConvertServerType(server_type.c_str()), OnProducts,
      (void *)this);
  if (!bRet) {
    LOG_ERROR("service_billing_get_products_list failed");
    return false;
  }
  return true;
}

bool BillingManager::GetPurchaseList(const flutter::EncodableValue *args) {
  LOG_INFO("start get purchase list");
  std::string app_id;
  std::string custom_id;
  std::string country_code;
  int page_num;
  std::string check_value;
  std::string server_type;
  flutter::EncodableMap encodables = std::get<flutter::EncodableMap>(*args);
  flutter::EncodableValue &app_id_value =
      encodables[flutter::EncodableValue("appId")];
  app_id = std::get<std::string>(app_id_value);
  flutter::EncodableValue &custom_id_value =
      encodables[flutter::EncodableValue("customId")];
  custom_id = std::get<std::string>(custom_id_value);
  flutter::EncodableValue &country_code_value =
      encodables[flutter::EncodableValue("countryCode")];
  country_code = std::get<std::string>(country_code_value);
  flutter::EncodableValue &page_num_value =
      encodables[flutter::EncodableValue("pageNum")];
  page_num = std::get<int>(page_num_value);
  flutter::EncodableValue &check_value_value =
      encodables[flutter::EncodableValue("checkValue")];
  check_value = std::get<std::string>(check_value_value);
  flutter::EncodableValue &server_type_value =
      encodables[flutter::EncodableValue("serverType")];
  server_type = std::get<std::string>(server_type_value);

  bool bRet = service_billing_get_purchase_list(
      app_id.c_str(), "810000047372", country_code.c_str(), page_num,
      check_value.c_str(), ConvertServerType(server_type.c_str()), OnPurchase,
      (void *)this);
  if (!bRet) {
    LOG_ERROR("service_billing_get_purchase_list failed");
    return false;
  }
  return true;
}

bool BillingManager::BuyItem(const flutter::EncodableValue *args) {
  LOG_INFO("start buy item");
  std::string pay_details;
  std::string app_id;
  std::string server_type;
  if (std::holds_alternative<flutter::EncodableMap>(*args)) {
    flutter::EncodableMap encodables = std::get<flutter::EncodableMap>(*args);
    flutter::EncodableValue &pay_details_value =
        encodables[flutter::EncodableValue("payDetails")];
    pay_details = std::get<std::string>(pay_details_value);
    flutter::EncodableValue &app_id_value =
        encodables[flutter::EncodableValue("appId")];
    app_id = std::get<std::string>(app_id_value);
    flutter::EncodableValue &server_type_value =
        encodables[flutter::EncodableValue("serverType")];
    server_type = std::get<std::string>(server_type_value);
  }
  LOG_INFO("BuyItem detail:%s", pay_details.c_str());
  bool bRet = service_billing_buyitem(app_id.c_str(), server_type.c_str(),
                                      pay_details.c_str());
  service_billing_set_buyitem_cb(OnBuyItem, this);
  if (!bRet) {
    LOG_ERROR("service_billing_buyitem failed");
    return false;
  }
  return true;
}

BillingManager::BillingManager(FlutterDesktopPluginRegistrarRef registrar_ref)
    : registrar_ref_(registrar_ref) {
  flutter::PluginRegistrar *plugin_registrar =
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrar>(registrar_ref_);

  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          plugin_registrar->messenger(),
          "plugins.flutter.tizen.io/in_app_purchase",
          &flutter::StandardMethodCodec::GetInstance());

  channel->SetMethodCallHandler(
      [this](const flutter::MethodCall<flutter::EncodableValue> &call,
             std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
                 result) { this->HandleMethodCall(call, std::move(result)); });
}

void BillingManager::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  LOG_INFO("HandleMethodCall");
  const auto &method_name = method_call.method_name();
  const flutter::EncodableValue *args = method_call.arguments();
  if (method_name == "getProductList") {
    if (std::holds_alternative<flutter::EncodableMap>(*args)) {
      if (GetProductList(args)) {
        method_result_ = std::move(result);
      } else {
        result->Error("getProductList failed");
      }
    }
  } else if (method_name == "getPurchaseList") {
    if (std::holds_alternative<flutter::EncodableMap>(*args)) {
      if (GetPurchaseList(args)) {
        method_result_ = std::move(result);
      } else {
        result->Error("getPurchaseList failed");
      }
    }
  } else if (method_name == "buyItem") {
    if (std::holds_alternative<flutter::EncodableMap>(*args)) {
      if (BuyItem(args)) {
        method_result_ = std::move(result);
      } else {
        result->Error("buyItem failed");
      }
    }
  } else if (method_name == "isAvailable") {
    if (BillingIsAvailable()) {
      method_result_ = std::move(result);
    } else {
      result->Error("isAvailable failed");
    }
  } else {
    result->NotImplemented();
  }
}

void BillingManager::SendResult(const flutter::EncodableValue &result) {
  if (method_result_) {
    method_result_->Success(result);
    method_result_ = nullptr;
  }
}

bool BillingManager::BillingIsAvailable() {
  bool bRet =
      service_billing_is_service_available(SERVERTYPE_DEV, OnAvailable, this);
  if (!bRet) {
    LOG_ERROR("service_billing_is_service_available failed");
    return false;
  }
  return true;
}

void BillingManager::OnAvailable(const char *detailResult, void *pUser) {
  LOG_INFO("OnAvailable detailResult:%s", detailResult);
  BillingManager *billing = reinterpret_cast<BillingManager *>(pUser);

  billing->SendResult(flutter::EncodableValue(std::string(detailResult)));
}

void BillingManager::OnProducts(const char *detailResult, void *pUser) {
  LOG_INFO("productlist:%s", detailResult);
  BillingManager *billing = reinterpret_cast<BillingManager *>(pUser);

  billing->SendResult(flutter::EncodableValue(std::string(detailResult)));
}

void BillingManager::OnPurchase(const char *detailResult, void *pUser) {
  LOG_INFO("purchaselist:%s", detailResult);
  BillingManager *billing = reinterpret_cast<BillingManager *>(pUser);

  billing->SendResult(flutter::EncodableValue(std::string(detailResult)));
}

bool BillingManager::OnBuyItem(const char *payResult, const char *detailInfo,
                               void *pUser) {
  LOG_INFO("OnBuyItem result:[%s] [%s]", payResult, detailInfo);
  BillingManager *billing = reinterpret_cast<BillingManager *>(pUser);
  flutter::EncodableMap result_map = {
      {flutter::EncodableValue("PayResult"),
       flutter::EncodableValue(payResult)},
  };
  billing->SendResult(flutter::EncodableValue(result_map));
}

void BillingManager::Dispose() {
  LOG_INFO("dispose");

  // close dlopen handle.
  CloseBillingApi(billing_api_handle_);
  billing_api_handle_ = nullptr;
}
