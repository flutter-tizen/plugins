// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_app_manager/app_running_context.dart';
import 'package:tizen_app_manager/tizen_app_manager.dart';

import 'package:tizen_app_manager_example/main.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('Can get current app info', (WidgetTester tester) async {
    // These test are based on the example app.
    final appId = await TizenAppManager.currentAppId;
    expect(appId, 'com.example.tizen_app_manager_example');
    final appInfo = await TizenAppManager.getAppInfo(appId);
    expect(appInfo.packageId, 'com.example.tizen_app_manager_example');
    expect(appInfo.label, 'tizen_app_manager_example');
    expect(appInfo.applicationType, 'dotnet');
  });

  testWidgets('Can get current app running context',
      (WidgetTester tester) async {
    await tester.pumpWidget(const MyApp());
    await tester.pumpAndSettle();

    final appId = await TizenAppManager.currentAppId;
    final isRunning = await TizenAppManager.isRunning(appId);
    final context = AppRunningContext(appId: appId);
    expect(context.appState, AppState.foreground);
    expect(context.processId, isPositive);
    expect(context.isTerminated, equals(!isRunning));
  });
}
