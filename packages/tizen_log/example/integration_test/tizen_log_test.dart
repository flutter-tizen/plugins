// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_log/tizen_log.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  const String tag = 'TizenLogTest';

  group('Log', () {
    testWidgets('verbose does not throw', (WidgetTester tester) async {
      expect(() => Log.verbose(tag, 'verbose message'), returnsNormally);
    });

    testWidgets('debug does not throw', (WidgetTester tester) async {
      expect(() => Log.debug(tag, 'debug message'), returnsNormally);
    });

    testWidgets('info does not throw', (WidgetTester tester) async {
      expect(() => Log.info(tag, 'info message'), returnsNormally);
    });

    testWidgets('warn does not throw', (WidgetTester tester) async {
      expect(() => Log.warn(tag, 'warn message'), returnsNormally);
    });

    testWidgets('error does not throw', (WidgetTester tester) async {
      expect(() => Log.error(tag, 'error message'), returnsNormally);
    });

    testWidgets('fatal does not throw', (WidgetTester tester) async {
      expect(() => Log.fatal(tag, 'fatal message'), returnsNormally);
    });

    testWidgets('isDebugEnabled is false by default',
        (WidgetTester tester) async {
      expect(Log.isDebugEnabled, isFalse);
    });

    testWidgets('log with optional file, func, and line does not throw',
        (WidgetTester tester) async {
      expect(
        () => Log.info(
          tag,
          'message with metadata',
          file: 'test.dart',
          func: 'main',
          line: 1,
        ),
        returnsNormally,
      );
    });
  });
}
