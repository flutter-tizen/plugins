// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';

import 'package:crypto/crypto.dart';
import 'package:flutter/foundation.dart';
import 'package:in_app_purchase_platform_interface/in_app_purchase_platform_interface.dart';

import 'in_app_purchase_tizen_platform.dart';
import 'messages.g.dart';

/// This class can be used directly to call Billing(Samsung Checkout) APIs.
///
/// Wraps a
/// [`BillingManager`](https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html#BillingManager)
/// instance.
class BillingManager {
  /// Creates a billing manager.
  BillingManager({@visibleForTesting InAppPurchaseApi? hostApi})
      : _hostApi = hostApi ?? InAppPurchaseApi();

  late RequestParameters _requestParameters;

  /// Call this to set tizen specific parameters.
  // ignore: use_setters_to_change_properties
  void setRequestParameters(RequestParameters requestParameters) {
    _requestParameters = requestParameters;
  }

  /// This is different from response [ItemType]. The value `2` means `all items`.
  ///
  /// [_requestItemType] is only used for [BillingManager.requestPurchases].
  static const String _requestItemType = '2';

  /// Interface for calling host-side code.
  final InAppPurchaseApi _hostApi;

  /// get country code
  Future<String> getCountryCode() async {
    return _hostApi.getCountryCode();
  }

  /// Calls
  /// [`BillingManager-isServiceAvailable`](https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html#BillingManager-isServiceAvailable)
  /// to check whether the Billing server is available.
  Future<bool> isAvailable() async {
    return _hostApi.isServiceAvailable();
  }

  /// Calls
  /// [`BillingManager-getProductsList`](https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html#BillingManager-getProductsList)
  /// to retrieves the list of products registered on the Billing (DPI) server.
  Future<ProductsListApiResult> requestProducts(
    List<String> requestparameters,
  ) async {
    final String countryCode = await _hostApi.getCountryCode();
    final String checkValue = base64.encode(
      Hmac(
        sha256,
        utf8.encode(_requestParameters.securityKey ?? ''),
      ).convert(utf8.encode((_requestParameters.appId) + countryCode)).bytes,
    );
    final ProductMessage product = ProductMessage(
      appId: _requestParameters.appId,
      countryCode: countryCode,
      pageSize: _requestParameters.pageSize,
      pageNum: _requestParameters.pageNum,
      checkValue: checkValue,
    );
    return _hostApi.getProductsList(product);
  }

  /// Calls
  /// [`BillingManager-getUserPurchaseList`](https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html#BillingManager-getUserPurchaseList)
  /// to retrieves the user's purchase list.
  Future<GetUserPurchaseListAPIResult> requestPurchases({
    String? applicationUserName,
  }) async {
    final String customId = await _hostApi.getCustomId();
    final String countryCode = await _hostApi.getCountryCode();
    final String checkValue = base64.encode(
      Hmac(sha256, utf8.encode(_requestParameters.securityKey ?? ''))
          .convert(
            utf8.encode(
              (_requestParameters.appId) +
                  customId +
                  countryCode +
                  _requestItemType +
                  _requestParameters.pageNum.toString(),
            ),
          )
          .bytes,
    );

    final PurchaseMessage purchase = PurchaseMessage(
      appId: _requestParameters.appId,
      customId: customId,
      countryCode: countryCode,
      pageNum: _requestParameters.pageNum,
      checkValue: checkValue,
    );

    return _hostApi.getUserPurchaseList(purchase);
  }

  /// Calls
  /// [`BillingManager-buyItem`](https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html#BillingManager-buyItem)
  /// to enables implementing the Samsung Checkout Client module within the application.
  /// After authenticating the purchase information through the application, the user can proceed to purchase payment.
  Future<BillingBuyData> buyItem({
    required String orderItemId,
    required String orderTitle,
    required String orderTotal,
    required String orderCurrencyId,
  }) async {
    final String customId = await _hostApi.getCustomId();
    final OrderDetails orderDetails = OrderDetails(
      orderItemId: orderItemId,
      orderTitle: orderTitle,
      orderTotal: orderTotal,
      orderCurrencyId: orderCurrencyId,
      orderCustomId: customId,
    );

    final BuyInfoMessage buyInfo = BuyInfoMessage(
      appId: _requestParameters.appId,
      payDetials: orderDetails,
    );

    return _hostApi.buyItem(buyInfo);
  }

  /// Calls
  /// [`BillingManager-verifyInvoice`](https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html#BillingManager-verifyInvoice)
  /// to enables implementing the Samsung Checkout Client module within the application.
  /// Checks whether a purchase, corresponding to a specific "InvoiceID", was successful.
  Future<VerifyInvoiceAPIResult> verifyInvoice({
    required String invoiceId,
  }) async {
    final String customId = await _hostApi.getCustomId();
    final String countryCode = await _hostApi.getCountryCode();
    final InvoiceMessage invoice = InvoiceMessage(
      invoiceId: invoiceId,
      appId: _requestParameters.appId,
      customId: customId,
      countryCode: countryCode,
    );

    return _hostApi.verifyInvoice(invoice);
  }
}

/// This class can be used to set tizen specific parameters.
class RequestParameters {
  /// This is application id.
  late String appId;

  /// The number of products retrieved per page.(>=1,<=100)
  /// Use it when call `queryProductDetails`.
  late int pageSize;

  /// The requested page number.(>=1)
  /// Use it when call `queryProductDetails` and `restorePurchases`.
  late int pageNum;

  /// The DPI security key.
  /// Use it when call `queryProductDetails` and `restorePurchases`
  late String? securityKey;
}

/// Dart wrapper around [`ItemDetails`] in (https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html).

/// Defines a dictionary for the ProductsListAPIResult dictionary 'ItemDetails' parameter.
/// This only can be used in [ProductsListApiResult].
@immutable
class ItemDetails {
  /// Creates a [ItemDetails] with the given purchase details.
  const ItemDetails({
    required this.seq,
    required this.itemId,
    required this.itemTitle,
    required this.itemDesc,
    required this.itemType,
    required this.price,
    required this.currencyId,
  });

  /// Sequence number (1 ~ TotalCount).
  final int seq;

  /// The ID of Product.
  final String itemId;

  /// The name of product.
  final String itemTitle;

  /// The description of product.
  final String itemDesc;

  /// The type of product.
  final ItemType itemType;

  /// The price of product, in "xxxx.yy" format.
  final num price;

  /// The currency code
  final String currencyId;
}

/// Dart wrapper around [`ProductSubscriptionInfo`] in (https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html).
///
/// Defines a dictionary for the ItemDetails dictionary 'SubscriptionInfo' parameter.
/// This only can be used in [ItemDetails].
@immutable
class ProductSubscriptionInfo {
  /// Creates a [ProductSubscriptionInfo] with the given purchase details.
  const ProductSubscriptionInfo({
    required this.paymentCycle,
    required this.paymentCycleFrq,
    required this.paymentCyclePeriod,
  });

  /// Subscription payment period:
  /// "D": Days
  /// "W": Weeks
  /// "M": Months
  final String paymentCyclePeriod;

  /// Payment cycle frequency.
  final int paymentCycleFrq;

  /// Number of payment cycles.
  final int paymentCycle;
}

/// The class represents the information of a product as registered in at
/// Samsung Checkout DPI portal.
class SamsungCheckoutProductDetails extends ProductDetails {
  /// Creates a new Samsung Checkout specific product details object with the
  /// provided details.
  SamsungCheckoutProductDetails({
    required super.id,
    required super.title,
    required super.description,
    required super.price,
    required super.currencyCode,
    required this.itemDetails,
    super.rawPrice = 0.0,
    super.currencySymbol,
  });

  /// Generate a [SamsungCheckoutProductDetails] object based on [ItemDetails] object.
  factory SamsungCheckoutProductDetails.fromProduct(ItemDetails itemDetails) {
    return SamsungCheckoutProductDetails(
      id: itemDetails.itemId,
      title: itemDetails.itemTitle,
      description: itemDetails.itemDesc,
      price: itemDetails.price.toString(),
      currencyCode: itemDetails.currencyId,
      itemDetails: itemDetails,
    );
  }

  /// Points back to the [ItemDetails] object that was used to generate
  /// this [SamsungCheckoutProductDetails] object.
  final ItemDetails itemDetails;
}

/// Dart wrapper around [`InvoiceDetails`] in (https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html).
///
/// Defines a dictionary for the GetUserPurchaseListAPIResult dictionary 'InvoiceDetails' parameter.
/// This only can be used in [GetUserPurchaseListAPIResult].
class InvoiceDetails {
  /// Creates a [InvoiceDetails] with the given purchase details.
  const InvoiceDetails({
    required this.seq,
    required this.invoiceId,
    required this.itemId,
    required this.itemTitle,
    required this.itemType,
    required this.orderTime,
    required this.price,
    required this.orderCurrencyId,
    required this.appliedStatus,
    required this.cancelStatus,
    this.appliedTime,
    this.period,
    this.limitEndTime,
    this.remainTime,
  });

  /// Sequence number (1 ~ TotalCount).
  final int seq;

  /// Invoice ID of this purchase history.
  final String invoiceId;

  /// The ID of product.
  final String itemId;

  /// The name of product.
  final String itemTitle;

  /// The type of product.
  final ItemType itemType;

  /// Payment time, in 14-digit UTC time.
  final String orderTime;

  /// Limited period product duration, in minutes.
  final int? period;

  /// Product price, in "xxxx.yy" format.
  final num price;

  /// Currency code.
  final String orderCurrencyId;

  /// Cancellation status:
  /// "true": Sale canceled
  /// "false" : Sale ongoing
  final bool cancelStatus;

  /// Product application status:
  /// "true": Applied
  /// "false": Not applied
  final bool appliedStatus;

  /// Time product applied, in 14-digit UTC time
  final String? appliedTime;

  /// Limited period product end time, in 14-digit UTC time
  final String? limitEndTime;

  /// Limited period product time remaining, in seconds
  final String? remainTime;
}

/// Dart wrapper around [`PurchaseSubscriptionInfo`] in (https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html).
///
/// Defines a dictionary for the InvoiceDetails dictionary 'SubscriptionInfo' parameter.
/// This only can be used in [InvoiceDetails].
@immutable
class PurchaseSubscriptionInfo {
  /// Creates a [PurchaseSubscriptionInfo] with the given purchase details.
  const PurchaseSubscriptionInfo({
    required this.subscriptionId,
    required this.subsStartTime,
    required this.subsEndTime,
    required this.subsStatus,
  });

  /// ID of subscription history.
  final String subscriptionId;

  /// Subscription start time, in 14-digit UTC time.
  final String subsStartTime;

  /// Subscription expiry time, in 14-digit UTC time.
  final String subsEndTime;

  /// Subscription status:
  /// "00": Active
  /// "01": Subscription expired
  /// "02": Canceled by buyer
  /// "03": Canceled for payment failure
  /// "04": Canceled by CP
  /// "05": Canceled by admin
  final String subsStatus;
}

/// The class represents the information of a purchase made using Samsung Checkout.
class SamsungCheckoutPurchaseDetails extends PurchaseDetails {
  /// Creates a new Samsung Checkout specific purchase details object with the
  /// provided details.
  SamsungCheckoutPurchaseDetails({
    required super.productID,
    required super.purchaseID,
    required super.status,
    required super.transactionDate,
    required super.verificationData,
    required this.invoiceDetails,
  });

  /// Generate a [SamsungCheckoutPurchaseDetails] object based on [PurchaseDetails] object.
  factory SamsungCheckoutPurchaseDetails.fromPurchase(
    InvoiceDetails invoiceDetails,
  ) {
    final SamsungCheckoutPurchaseDetails purchaseDetails =
        SamsungCheckoutPurchaseDetails(
      purchaseID: invoiceDetails.invoiceId,
      productID: invoiceDetails.itemId,
      verificationData: PurchaseVerificationData(
        localVerificationData: invoiceDetails.invoiceId,
        serverVerificationData: invoiceDetails.invoiceId,
        source: kIAPSource,
      ),
      transactionDate: invoiceDetails.orderTime,
      status: const PurchaseStateConverter().toPurchaseStatus(
        invoiceDetails.cancelStatus,
      ),
      invoiceDetails: invoiceDetails,
    );

    if (purchaseDetails.status == PurchaseStatus.error) {
      purchaseDetails.error = IAPError(
        source: kIAPSource,
        code: kPurchaseErrorCode,
        message: '',
      );
    }

    return purchaseDetails;
  }

  /// Points back to the [InvoiceDetails] which was used to generate this
  /// [SamsungCheckoutPurchaseDetails] object.
  final InvoiceDetails invoiceDetails;
}

/// Convert bool value to purchase status.ll
class PurchaseStateConverter {
  /// Default const constructor.
  const PurchaseStateConverter();

  /// Converts the purchase state stored in `object` to a [PurchaseStatus].
  PurchaseStatus toPurchaseStatus(bool object) {
    switch (object) {
      case false:
        return PurchaseStatus.purchased;
      case true:
        return PurchaseStatus.canceled;
    }
  }
}

// The type of product.
/// Enum representing potential [ItemDetails.itemType]s and [InvoiceDetails.itemType]s.
/// Wraps
/// [`Product`]（https://developer.samsung.com/smarttv/develop/guides/samsung-checkout/samsung-checkout-dpi-portal.html#Product)
/// See the linked documentation for an explanation of the different constants.
enum ItemType {
  /// None type.
  none,

  /// Consumers can purchase this type of product anytime.
  consumable,

  /// Consumers can purchase this type of product only once.
  nonComsumabel,

  /// Once this type of product is purchased, repurchase cannot be made during the time when the product effect set by CP lasts.
  limitedPeriod,

  /// DPI system processes automatic payment on a certain designated cycle.
  subscription,
}
