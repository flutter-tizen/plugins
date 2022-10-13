// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:math';

import 'package:flutter/foundation.dart';
import 'package:flutter_secure_storage/flutter_secure_storage.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

String _randomValue() {
  final rand = Random();
  final codeUnits = List.generate(20, (index) {
    return rand.nextInt(26) + 65;
  });

  return String.fromCharCodes(codeUnits);
}

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();
  late FlutterSecureStorage storage;
  late final Map<String, String> testInput = {};

  setUpAll(() async {
    storage = const FlutterSecureStorage();

    for (int i = 0; i < 5; i++) {
      testInput[_randomValue()] = _randomValue();
    }
  });

  testWidgets('write', (WidgetTester tester) async {
    await Future.forEach(testInput.entries, (MapEntry entry) async {
      final String key = entry.key as String;
      final String value = entry.value as String;
      await storage.write(key: key, value: value);
      final String? result = await storage.read(key: key);
      expect(result, value);
    });
  });

  testWidgets('read', (WidgetTester tester) async {
    final String? result = await storage.read(key: 'invaild_key');
    expect(result, isNull);
  });

  testWidgets('readAll', (WidgetTester tester) async {
    final Map<String, String> result = await storage.readAll();
    expect(true, mapEquals(testInput, result));
  });

  testWidgets('containsKey', (WidgetTester tester) async {
    await Future.forEach(testInput.entries, (MapEntry entry) async {
      expect(await storage.containsKey(key: entry.key as String), isTrue);
    });
  });

  testWidgets('delete', (WidgetTester tester) async {
    await storage.delete(key: testInput.keys.first);
    expect(await storage.containsKey(key: testInput.keys.first), isFalse);
  });

  testWidgets('deleteAll', (WidgetTester tester) async {
    await storage.deleteAll();
    await Future.forEach(testInput.entries, (MapEntry entry) async {
      expect(await storage.containsKey(key: entry.key as String), isFalse);
    });
  });
}
