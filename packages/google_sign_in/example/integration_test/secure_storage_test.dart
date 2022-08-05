// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/foundation.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:google_sign_in_tizen/google_sign_in_tizen.dart';
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  late SecureStorage storage;
  late Map<String, Object> data1;
  late Map<String, Object> data2;

  setUpAll(() async {
    storage = SecureStorage();
    data1 = <String, Object>{
      'key11': 'value11',
      'key12': 12,
    };
    data2 = <String, Object>{
      'key21': 'value21',
      'key22': 22,
    };
  });

  tearDown(() async {
    await storage.destroy();
  });

  testWidgets('Can save get and remove', (WidgetTester tester) async {
    await storage.saveJson('a', data1);
    await storage.saveJson('b', data2);

    final Map<String, Object?>? retrievedData1 = await storage.getJson('a');
    final Map<String, Object?>? retrievedData2 = await storage.getJson('b');
    expect(retrievedData1, isNotNull);
    expect(retrievedData2, isNotNull);

    expect(true, mapEquals(data1, retrievedData1));
    expect(true, mapEquals(data2, retrievedData2));

    await storage.remove('a');
    await storage.remove('b');

    expect(await storage.getJson('a'), isNull);
    expect(await storage.getJson('b'), isNull);
  });
}
