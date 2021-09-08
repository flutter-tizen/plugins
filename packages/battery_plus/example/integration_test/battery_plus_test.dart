// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// @dart = 2.9

import 'package:flutter_test/flutter_test.dart';
import 'package:battery_plus/battery_plus.dart';
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('Can get battery level', (WidgetTester tester) async {
    final battery = Battery();
    final batteryLevel = await battery.batteryLevel;
    expect(batteryLevel, isNotNull);
  });
}
