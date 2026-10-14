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

  test('Can get current app info', () async {
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
    final context = AppRunningContext(appId: appId);

    // TODO(seungsoo47): The `AppRunningContext.appState` API always returns the
    // appropriate value. However, we decided to comment out this testcase
    // because the tizen test farm does not support the 'foreground' state.
    // expect(context.appState, AppState.foreground);
    expect(context.processId, isPositive);
    expect(context.isTerminated, isFalse);
  });

  group('AppManager', () {
    group('getInstalledApps', () {
      test('returns non-empty list', () async {
        final List<AppInfo> apps = await AppManager.getInstalledApps();
        expect(apps, isNotEmpty);
      });

      test('includes the current app', () async {
        final String appId = await AppManager.currentAppId;
        final List<AppInfo> apps = await AppManager.getInstalledApps();
        expect(apps.any((AppInfo app) => app.appId == appId), isTrue);
      });
    });

    group('isRunning', () {
      test('returns true for running app', () async {
        final String appId = await AppManager.currentAppId;
        expect(await AppManager.isRunning(appId), isTrue);
      });

      test('returns false for non-running app', () async {
        expect(
          await AppManager.isRunning('org.tizen.nonexistent'),
          isFalse,
        );
      });

      test('throws ArgumentError for empty appId', () async {
        await expectLater(AppManager.isRunning(''), throwsArgumentError);
      });
    });

    group('getAppInfo', () {
      test('returns all AppInfo fields', () async {
        final String appId = await AppManager.currentAppId;
        final AppInfo appInfo = await AppManager.getAppInfo(appId);
        expect(appInfo.appId, appId);
        expect(appInfo.executablePath, isNotEmpty);
        expect(appInfo.sharedResourcePath, isNotEmpty);
      });

      test('throws ArgumentError for empty appId', () async {
        await expectLater(AppManager.getAppInfo(''), throwsArgumentError);
      });
    });
  });

  group('AppRunningContext', () {
    test('packageId matches current app', () async {
      final String appId = await AppManager.currentAppId;
      final context = AppRunningContext(appId: appId);
      expect(context.packageId, 'org.tizen.tizen_app_manager_example');
    });

    test('throws PlatformException for non-running appId', () async {
      expect(
        () => AppRunningContext(appId: 'org.tizen.nonexistent'),
        throwsA(isA<PlatformException>()),
      );
    });
  });
}
