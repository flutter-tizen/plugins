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

  /// Check whether a purchase, corresponding to the requested "InvoiceID", was successful.
  Future<bool> verifyPurchase(PurchaseDetails purchaseDetails, String appId,
      String customId, String countryCode, String serverType) async {
    VerifyInvoiceAPIResult verifyPurchaseResult;
    try {
      verifyPurchaseResult = await _billingManager.verifyInvoice(
          invoiceId: purchaseDetails.verificationData.serverVerificationData,
          appId: appId,
          customId: customId,
          countryCode: countryCode,
          serverType: serverType);
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
