// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:package_info_plus/package_info_plus.dart';
import 'package:package_info_plus_example/main.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('fromPlatform', (WidgetTester tester) async {
    final info = await PackageInfo.fromPlatform();
    // These tests are based on the example app. The tests should be updated if any related info changes.
    expect(info.appName, 'package_info_plus_tizen_example');
    expect(info.buildNumber, isEmpty);
    expect(info.buildSignature, isEmpty);
    expect(info.packageName, 'org.tizen.package_info_plus_tizen_example');
    expect(info.version, '1.2.3');
    expect(info.installerStore, null);
  });

  testWidgets('example', (WidgetTester tester) async {
    await tester.pumpWidget(const MyApp());
    await tester.pumpAndSettle();
    expect(find.text('package_info_plus_tizen_example'), findsOneWidget);
    expect(
        find.text('org.tizen.package_info_plus_tizen_example'), findsOneWidget);
    expect(find.text('1.2.3'), findsOneWidget);
  });
}
