// Copyright 2019, the Chromium project authors.  Please see the AUTHORS file
// for details. All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

import 'dart:io';

import 'package:flutter_test/flutter_test.dart';
import 'package:device_info_plus_tizen/device_info_plus_tizen.dart';
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  late TizenDeviceInfo tizenInfo;

  setUpAll(() async {
    final deviceInfoPlugin = DeviceInfoPluginTizen();
    tizenInfo = await deviceInfoPlugin.tizenInfo;
  });

  testWidgets('Can get non-null device model', (WidgetTester tester) async {
    expect(tizenInfo.modelName, isNotNull);
  });

  testWidgets('Check all Tizen info values are available',
      (WidgetTester tester) async {
    expect(tizenInfo.modelName, isNotNull);
    expect(tizenInfo.cpuArch, isNotNull);
    expect(tizenInfo.nativeApiVersion, isNotNull);
    expect(tizenInfo.platformVersion, isNotNull);
    expect(tizenInfo.webApiVersion, isNotNull);
    expect(tizenInfo.profile, isNotNull);
    expect(tizenInfo.buildDate, isNotNull);
    expect(tizenInfo.buildId, isNotNull);
    expect(tizenInfo.buildString, isNotNull);
    expect(tizenInfo.buildTime, isNotNull);
    expect(tizenInfo.buildType, isNotNull);
    expect(tizenInfo.buildVariant, isNotNull);
    expect(tizenInfo.buildRelease, isNotNull);
    expect(tizenInfo.deviceType, isNotNull);
    expect(tizenInfo.manufacturer, isNotNull);
    expect(tizenInfo.platformName, isNotNull);
    expect(tizenInfo.platformProcessor, isNotNull);
    expect(tizenInfo.tizenId, isNotNull);
  }, skip: !Platform.isLinux);
}
