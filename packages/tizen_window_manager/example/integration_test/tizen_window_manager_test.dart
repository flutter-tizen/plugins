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
}
