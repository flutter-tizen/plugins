// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_window_manager/tizen_window_manager.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('Can get window geometry info', (WidgetTester tester) async {
    final Map<String, int> geometry = await WindowManager.getGeometry();
    expect(geometry['x'], 100);
    expect(geometry['y'], 100);
    expect(geometry['width'], 1000);
    expect(geometry['height'], 700);
  });

  testWidgets('Control lower/activate window', (WidgetTester tester) async {
    await WindowManager.lower();
    await Future<void>.delayed(const Duration(seconds: 2));

    expect(WidgetsBinding.instance.lifecycleState, AppLifecycleState.paused);

    await WindowManager.activate();
    await Future<void>.delayed(const Duration(seconds: 2));

    expect(WidgetsBinding.instance.lifecycleState, AppLifecycleState.resumed);
  });

  group('WindowManager.getGeometry', () {
    testWidgets('returns a map with all required keys',
        (WidgetTester tester) async {
      final Map<String, int> geometry = await WindowManager.getGeometry();
      expect(geometry.containsKey('x'), isTrue);
      expect(geometry.containsKey('y'), isTrue);
      expect(geometry.containsKey('width'), isTrue);
      expect(geometry.containsKey('height'), isTrue);
    });

    testWidgets('returns integer values for all geometry fields',
        (WidgetTester tester) async {
      final Map<String, int> geometry = await WindowManager.getGeometry();
      expect(geometry['x'], isA<int>());
      expect(geometry['y'], isA<int>());
      expect(geometry['width'], isA<int>());
      expect(geometry['height'], isA<int>());
    });

    testWidgets('returns positive dimensions', (WidgetTester tester) async {
      final Map<String, int> geometry = await WindowManager.getGeometry();
      expect(geometry['width']!, greaterThan(0));
      expect(geometry['height']!, greaterThan(0));
    });
  });

  group('WindowManager.lower', () {
    testWidgets('completes without error and window can be reactivated',
        (WidgetTester tester) async {
      await expectLater(WindowManager.lower(), completes);
      await Future<void>.delayed(const Duration(seconds: 1));

      // Restore the window so subsequent tests run in a normal state.
      await WindowManager.activate();
      await Future<void>.delayed(const Duration(seconds: 2));

      expect(WidgetsBinding.instance.lifecycleState, AppLifecycleState.resumed);
    });
  });
}
