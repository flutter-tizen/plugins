// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert' show json;

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
import 'package:in_app_purchase_platform_interface/in_app_purchase_platform_interface.dart';
import 'package:json_annotation/json_annotation.dart';

import '../in_app_purchase_tizen_platform.dart';
import '../channel.dart';

// WARNING: Changes to `@JsonSerializable` classes need to be reflected in the
// below generated file. Run `flutter-tizen packages pub run build_runner watch` to
// rebuild and watch for further changes.
part 'billing_client_wrapper.g.dart';

class BillingClient {
  /// Creates a billing client.
  BillingClient();

  Future<bool> isAvailable() async {
    final String? isAvailableResult =
        await channel.invokeMethod<String>('isAvailable');
    if (isAvailableResult == null) {
      throw PlatformException(
        code: 'no_response',
        message: 'Failed to get response from platform.',
      );
    }
    final IsAvailableResult isAvailable =
        IsAvailableResult.fromJson(json.decode(isAvailableResult));
    final bool resultRet;
    switch (isAvailable.result) {
      case 'Success':
        resultRet = true;
        break;
      default:
        resultRet = false;
    }
    return resultRet;
  }

  Future<ProductsListApiResult> requestProducts(
      List<String> requestparameters) async {
    final String? productResponse =
        await channel.invokeMethod<String?>('getProductList', {
      'appId': requestparameters[0],
      'countryCode': requestparameters[1],
      'itemType': requestparameters[2],
      'pageSize': int.parse(requestparameters[3]),
      'pageNum': int.parse(requestparameters[4]),
      'serverType': requestparameters[5],
      'checkValue': requestparameters[6],
    });

    if (productResponse == null) {
      throw PlatformException(
        code: 'no_response',
        message: 'Failed to get response from platform.',
      );
    }
    return ProductsListApiResult.fromJson(json.decode(productResponse));
  }

  Future<PurchaseListAPIResult> requestPurchases(
      String? costomId, List<String> requestparameters) async {
    final String? purchaseResponse =
        await channel.invokeMethod<String?>('getPurchaseList', {
      'appId': requestparameters[0],
      'customId': costomId as String,
      'countryCode': requestparameters[1],
      'pageNum': int.parse(requestparameters[4]),
      'serverType': requestparameters[5],
      'checkValue': requestparameters[7],
    });
    if (purchaseResponse == null) {
      throw PlatformException(
        code: 'no_response',
        message: 'Failed to get response from platform.',
      );
    }
    return PurchaseListAPIResult.fromJson(json.decode(purchaseResponse));
  }

  Future<BillingResultWrapper> butItem({
    required String appId,
    required String serverType,
    required String orderItemId,
    required String orderTitle,
    required String orderTotal,
    required String orderCurrencyId,
  }) async {
    final Map<String, dynamic> orderDetails = <String, dynamic>{
      'OrderItemID': orderItemId,
      'OrderTitle': orderTitle,
      'OrderTotal': orderTotal,
      'OrderCurrencyID': orderCurrencyId
    };

    final Map<String, dynamic> map =
        await channel.invokeMapMethod<String, dynamic>('buyItem', {
              "appId": appId,
              "serverType": serverType,
              "payDetails": json.encode(orderDetails)
            }) ??
            <String, dynamic>{};
    return BillingResultWrapper.fromJson(map);
  }
}

@JsonSerializable()
@immutable
class IsAvailableResult {
  const IsAvailableResult({
    required this.status,
    required this.result,
    this.serviceYn,
    this.resultTitle,
  });

  @JsonKey(defaultValue: '')
  final String status;
  @JsonKey(defaultValue: '')
  final String result;
  @JsonKey(defaultValue: '')
  final String? serviceYn;
  @JsonKey(defaultValue: '')
  final String? resultTitle;

  factory IsAvailableResult.fromJson(Map<String, dynamic> map) =>
      _$IsAvailableResultFromJson(map);

  Map<String, dynamic> toJson() => _$IsAvailableResultToJson(this);
}

@JsonSerializable()
@immutable
class ProductsListApiResult {
  const ProductsListApiResult({
    required this.cPStatus,
    required this.cPResult,
    required this.itemDetails,
    this.result,
    this.resultTitle,
    this.status,
    this.checkValue,
    this.totalCount,
  });
  @JsonKey(defaultValue: '', name: 'CPStatus')
  final String cPStatus;
  @JsonKey(defaultValue: '', name: 'CPResult')
  final String cPResult;
  @JsonKey(defaultValue: 0, name: 'TotalCount')
  final int? totalCount;
  @JsonKey(defaultValue: '', name: 'CheckValue')
  final String? checkValue;
  @JsonKey(defaultValue: '')
  final String? result;
  @JsonKey(defaultValue: '')
  final String? resultTitle;
  @JsonKey(defaultValue: '')
  final String? status;
  @JsonKey(defaultValue: <ProductWrapper>[], name: 'ItemDetails')
  final List<ProductWrapper> itemDetails;

  factory ProductsListApiResult.fromJson(Map<String, dynamic> map) =>
      _$ProductsListApiResultFromJson(map);

  Map<String, dynamic> toJson() => _$ProductsListApiResultToJson(this);
}

@JsonSerializable()
@immutable
class ProductWrapper {
  const ProductWrapper({
    this.seq,
    required this.itemID,
    required this.itemTitle,
    required this.itemDesc,
    required this.itemType,
    required this.price,
    required this.currencyID,
  });
  @JsonKey(defaultValue: 0, name: 'Seq')
  final int? seq;
  @JsonKey(defaultValue: 0, name: 'ItemType')
  final int? itemType;
  @JsonKey(defaultValue: '', name: 'ItemID')
  final String itemID;
  @JsonKey(defaultValue: '', name: 'ItemTitle')
  final String itemTitle;
  @JsonKey(defaultValue: '', name: 'ItemDesc')
  final String itemDesc;
  @JsonKey(defaultValue: 0, name: 'Price')
  final num price;
  @JsonKey(defaultValue: '', name: 'CurrencyID')
  final String currencyID;

  factory ProductWrapper.fromJson(Map<String, dynamic> map) =>
      _$ProductWrapperFromJson(map);

  Map<String, dynamic> toJson() => _$ProductWrapperToJson(this);
}

@JsonSerializable()
@immutable
class ProductSubscriptionInfo {
  const ProductSubscriptionInfo({
    required this.paymentCycle,
    required this.paymentCycleFrq,
    required this.paymentCyclePeriod,
  });
  @JsonKey(defaultValue: '', name: 'PaymentCyclePeriod')
  final String paymentCyclePeriod;
  @JsonKey(defaultValue: 0, name: 'PaymentCycleFrq')
  final int paymentCycleFrq;
  @JsonKey(defaultValue: 0, name: 'PaymentCycle')
  final int paymentCycle;

  factory ProductSubscriptionInfo.fromJson(Map<String, dynamic> map) =>
      _$ProductSubscriptionInfoFromJson(map);

  Map<String, dynamic> toJson() => _$ProductSubscriptionInfoToJson(this);
}

class SamsungCheckoutProductDetails extends ProductDetails {
  SamsungCheckoutProductDetails({
    required super.id,
    required super.title,
    required super.description,
    required super.price,
    required super.currencyCode,
    required this.productWrapper,
    super.rawPrice = 0.0,
    super.currencySymbol,
  });

  final ProductWrapper productWrapper;

  factory SamsungCheckoutProductDetails.fromProduct(ProductWrapper product) {
    return SamsungCheckoutProductDetails(
      id: product.itemID,
      title: product.itemTitle,
      description: product.itemDesc,
      price: product.price.toString(),
      currencyCode: product.currencyID,
      productWrapper: product,
    );
  }
}

@JsonSerializable()
@immutable
class BillingResultWrapper {
  const BillingResultWrapper({required this.payResult, this.payDetails});
  @JsonKey(defaultValue: '')
  final String payResult;
  @JsonKey(defaultValue: <PaymentDetails>[])
  final List<PaymentDetails>? payDetails;

  factory BillingResultWrapper.fromJson(Map<String, dynamic> map) =>
      _$BillingResultWrapperFromJson(map);

  Map<String, dynamic> toJson() => _$BillingResultWrapperToJson(this);
}

@JsonSerializable()
@immutable
class PaymentDetails {
  const PaymentDetails(
      {this.orderItemID,
      this.orderTitle,
      this.orderTotal,
      this.orderCurrencyID,
      this.invoiceId});
  @JsonKey(defaultValue: '', name: 'OrderItemID')
  final String? orderItemID;
  @JsonKey(defaultValue: '', name: 'OrderTitle')
  final String? orderTitle;
  @JsonKey(defaultValue: '', name: 'OrderTotal')
  final String? orderTotal;
  @JsonKey(defaultValue: '', name: 'OrderCurrencyID')
  final String? orderCurrencyID;
  @JsonKey(defaultValue: '', name: 'InvoiceId')
  final String? invoiceId;

  factory PaymentDetails.fromJson(Map<String, dynamic> map) =>
      _$PaymentDetailsFromJson(map);

  Map<String, dynamic> toJson() => _$PaymentDetailsToJson(this);
}

@JsonSerializable()
@immutable
class PurchaseListAPIResult {
  const PurchaseListAPIResult(
      {required this.cPResult,
      required this.cPStatus,
      required this.invoiceDetails,
      this.totalCount,
      this.checkValue});
  @JsonKey(defaultValue: '', name: 'CPStatus')
  final String cPStatus;
  @JsonKey(defaultValue: '', name: 'CPResult')
  final String cPResult;
  @JsonKey(defaultValue: 0, name: 'TotalCount')
  final int? totalCount;
  @JsonKey(defaultValue: '', name: 'CheckValue')
  final String? checkValue;
  @JsonKey(defaultValue: <PurchaseWrapper>[])
  final List<PurchaseWrapper> invoiceDetails;

  factory PurchaseListAPIResult.fromJson(Map<String, dynamic> map) =>
      _$PurchaseListAPIResultFromJson(map);

  Map<String, dynamic> toJson() => _$PurchaseListAPIResultToJson(this);
}

@JsonSerializable()
@immutable
class PurchaseWrapper {
  const PurchaseWrapper({
    required this.itemType,
    required this.invoiceID,
    required this.itemID,
    required this.itemTitle,
    required this.price,
    required this.orderCurrencyID,
    required this.orderTime,
    required this.appliedStatus,
    required this.cancelStatus,
    this.appliedTime,
    this.seq,
    this.period,
    this.limitEndTime,
    this.remainTime,
  });
  @JsonKey(defaultValue: 0, name: 'Seq')
  final int? seq;
  @JsonKey(defaultValue: '', name: 'InvoiceID')
  final String invoiceID;
  @JsonKey(defaultValue: '', name: 'ItemID')
  final String itemID;
  @JsonKey(defaultValue: '', name: 'ItemTitle')
  final String itemTitle;
  @JsonKey(defaultValue: 0, name: 'ItemType')
  final int itemType;
  @JsonKey(defaultValue: '', name: 'OrderTime')
  final String orderTime;
  @JsonKey(defaultValue: 0, name: 'Period')
  final int? period;
  @JsonKey(defaultValue: 0, name: 'Price')
  final num price;
  @JsonKey(defaultValue: '', name: 'OrderCurrencyID')
  final String orderCurrencyID;
  @JsonKey(defaultValue: false, name: 'CancelStatus')
  final bool cancelStatus;
  @JsonKey(defaultValue: false, name: 'AppliedStatus')
  final bool appliedStatus;
  @JsonKey(defaultValue: '', name: 'AppliedTime')
  final String? appliedTime;
  @JsonKey(defaultValue: '', name: 'LimitEndTime')
  final String? limitEndTime;
  @JsonKey(defaultValue: '', name: 'RemainTime')
  final String? remainTime;

  factory PurchaseWrapper.fromJson(Map<String, dynamic> map) =>
      _$PurchaseWrapperFromJson(map);

  Map<String, dynamic> toJson() => _$PurchaseWrapperToJson(this);
}

class SamsungCheckoutPurchaseDetails extends PurchaseDetails {
  SamsungCheckoutPurchaseDetails({
    required super.productID, //itemID
    required super.purchaseID,
    required super.status,
    required super.transactionDate,
    required super.verificationData,
    required this.purchaseWrapper, //invoiceID
  });

  final PurchaseWrapper purchaseWrapper;

  factory SamsungCheckoutPurchaseDetails.fromPurchase(
      PurchaseWrapper purchase) {
    final SamsungCheckoutPurchaseDetails purchaseDetails =
        SamsungCheckoutPurchaseDetails(
      purchaseID: purchase.invoiceID,
      productID: purchase.itemID,
      verificationData: PurchaseVerificationData(
          localVerificationData: purchase.invoiceID,
          serverVerificationData: purchase.invoiceID,
          source: kIAPSource),
      transactionDate: purchase.orderTime.toString(),
      status: const PurchaseStateConverter()
          .toPurchaseStatus(purchase.appliedStatus),
      purchaseWrapper: purchase,
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
}

@JsonSerializable()
@immutable
class PurchaseSubscriptionInfo {
  const PurchaseSubscriptionInfo({
    required this.subscriptionId,
    required this.subsStartTime,
    required this.subsEndTime,
    required this.subsStatus,
  });
  @JsonKey(defaultValue: '', name: 'SubscriptionId')
  final String subscriptionId;
  @JsonKey(defaultValue: '', name: 'SubsStartTime')
  final String subsStartTime;
  @JsonKey(defaultValue: '', name: 'SubsEndTime')
  final String subsEndTime;
  @JsonKey(defaultValue: '', name: 'SubsStatus')
  final String subsStatus;

  factory PurchaseSubscriptionInfo.fromJson(Map<String, dynamic> map) =>
      _$PurchaseSubscriptionInfoFromJson(map);

  Map<String, dynamic> toJson() => _$PurchaseSubscriptionInfoToJson(this);
}

class PurchaseStateConverter {
  /// Default const constructor.
  const PurchaseStateConverter();

  PurchaseStatus toPurchaseStatus(bool object) {
    switch (object) {
      case false:
        return PurchaseStatus.pending;
      case true:
        return PurchaseStatus.purchased;
      default:
        return PurchaseStatus.error;
    }
  }
}
