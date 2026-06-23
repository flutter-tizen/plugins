// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/services.dart';
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

  testWidgets('Can get current app running context', (
    WidgetTester tester,
  ) async {
    await tester.pumpWidget(const MyApp());

    final String appId = await AppManager.currentAppId;
    final AppRunningContext context = AppRunningContext(appId: appId);

    // TODO(seungsoo47): The `AppRunningContext.appState` API always returns the
    // appropriate value. However, we decided to comment out this testcase
    // because the tizen test farm does not support the 'foreground' state.
    // expect(context.appState, AppState.foreground);
    expect(context.processId, isPositive);
    expect(context.isTerminated, isFalse);
  });

  group('AppManager', () {
    group('getInstalledApps', () {
      testWidgets('returns non-empty list', (WidgetTester _) async {
        final List<AppInfo> apps = await AppManager.getInstalledApps();
        expect(apps, isNotEmpty);
      });

      testWidgets('includes the current app', (WidgetTester _) async {
        final String appId = await AppManager.currentAppId;
        final List<AppInfo> apps = await AppManager.getInstalledApps();
        expect(apps.any((AppInfo app) => app.appId == appId), isTrue);
      });
    });

    group('isRunning', () {
      testWidgets('returns true for running app', (WidgetTester _) async {
        final String appId = await AppManager.currentAppId;
        expect(await AppManager.isRunning(appId), isTrue);
      });

      testWidgets('returns false for non-running app', (WidgetTester _) async {
        expect(
          await AppManager.isRunning('org.tizen.nonexistent'),
          isFalse,
        );
      });

      testWidgets('throws ArgumentError for empty appId', (WidgetTester _) async {
        await expectLater(AppManager.isRunning(''), throwsArgumentError);
      });
    });

    group('getAppInfo', () {
      testWidgets('returns all AppInfo fields', (WidgetTester _) async {
        final String appId = await AppManager.currentAppId;
        final AppInfo appInfo = await AppManager.getAppInfo(appId);
        expect(appInfo.appId, appId);
        expect(appInfo.executablePath, isNotEmpty);
        expect(appInfo.sharedResourcePath, isNotEmpty);
      });

      testWidgets('throws ArgumentError for empty appId', (WidgetTester _) async {
        await expectLater(AppManager.getAppInfo(''), throwsArgumentError);
      });
    });
  });

  group('AppRunningContext', () {
    testWidgets('packageId matches current app', (WidgetTester _) async {
      final String appId = await AppManager.currentAppId;
      final AppRunningContext context = AppRunningContext(appId: appId);
      expect(context.packageId, 'org.tizen.tizen_app_manager_example');
    });

    testWidgets('throws PlatformException for non-running appId', (
      WidgetTester _,
    ) async {
      expect(
        () => AppRunningContext(appId: 'org.tizen.nonexistent'),
        throwsA(isA<PlatformException>()),
      );
    });
  });
}
