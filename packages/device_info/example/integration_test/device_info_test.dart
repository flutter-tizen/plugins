// Copyright 2019, the Chromium project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

// TODO(cyanglaz): Remove once https://github.com/flutter/plugins/pull/3158 is landed.
// @dart = 2.9

import 'dart:io';
import 'package:flutter_test/flutter_test.dart';
import 'package:device_info_tizen/device_info_tizen.dart';
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();
  TizenDeviceInfo tizenInfo;

  setUpAll(() async {
    final DeviceInfoPluginTizen deviceInfo = DeviceInfoPluginTizen();
    tizenInfo = await deviceInfo.tizenInfo;
  });

  testWidgets('Can get non-null device modelName', (WidgetTester tester) async {
    expect(tizenInfo.modelName, isNotNull);
  });
}
