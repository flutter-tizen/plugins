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

class BillingManager {
 public:
  explicit BillingManager(flutter::PluginRegistrar *plugin_registrar);
  ~BillingManager(){};

  bool Init();
  void Dispose();

 private:
  bool BillingIsAvailable();
  bool BuyItem(const flutter::EncodableMap *encodables);
  bool GetProductList(const flutter::EncodableMap *encodables);
  bool GetPurchaseList(const flutter::EncodableMap *encodables);
  bool VerifyInvoice(const flutter::EncodableMap *encodables);

  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);
  void SendResult(const flutter::EncodableValue &result);

  static void OnProducts(const char *detail_result, void *user_data);
  static void OnPurchase(const char *detail_result, void *user_data);
  static bool OnBuyItem(const char *pay_result, const char *detail_info,
                        void *user_data);
  static void OnAvailable(const char *detail_result, void *user_data);
  static void OnVerify(const char *detail_result, void *user_data);

  void *billing_api_handle_ = nullptr;
  std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>>
      method_result_ = nullptr;
  flutter::PluginRegistrar *plugin_registrar_ = nullptr;
};

#endif  // FLUTTER_PLUGIN_BILLING_MANAGER_H
