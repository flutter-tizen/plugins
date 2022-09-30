// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_app_manager/tizen_app_manager.dart';
import 'package:tizen_app_manager_example/main.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('Can get current app info', (WidgetTester tester) async {
    // These test are based on the example app.
    final String appId = await AppManager.currentAppId;
    expect(appId, 'org.tizen.tizen_app_manager_example');
    final AppInfo appInfo = await AppManager.getAppInfo(appId);
    expect(appInfo.packageId, 'org.tizen.tizen_app_manager_example');
    expect(appInfo.label, 'tizen_app_manager_example');
    expect(appInfo.appType, 'dotnet');
  });

  testWidgets('Can get current app running context',
      (WidgetTester tester) async {
    await tester.pumpWidget(const MyApp());

    final String appId = await AppManager.currentAppId;
    final AppRunningContext context = AppRunningContext(appId: appId);
    expect(context.appState, AppState.foreground);
    expect(context.processId, isPositive);
    expect(context.isTerminated, isFalse);
  });
}
