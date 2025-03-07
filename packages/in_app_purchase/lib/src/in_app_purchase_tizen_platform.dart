// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:flutter/widgets.dart';
import 'package:in_app_purchase_platform_interface/in_app_purchase_platform_interface.dart';

import '../in_app_purchase_tizen.dart';
import 'billing_manager.dart';
import 'messages.g.dart';

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
  late final Stream<List<PurchaseDetails>> purchaseStream =
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

  /// Converts integer to [ItemType].
  static ItemType _intToEnum(int number) {
    if (number == 1) {
      return ItemType.consumable;
    }
    if (number == 2) {
      return ItemType.nonComsumabel;
    }
    if (number == 3) {
      return ItemType.limitedPeriod;
    }
    if (number == 4) {
      return ItemType.subscription;
    }
    return ItemType.none;
  }

  /// Converts Map<Object?, Object?>? to the list of [ItemDetails].
  List<ItemDetails> _getItemDetails(ProductsListApiResult response) {
    final List<ItemDetails> itemDetails = <ItemDetails>[];
    for (final Map<Object?, Object?>? detail in response.itemDetails) {
      final int seq = detail!['Seq']! as int;
      final String itemId = detail['ItemID']! as String;
      final String itemTitle = detail['ItemTitle']! as String;
      final String itemDesc = detail['ItemDesc']! as String;
      final int itemType = detail['ItemType']! as int;
      final num price = detail['Price']! as num;
      final String currencyId = detail['CurrencyID']! as String;

      itemDetails.add(
        ItemDetails(
          seq: seq,
          itemId: itemId,
          itemTitle: itemTitle,
          itemDesc: itemDesc,
          itemType: _intToEnum(itemType),
          price: price,
          currencyId: currencyId,
        ),
      );
    }
    return itemDetails;
  }

  /// Performs a network query for the details of products available.
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
      response = ProductsListApiResult(
        cpStatus: 'fail to receive response',
        cpResult: 'fail to receive response',
        checkValue: 'fail to receive response',
        totalCount: 0,
        itemDetails: <Map<Object?, Object?>?>[],
      );
    }

    List<SamsungCheckoutProductDetails> productDetailsList =
        <SamsungCheckoutProductDetails>[];
    final List<String> invalidMessage = <String>[];

    if (response.cpStatus == '100000') {
      productDetailsList =
          _getItemDetails(response)
              .map(
                (ItemDetails productWrapper) =>
                    SamsungCheckoutProductDetails.fromProduct(productWrapper),
              )
              .toList();
    } else {
      invalidMessage.add(response.encode().toString());
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

  /// Converts Map<Object?, Object?>? to the list of [InvoiceDetails].
  List<InvoiceDetails> _getInvoiceDetails(
    GetUserPurchaseListAPIResult response,
  ) {
    final List<InvoiceDetails> invoiceDetails = <InvoiceDetails>[];
    for (final Map<Object?, Object?>? detail in response.invoiceDetails) {
      final int seq = detail!['Seq']! as int;
      final String invoiceId = detail['InvoiceID']! as String;
      final String itemId = detail['ItemID']! as String;
      final String itemTitle = detail['ItemTile']! as String;
      final int itemType = detail['ItemType']! as int;
      final String orderTime = detail['OrderTime']! as String;
      final int? period = detail['Period'] as int?;
      final num price = detail['Price']! as num;
      final String orderCurrencyId = detail['OrderCurrencyID']! as String;
      final bool cancelStatus = detail['CancelStatus']! as bool;
      final bool appliedStatus = detail['AppliedStatus']! as bool;
      final String? appliedTime = detail['AppliedTime'] as String?;
      final String? limitEndTime = detail['LimitEndTime'] as String?;
      final String? remainTime = detail['RemainTime'] as String?;

      invoiceDetails.add(
        InvoiceDetails(
          seq: seq,
          invoiceId: invoiceId,
          itemId: itemId,
          itemTitle: itemTitle,
          itemType: _intToEnum(itemType),
          orderTime: orderTime,
          period: period,
          price: price,
          orderCurrencyId: orderCurrencyId,
          cancelStatus: cancelStatus,
          appliedStatus: appliedStatus,
          appliedTime: appliedTime,
          limitEndTime: limitEndTime,
          remainTime: remainTime,
        ),
      );
    }
    return invoiceDetails;
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
                return _getInvoiceDetails(response);
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
                if (_getInvoiceDetails(responses)[i].invoiceId == invoiceId) {
                  final List<PurchaseDetails> purchases = <PurchaseDetails>[];
                  purchases.add(
                    PurchaseDetails(
                      purchaseID: _getInvoiceDetails(responses)[i].invoiceId,
                      productID: _getInvoiceDetails(responses)[i].itemId,
                      verificationData: PurchaseVerificationData(
                        localVerificationData:
                            _getInvoiceDetails(responses)[i].invoiceId,
                        serverVerificationData:
                            _getInvoiceDetails(responses)[i].invoiceId,
                        source: kIAPSource,
                      ),
                      transactionDate:
                          _getInvoiceDetails(responses)[i].orderTime,
                      status: const PurchaseStateConverter().toPurchaseStatus(
                        _getInvoiceDetails(responses)[i].cancelStatus,
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

  @override
  Future<String> countryCode() async {
    return billingManager.getCountryCode();
  }

  /// Use countryCode instead.
  @Deprecated('Use countryCode')
  Future<String> getCountryCode() => countryCode();
}
