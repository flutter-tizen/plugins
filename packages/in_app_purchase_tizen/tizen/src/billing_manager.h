// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_PLUGIN_BILLING_MANAGER_H
#define FLUTTER_PLUGIN_BILLING_MANAGER_H

#include <flutter/encodable_value.h>
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
  bool GetProductList(const flutter::EncodableMap *encodables);
  bool GetPurchaseList(const flutter::EncodableMap *encodables);
  bool BuyItem(const flutter::EncodableMap *encodables);
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

#endif  // FLUTTER_PLUGIN_BILLING_MANAGER_H