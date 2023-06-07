// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/services.dart';
import 'package:in_app_purchase_platform_interface/in_app_purchase_platform_interface.dart';

import '../billing_manager_wrappers.dart';

/// Contains InApp Purchase features that are only available on SamsungCheckout.
class InAppPurchaseTizenPlatformAddition extends InAppPurchasePlatformAddition {
  /// Creates a [InAppPurchaseTizenPlatformAddition] which uses the supplied
  /// `BillingManager` to provide Tizen specific features.
  InAppPurchaseTizenPlatformAddition(this._billingManager);

  final BillingManager _billingManager;

  /// Set all request parameters that SamsungCheckout DPI Portal needed.
  void setRequestParameters({
    required String appId,
    required String countryCode,
    required String pageSize,
    required String pageNum,
    required String itemType,
    required String serverType,
    required String customId,
    required String securityKey,
  }) {
    final Map<String, dynamic> requestParameters = <String, dynamic>{
      'appId': appId,
      'countryCode': countryCode,
      'pageSize': pageSize,
      'pageNum': pageNum,
      'itemType': itemType,
      'serverType': serverType,
      'customId': customId,
      'securityKey': securityKey,
    };
    _billingManager.setRequestParameters(requestParameters);
  }

  /// Check whether a purchase, corresponding to the requested "InvoiceID", was successful.
  Future<bool> verifyPurchase(
      {required PurchaseDetails purchaseDetails}) async {
    VerifyInvoiceAPIResult verifyPurchaseResult;
    try {
      verifyPurchaseResult = await _billingManager.verifyInvoice(
          invoiceId: purchaseDetails.verificationData.serverVerificationData);
    } on PlatformException {
      verifyPurchaseResult = const VerifyInvoiceAPIResult(
          appId: 'error appId',
          cpStatus: 'error cpStatus',
          invoiceId: 'error invoiceId');
    }

    if (verifyPurchaseResult.cpStatus == '100000') {
      return true;
    } else {
      return false;
    }
  }
}
