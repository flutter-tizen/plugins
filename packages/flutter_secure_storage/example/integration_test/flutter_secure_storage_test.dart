// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:math';

import 'package:flutter/foundation.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:flutter_secure_storage/flutter_secure_storage.dart';
import 'package:integration_test/integration_test.dart';

String _randomValue() {
  final rand = Random();
  final codeUnits = List.generate(20, (index) {
    return rand.nextInt(26) + 65;
  });

  return String.fromCharCodes(codeUnits);
}

Future<void> main() async {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();
  late FlutterSecureStorage storage;
  late Map<String, String> test_input = {};

  setUpAll(() async {
    storage = new FlutterSecureStorage();

    for (int i = 0; i < 5; i++) {
      test_input[_randomValue()] = _randomValue();
    }
  });

  testWidgets('write', (WidgetTester tester) async {
    await Future.forEach(test_input.entries, (MapEntry entry) async {
      await storage.write(key: entry.key, value: entry.value);
      String? result = await storage.read(key: entry.key);
      expect(result, entry.value);
    });
  });

  testWidgets('read', (WidgetTester tester) async {
    String? result = await storage.read(key: 'invaild_key');
    expect(result, isNull);
  });

  testWidgets('readAll', (WidgetTester tester) async {
    Map<String, String>? result = await storage.readAll();
    expect(true, mapEquals(test_input, result));
  });

  testWidgets('containsKey', (WidgetTester tester) async {
    await Future.forEach(test_input.entries, (MapEntry entry) async {
      expect(await storage.containsKey(key: entry.key), isTrue);
    });
  });

  testWidgets('delete', (WidgetTester tester) async {
    await storage.delete(key: test_input.keys.first);
    expect(await storage.containsKey(key: test_input.keys.first), isFalse);
  });

  testWidgets('deleteAll', (WidgetTester tester) async {
    await storage.deleteAll();
    await Future.forEach(test_input.entries, (MapEntry entry) async {
      expect(await storage.containsKey(key: entry.key), isFalse);
    });
  });
}
