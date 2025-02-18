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
  /// Creates an [InAppPurchaseTizenPlatform] instance.
  InAppPurchaseTizenPlatform();

  /// Registers this class as the default instance of [InAppPurchasePlatform].
  static void register() {
    // Register the [InAppPurchaseTizenPlatformAddition] containing
    // Samsung Checkout-specific functionality.
    InAppPurchasePlatformAddition.instance = InAppPurchaseTizenPlatformAddition(
      billingManager,
    );

    // Register the platform-specific implementation of the idiomatic
    // InAppPurchase API.
    InAppPurchasePlatform.instance = InAppPurchaseTizenPlatform();
  }

  static final StreamController<List<PurchaseDetails>>
  _purchaseUpdatedController =
      StreamController<List<PurchaseDetails>>.broadcast();

  @override
  Stream<List<PurchaseDetails>> get purchaseStream =>
      _purchaseUpdatedController.stream;

  /// The [BillingManager] that's abstracted by Samsung Checkout.
  ///
  /// This field should not be used out of test code.
  @visibleForTesting
  static final BillingManager billingManager = BillingManager();

  @override
  Future<bool> isAvailable() async {
    return billingManager.isAvailable();
  }

  @override
  Future<ProductDetailsResponse> queryProductDetails(
    Set<String> identifiers,
  ) async {
    ProductsListApiResult response;
    PlatformException? exception;
    try {
      response = await billingManager.requestProducts(identifiers.toList());
    } on PlatformException catch (e) {
      exception = e;
      response = const ProductsListApiResult(
        cpStatus: 'fail to receive response',
        cpResult: 'fail to receive response',
        checkValue: 'fail to receive response',
        totalCount: 0,
        itemDetails: <ItemDetails>[],
      );
    }

    List<SamsungCheckoutProductDetails> productDetailsList =
        <SamsungCheckoutProductDetails>[];
    final List<String> invalidMessage = <String>[];

    if (response.cpStatus == '100000') {
      productDetailsList =
          response.itemDetails
              .map(
                (ItemDetails productWrapper) =>
                    SamsungCheckoutProductDetails.fromProduct(productWrapper),
              )
              .toList();
    } else {
      invalidMessage.add(response.toJson().toString());
    }

    final ProductDetailsResponse productDetailsResponse =
        ProductDetailsResponse(
          productDetails: productDetailsList,
          notFoundIDs: invalidMessage,
          error:
              exception == null
                  ? null
                  : IAPError(
                    source: kIAPSource,
                    code: exception.code,
                    message: exception.message ?? '',
                    details: exception.details,
                  ),
        );
    return productDetailsResponse;
  }

  @override
  Future<void> restorePurchases({String? applicationUserName}) async {
    List<GetUserPurchaseListAPIResult> responses;

    responses = await Future.wait(<Future<GetUserPurchaseListAPIResult>>[
      billingManager.requestPurchases(),
    ]);

    final List<PurchaseDetails> pastPurchases =
        responses
            .expand((GetUserPurchaseListAPIResult response) {
              if (response.cpStatus == '100000') {
                return response.invoiceDetails;
              } else {
                return <InvoiceDetails>[];
              }
            })
            .map((InvoiceDetails purchaseWrapper) {
              final SamsungCheckoutPurchaseDetails purchaseDetails =
                  SamsungCheckoutPurchaseDetails.fromPurchase(purchaseWrapper);

              purchaseDetails.status = PurchaseStatus.restored;
              return purchaseDetails;
            })
            .toList();
    _purchaseUpdatedController.add(pastPurchases);
  }

  @override
  Future<bool> buyNonConsumable({required PurchaseParam purchaseParam}) async {
    final BillingBuyData billingResultWrapper = await billingManager.buyItem(
      orderItemId: purchaseParam.productDetails.id,
      orderTitle: purchaseParam.productDetails.title,
      orderTotal: purchaseParam.productDetails.price,
      orderCurrencyId: purchaseParam.productDetails.currencyCode,
    );

    if (billingResultWrapper.payResult == 'SUCCESS') {
      final String invoiceId =
          billingResultWrapper.payDetails['InvoiceID'] ?? '';

      unawaited(
        billingManager
            .requestPurchases()
            .then((GetUserPurchaseListAPIResult responses) {
              for (int i = 0; i < responses.invoiceDetails.length; i++) {
                if (responses.invoiceDetails[i].invoiceId == invoiceId) {
                  final List<PurchaseDetails> purchases = <PurchaseDetails>[];
                  purchases.add(
                    PurchaseDetails(
                      purchaseID: responses.invoiceDetails[i].invoiceId,
                      productID: responses.invoiceDetails[i].itemId,
                      verificationData: PurchaseVerificationData(
                        localVerificationData:
                            responses.invoiceDetails[i].invoiceId,
                        serverVerificationData:
                            responses.invoiceDetails[i].invoiceId,
                        source: kIAPSource,
                      ),
                      transactionDate: responses.invoiceDetails[i].orderTime,
                      status: const PurchaseStateConverter().toPurchaseStatus(
                        responses.invoiceDetails[i].cancelStatus,
                      ),
                    ),
                  );

                  _purchaseUpdatedController.add(purchases);
                }
              }
            })
            .catchError((Object error) {
              _purchaseUpdatedController.addError(error);
            }),
      );

      return true;
    } else {
      return false;
    }
  }

  @override
  Future<bool> buyConsumable({
    required PurchaseParam purchaseParam,
    bool autoConsume = true,
  }) {
    assert(autoConsume, 'On Tizen, we should always auto consume');
    return buyNonConsumable(purchaseParam: purchaseParam);
  }
}
