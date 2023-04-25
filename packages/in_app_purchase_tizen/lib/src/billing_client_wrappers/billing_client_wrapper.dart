// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: non_constant_identifier_names

import 'dart:async';
import 'dart:convert' show json;

import 'package:flutter/services.dart';
import 'package:in_app_purchase_platform_interface/in_app_purchase_platform_interface.dart';
import 'package:json_annotation/json_annotation.dart';
import 'dart:convert';

import '../in_app_purchase_tizen_platform.dart';
import '../channel.dart';

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
        throw PlatformException(
            code: isAvailable.result, message: isAvailable.resultTitle);
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
class IsAvailableResult {
  IsAvailableResult({
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

  factory IsAvailableResult.fromJson(Map<String, dynamic> map) {
    return _$IsAvailableResultFromJson(map);
  }
}

@JsonSerializable()
class ProductsListApiResult {
  ProductsListApiResult({
    required this.CPStatus,
    required this.CPResult,
    required this.ItemDetails,
    this.result,
    this.resultTitle,
    this.status,
    this.checkValue,
    this.totalCount,
  });
  @JsonKey(defaultValue: '')
  final String CPStatus;
  @JsonKey(defaultValue: '')
  final String CPResult;
  @JsonKey(defaultValue: 0)
  final int? totalCount;
  @JsonKey(defaultValue: '')
  final String? checkValue;
  @JsonKey(defaultValue: '')
  final String? result;
  @JsonKey(defaultValue: '')
  final String? resultTitle;
  @JsonKey(defaultValue: '')
  final String? status;
  @JsonKey(defaultValue: <ProductWrapper>[])
  List<ProductWrapper> ItemDetails;

  factory ProductsListApiResult.fromJson(Map<String, dynamic> map) {
    return _$ProductsListApiResultFromJson(map);
  }
}

@JsonSerializable()
class ProductWrapper {
  ProductWrapper({
    this.Seq,
    required this.ItemID,
    required this.ItemTitle,
    required this.ItemDesc,
    required this.ItemType,
    required this.Price,
    required this.CurrencyID,
  });
  @JsonKey(defaultValue: 0)
  final int? Seq;
  @JsonKey(defaultValue: 0)
  final int? ItemType;
  @JsonKey(defaultValue: '')
  final String ItemID;
  @JsonKey(defaultValue: '')
  final String ItemTitle;
  @JsonKey(defaultValue: '')
  final String ItemDesc;
  @JsonKey(defaultValue: 0)
  final num Price;
  @JsonKey(defaultValue: '')
  final String CurrencyID;

  factory ProductWrapper.fromJson(Map<String, dynamic> map) {
    return _$ProductWrapperFromJson(map);
  }
}

@JsonSerializable()
class ProductSubscriptionInfo {
  ProductSubscriptionInfo({
    required this.PaymentCycle,
    required this.PaymentCycleFrq,
    required this.PaymentCyclePeriod,
  });
  @JsonKey(defaultValue: '')
  final String PaymentCyclePeriod;
  @JsonKey(defaultValue: 0)
  final int PaymentCycleFrq;
  @JsonKey(defaultValue: 0)
  final int PaymentCycle;

  factory ProductSubscriptionInfo.fromJson(Map<String, dynamic> map) {
    return _$ProductSubscriptionInfoFromJson(map);
  }
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
      id: product.ItemID,
      title: product.ItemTitle,
      description: product.ItemDesc,
      price: product.Price.toString(),
      currencyCode: product.CurrencyID,
      productWrapper: product,
    );
  }
}

@JsonSerializable()
class BillingResultWrapper {
  BillingResultWrapper({required this.payResult, this.payDetails});
  @JsonKey(defaultValue: '')
  final String payResult;
  @JsonKey(defaultValue: <PaymentDetails>[])
  final List<PaymentDetails>? payDetails;

  factory BillingResultWrapper.fromJson(Map<String, dynamic>? map) {
    return _$BillingResultWrapperFromJson(map!);
  }
}

@JsonSerializable()
class PaymentDetails {
  PaymentDetails(
      {this.OrderItemID,
      this.OrderTitle,
      this.OrderTotal,
      this.OrderCurrencyID,
      this.InvoiceId});
  @JsonKey(defaultValue: '')
  final String? OrderItemID;
  @JsonKey(defaultValue: '')
  final String? OrderTitle;
  @JsonKey(defaultValue: '')
  final String? OrderTotal;
  @JsonKey(defaultValue: '')
  final String? OrderCurrencyID;
  @JsonKey(defaultValue: '')
  final String? InvoiceId;

  factory PaymentDetails.fromJson(Map<String, dynamic> map) {
    return _$PaymentDetailsFromJson(map);
  }
}

@JsonSerializable()
class PurchaseListAPIResult {
  PurchaseListAPIResult(
      {required this.CPResult,
      required this.CPStatus,
      required this.InvoiceDetails,
      this.TotalCount,
      this.CheckValue});
  @JsonKey(defaultValue: '')
  final String CPStatus;
  @JsonKey(defaultValue: '')
  final String CPResult;
  @JsonKey(defaultValue: 0)
  final int? TotalCount;
  @JsonKey(defaultValue: '')
  final String? CheckValue;
  @JsonKey(defaultValue: <PurchaseWrapper>[])
  final List<PurchaseWrapper> InvoiceDetails;

  factory PurchaseListAPIResult.fromJson(Map<String, dynamic> map) =>
      _$PurchaseListAPIResultFromJson(map);
}

@JsonSerializable()
class PurchaseWrapper {
  PurchaseWrapper({
    required this.ItemType,
    required this.InvoiceID,
    required this.ItemID,
    required this.ItemTitle,
    required this.Price,
    required this.OrderCurrencyID,
    required this.OrderTime,
    required this.AppliedStatus,
    required this.CancelStatus,
    this.AppliedTime,
    this.Seq,
    this.Period,
    this.LimitEndTime,
    this.RemainTime,
  });
  @JsonKey(defaultValue: 0)
  final int? Seq;
  @JsonKey(defaultValue: '')
  final String InvoiceID;
  @JsonKey(defaultValue: '')
  final String ItemID;
  @JsonKey(defaultValue: '')
  final String ItemTitle;
  @JsonKey(defaultValue: 0)
  final int ItemType;
  @JsonKey(defaultValue: '')
  final String OrderTime;
  @JsonKey(defaultValue: 0)
  final int? Period;
  @JsonKey(defaultValue: 0)
  final num Price;
  @JsonKey(defaultValue: '')
  final String OrderCurrencyID;
  @JsonKey(defaultValue: false)
  final bool CancelStatus;
  @JsonKey(defaultValue: false)
  final bool AppliedStatus;
  @JsonKey(defaultValue: '')
  final String? AppliedTime;
  @JsonKey(defaultValue: '')
  final String? LimitEndTime;
  @JsonKey(defaultValue: '')
  final String? RemainTime;

  factory PurchaseWrapper.fromJson(Map<String, dynamic> map) =>
      _$PurchaseWrapperFromJson(map);
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
      purchaseID: purchase.InvoiceID,
      productID: purchase.ItemID,
      verificationData: PurchaseVerificationData(
          localVerificationData: purchase.InvoiceID,
          serverVerificationData: purchase.InvoiceID,
          source: kIAPSource),
      transactionDate: purchase.OrderTime.toString(),
      status: const PurchaseStateConverter()
          .toPurchaseStatus(purchase.AppliedStatus),
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
class PurchaseSubscriptionInfo {
  PurchaseSubscriptionInfo({
    required this.SubscriptionId,
    required this.SubsStartTime,
    required this.SubsEndTime,
    required this.SubsStatus,
  });
  @JsonKey(defaultValue: '')
  String SubscriptionId;
  @JsonKey(defaultValue: '')
  String SubsStartTime;
  @JsonKey(defaultValue: '')
  String SubsEndTime;
  @JsonKey(defaultValue: '')
  String SubsStatus;

  factory PurchaseSubscriptionInfo.fromJson(Map<String, dynamic> map) =>
      _$PurchaseSubscriptionInfoFromJson(map);
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
