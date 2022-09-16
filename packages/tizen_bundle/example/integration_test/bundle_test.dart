import 'dart:typed_data';

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_bundle/tizen_bundle.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  group('$Bundle', () {
    testWidgets('can create a bundle', (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testName'] = 'bundleTest';
      expect(bundle['testName'], 'bundleTest');
    });

    testWidgets('can create a bundle from another bundle',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testName'] = 'bundleFromBundleTest';

      final Bundle newBundle = Bundle.fromBundle(bundle);
      expect(newBundle['testName'], 'bundleFromBundleTest');
    });

    testWidgets('can create a bundle from a map', (WidgetTester _) async {
      const String string = 'String';
      final List<String> strings = <String>['String 1', 'String 2'];
      final Uint8List bytes = Uint8List(1);
      bytes[0] = 0x03;
      final Map<String, Object> map = <String, Object>{
        'keyString': string,
        'keyListString': strings,
        'keyBytes': bytes
      };

      final Bundle bundle = Bundle.fromMap(map);
      expect(bundle.length, 3);
      expect(bundle['keyString'], string);
      expect(bundle['keyListString'], strings);
      expect(bundle['keyBytes'], bytes);
    });

    testWidgets('Bundle.entries returns entries of the Bundle',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testName'] = 'entriesTest';
      bundle['testName2'] = 'entriesTest2';

      final Iterable<MapEntry<String, Object>> entries = bundle.entries;
      for (final MapEntry<String, Object> entry in entries) {
        bool matched = false;
        if (entry.key == 'testName' && entry.value == 'entriesTest') {
          matched = true;
        } else if (entry.key == 'testName2' && entry.value == 'entriesTest2') {
          matched = true;
        }

        expect(matched, true);
      }

      expect(entries.length, 2);
    });

    testWidgets('Bundle.keys returns all stored keys', (WidgetTester _) async {
      final Bundle bundle = Bundle();
      final List<String> keys = <String>['testKey', 'key1', 'key2', 'key3'];
      final List<String> values = <String>[
        'getKeyTest',
        'value1',
        'value2',
        'value3'
      ];
      for (int index = 0; index < keys.length; ++index) {
        bundle[keys[index]] = values[index];
      }

      final Iterable<String> keysFromBundle = bundle.keys;
      for (final String key in bundle.keys) {
        expect(keys.contains(key), true);
      }

      expect(keysFromBundle.length, keys.length);
    });

    testWidgets('Bundle.values returns all stored values',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      final List<String> keys = <String>['testKey', 'key1', 'key2', 'key3'];
      final List<String> values = <String>[
        'getKeyTest',
        'value1',
        'value2',
        'value3'
      ];
      for (int index = 0; index < keys.length; ++index) {
        bundle[keys[index]] = values[index];
      }

      final Iterable<Object> valuesFromBundle = bundle.values;
      for (final Object value in valuesFromBundle) {
        expect(values.contains(value), true);
      }

      expect(valuesFromBundle.length, values.length);
    });

    testWidgets('Bundle.length returns the number of key/value pairs',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      expect(bundle.length, 0);
      bundle['key1'] = 'value1';
      expect(bundle.length, 1);
      bundle['key2'] = 'value2';
      expect(bundle.length, 2);
      bundle['key3'] = 'value3';
      expect(bundle.length, 3);
      bundle['key4'] = 'value4';
      bundle['testName'] = 'lengthTest';
      expect(bundle.length, 5);
    });

    testWidgets('can check whether there is no key/value pair in the bundle',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testName'] = 'isEmptyPropertyTest';
      expect(bundle.isEmpty, false);
      bundle.remove('testName');
      expect(bundle.isEmpty, true);
    });

    testWidgets(
        'can check whether there is at least on key/value pair in the bundle',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testName'] = 'isNotEmptyPropertyTest';
      expect(bundle.isNotEmpty, true);
      bundle.remove('testName');
      expect(bundle.isNotEmpty, false);
    });

    testWidgets('can set the key with the given String value to the bundle',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testKey'] = 'testValue';
      expect(bundle.length, 1);
      expect(bundle['testKey'], 'testValue');
    });

    testWidgets('can set the key with the given Uint8List value to the bundle',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      final Uint8List bytes = Uint8List(10);
      bundle['testKey'] = bytes;
      expect(bundle.length, 1);
      final Uint8List resultBytes = bundle['testKey']! as Uint8List;
      expect(bytes, resultBytes);
      expect(bytes.length, resultBytes.length);
    });

    testWidgets(
        'can set the key with the given List<String> value to the bundle',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      final List<String> strings = <String>['Hello', 'Tizen', 'Bundle'];
      bundle['testKey'] = strings;
      bundle['testName'] = 'operatorTestForStrings';
      expect(bundle.length, 2);
      expect(bundle['testName'], 'operatorTestForStrings');
      final List<String> resultStrings = bundle['testKey']! as List<String>;
      expect(resultStrings.length, strings.length);
      expect(resultStrings, strings);
    });

    testWidgets('can add all key/value pairs of other to this bundle',
        (WidgetTester _) async {
      const String string = 'String';
      final List<String> strings = <String>['String 1', 'String 2'];
      final Uint8List bytes = Uint8List(1);
      bytes[0] = 0x02;
      final Map<String, Object> map = <String, Object>{
        'keyString': string,
        'keyListString': strings,
        'keyBytes': bytes
      };

      final Bundle bundle = Bundle();
      bundle.addAll(map);
      expect(bundle.length, 3);
      expect(bundle['keyString'], string);
      expect(bundle['keyListString'], strings);
      expect(bundle['keyBytes'], bytes);
    });

    testWidgets('can add all key/value pairs of entries to this bundle',
        (WidgetTester _) async {
      const String string = 'String';
      final List<String> strings = <String>['String 1', 'String 2'];
      final Uint8List bytes = Uint8List(2);
      bytes[0] = 0x01;
      bytes[1] = 0x02;
      final Map<String, Object> map = <String, Object>{
        'keyString': string,
        'keyListString': strings,
        'keyBytes': bytes
      };

      final Bundle bundle = Bundle();
      bundle.addEntries(map.entries);
      expect(bundle.length, 3);
      expect(bundle['keyString'], string);
      expect(bundle['keyListString'], strings);
      expect(bundle['keyBytes'], bytes);
    });

    testWidgets('can remove all entries from the bundle',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testName'] = 'clearTest';
      bundle['testName2'] = 'clearTest2';
      expect(bundle.length, 2);
      bundle.clear();
      expect(bundle.length, 0);
    });

    testWidgets('can apply action to each key/value pair of the bundle',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testName'] = 'forEachTest';
      bundle['testName2'] = 'forEachTest2';
      bundle['testName3'] = 'forEachTest3';
      bundle.forEach((String key, Object value) {
        bool matched = false;
        if (key == 'testName' && value == 'forEachTest') {
          matched = true;
        } else if (key == 'testName2' && value == 'forEachTest2') {
          matched = true;
        } else if (key == 'testName3' && value == 'forEachTest3') {
          matched = true;
        }

        expect(matched, true);
      });
    });

    testWidgets('can check whether the bundle contains the given key or not',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testName'] = 'containsKeyTest';
      expect(bundle.containsKey('testName'), true);
    });

    testWidgets('can check whether the bundle contains the given value or not',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testName'] = 'containsValueTest';
      expect(bundle.containsValue('containsValueTest'), true);
    });

    testWidgets('Bundle.map returns a new map where all entries of the bundle',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testName'] = 'mapTest';
      bundle['testName2'] = 'mapTest';
      bundle['testName3'] = 'mapTest';
      bundle.map<String, String>((String key, Object value) {
        return MapEntry<String, String>(key, value as String);
      }).forEach((String key, String value) {
        bool matched = false;
        if (key == 'testName' && value == 'mapTest') {
          matched = true;
        } else if (key == 'testName2' && value == 'mapTest') {
          matched = true;
        } else if (key == 'testName3' && value == 'mapTest') {
          matched = true;
        }

        expect(matched, true);
      });
    });

    testWidgets('can add a new entry if it is not there',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testName'] = 'putIfAbsentTest';
      bundle.putIfAbsent('testName', () => 'putIfAbsentTest2');
      expect(bundle.length, 1);
      expect(bundle['testName'], 'putIfAbsentTest');
    });

    testWidgets('can remove key and its associated value',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testName'] = 'removeTest';
      expect(bundle.length, 1);
      expect(bundle['testName'], 'removeTest');
      bundle.remove('testName');
      expect(bundle.length, 0);
    });

    testWidgets(
        'can remove all entries of the bundle that stisfy the given function',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testName'] = 'removeWhereTest';
      bundle['testName2'] = 'removeWhereTest2';
      expect(bundle.length, 2);
      expect(bundle['testName'], 'removeWhereTest');
      bundle.removeWhere((String key, Object value) {
        return key == 'testName';
      });
      expect(bundle.length, 1);
      expect(bundle['testName2'], 'removeWhereTest2');
    });

    testWidgets('can update the value for the provided key',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testName'] = 'updateTest';
      expect(bundle.length, 1);
      expect(bundle['testName'], 'updateTest');

      bundle.update('testName', (Object value) {
        final String result = '${value as String}2';
        return result;
      });
      expect(bundle.length, 1);
      expect(bundle['testName'], 'updateTest2');
    });

    testWidgets('can update all values', (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testName'] = 'updateAllTest';
      bundle['testName2'] = 'updateAllTest';
      bundle['testName3'] = 'updateAllTest';
      expect(bundle.length, 3);
      expect(bundle['testName'], 'updateAllTest');
      expect(bundle['testName2'], 'updateAllTest');
      expect(bundle['testName3'], 'updateAllTest');

      bundle.updateAll((String key, Object value) {
        final String result = '$key+${value as String}';
        return result;
      });
      expect(bundle.length, 3);
      expect(bundle['testName'], 'testName+updateAllTest');
      expect(bundle['testName2'], 'testName2+updateAllTest');
      expect(bundle['testName3'], 'testName3+updateAllTest');
    });

    testWidgets('can create a bundle from the encoded raw data',
        (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testName'] = 'bundleDecodeTest';
      final String bundleRaw = bundle.encode();
      final Bundle newBundle = Bundle.decode(bundleRaw);
      expect(newBundle['testName'], 'bundleDecodeTest');
    });

    testWidgets('can encode the bundle to the String', (WidgetTester _) async {
      final Bundle bundle = Bundle();
      bundle['testName'] = 'encodeTest';
      expect(bundle.length, 1);

      final String bundleRaw = bundle.encode();
      expect(bundleRaw.isNotEmpty, true);

      final Bundle newBundle = Bundle.decode(bundleRaw);
      expect(newBundle['testName'], 'encodeTest');
    });
  });
}
