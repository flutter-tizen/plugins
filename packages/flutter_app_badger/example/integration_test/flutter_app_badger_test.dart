// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:flutter_app_badger/flutter_app_badger.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('get supported', (tester) async {
    expect(await FlutterAppBadger.isAppBadgeSupported(), true);
  });

  testWidgets('update badge count', (tester) async {
    await expectLater(FlutterAppBadger.updateBadgeCount(1), completes);
  });

  testWidgets('remove badge count', (tester) async {
    await expectLater(FlutterAppBadger.removeBadge(), completes);
  });
}
