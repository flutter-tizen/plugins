// Copyright 2019, the Chromium project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

// @dart=2.9

import 'package:flutter_test/flutter_test.dart';
import 'package:device_info_plus_tizen/device_info_plus_tizen.dart';
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  TizenDeviceInfo tizenInfo;

  setUpAll(() async {
    final deviceInfoPlugin = DeviceInfoPluginTizen();
    tizenInfo = await deviceInfoPlugin.tizenInfo;
  });

  testWidgets('Can get non-null device model', (WidgetTester tester) async {
    expect(tizenInfo.modelName, isNotNull);
  });
}
