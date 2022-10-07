// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_bundle/tizen_bundle.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('can create a bundle', (WidgetTester _) async {
    final Bundle bundle = Bundle();
    bundle['stringKey'] = 'stringValue';
    expect(bundle['stringKey'], 'stringValue');
  });

  testWidgets('can create a bundle from another bundle',
      (WidgetTester _) async {
    final Bundle bundle = Bundle();
    bundle['stringKey'] = 'stringValue';

    final Bundle newBundle = Bundle.fromBundle(bundle);
    expect(newBundle['stringKey'], 'stringValue');
  });

  testWidgets('can create a bundle from a map', (WidgetTester _) async {
    final Bundle bundle = Bundle.fromMap(<String, Object>{
      'stringKey': 'stringValue',
      'stringsKey': <String>['value1', 'value2'],
      'bytesKey': Uint8List.fromList(<int>[0x03]),
    });
    expect(bundle.length, 3);
    expect(bundle['stringKey'], 'stringValue');
    expect(bundle['stringsKey'], <String>['value1', 'value2']);
    expect(bundle['bytesKey'], Uint8List.fromList(<int>[0x03]));
  });

  testWidgets('returns all stored keys and values', (WidgetTester _) async {
    final Bundle bundle = Bundle.fromMap(<String, String>{
      'key1': 'value1',
      'key2': 'value2',
      'key3': 'value3',
    });
    expect(bundle.keys, unorderedEquals(<String>['key1', 'key2', 'key3']));
    expect(
        bundle.values, unorderedEquals(<String>['value1', 'value2', 'value3']));
  });

  testWidgets('checks whether the bundle is empty', (WidgetTester _) async {
    final Bundle bundle = Bundle();
    bundle['stringKey'] = 'stringValue';
    expect(bundle.isNotEmpty, true);

    bundle.remove('stringKey');
    expect(bundle.isEmpty, true);
  });

  testWidgets('can add a Uint8List value', (WidgetTester _) async {
    final Bundle bundle = Bundle();
    bundle['bytesKey'] = Uint8List(10);

    final Uint8List bytes = bundle['bytesKey']! as Uint8List;
    expect(bytes.length, 10);
  });

  testWidgets('can add a List<String> value', (WidgetTester _) async {
    final Bundle bundle = Bundle();
    bundle['stringsKey'] = <String>['value1', 'value2'];

    final List<String> strings = bundle['stringsKey']! as List<String>;
    expect(strings, <String>['value1', 'value2']);
  });

  testWidgets('can remove all entries', (WidgetTester _) async {
    final Bundle bundle = Bundle();
    bundle['stringKey1'] = 'stringValue1';
    bundle['stringKey2'] = 'stringValue2';
    expect(bundle.length, 2);

    bundle.clear();
    expect(bundle.length, 0);
  });

  testWidgets('can remove key and its associated value',
      (WidgetTester _) async {
    final Bundle bundle = Bundle();
    bundle['stringKey'] = 'stringValue';
    expect(bundle.containsKey('stringKey'), true);

    bundle.remove('stringKey');
    expect(bundle.containsKey('stringKey'), false);
  });

  testWidgets('can encode and decode raw data', (WidgetTester _) async {
    final Bundle bundle = Bundle();
    bundle['stringKey'] = 'stringValue';
    final String encoded = bundle.encode();
    final Bundle newBundle = Bundle.decode(encoded);
    expect(newBundle['stringKey'], 'stringValue');
  });
}
