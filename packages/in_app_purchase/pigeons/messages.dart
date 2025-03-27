// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:pigeon/pigeon.dart';

@ConfigurePigeon(
  PigeonOptions(
    dartOut: 'lib/src/messages.g.dart',
    cppHeaderOut: 'tizen/src/messages.h',
    cppSourceOut: 'tizen/src/messages.cc',
  ),
)
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
    required this.totalCount,
    required this.checkValue,
    required this.invoiceDetails,
  });

  /// It returns "100000" in success and other codes in failure. Refer to DPI Error Code.
  final String cpStatus;

  /// The result message:
  /// "EOF":Last page of the product list
  /// "hasNext:TRUE" Product list has further pages
  /// Other error message, depending on the DPI result code
  final String? cpResult;

  /// Total number of invoices.
  final int totalCount;

  /// Security check value.
  final String checkValue;

  /// InvoiceDetailsin JSON format.
  final List<Map<Object?, Object?>?> invoiceDetails;
}

/// Dart wrapper around [`BillingBuyData`](https://developer.samsung.com/smarttv/develop/api-references/samsung-product-api-references/billing-api.html#BillingBuyData).
///
/// Defines the payment result and information.
class BillingBuyData {
  /// Creates a [BillingBuyData] with the given purchase details.
  const BillingBuyData({required this.payResult, required this.payDetails});

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
    required this.serviceYn,
  });

  /// The result code of connecting to billing server.
  /// Returns "100000" on success and other codes on failure.
  final String status;

  /// The result message of connecting to billing server.
  /// Returns "Success" on success.
  final String result;

  /// Returns "Y" if the service is available.
  /// It will be null, if disconnect to billing server.
  final String serviceYn;
}

class ProductMessage {
  ProductMessage({
    required this.appId,
    required this.countryCode,
    required this.pageSize,
    required this.pageNum,
    required this.checkValue,
  });

  /// Application ID.
  final String appId;

  /// TV country code.
  final String countryCode;

  /// Number of products retrieved per page (maximum 100).
  final int pageSize;

  /// Requested page number (1 ~ N).
  final int pageNum;

  /// Security check value. Required parameters = "appId" + "countryCode".
  /// The check value is used by the DPI service to verify API requests.
  /// It is a Base64 hash generated by applying the HMAC SHA256 algorithm on a concatenated string of parameters using the DPI security key.
  final String checkValue;
}

class PurchaseMessage {
  PurchaseMessage({
    required this.appId,
    required this.customId,
    required this.countryCode,
    required this.pageNum,
    required this.checkValue,
  });

  /// Application ID.
  final String appId;

  /// Same value as "OrderCustomID" parameter for the BuyItem API (Samsung Account UID)
  final String customId;

  /// TV country code.
  final String countryCode;

  /// Requested page number (1 ~ N).
  final int pageNum;

  /// Security check value. Required parameters = "appId" + "customId" + "countryCode" + "ItemType" + "pageNumber".
  /// ItemType, MUST use 2 as value ("all items")
  final String checkValue;
}

class OrderDetails {
  OrderDetails({
    required this.orderItemId,
    required this.orderTitle,
    required this.orderTotal,
    required this.orderCurrencyId,
    required this.orderCustomId,
  });
  final String orderItemId;
  final String orderTitle;
  final String orderTotal;
  final String orderCurrencyId;
  final String orderCustomId;
}

class BuyInfoMessage {
  BuyInfoMessage({required this.appId, required this.payDetials});

  /// Application ID.
  final String appId;

  /// Payment parameters.
  final OrderDetails payDetials;
}

class InvoiceMessage {
  InvoiceMessage({
    required this.appId,
    required this.customId,
    required this.invoiceId,
    required this.countryCode,
  });

  /// Application ID.
  final String appId;

  /// Same value as "OrderCustomID" parameter for the BuyItem API (Samsung Account UID).
  final String customId;

  /// Invoice ID that you want to verify whether a purchase was successful.
  final String invoiceId;

  ///  TV country code.
  final String countryCode;
}

@HostApi()
abstract class InAppPurchaseApi {
  /// Retrieves the list of products registered on the Billing (DPI) server.
  @async
  ProductsListApiResult getProductsList(ProductMessage product);

  /// Retrieves the user's purchase list.
  @async
  GetUserPurchaseListAPIResult getUserPurchaseList(PurchaseMessage purchase);

  /// Enables implementing the Samsung Checkout Client module within the application.
  /// After authenticating the purchase information through the application, the user can proceed to purchase payment.
  @async
  BillingBuyData buyItem(BuyInfoMessage buyInfo);

  /// Checks whether a purchase, corresponding to a specific "InvoiceID", was successful.
  @async
  VerifyInvoiceAPIResult verifyInvoice(InvoiceMessage invoice);

  /// Checks whether the Billing server is available.
  @async
  bool isServiceAvailable();

  String getCustomId();

  String getCountryCode();
}
