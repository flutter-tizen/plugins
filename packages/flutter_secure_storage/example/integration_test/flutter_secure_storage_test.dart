// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/foundation.dart';
import 'package:flutter_secure_storage/flutter_secure_storage.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  late FlutterSecureStorage storage;

  setUpAll(() {
    storage = const FlutterSecureStorage();
  });

  tearDown(() async {
    await storage.deleteAll();
  });

  testWidgets('read and write', (WidgetTester tester) async {
    await storage.write(key: 'key1', value: 'value1');
    await storage.write(key: 'key2', value: 'value2');

    expect(await storage.read(key: 'key1'), 'value1');
    expect(await storage.read(key: 'key2'), 'value2');
  });

  testWidgets('read non-existing key', (WidgetTester tester) async {
    final String? result = await storage.read(key: 'foobar');
    expect(result, isNull);
  });

  testWidgets('readAll', (WidgetTester tester) async {
    final Map<String, String> input = {
      'foo': 'bar',
      'baz': 'qux',
      'waldo': 'fred',
    };

    for (final MapEntry<String, String> enrty in input.entries) {
      await storage.write(key: enrty.key, value: enrty.value);
    }

    final Map<String, String> result = await storage.readAll();
    expect(mapEquals(input, result), isTrue);
  });

  testWidgets('containsKey', (WidgetTester tester) async {
    await storage.write(key: 'dgx', value: 'dfs');
    await storage.write(key: 'corge', value: 'grault');

    expect(await storage.containsKey(key: 'dgx'), isTrue);
    expect(await storage.containsKey(key: 'corge'), isTrue);

    expect(await storage.containsKey(key: 'foo'), isFalse);
  });

  testWidgets('delete', (WidgetTester tester) async {
    await storage.write(key: 'wibble', value: 'flob');
    expect(await storage.containsKey(key: 'wibble'), isTrue);

    await storage.delete(key: 'wibble');
    expect(await storage.containsKey(key: 'wibble'), isFalse);
  });

  testWidgets('deleteAll', (WidgetTester tester) async {
    final Map<String, String> input = {
      'foo': 'bar',
      'baz': 'qux',
      'waldo': 'fred',
    };

    for (final MapEntry<String, String> enrty in input.entries) {
      await storage.write(key: enrty.key, value: enrty.value);
    }
    expect(await storage.readAll(), isNotEmpty);

    await storage.deleteAll();
    expect(await storage.readAll(), isEmpty);
  });
}
