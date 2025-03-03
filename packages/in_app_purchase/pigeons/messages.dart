// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:pigeon/pigeon.dart';

@ConfigurePigeon(PigeonOptions(
  dartOut: 'lib/src/messages.g.dart',
  cppHeaderOut: 'tizen/src/messages.h',
  cppSourceOut: 'tizen/src/messages.cc',
))

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
  subscription
}

/// Dart wrapper around [`ProductsListApiResult`] in (https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html).
///
/// Defines a dictionary for product list data returned by the getProductsList API.
/// This only can be used in [BillingManager.requestProducts].
class ProductsListApiResult {
  /// Creates a [ProductsListApiResult] with the given purchase details.
  const ProductsListApiResult({
    required this.cpStatus,
    this.cpResult,
    required this.checkValue,
    required this.totalCount,
    required this.itemDetails,
  });

  /// DPI result code.
  /// Returns "100000" on success and other codes on failure.
  final String cpStatus;

  /// The result message.
  /// "EOF":Last page of the product list.
  /// "hasNext:TRUE" Product list has further pages.
  /// Other error message, depending on the DPI result code.
  final String? cpResult;

  /// Total number of invoices.
  final int totalCount;

  /// Security check value.
  final String checkValue;

  /// ItemDetails in JSON format
  final List<Map<Object?, Object?>?> itemDetails;
}

/// Dart wrapper around [`GetUserPurchaseListAPIResult`] in (https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html).
///
/// Defines a dictionary for data returned by the getUserPurchaseList API.
/// This only can be used in [BillingManager.requestPurchases]
class GetUserPurchaseListAPIResult {
  /// Creates a [GetUserPurchaseListAPIResult] with the given purchase details.
  const GetUserPurchaseListAPIResult({
    required this.cpStatus,
    this.cpResult,
    required this.invoiceDetails,
    required this.totalCount,
    required this.checkValue,
  });

  /// It returns "100000" in success and other codes in failure. Refer to DPI Error Code.
  final String cpStatus;

  /// The result message:
  /// "EOF":Last page of the product list
  /// "hasNext:TRUE" Product list has further pages
  /// Other error message, depending on the DPI result code
  final String? cpResult;

  /// Total number of invoices.
  final int? totalCount;

  /// Security check value.
  final String? checkValue;

  /// InvoiceDetailsin JSON format.
  final List<Map<Object?, Object?>?> invoiceDetails;
}

/// Dart wrapper around [`BillingBuyData`](https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html#BillingBuyData).
///
/// Defines the payment result and information.
class BillingBuyData {
  /// Creates a [BillingBuyData] with the given purchase details.
  const BillingBuyData({
    required this.payResult,
    required this.payDetails,
  });

  /// The payment result
  final String payResult;

  /// The payment information. It is same with paymentDetails param of buyItem.
  final Map<String, String> payDetails;
}

/// Dart wrapper around [`VerifyInvoiceAPIResult`] in (https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html).
///
/// This only can be used in [BillingManager.verifyInvoice].
class VerifyInvoiceAPIResult {
  /// Creates a [VerifyInvoiceAPIResult] with the given purchase details.
  const VerifyInvoiceAPIResult({
    required this.cpStatus,
    this.cpResult,
    required this.appId,
    required this.invoiceId,
  });

  /// DPI result code. Returns "100000" on success and other codes on failure.
  final String cpStatus;

  /// The result message:
  /// "SUCCESS" and Other error message, depending on the DPI result code.
  final String? cpResult;

  /// The application ID.
  final String appId;

  /// Invoice ID that you want to verify whether a purchase was successful.
  final String invoiceId;
}

/// Dart wrapper around [`ServiceAvailableAPIResult`] in (https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html).
///
/// Defines a dictionary for data returned by the IsServiceAvailable API.
/// This only can be used in [BillingManager.isAvailable].
class ServiceAvailableAPIResult {
  /// Creates a [ServiceAvailableAPIResult] with the given purchase details.
  const ServiceAvailableAPIResult({
    required this.status,
    required this.result,
    this.serviceYn,
  });

  /// The result code of connecting to billing server.
  /// Returns "100000" on success and other codes on failure.
  final String status;

  /// The result message of connecting to billing server.
  /// Returns "Success" on success.
  final String result;

  /// Returns "Y" if the service is available.
  /// It will be null, if disconnect to billing server.
  final String? serviceYn;
}

class ProductMessage {
  ProductMessage({
    required this.appId,
    required this.countryCode,
    this.pageSize,
    this.pageNum,
    required this.checkValue,
  });

  final String appId;
  final String countryCode;
  final int? pageSize;
  final int? pageNum;
  final String checkValue;
}

class PurchaseMessage {
  PurchaseMessage({
    required this.appId,
    this.customId,
    required this.countryCode,
    this.pageNum,
    required this.checkValue,
  });

  final String appId;
  final String? customId;
  final String countryCode;
  final int? pageNum;
  final String checkValue;
}

class OrderDetails {
  OrderDetails({
    required this.orderItemId,
    required this.orderTitle,
    required this.orderTotal,
    required this.orderCurrencyId,
  });
  final String orderItemId;
  final String orderTitle;
  final String orderTotal;
  final String orderCurrencyId;
}

class BuyInfoMessage {
  BuyInfoMessage({required this.appId, required this.payDetials});

  final String appId;
  final OrderDetails payDetials;
}

class InvoiceMessage {
  InvoiceMessage({
    required this.appId,
    this.customId,
    required this.invoiceId,
    required this.countryCode,
  });

  final String appId;
  final String? customId;
  final String invoiceId;
  final String countryCode;
}

@HostApi()
abstract class InAppPurchaseApi {
  @async
  ProductsListApiResult getProductList(ProductMessage product);

  @async
  GetUserPurchaseListAPIResult getPurchaseList(PurchaseMessage purchase);

  @async
  BillingBuyData buyItem(BuyInfoMessage buyInfo);

  @async
  VerifyInvoiceAPIResult verifyInvoice(InvoiceMessage invoice);

  @async
  bool isAvailable();

  String? getCustomId();

  String getCountryCode();
}
