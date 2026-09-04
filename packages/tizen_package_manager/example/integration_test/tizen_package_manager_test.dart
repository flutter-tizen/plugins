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

  test('Can get current package info', () async {
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

  test('Can get current package size info', () async {
    final PackageSizeInfo info = await PackageManager.getPackageSizeInfo(
      currentPackageId,
    );
    expect(info.appSize, greaterThan(0)); // package size -> 160935936
  });

  test('Can get all installed packages info', () async {
    final List<PackageInfo> infos = await PackageManager.getPackagesInfo();
    expect(infos.isNotEmpty, true);
  });

  group('PackageInfo fields', () {
    test('iconPath is null or a non-empty string', () async {
      final PackageInfo info =
          await PackageManager.getPackageInfo(currentPackageId);
      expect(info.iconPath, anyOf(isNull, isNotEmpty));
    });
  });

  group('PackageSizeInfo fields', () {
    test('all size fields are non-negative', () async {
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
    test('each PackageInfo item has valid field values', () async {
      final List<PackageInfo> infos = await PackageManager.getPackagesInfo();
      expect(infos, isNotEmpty);
      for (final PackageInfo info in infos) {
        // Field types are already guaranteed by PackageInfo.fromMap (it casts
        // each field), so only the value-level expectations are asserted here.
        expect(info.packageId, isNotEmpty);
        expect(info.version, isNotEmpty);
        expect(info.iconPath, anyOf(isNull, isNotEmpty));
      }
    });

    test('getPackagesInfo contains current package', () async {
      final List<PackageInfo> infos = await PackageManager.getPackagesInfo();
      final Iterable<String> ids = infos.map((PackageInfo i) => i.packageId);
      expect(ids, contains(currentPackageId));
    });
  });

  group('getPackageInfo error paths', () {
    test('throws ArgumentError for empty packageId', () async {
      await expectLater(
        PackageManager.getPackageInfo(''),
        throwsArgumentError,
      );
    });

    test('throws PlatformException for invalid packageId', () async {
      await expectLater(
        PackageManager.getPackageInfo('invalid.package.id.that.does.not.exist'),
        throwsA(isA<PlatformException>()),
      );
    });
  });

  group('getPackageSizeInfo error paths', () {
    test('throws ArgumentError for empty packageId', () async {
      await expectLater(
        PackageManager.getPackageSizeInfo(''),
        throwsArgumentError,
      );
    });

    // NOTE: The Tizen package_manager_get_package_size_info API does not
    // return an error for a non-existent package ID; it silently returns
    // zero-valued size info. This is a platform limitation, so no
    // PlatformException test is included here.
  });

  group('install error paths', () {
    test('throws ArgumentError for empty packagePath', () async {
      await expectLater(PackageManager.install(''), throwsArgumentError);
    });
  });

  group('uninstall error paths', () {
    test('throws ArgumentError for empty packageId', () async {
      await expectLater(PackageManager.uninstall(''), throwsArgumentError);
    });
  });

  // These only verify that subscribing to and cancelling the stream does not
  // throw. Actually exercising an event requires installing/uninstalling/
  // updating a real package on the device, which is out of scope here.
  group('event streams', () {
    test('can subscribe to onInstallProgressChanged and cancel', () async {
      final StreamSubscription<PackageEvent> subscription =
          PackageManager.onInstallProgressChanged.listen(null);
      await subscription.cancel();
    });

    test('can subscribe to onUninstallProgressChanged and cancel', () async {
      final StreamSubscription<PackageEvent> subscription =
          PackageManager.onUninstallProgressChanged.listen(null);
      await subscription.cancel();
    });

    test('can subscribe to onUpdateProgressChanged and cancel', () async {
      final StreamSubscription<PackageEvent> subscription =
          PackageManager.onUpdateProgressChanged.listen(null);
      await subscription.cancel();
    });
  });
}
