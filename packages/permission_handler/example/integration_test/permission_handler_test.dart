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
      expect(await Permission.camera.status, PermissionStatus.granted);
    });

    testWidgets('microphone permission is granted', (tester) async {
      expect(await Permission.microphone.status, PermissionStatus.granted);
    });

    testWidgets('location permission is granted', (tester) async {
      expect(await Permission.location.status, PermissionStatus.granted);
    });

    testWidgets('mediaLibrary permission is granted', (tester) async {
      expect(await Permission.mediaLibrary.status, PermissionStatus.granted);
    });

    testWidgets('storage permission is granted', (tester) async {
      expect(await Permission.storage.status, PermissionStatus.granted);
    });

    testWidgets('contacts permission is granted', (tester) async {
      expect(await Permission.contacts.status, PermissionStatus.granted);
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
      expect(status, PermissionStatus.granted);
    });

    testWidgets('requesting multiple permissions returns granted',
        (tester) async {
      final statuses = await [
        Permission.camera,
        Permission.microphone,
        Permission.location,
      ].request();
      expect(statuses[Permission.camera], PermissionStatus.granted);
      expect(statuses[Permission.microphone], PermissionStatus.granted);
      expect(statuses[Permission.location], PermissionStatus.granted);
    });
  });

  testWidgets('open app settings', (tester) async {
    expect(await openAppSettings(), true);
  }, skip: true);
}
