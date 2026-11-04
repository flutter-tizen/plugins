// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter_test/flutter_test.dart';
import 'package:in_app_purchase/in_app_purchase.dart';
import 'package:in_app_purchase_tizen/in_app_purchase_tizen.dart';
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  test('Can create InAppPurchase instance', () async {
    final InAppPurchase iapInstance = InAppPurchase.instance;
    expect(iapInstance, isNotNull);
  });

  test('FFI host API smoke test', () async {
    final InAppPurchase iap = InAppPurchase.instance;
    final InAppPurchaseTizenPlatformAddition addition =
        iap.getPlatformAddition<InAppPurchaseTizenPlatformAddition>();
    addition.setRequestParameters(
      appId: 'test',
      pageSize: 15,
      pageNum: 1,
      securityKey: 'test',
    );

    Object? asyncError;
    bool? available;
    try {
      available = await iap.isAvailable().timeout(const Duration(seconds: 15));
    } catch (error) {
      asyncError = error;
    }
    expect(
      asyncError,
      isNot(isA<TimeoutException>()),
      reason: 'isAvailable() never completed: FFI reply never arrived',
    );
    final asyncErrorText = '$asyncError';
    expect(asyncErrorText, isNot(contains('Failed to lookup symbol')));
    expect(asyncErrorText, isNot(contains('has not been set up')));
    expect(asyncErrorText, isNot(contains('BinaryMessenger is not supported')));
    if (available == null && asyncError == null) {
      fail('isAvailable() produced neither a value nor an error');
    }

    Object? queryError;
    ProductDetailsResponse? response;
    try {
      response = await iap
          .queryProductDetails(<String>{})
          .timeout(const Duration(seconds: 30));
    } catch (error) {
      queryError = error;
    }
    expect(
      queryError,
      isNot(isA<TimeoutException>()),
      reason: 'queryProductDetails() never completed: FFI reply never arrived',
    );
    final queryErrorText = '$queryError';
    expect(queryErrorText, isNot(contains('Failed to lookup symbol')));
    expect(queryErrorText, isNot(contains('has not been set up')));
    expect(queryErrorText, isNot(contains('BinaryMessenger is not supported')));
    if (response == null && queryError == null) {
      fail('queryProductDetails() produced neither a value nor an error');
    }
  });
}
