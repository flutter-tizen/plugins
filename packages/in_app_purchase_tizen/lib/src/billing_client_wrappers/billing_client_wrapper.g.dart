// GENERATED CODE - DO NOT MODIFY BY HAND

part of 'billing_client_wrapper.dart';

// **************************************************************************
// JsonSerializableGenerator
// **************************************************************************

IsAvailableResult _$IsAvailableResultFromJson(Map<String, dynamic> json) =>
    IsAvailableResult(
      status: json['status'] as String? ?? '',
      result: json['result'] as String? ?? '',
      serviceYn: json['serviceYn'] as String? ?? '',
      resultTitle: json['resultTitle'] as String? ?? '',
    );

Map<String, dynamic> _$IsAvailableResultToJson(IsAvailableResult instance) =>
    <String, dynamic>{
      'status': instance.status,
      'result': instance.result,
      'serviceYn': instance.serviceYn,
      'resultTitle': instance.resultTitle,
    };

ProductsListApiResult _$ProductsListApiResultFromJson(
        Map<String, dynamic> json) =>
    ProductsListApiResult(
      cPStatus: json['CPStatus'] as String? ?? '',
      cPResult: json['CPResult'] as String? ?? '',
      itemDetails: (json['ItemDetails'] as List<dynamic>?)
              ?.map((e) => ProductWrapper.fromJson(e as Map<String, dynamic>))
              .toList() ??
          [],
      result: json['result'] as String? ?? '',
      resultTitle: json['resultTitle'] as String? ?? '',
      status: json['status'] as String? ?? '',
      checkValue: json['CheckValue'] as String? ?? '',
      totalCount: json['TotalCount'] as int? ?? 0,
    );

Map<String, dynamic> _$ProductsListApiResultToJson(
        ProductsListApiResult instance) =>
    <String, dynamic>{
      'CPStatus': instance.cPStatus,
      'CPResult': instance.cPResult,
      'TotalCount': instance.totalCount,
      'CheckValue': instance.checkValue,
      'result': instance.result,
      'resultTitle': instance.resultTitle,
      'status': instance.status,
      'ItemDetails': instance.itemDetails,
    };

ProductWrapper _$ProductWrapperFromJson(Map<String, dynamic> json) =>
    ProductWrapper(
      seq: json['Seq'] as int? ?? 0,
      itemID: json['ItemID'] as String? ?? '',
      itemTitle: json['ItemTitle'] as String? ?? '',
      itemDesc: json['ItemDesc'] as String? ?? '',
      itemType: json['ItemType'] as int? ?? 0,
      price: json['Price'] as num? ?? 0,
      currencyID: json['CurrencyID'] as String? ?? '',
    );

Map<String, dynamic> _$ProductWrapperToJson(ProductWrapper instance) =>
    <String, dynamic>{
      'Seq': instance.seq,
      'ItemType': instance.itemType,
      'ItemID': instance.itemID,
      'ItemTitle': instance.itemTitle,
      'ItemDesc': instance.itemDesc,
      'Price': instance.price,
      'CurrencyID': instance.currencyID,
    };

ProductSubscriptionInfo _$ProductSubscriptionInfoFromJson(
        Map<String, dynamic> json) =>
    ProductSubscriptionInfo(
      paymentCycle: json['PaymentCycle'] as int? ?? 0,
      paymentCycleFrq: json['PaymentCycleFrq'] as int? ?? 0,
      paymentCyclePeriod: json['PaymentCyclePeriod'] as String? ?? '',
    );

Map<String, dynamic> _$ProductSubscriptionInfoToJson(
        ProductSubscriptionInfo instance) =>
    <String, dynamic>{
      'PaymentCyclePeriod': instance.paymentCyclePeriod,
      'PaymentCycleFrq': instance.paymentCycleFrq,
      'PaymentCycle': instance.paymentCycle,
    };

BillingResultWrapper _$BillingResultWrapperFromJson(
        Map<String, dynamic> json) =>
    BillingResultWrapper(
      payResult: json['payResult'] as String? ?? '',
      payDetails: (json['payDetails'] as List<dynamic>?)
              ?.map((e) => PaymentDetails.fromJson(e as Map<String, dynamic>))
              .toList() ??
          [],
    );

Map<String, dynamic> _$BillingResultWrapperToJson(
        BillingResultWrapper instance) =>
    <String, dynamic>{
      'payResult': instance.payResult,
      'payDetails': instance.payDetails,
    };

PaymentDetails _$PaymentDetailsFromJson(Map<String, dynamic> json) =>
    PaymentDetails(
      orderItemID: json['OrderItemID'] as String? ?? '',
      orderTitle: json['OrderTitle'] as String? ?? '',
      orderTotal: json['OrderTotal'] as String? ?? '',
      orderCurrencyID: json['OrderCurrencyID'] as String? ?? '',
      invoiceId: json['InvoiceId'] as String? ?? '',
    );

Map<String, dynamic> _$PaymentDetailsToJson(PaymentDetails instance) =>
    <String, dynamic>{
      'OrderItemID': instance.orderItemID,
      'OrderTitle': instance.orderTitle,
      'OrderTotal': instance.orderTotal,
      'OrderCurrencyID': instance.orderCurrencyID,
      'InvoiceId': instance.invoiceId,
    };

PurchaseListAPIResult _$PurchaseListAPIResultFromJson(
        Map<String, dynamic> json) =>
    PurchaseListAPIResult(
      cPResult: json['CPResult'] as String? ?? '',
      cPStatus: json['CPStatus'] as String? ?? '',
      invoiceDetails: (json['invoiceDetails'] as List<dynamic>?)
              ?.map((e) => PurchaseWrapper.fromJson(e as Map<String, dynamic>))
              .toList() ??
          [],
      totalCount: json['TotalCount'] as int? ?? 0,
      checkValue: json['CheckValue'] as String? ?? '',
    );

Map<String, dynamic> _$PurchaseListAPIResultToJson(
        PurchaseListAPIResult instance) =>
    <String, dynamic>{
      'CPStatus': instance.cPStatus,
      'CPResult': instance.cPResult,
      'TotalCount': instance.totalCount,
      'CheckValue': instance.checkValue,
      'invoiceDetails': instance.invoiceDetails,
    };

PurchaseWrapper _$PurchaseWrapperFromJson(Map<String, dynamic> json) =>
    PurchaseWrapper(
      itemType: json['ItemType'] as int? ?? 0,
      invoiceID: json['InvoiceID'] as String? ?? '',
      itemID: json['ItemID'] as String? ?? '',
      itemTitle: json['ItemTitle'] as String? ?? '',
      price: json['Price'] as num? ?? 0,
      orderCurrencyID: json['OrderCurrencyID'] as String? ?? '',
      orderTime: json['OrderTime'] as String? ?? '',
      appliedStatus: json['AppliedStatus'] as bool? ?? false,
      cancelStatus: json['CancelStatus'] as bool? ?? false,
      appliedTime: json['AppliedTime'] as String? ?? '',
      seq: json['Seq'] as int? ?? 0,
      period: json['Period'] as int? ?? 0,
      limitEndTime: json['LimitEndTime'] as String? ?? '',
      remainTime: json['RemainTime'] as String? ?? '',
    );

Map<String, dynamic> _$PurchaseWrapperToJson(PurchaseWrapper instance) =>
    <String, dynamic>{
      'Seq': instance.seq,
      'InvoiceID': instance.invoiceID,
      'ItemID': instance.itemID,
      'ItemTitle': instance.itemTitle,
      'ItemType': instance.itemType,
      'OrderTime': instance.orderTime,
      'Period': instance.period,
      'Price': instance.price,
      'OrderCurrencyID': instance.orderCurrencyID,
      'CancelStatus': instance.cancelStatus,
      'AppliedStatus': instance.appliedStatus,
      'AppliedTime': instance.appliedTime,
      'LimitEndTime': instance.limitEndTime,
      'RemainTime': instance.remainTime,
    };

PurchaseSubscriptionInfo _$PurchaseSubscriptionInfoFromJson(
        Map<String, dynamic> json) =>
    PurchaseSubscriptionInfo(
      subscriptionId: json['SubscriptionId'] as String? ?? '',
      subsStartTime: json['SubsStartTime'] as String? ?? '',
      subsEndTime: json['SubsEndTime'] as String? ?? '',
      subsStatus: json['SubsStatus'] as String? ?? '',
    );

Map<String, dynamic> _$PurchaseSubscriptionInfoToJson(
        PurchaseSubscriptionInfo instance) =>
    <String, dynamic>{
      'SubscriptionId': instance.subscriptionId,
      'SubsStartTime': instance.subsStartTime,
      'SubsEndTime': instance.subsEndTime,
      'SubsStatus': instance.subsStatus,
    };
