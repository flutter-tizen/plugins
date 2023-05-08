// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';
import 'package:in_app_purchase_platform_interface/in_app_purchase_platform_interface.dart';

import '../billing_client_wrappers.dart';

const String kPurchaseErrorCode = 'purchase_error';

const String kIAPSource = 'samsung_checkout';

class InAppPurchaseTizenPlatform extends InAppPurchasePlatform {
  InAppPurchaseTizenPlatform();
  InAppPurchaseTizenPlatform._() {
    billingClient = BillingClient();

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
  static late String _appId;
  static late String _serverType;
  static late Set<String> _identifiers;

  @override
  Stream<List<PurchaseDetails>> get purchaseStream =>
      _purchaseUpdatedController.stream;

  @visibleForTesting
  late final BillingClient billingClient;

  static final Set<String> _productIdsToConsume = <String>{};

  @override
  Future<bool> isAvailable() async {
    return await billingClient.isAvailable();
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
      response = await billingClient.requestProducts(identifiers.toList());
    } on PlatformException catch (e) {
      exception = e;
      response = const ProductsListApiResult(
          cPStatus: 'fail to receive response',
          cPResult: 'fail to receive response',
          itemDetails: <ProductWrapper>[]);
    }
    if (response.result != '') {
      throw PlatformException(
          code: response.result!, message: response.resultTitle);
    }

    List<SamsungCheckoutProductDetails> productDetailsList =
        <SamsungCheckoutProductDetails>[];

    productDetailsList = response.itemDetails
        .map((ProductWrapper productWrapper) =>
            SamsungCheckoutProductDetails.fromProduct(productWrapper))
        .toList();
    List<String> invalidMessage;
    switch (response.cPStatus) {
      case "100000":
        invalidMessage = [];
        break;
      default:
        invalidMessage = [response.cPStatus, response.cPResult];
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
    List<PurchaseListAPIResult> responses;

    responses = await Future.wait(<Future<PurchaseListAPIResult>>[
      billingClient.requestPurchases(applicationUserName, _identifiers.toList())
    ]);

    final List<PurchaseDetails> pastPurchases =
        responses.expand((PurchaseListAPIResult response) {
      return response.invoiceDetails;
    }).map((PurchaseWrapper purchaseWrapper) {
      final SamsungCheckoutPurchaseDetails purchaseDetails =
          SamsungCheckoutPurchaseDetails.fromPurchase(purchaseWrapper);

      purchaseDetails.status = PurchaseStatus.restored;
      return purchaseDetails;
    }).toList();
    _purchaseUpdatedController.add(pastPurchases);
  }

  @override
  Future<bool> buyNonConsumable({required PurchaseParam purchaseParam}) async {
    final BillingResultWrapper billingResultWrapper =
        await billingClient.butItem(
            appId: _appId,
            serverType: _serverType,
            orderItemId: purchaseParam.productDetails.id,
            orderTitle: purchaseParam.productDetails.title,
            orderTotal: purchaseParam.productDetails.price,
            orderCurrencyId: purchaseParam.productDetails.currencyCode);
    if (billingResultWrapper.payResult == 'SUCCESS') {
      return true;
    } else {
      return false;
    }
  }

  @override
  Future<bool> buyConsumable(
      {required PurchaseParam purchaseParam, bool autoConsume = true}) {
    if (autoConsume) {
      _productIdsToConsume.add(purchaseParam.productDetails.id);
    }
    return buyNonConsumable(purchaseParam: purchaseParam);
  }
}
