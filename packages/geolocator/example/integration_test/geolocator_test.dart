// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter_test/flutter_test.dart';
import 'package:geolocator/geolocator.dart';
import 'package:integration_test/integration_test.dart';

// Note: To pass all tests on the emulator, please inject the location using the control panel

Future<void> main() async {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  test('request permission', () async {
    expect(await Geolocator.requestPermission(), LocationPermission.always);
  }, timeout: const Timeout(Duration(seconds: 10)));

  test('check permission', () async {
    expect(await Geolocator.checkPermission(), LocationPermission.always);
  }, timeout: const Timeout(Duration(seconds: 10)));

  test('is location service enabled', () async {
    expect(await Geolocator.isLocationServiceEnabled(), true);
  }, timeout: const Timeout(Duration(seconds: 10)));

  test('get current position', () async {
    expect(await Geolocator.getCurrentPosition(), isA<Position>());
  }, timeout: const Timeout(Duration(seconds: 10)));

  test('get last known position', () async {
    expect(await Geolocator.getLastKnownPosition(), isA<Position>());
  }, timeout: const Timeout(Duration(seconds: 10)));

  test('listen location', () async {
    final Completer<Object> completer = Completer<Object>();
    final StreamSubscription<Position> subscription =
        Geolocator.getPositionStream().listen(
      (position) {
        if (!completer.isCompleted) {
          completer.complete(position);
        }
      },
      onError: (Object error) {
        completer.completeError(error);
      },
    );
    expect(await completer.future, isA<Position>());
    await subscription.cancel();
  }, timeout: const Timeout(Duration(seconds: 10)));
}
