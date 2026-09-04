// Copyright 2024 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:permission_handler/permission_handler.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  group('Permission.status', () {
    test('camera permission is granted', () async {
      expect(await Permission.camera.status, PermissionStatus.granted);
    });

    test('microphone permission is granted', () async {
      expect(await Permission.microphone.status, PermissionStatus.granted);
    });

    test('location permission is granted', () async {
      expect(await Permission.location.status, PermissionStatus.granted);
    });

    test('mediaLibrary permission is granted', () async {
      expect(await Permission.mediaLibrary.status, PermissionStatus.granted);
    });

    test('storage permission is granted', () async {
      expect(await Permission.storage.status, PermissionStatus.granted);
    });

    test('contacts permission is granted', () async {
      expect(await Permission.contacts.status, PermissionStatus.granted);
    });
  });

  group('Permission.serviceStatus', () {
    test('location service status is valid', () async {
      final status = await Permission.location.serviceStatus;
      expect(status.isEnabled || status.isDisabled, true);
    });
  });

  group('Permission.request', () {
    test('requesting camera permission returns granted', () async {
      final status = await Permission.camera.request();
      expect(status, PermissionStatus.granted);
    });

    test('requesting multiple permissions returns granted', () async {
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

  test('open app settings', () async {
    expect(await openAppSettings(), true);
  }, skip: true);
}
