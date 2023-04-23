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
      CPStatus: json['CPStatus'] as String? ?? '',
      CPResult: json['CPResult'] as String? ?? '',
      ItemDetails: (json['ItemDetails'] as List<dynamic>?)
              ?.map((e) => ProductWrapper.fromJson(e as Map<String, dynamic>))
              .toList() ??
          [],
      result: json['result'] as String? ?? '',
      resultTitle: json['resultTitle'] as String? ?? '',
      status: json['status'] as String? ?? '',
      checkValue: json['checkValue'] as String? ?? '',
      totalCount: json['totalCount'] as int? ?? 0,
    );

Map<String, dynamic> _$ProductsListApiResultToJson(
        ProductsListApiResult instance) =>
    <String, dynamic>{
      'CPStatus': instance.CPStatus,
      'CPResult': instance.CPResult,
      'totalCount': instance.totalCount,
      'checkValue': instance.checkValue,
      'result': instance.result,
      'resultTitle': instance.resultTitle,
      'status': instance.status,
      'ItemDetails': instance.ItemDetails,
    };

ProductWrapper _$ProductWrapperFromJson(Map<String, dynamic> json) =>
    ProductWrapper(
      Seq: json['Seq'] as int? ?? 0,
      ItemID: json['ItemID'] as String? ?? '',
      ItemTitle: json['ItemTitle'] as String? ?? '',
      ItemDesc: json['ItemDesc'] as String? ?? '',
      ItemType: json['ItemType'] as int? ?? 0,
      Price: json['Price'] as num? ?? 0,
      CurrencyID: json['CurrencyID'] as String? ?? '',
    );

Map<String, dynamic> _$ProductWrapperToJson(ProductWrapper instance) =>
    <String, dynamic>{
      'Seq': instance.Seq,
      'ItemType': instance.ItemType,
      'ItemID': instance.ItemID,
      'ItemTitle': instance.ItemTitle,
      'ItemDesc': instance.ItemDesc,
      'Price': instance.Price,
      'CurrencyID': instance.CurrencyID,
    };

ProductSubscriptionInfo _$ProductSubscriptionInfoFromJson(
        Map<String, dynamic> json) =>
    ProductSubscriptionInfo(
      PaymentCycle: json['PaymentCycle'] as int? ?? 0,
      PaymentCycleFrq: json['PaymentCycleFrq'] as int? ?? 0,
      PaymentCyclePeriod: json['PaymentCyclePeriod'] as String? ?? '',
    );

Map<String, dynamic> _$ProductSubscriptionInfoToJson(
        ProductSubscriptionInfo instance) =>
    <String, dynamic>{
      'PaymentCyclePeriod': instance.PaymentCyclePeriod,
      'PaymentCycleFrq': instance.PaymentCycleFrq,
      'PaymentCycle': instance.PaymentCycle,
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
      OrderItemID: json['OrderItemID'] as String? ?? '',
      OrderTitle: json['OrderTitle'] as String? ?? '',
      OrderTotal: json['OrderTotal'] as String? ?? '',
      OrderCurrencyID: json['OrderCurrencyID'] as String? ?? '',
      InvoiceId: json['InvoiceId'] as String? ?? '',
    );

Map<String, dynamic> _$PaymentDetailsToJson(PaymentDetails instance) =>
    <String, dynamic>{
      'OrderItemID': instance.OrderItemID,
      'OrderTitle': instance.OrderTitle,
      'OrderTotal': instance.OrderTotal,
      'OrderCurrencyID': instance.OrderCurrencyID,
      'InvoiceId': instance.InvoiceId,
    };

PurchaseListAPIResult _$PurchaseListAPIResultFromJson(
        Map<String, dynamic> json) =>
    PurchaseListAPIResult(
      CPResult: json['CPResult'] as String? ?? '',
      CPStatus: json['CPStatus'] as String? ?? '',
      InvoiceDetails: (json['InvoiceDetails'] as List<dynamic>?)
              ?.map((e) => PurchaseWrapper.fromJson(e as Map<String, dynamic>))
              .toList() ??
          [],
      TotalCount: json['TotalCount'] as int? ?? 0,
      CheckValue: json['CheckValue'] as String? ?? '',
    );

Map<String, dynamic> _$PurchaseListAPIResultToJson(
        PurchaseListAPIResult instance) =>
    <String, dynamic>{
      'CPStatus': instance.CPStatus,
      'CPResult': instance.CPResult,
      'TotalCount': instance.TotalCount,
      'CheckValue': instance.CheckValue,
      'InvoiceDetails': instance.InvoiceDetails,
    };

PurchaseWrapper _$PurchaseWrapperFromJson(Map<String, dynamic> json) =>
    PurchaseWrapper(
      ItemType: json['ItemType'] as int? ?? 0,
      InvoiceID: json['InvoiceID'] as String? ?? '',
      ItemID: json['ItemID'] as String? ?? '',
      ItemTitle: json['ItemTitle'] as String? ?? '',
      Price: json['Price'] as num? ?? 0,
      OrderCurrencyID: json['OrderCurrencyID'] as String? ?? '',
      OrderTime: json['OrderTime'] as String? ?? '',
      AppliedStatus: json['AppliedStatus'] as bool? ?? false,
      CancelStatus: json['CancelStatus'] as bool? ?? false,
      AppliedTime: json['AppliedTime'] as String? ?? '',
      Seq: json['Seq'] as int? ?? 0,
      Period: json['Period'] as int? ?? 0,
      LimitEndTime: json['LimitEndTime'] as String? ?? '',
      RemainTime: json['RemainTime'] as String? ?? '',
    );

Map<String, dynamic> _$PurchaseWrapperToJson(PurchaseWrapper instance) =>
    <String, dynamic>{
      'Seq': instance.Seq,
      'InvoiceID': instance.InvoiceID,
      'ItemID': instance.ItemID,
      'ItemTitle': instance.ItemTitle,
      'ItemType': instance.ItemType,
      'OrderTime': instance.OrderTime,
      'Period': instance.Period,
      'Price': instance.Price,
      'OrderCurrencyID': instance.OrderCurrencyID,
      'CancelStatus': instance.CancelStatus,
      'AppliedStatus': instance.AppliedStatus,
      'AppliedTime': instance.AppliedTime,
      'LimitEndTime': instance.LimitEndTime,
      'RemainTime': instance.RemainTime,
    };

PurchaseSubscriptionInfo _$PurchaseSubscriptionInfoFromJson(
        Map<String, dynamic> json) =>
    PurchaseSubscriptionInfo(
      SubscriptionId: json['SubscriptionId'] as String? ?? '',
      SubsStartTime: json['SubsStartTime'] as String? ?? '',
      SubsEndTime: json['SubsEndTime'] as String? ?? '',
      SubsStatus: json['SubsStatus'] as String? ?? '',
    );

Map<String, dynamic> _$PurchaseSubscriptionInfoToJson(
        PurchaseSubscriptionInfo instance) =>
    <String, dynamic>{
      'SubscriptionId': instance.SubscriptionId,
      'SubsStartTime': instance.SubsStartTime,
      'SubsEndTime': instance.SubsEndTime,
      'SubsStatus': instance.SubsStatus,
    };
