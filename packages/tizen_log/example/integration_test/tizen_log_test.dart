// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_log/tizen_log.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  const tag = 'TizenLogTest';

  group('Log', () {
    test('verbose does not throw', () async {
      expect(() => Log.verbose(tag, 'verbose message'), returnsNormally);
    });

    test('debug does not throw', () async {
      expect(() => Log.debug(tag, 'debug message'), returnsNormally);
    });

    test('info does not throw', () async {
      expect(() => Log.info(tag, 'info message'), returnsNormally);
    });

    test('warn does not throw', () async {
      expect(() => Log.warn(tag, 'warn message'), returnsNormally);
    });

    test('error does not throw', () async {
      expect(() => Log.error(tag, 'error message'), returnsNormally);
    });

    test('fatal does not throw', () async {
      expect(() => Log.fatal(tag, 'fatal message'), returnsNormally);
    });

    test('isDebugEnabled is false by default', () async {
      expect(Log.isDebugEnabled, isFalse);
    });

    test('log with optional file, func, and line does not throw', () async {
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
