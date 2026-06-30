// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_bundle/tizen_bundle.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  // ---------------------------------------------------------------------------
  // Bundle() — default constructor
  // ---------------------------------------------------------------------------
  group('Bundle() default constructor', () {
    testWidgets('creates an empty bundle with length 0', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      expect(bundle.length, 0);
    });

    testWidgets('isEmpty returns true for a newly created bundle', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      expect(bundle.isEmpty, true);
    });

    testWidgets('isNotEmpty returns false for a newly created bundle', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      expect(bundle.isNotEmpty, false);
    });

    testWidgets('keys returns empty iterable for an empty bundle', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      expect(bundle.keys, isEmpty);
    });
  });

  // ---------------------------------------------------------------------------
  // Bundle.fromMap — factory constructor
  // ---------------------------------------------------------------------------
  group('Bundle.fromMap', () {
    testWidgets('creates an empty bundle from an empty map', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle.fromMap(<String, Object>{});
      expect(bundle.length, 0);
      expect(bundle.isEmpty, true);
    });

    testWidgets(
        'creates a bundle with String, List<String>, and Uint8List entries', (
      WidgetTester _,
    ) async {
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
  });

  // ---------------------------------------------------------------------------
  // Bundle.fromBundle — copy constructor independence
  // ---------------------------------------------------------------------------
  group('Bundle.fromBundle copy independence', () {
    testWidgets(
      'mutating the original after copy does not affect the copy',
      (WidgetTester _) async {
        final Bundle original = Bundle();
        original['key'] = 'originalValue';

        final Bundle copy = Bundle.fromBundle(original);
        original['key'] = 'mutatedValue';

        // copy should still hold the value at the time of duplication
        expect(copy['key'], 'originalValue');
      },
    );

    testWidgets(
        'copy has independent entries — adding to copy does not affect original',
        (
      WidgetTester _,
    ) async {
      final Bundle original = Bundle();
      original['key'] = 'value';

      final Bundle copy = Bundle.fromBundle(original);
      copy['newKey'] = 'newValue';

      expect(original.containsKey('newKey'), false);
    });
  });

  // ---------------------------------------------------------------------------
  // Bundle.decode — constructor from encoded string
  // ---------------------------------------------------------------------------
  group('Bundle.decode', () {
    testWidgets('round-trips a List<String> value through encode/decode', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      bundle['stringsKey'] = <String>['alpha', 'beta', 'gamma'];
      final String encoded = bundle.encode();
      final Bundle decoded = Bundle.decode(encoded);
      expect(decoded['stringsKey'], <String>['alpha', 'beta', 'gamma']);
    });

    testWidgets('round-trips a Uint8List value through encode/decode', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      bundle['bytesKey'] = Uint8List.fromList(<int>[0x00, 0x7f, 0xff]);
      final String encoded = bundle.encode();
      final Bundle decoded = Bundle.decode(encoded);
      expect(decoded['bytesKey'], Uint8List.fromList(<int>[0x00, 0x7f, 0xff]));
    });

    testWidgets('round-trips multiple keys of mixed types', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      bundle['s'] = 'hello';
      bundle['ss'] = <String>['x', 'y'];
      bundle['b'] = Uint8List.fromList(<int>[1, 2, 3]);
      final String encoded = bundle.encode();
      final Bundle decoded = Bundle.decode(encoded);
      expect(decoded.length, 3);
      expect(decoded['s'], 'hello');
      expect(decoded['ss'], <String>['x', 'y']);
      expect(decoded['b'], Uint8List.fromList(<int>[1, 2, 3]));
    });
  });

  // ---------------------------------------------------------------------------
  // operator[] — get
  // ---------------------------------------------------------------------------
  group('operator[] (get)', () {
    testWidgets('returns null for a missing key', (WidgetTester _) async {
      final Bundle bundle = Bundle();
      expect(bundle['nonExistentKey'], isNull);
    });

    testWidgets('returns null when key is null', (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['someKey'] = 'someValue';
      // The implementation accepts Object? key and returns null for null.
      expect(bundle[null], isNull);
    });

    testWidgets('returns the correct value after key is overwritten', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      bundle['key'] = 'first';
      bundle['key'] = 'second';
      expect(bundle['key'], 'second');
    });

    testWidgets(
        'same read-only call twice returns identical results (idempotency)', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      bundle['key'] = 'value';
      expect(bundle['key'], bundle['key']);
    });
  });

  // ---------------------------------------------------------------------------
  // operator[]= — set (error path)
  // ---------------------------------------------------------------------------
  group('operator[]= (set)', () {
    testWidgets('throws ArgumentError for an unsupported value type', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      expect(
        () => bundle['key'] = 42,
        throwsA(isA<ArgumentError>()),
      );
    });

    testWidgets('stores an empty string value', (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['emptyString'] = '';
      expect(bundle['emptyString'], '');
    });

    testWidgets('stores a single-element List<String>', (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['single'] = <String>['only'];
      expect(bundle['single'], <String>['only']);
    });

    testWidgets('stores an empty Uint8List', (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['emptyBytes'] = Uint8List(0);
      final Uint8List result = bundle['emptyBytes']! as Uint8List;
      expect(result.length, 0);
    });

    testWidgets('preserves full byte range 0x00–0xff in Uint8List', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      final Uint8List allBytes =
          Uint8List.fromList(List<int>.generate(256, (int i) => i));
      bundle['allBytes'] = allBytes;
      final Uint8List result = bundle['allBytes']! as Uint8List;
      expect(result, allBytes);
    });
  });

  // ---------------------------------------------------------------------------
  // remove()
  // ---------------------------------------------------------------------------
  group('remove()', () {
    testWidgets('remove(null) is a no-op and does not throw', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      bundle['key'] = 'value';
      expect(() => bundle.remove(null), returnsNormally);
      expect(bundle.length, 1);
    });

    testWidgets('removing a non-existent key throws PlatformException', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      expect(
        () => bundle.remove('doesNotExist'),
        throwsA(isA<PlatformException>()),
      );
    });

    testWidgets('add then remove then verify gone (state transition)', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      bundle['key'] = 'value';
      expect(bundle.containsKey('key'), true);
      bundle.remove('key');
      expect(bundle.containsKey('key'), false);
      expect(bundle['key'], isNull);
    });
  });

  // ---------------------------------------------------------------------------
  // clear()
  // ---------------------------------------------------------------------------
  group('clear()', () {
    testWidgets('clear() on an already-empty bundle is idempotent', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      bundle.clear();
      expect(bundle.length, 0);
      expect(bundle.isEmpty, true);
    });

    testWidgets('clear() removes all entries including mixed types', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      bundle['s'] = 'hello';
      bundle['ss'] = <String>['a', 'b'];
      bundle['b'] = Uint8List.fromList(<int>[1]);
      bundle.clear();
      expect(bundle.isEmpty, true);
      expect(bundle.keys, isEmpty);
    });
  });

  // ---------------------------------------------------------------------------
  // length / isEmpty / isNotEmpty — state after modifications
  // ---------------------------------------------------------------------------
  group('length, isEmpty, isNotEmpty', () {
    testWidgets('length reflects current entry count correctly', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      expect(bundle.length, 0);
      bundle['k1'] = 'v1';
      expect(bundle.length, 1);
      bundle['k2'] = 'v2';
      expect(bundle.length, 2);
      bundle.remove('k1');
      expect(bundle.length, 1);
    });

    testWidgets('isNotEmpty becomes true after adding an entry', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      expect(bundle.isNotEmpty, false);
      bundle['key'] = 'value';
      expect(bundle.isNotEmpty, true);
    });

    testWidgets('isEmpty becomes true after removing the last entry', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      bundle['key'] = 'value';
      expect(bundle.isEmpty, false);
      bundle.remove('key');
      expect(bundle.isEmpty, true);
    });
  });

  // ---------------------------------------------------------------------------
  // Inherited MapMixin members
  // ---------------------------------------------------------------------------
  group('inherited MapMixin members', () {
    testWidgets('containsKey returns true for existing key', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      bundle['key'] = 'value';
      expect(bundle.containsKey('key'), true);
    });

    testWidgets('containsKey returns false for missing key', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      expect(bundle.containsKey('missing'), false);
    });

    testWidgets('keys and values return all stored entries', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle.fromMap(<String, String>{
        'key1': 'value1',
        'key2': 'value2',
        'key3': 'value3',
      });
      expect(bundle.keys, unorderedEquals(<String>['key1', 'key2', 'key3']));
      expect(
        bundle.values,
        unorderedEquals(<String>['value1', 'value2', 'value3']),
      );
    });

    testWidgets('containsValue returns true for existing string value', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      bundle['key'] = 'needle';
      expect(bundle.containsValue('needle'), true);
    });

    testWidgets('containsValue returns false for absent value', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      bundle['key'] = 'value';
      expect(bundle.containsValue('absent'), false);
    });

    testWidgets('entries exposes key-value pairs correctly', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      bundle['k'] = 'v';
      final MapEntry<String, Object> entry = bundle.entries.single;
      expect(entry.key, 'k');
      expect(entry.value, 'v');
    });

    testWidgets('forEach iterates over all entries', (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['a'] = '1';
      bundle['b'] = '2';
      final Map<String, Object> collected = <String, Object>{};
      bundle.forEach((String key, Object value) {
        collected[key] = value;
      });
      expect(collected.length, 2);
      expect(collected['a'], '1');
      expect(collected['b'], '2');
    });

    testWidgets('addAll adds all entries from a map', (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle.addAll(<String, Object>{
        'x': 'valueX',
        'y': 'valueY',
      });
      expect(bundle.length, 2);
      expect(bundle['x'], 'valueX');
      expect(bundle['y'], 'valueY');
    });

    testWidgets('putIfAbsent inserts only when key is absent', (
      WidgetTester _,
    ) async {
      final Bundle bundle = Bundle();
      bundle['existing'] = 'original';

      final Object result = bundle.putIfAbsent('existing', () => 'new');
      expect(result, 'original');
      expect(bundle['existing'], 'original');

      final Object inserted = bundle.putIfAbsent('fresh', () => 'inserted');
      expect(inserted, 'inserted');
      expect(bundle['fresh'], 'inserted');
    });
  });
}
