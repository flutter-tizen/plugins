#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "billing_service_proxy.h"
// #include "hmac.h"
// #include "messages.h"

class BillingManager {
 public:
  BillingManager(flutter::PluginRegistrar *plugin_registrar);
  ~BillingManager(){};
  bool Init();
  bool BillingIsAvailable();
  bool GetProductList(const flutter::EncodableValue *args);
  bool GetPurchaseList(const flutter::EncodableValue *args);
  bool BuyItem(const flutter::EncodableValue *args);
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  void Dispose();
  void SendResult(const flutter::EncodableValue &result);

 private:
  static void OnProducts(const char *detailResult, void *pUser);
  static void OnPurchase(const char *detailResult, void *pUser);
  static bool OnBuyItem(const char *payResult, const char *detailInfo,
                        void *pUser);
  static void OnAvailable(const char *detailResult, void *pUser);

  void *billing_api_handle_ = nullptr;
  std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
      method_result_ = nullptr;
  flutter::PluginRegistrar *plugin_registrar_ = nullptr;
};
