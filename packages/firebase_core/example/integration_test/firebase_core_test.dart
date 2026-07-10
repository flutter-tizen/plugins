// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Integration tests ported from upstream firebase_core v2.17.0:
// https://github.com/firebase/flutterfire/blob/firebase_core-v2.17.0/tests/integration_test/firebase_core/firebase_core_e2e_test.dart
// Originally authored by the Chromium project authors under a BSD-style
// license.

import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_core_platform_interface/firebase_core_platform_interface.dart';
import 'package:firebase_core_tizen_example/firebase_options.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  group('firebase_core', () {
    const String testAppName = '[DEFAULT]';

    setUpAll(() async {
      await Firebase.initializeApp(
        options: DefaultFirebaseOptions.currentPlatform,
      );
    });

    test('Firebase.apps', () async {
      final List<FirebaseApp> apps = Firebase.apps;
      expect(apps.length, 1);
      expect(apps[0].name, testAppName);
      expect(apps[0].options, DefaultFirebaseOptions.currentPlatform);
    });

    test('Firebase.app()', () async {
      final FirebaseApp app = Firebase.app();
      expect(app.name, testAppName);
      expect(app.options, DefaultFirebaseOptions.currentPlatform);
    });

    test('Firebase.app() Exception', () async {
      expect(
        () => Firebase.app('NoApp'),
        throwsA(noAppExists('NoApp')),
      );
    });

    test(
      'FirebaseApp.delete()',
      () async {
        await Firebase.initializeApp(
          name: 'SecondaryApp',
          options: DefaultFirebaseOptions.currentPlatform,
        );

        expect(Firebase.apps.length, 2);

        final FirebaseApp app = Firebase.app('SecondaryApp');

        await app.delete();

        expect(Firebase.apps.length, 1);
        // TODO(russellwheatley): test randomly causes an auth sign-in failure due to duplicate accounts.
      },
      skip: TargetPlatform.android == defaultTargetPlatform,
    );

    test('FirebaseApp.setAutomaticDataCollectionEnabled()', () async {
      final FirebaseApp app = Firebase.app();
      await app.setAutomaticDataCollectionEnabled(false);

      expect(app.isAutomaticDataCollectionEnabled, false);

      await app.setAutomaticDataCollectionEnabled(true);

      expect(app.isAutomaticDataCollectionEnabled, true);
    });

    test('FirebaseApp.setAutomaticResourceManagementEnabled()', () async {
      final FirebaseApp app = Firebase.app();

      await app.setAutomaticResourceManagementEnabled(true);
    });
  });
}
