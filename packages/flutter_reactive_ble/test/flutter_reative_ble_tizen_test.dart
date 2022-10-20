// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/services.dart';
import 'package:flutter_reactive_ble_tizen/flutter_reactive_ble_tizen.dart';
import 'package:flutter_reactive_ble_tizen/src/models.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:reactive_ble_platform_interface/reactive_ble_platform_interface.dart';

void main() {
  TestWidgetsFlutterBinding.ensureInitialized();

  late ReactiveBleTizen plugin;
  late List<MethodCall> methodCalls;

  setUp(() {
    plugin = ReactiveBleTizen();
    methodCalls = <MethodCall>[];

    plugin.methodChannel.setMockMethodCallHandler((MethodCall methodCall) {
      methodCalls.add(methodCall);
      return null;
    });
  });

  test('scanForDevices', () {
    final uuids = [
      // 16-bit UUID.
      Uuid.parse('0A0A'),
      // 32-bit UUID.
      Uuid.parse('0A0A0A0A'),
      // 128-bit UUID.
      Uuid.parse('0A0A0A0A-0A0A-0A0A-0A0A-0A0A0A0A0A0A'),
    ];
    plugin.scanForDevices(
      withServices: uuids,
      scanMode: ScanMode.balanced,
      requireLocationServicesEnabled: true,
    );

    expect(
      methodCalls.first,
      isMethodCall(
        'scanForDevices',
        arguments: <String, Object>{
          kServiceIds: <String>[
            '0a0a',
            '0a0a0a0a',
            '0a0a0a0a-0a0a-0a0a-0a0a-0a0a0a0a0a0a',
          ],
        },
      ),
    );
  });

  test('subscribeToNotifications', () {
    plugin.subscribeToNotifications(QualifiedCharacteristic(
      deviceId: '01:23:45:67:89:AB',
      // Battery Service UUID.
      serviceId: Uuid([0x18, 0x0f]),
      // Battery Level Characteristic UUID.
      characteristicId: Uuid([0x2a, 0x19]),
    ));

    expect(
      methodCalls.first,
      isMethodCall(
        'subscribeToNotifications',
        arguments: <String, Object>{
          kQualifiedCharacteristic: <String, Object>{
            'device_id': '01:23:45:67:89:AB',
            'service_id': '180f',
            'characteristic_id': '2a19',
          }
        },
      ),
    );
  });
}
