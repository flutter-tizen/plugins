// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_package_manager/tizen_package_manager.dart';

import 'package:tizen_package_manager_example/main.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('Can get current package info', (WidgetTester tester) async {
    final PackageInfo info =
        await PackageManager.getPackageInfo(currentPackageId);
    expect(info.packageId, 'org.tizen.tizen_package_manager_example');
    expect(info.label, 'tizen_package_manager_example');
    expect(info.packageType, PackageType.tpk);
    expect(info.version, '1.0.0');
    expect(info.isPreloaded, false);
    expect(info.isSystem, false);
    expect(info.isRemovable, true);
  });

  testWidgets('Can get all installed packages info',
      (WidgetTester tester) async {
    final List<PackageInfo> infos = await PackageManager.getPackagesInfo();
    expect(infos.isNotEmpty, true);
  });
}
