// Copyright 2024 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:permission_handler/permission_handler.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  group('Permission.status', () {
    testWidgets('camera permission is granted', (tester) async {
      expect(await Permission.camera.status.isGranted, true);
    });

    testWidgets('microphone permission is granted', (tester) async {
      expect(await Permission.microphone.status.isGranted, true);
    });

    testWidgets('location permission is granted', (tester) async {
      expect(await Permission.location.status.isGranted, true);
    });

    testWidgets('mediaLibrary permission is granted', (tester) async {
      expect(await Permission.mediaLibrary.status.isGranted, true);
    });

    testWidgets('storage permission is granted', (tester) async {
      expect(await Permission.storage.status.isGranted, true);
    });

    testWidgets('contacts permission is granted', (tester) async {
      expect(await Permission.contacts.status.isGranted, true);
    });
  });

  group('Permission.serviceStatus', () {
    testWidgets('location service status is valid', (tester) async {
      final status = await Permission.location.serviceStatus;
      expect(status.isEnabled || status.isDisabled, true);
    });
  });

  group('Permission.request', () {
    testWidgets('requesting camera permission returns granted', (tester) async {
      final status = await Permission.camera.request();
      expect(status.isGranted, true);
    });

    testWidgets('requesting multiple permissions returns granted',
        (tester) async {
      final statuses = await [
        Permission.camera,
        Permission.microphone,
        Permission.location,
      ].request();
      expect(statuses[Permission.camera]!.isGranted, true);
      expect(statuses[Permission.microphone]!.isGranted, true);
      expect(statuses[Permission.location]!.isGranted, true);
    });
  });

  testWidgets('open app settings', (tester) async {
    expect(await openAppSettings(), true);
  }, skip: true);
}
