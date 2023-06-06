// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';
import 'package:in_app_purchase_platform_interface/in_app_purchase_platform_interface.dart';

import '../billing_manager_wrappers.dart';
import '../in_app_purchase_tizen.dart';

/// [IAPError.code] code for failed purchases.
const String kPurchaseErrorCode = 'purchase_error';

/// Indicates store front is Samsung Checkout
const String kIAPSource = 'samsung_checkout';

/// An [InAppPurchasePlatform] that wraps BillingManager.
///
/// This translates various `BillingManager` calls and responses into the
/// generic plugin API.
class InAppPurchaseTizenPlatform extends InAppPurchasePlatform {
  InAppPurchaseTizenPlatform._() {
    billingManager = BillingManager();

    // Register [InAppPurchaseTizenPlatformAddition].
    InAppPurchasePlatformAddition.instance =
        InAppPurchaseTizenPlatformAddition(billingManager);

    _purchaseUpdatedController =
        StreamController<List<PurchaseDetails>>.broadcast();
  }

  /// Registers this class as the default instance of [InAppPurchasePlatform].
  static void registerPlatform() {
    // Register the platform instance with the plugin platform
    // interface.
    InAppPurchasePlatform.instance = InAppPurchaseTizenPlatform._();
  }

  static late StreamController<List<PurchaseDetails>>
      _purchaseUpdatedController;

  @override
  Stream<List<PurchaseDetails>> get purchaseStream =>
      _purchaseUpdatedController.stream;

  /// The [_appId] is used for [queryProductDetails] and [buyNonConsumable] and [buyConsumable].
  static late String _appId;

  /// The [_serverType] is used for [buyNonConsumable] and [buyConsumable].
  static late String _serverType;

  /// The [_identifiers] is used for [restorePurchases].
  static late Set<String> _identifiers;

  /// The [BillingManager] that's abstracted by Samsung Checkout.
  ///
  /// This field should not be used out of test code.
  @visibleForTesting
  late final BillingManager billingManager;

  @override
  Future<bool> isAvailable() async {
    return billingManager.isAvailable();
  }

  @override
  Future<ProductDetailsResponse> queryProductDetails(
      Set<String> identifiers) async {
    ProductsListApiResult response;
    PlatformException? exception;
    _identifiers = identifiers;
    _appId = identifiers.toList()[0];
    _serverType = identifiers.toList()[5];
    try {
      response = await billingManager.requestProducts(identifiers.toList());
    } on PlatformException catch (e) {
      exception = e;
      response = const ProductsListApiResult(
          cPStatus: 'fail to receive response',
          cPResult: 'fail to receive response',
          checkValue: 'fail to receive response',
          totalCount: 0,
          itemDetails: <ItemDetails>[]);
    }

    List<SamsungCheckoutProductDetails> productDetailsList =
        <SamsungCheckoutProductDetails>[];

    productDetailsList = response.itemDetails
        .map((ItemDetails productWrapper) =>
            SamsungCheckoutProductDetails.fromProduct(productWrapper))
        .toList();
    final List<String> invalidMessage = <String>[];
    switch (response.cPStatus) {
      case '100000':
        break;
      default:
        invalidMessage.add(response.toJson().toString());
    }

    final ProductDetailsResponse productDetailsResponse =
        ProductDetailsResponse(
      productDetails: productDetailsList,
      notFoundIDs: invalidMessage,
      error: exception == null
          ? null
          : IAPError(
              source: kIAPSource,
              code: exception.code,
              message: exception.message ?? '',
              details: exception.details),
    );
    return productDetailsResponse;
  }

  @override
  Future<void> restorePurchases({
    String? applicationUserName,
  }) async {
    List<GetUserPurchaseListAPIResult> responses;

    responses = await Future.wait(<Future<GetUserPurchaseListAPIResult>>[
      billingManager.requestPurchases(
          applicationUserName, _identifiers.toList())
    ]);

    final List<PurchaseDetails> pastPurchases =
        responses.expand((GetUserPurchaseListAPIResult response) {
      return response.invoiceDetails;
    }).map((InvoiceDetails purchaseWrapper) {
      final SamsungCheckoutPurchaseDetails purchaseDetails =
          SamsungCheckoutPurchaseDetails.fromPurchase(purchaseWrapper);

      purchaseDetails.status = PurchaseStatus.restored;
      return purchaseDetails;
    }).toList();
    _purchaseUpdatedController.add(pastPurchases);
  }

  @override
  Future<bool> buyNonConsumable({required PurchaseParam purchaseParam}) async {
    final BillingBuyData billingResultWrapper = await billingManager.buyItem(
        appId: _appId,
        serverType: _serverType,
        orderItemId: purchaseParam.productDetails.id,
        orderTitle: purchaseParam.productDetails.title,
        orderTotal: purchaseParam.productDetails.price,
        orderCurrencyId: purchaseParam.productDetails.currencyCode);

    if (billingResultWrapper.payResult == 'SUCCESS') {
      final String invoiceId =
          billingResultWrapper.payDetails['InvoiceID'] ?? '';

      billingManager
          .requestPurchases(_identifiers.toList()[8], _identifiers.toList())
          .then((GetUserPurchaseListAPIResult responses) {
        for (int i = 0; i < responses.invoiceDetails.length; i++) {
          if (responses.invoiceDetails[i].invoiceID == invoiceId) {
            final List<PurchaseDetails> purchases = <PurchaseDetails>[];
            purchases.add(PurchaseDetails(
              purchaseID: responses.invoiceDetails[i].invoiceID,
              productID: responses.invoiceDetails[i].itemID,
              verificationData: PurchaseVerificationData(
                  localVerificationData: responses.invoiceDetails[i].invoiceID,
                  serverVerificationData: responses.invoiceDetails[i].invoiceID,
                  source: kIAPSource),
              transactionDate: responses.invoiceDetails[i].orderTime,
              status: const PurchaseStateConverter()
                  .toPurchaseStatus(responses.invoiceDetails[i].cancelStatus),
            ));

            _purchaseUpdatedController.add(purchases);
          }
        }
      }).catchError((Object error) {
        throw PlatformException(
            code: 'Failed to get response from platform:$error');
      });

      return true;
    } else {
      return false;
    }
  }

  @override
  Future<bool> buyConsumable(
      {required PurchaseParam purchaseParam, bool autoConsume = true}) {
    assert(autoConsume == true, 'On Tizen, we should always auto consume');
    return buyNonConsumable(purchaseParam: purchaseParam);
  }
}
