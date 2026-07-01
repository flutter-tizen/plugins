// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_package_manager/tizen_package_manager.dart';

import 'package:tizen_package_manager_example/main.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('Can get current package info', (WidgetTester tester) async {
    final PackageInfo info = await PackageManager.getPackageInfo(
      currentPackageId,
    );
    expect(info.packageId, 'org.tizen.tizen_package_manager_example');
    expect(info.label, 'tizen_package_manager_example');
    expect(info.packageType, PackageType.tpk);
    expect(info.version, '1.0.0');
    expect(info.isPreloaded, false);
    expect(info.isSystem, false);
    expect(info.isRemovable, true);
  });

  testWidgets('Can get current package size info', (WidgetTester tester) async {
    final PackageSizeInfo info = await PackageManager.getPackageSizeInfo(
      currentPackageId,
    );
    expect(info.appSize, greaterThan(0)); // package size -> 160935936
  });

  testWidgets('Can get all installed packages info', (
    WidgetTester tester,
  ) async {
    final List<PackageInfo> infos = await PackageManager.getPackagesInfo();
    expect(infos.isNotEmpty, true);
  });

  group('PackageInfo fields', () {
    testWidgets('installedStorageType is a valid StorageType value', (
      WidgetTester tester,
    ) async {
      final PackageInfo info =
          await PackageManager.getPackageInfo(currentPackageId);
      expect(StorageType.values, contains(info.installedStorageType));
    });

    testWidgets('iconPath is null or a non-empty string', (
      WidgetTester tester,
    ) async {
      final PackageInfo info =
          await PackageManager.getPackageInfo(currentPackageId);
      if (info.iconPath != null) {
        expect(info.iconPath, isNotEmpty);
      }
    });
  });

  group('PackageSizeInfo fields', () {
    testWidgets('all size fields are non-negative', (
      WidgetTester tester,
    ) async {
      final PackageSizeInfo info =
          await PackageManager.getPackageSizeInfo(currentPackageId);
      expect(info.dataSize, greaterThanOrEqualTo(0));
      expect(info.cacheSize, greaterThanOrEqualTo(0));
      expect(info.appSize, greaterThanOrEqualTo(0));
      expect(info.externalDataSize, greaterThanOrEqualTo(0));
      expect(info.externalCacheSize, greaterThanOrEqualTo(0));
      expect(info.externalAppSize, greaterThanOrEqualTo(0));
    });
  });

  group('getPackagesInfo list items', () {
    testWidgets('each PackageInfo item has valid field types', (
      WidgetTester tester,
    ) async {
      final List<PackageInfo> infos = await PackageManager.getPackagesInfo();
      expect(infos, isNotEmpty);
      for (final PackageInfo info in infos) {
        expect(info.packageId, isA<String>());
        expect(info.packageId, isNotEmpty);
        expect(info.label, isA<String>());
        expect(info.version, isA<String>());
        expect(info.version, isNotEmpty);
        expect(PackageType.values, contains(info.packageType));
        expect(StorageType.values, contains(info.installedStorageType));
        expect(info.isSystem, isA<bool>());
        expect(info.isPreloaded, isA<bool>());
        expect(info.isRemovable, isA<bool>());
        if (info.iconPath != null) {
          expect(info.iconPath, isNotEmpty);
        }
      }
    });

    testWidgets('getPackagesInfo contains current package', (
      WidgetTester tester,
    ) async {
      final List<PackageInfo> infos = await PackageManager.getPackagesInfo();
      final Iterable<String> ids = infos.map((PackageInfo i) => i.packageId);
      expect(ids, contains(currentPackageId));
    });
  });

  group('getPackageInfo error paths', () {
    test('throws ArgumentError for empty packageId', () async {
      expect(
        () => PackageManager.getPackageInfo(''),
        throwsA(isA<ArgumentError>()),
      );
    });

    test('throws PlatformException for invalid packageId', () async {
      expect(
        () => PackageManager.getPackageInfo(
            'invalid.package.id.that.does.not.exist'),
        throwsA(isA<PlatformException>()),
      );
    });
  });

  group('getPackageSizeInfo error paths', () {
    test('throws ArgumentError for empty packageId', () async {
      expect(
        () => PackageManager.getPackageSizeInfo(''),
        throwsA(isA<ArgumentError>()),
      );
    });

    // NOTE: The Tizen package_manager_get_package_size_info API does not
    // return an error for a non-existent package ID; it silently returns
    // zero-valued size info. This is a platform limitation, so no
    // PlatformException test is included here.
  });

  group('install error paths', () {
    test('throws ArgumentError for empty packagePath', () async {
      expect(
        () => PackageManager.install(''),
        throwsA(isA<ArgumentError>()),
      );
    });
  });

  group('uninstall error paths', () {
    test('throws ArgumentError for empty packageId', () async {
      expect(
        () => PackageManager.uninstall(''),
        throwsA(isA<ArgumentError>()),
      );
    });
  });

  group('event streams', () {
    test('onInstallProgressChanged can be listened to without error', () async {
      final StreamSubscription<PackageEvent> subscription =
          PackageManager.onInstallProgressChanged.listen(null);
      await subscription.cancel();
    });

    test('onUninstallProgressChanged can be listened to without error',
        () async {
      final StreamSubscription<PackageEvent> subscription =
          PackageManager.onUninstallProgressChanged.listen(null);
      await subscription.cancel();
    });

    test('onUpdateProgressChanged can be listened to without error', () async {
      final StreamSubscription<PackageEvent> subscription =
          PackageManager.onUpdateProgressChanged.listen(null);
      await subscription.cancel();
    });
  });
}
