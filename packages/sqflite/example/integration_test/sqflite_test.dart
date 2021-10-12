// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.9

import 'dart:async';

import 'package:flutter_test/flutter_test.dart';
import 'package:sqflite/sqflite.dart';
import 'package:integration_test/integration_test.dart';

// Note: To pass all tests on the emulator, please inject the location using the control panel

Future<void> main() async {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('check permission', (WidgetTester tester) async {
    expect(await Sqflite.instance.checkPermission(), isA<LocationPermission>());
  }, timeout: const Timeout(Duration(seconds: 10)));

  testWidgets('request permission', (WidgetTester tester) async {
    expect(await GeolocatorPlatform.instance.requestPermission(),
        isA<LocationPermission>());
  }, timeout: const Timeout(Duration(seconds: 10)));

  testWidgets('get location service enabled', (WidgetTester tester) async {
    expect(await GeolocatorPlatform.instance.isLocationServiceEnabled(),
        isA<bool>());
  }, timeout: const Timeout(Duration(seconds: 10)));

  testWidgets('get current position', (WidgetTester tester) async {
    expect(await GeolocatorPlatform.instance.getCurrentPosition(),
        isA<Position>());
  }, timeout: const Timeout(Duration(seconds: 10)));

  testWidgets('get last known position', (WidgetTester tester) async {
    expect(await GeolocatorPlatform.instance.getLastKnownPosition(),
        isA<Position>());
  }, timeout: const Timeout(Duration(seconds: 10)));

  StreamSubscription<Position> positionStreamSubscription;
  testWidgets('listen location', (WidgetTester tester) async {
    final Completer<dynamic> started = Completer<dynamic>();
    final Stream<Position> positionStream =
        GeolocatorPlatform.instance.getPositionStream();
    positionStreamSubscription = positionStream.handleError((error) {
      positionStreamSubscription?.cancel();
      positionStreamSubscription = null;
      started.completeError(error);
    }).listen((position) {
      if (!started.isCompleted) {
        started.complete(position);
      }
    });
    final dynamic value = await started.future;
    expect(value, isA<Position>());
  }, timeout: const Timeout(Duration(seconds: 10)));
}
