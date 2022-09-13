import 'dart:typed_data';

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_bundle/tizen_bundle.dart';
import 'package:tizen_log/tizen_log.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  group('$Bundle', () {
    const String _logTag = 'BundleTest';

    setUp(() async {});

    tearDown(() {});

    testWidgets('bundleTest', (WidgetTester _) async {
      Log.info(_logTag, 'bundleTest');
      final Bundle bundle = Bundle();
      bundle['testName'] = 'bundleTest';
      expect(bundle['testName'], 'bundleTest');
    });

    testWidgets('bundleFromRawTest', (WidgetTester _) async {
      Log.info(_logTag, 'bundleFromRawTest');
      final Bundle bundle = Bundle();
      bundle['testName'] = 'bundleFromRawTest';
      final String bundleRaw = bundle.toRaw();

      final Bundle newBundle = Bundle.fromRaw(bundleRaw);
      expect(newBundle['testName'], 'bundleFromRawTest');
    });

    testWidgets('bundleFromBundleTest', (WidgetTester _) async {
      Log.info(_logTag, 'bundleFromBundleTest');
      final Bundle bundle = Bundle();
      bundle['testName'] = 'bundleFromBundleTest';

      final Bundle newBundle = Bundle.fromBundle(bundle);
      expect(newBundle['testName'], 'bundleFromBundleTest');
    });

    testWidgets('addStringAndGetString', (WidgetTester _) async {
      Log.info(_logTag, 'addStringAndGetStringTest');
      final Bundle bundle = Bundle();
      bundle['testKey'] = 'testValue';
      expect(bundle.length, 1);
      expect(bundle['testKey'], 'testValue');
    });

    testWidgets('addBytesAndGetBytes', (WidgetTester _) async {
      Log.info(_logTag, 'addBytesAndGetBytesTest');
      final Bundle bundle = Bundle();
      final Uint8List bytes = Uint8List(10);
      bundle['testKey'] = bytes;
      expect(bundle.length, 1);
      final Uint8List resultBytes = bundle['testKey']! as Uint8List;
      expect(bytes, resultBytes);
      expect(bytes.length, resultBytes.length);
    });

    testWidgets('addStringsAndGetStrings', (WidgetTester _) async {
      Log.info(_logTag, 'addStringsAndGetStringsTest');
      final Bundle bundle = Bundle();
      final List<String> strings = ['Hello', 'Tizen', 'Bundle'];
      bundle['testKey'] = strings;
      bundle['testName'] = 'addStringsAndGetStringsTest';
      expect(bundle.length, 2);
      expect(bundle['testName'], 'addStringsAndGetStringsTest');
      final List<String> resultStrings = bundle['testKey']! as List<String>;
      expect(resultStrings.length, strings.length);
      expect(resultStrings, strings);
    });

    testWidgets('toRawTest', (WidgetTester _) async {
      Log.info(_logTag, 'toRawTest');
      final Bundle bundle = Bundle();
      bundle['testName'] = 'toRawTest';
      expect(bundle.length, 1);

      final String bundleRaw = bundle.toRaw();
      expect(bundleRaw.isNotEmpty, true);
      expect(bundleRaw.isEmpty, false);

      final Bundle newBundle = Bundle.fromRaw(bundleRaw);
      expect(newBundle['testName'], 'toRawTest');
    });

    testWidgets('lengthTest', (WidgetTester _) async {
      Log.info(_logTag, 'lengthTest');
      final Bundle bundle = Bundle();
      expect(bundle.length, 0);
      bundle['key1'] = 'value1';
      expect(bundle.length, 1);
      bundle['key2'] = 'value2';
      expect(bundle.length, 2);
      bundle['key3'] = 'value3';
      expect(bundle.length, 3);
      bundle['key4'] = 'value4';
      bundle['testName'] = 'getCountTest';
      expect(bundle.length, 5);
    });

    testWidgets('removeTest', (WidgetTester _) async {
      Log.info(_logTag, 'removeTest');
      final Bundle bundle = Bundle();
      bundle['testName'] = 'removeTest';
      expect(bundle.length, 1);
      expect(bundle['testName'], 'removeTest');
      bundle.remove('testName');
      expect(bundle.length, 0);
    });

    testWidgets('removeWhereTest', (WidgetTester _) async {
      Log.info(_logTag, 'removeWhereTest');
      final Bundle bundle = Bundle();
      bundle['testName'] = 'removeWhereTest';
      bundle['testName2'] = 'removeWhereTest2';
      expect(bundle.length, 2);
      expect(bundle['testName'], 'removeWhereTest');
      bundle.removeWhere((key, value) {
        return key == 'testName';
      });
      expect(bundle.length, 1);
      expect(bundle['testName2'], 'removeWhereTest2');
    });

    testWidgets('updateTest', (WidgetTester _) async {
      Log.info(_logTag, 'updateAllTest');
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

    testWidgets('updateAllTest', (WidgetTester _) async {
      Log.info(_logTag, 'removeWhereTest');
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

    testWidgets('isEmptyTest', (WidgetTester _) async {
      Log.info(_logTag, 'isEmptyTest');
      final Bundle bundle = Bundle();
      bundle['testName'] = 'isEmptyTest';
      expect(bundle.isEmpty, false);
      bundle.remove('testName');
      expect(bundle.isEmpty, true);
    });

    testWidgets('isNotEmptyTest', (WidgetTester _) async {
      Log.info(_logTag, 'isNotEmptyTest');
      final Bundle bundle = Bundle();
      bundle['testName'] = 'isNotEmptyTest';
      expect(bundle.isNotEmpty, true);
      bundle.remove('testName');
      expect(bundle.isNotEmpty, false);
    });

    testWidgets('entriesTest', (WidgetTester _) async {
      Log.info(_logTag, 'entriesTest');
      final Bundle bundle = Bundle();
      bundle['testName'] = 'entriesTest';
      bundle['testName2'] = 'entriesTest2';

      final Iterable<MapEntry<String, Object>> entries = bundle.entries;
      for (MapEntry<String, Object> entry in entries) {
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

    testWidgets('containsKeyTest', (WidgetTester _) async {
      Log.info(_logTag, 'containsKeyTest');
      final Bundle bundle = Bundle();
      bundle['testName'] = 'containsKeyTest';
      expect(bundle.containsKey('testName'), true);
    });

    testWidgets('containsValueTest', (WidgetTester _) async {
      Log.info(_logTag, 'containsValueTest');
      final Bundle bundle = Bundle();
      bundle['testName'] = 'containsValueTest';
      expect(bundle.containsValue('containsValueTest'), true);
    });

    testWidgets('keysAndvaluesTest', (WidgetTester _) async {
      Log.info(_logTag, 'keysAndvaluesTest');
      final Bundle bundle = Bundle();
      final List<String> keys = ['testKey', 'key1', 'key2', 'key3'];
      final List<String> values = ['getKeyTest', 'value1', 'value2', 'value3'];
      for (int index = 0; index < keys.length; ++index) {
        bundle[keys[index]] = values[index];
      }

      final Iterable<String> keysFromBundle = bundle.keys;
      for (String key in bundle.keys) {
        expect(keys.contains(key), true);
      }

      expect(keysFromBundle.length, keys.length);

      final Iterable<Object> valuesFromBundle = bundle.values;
      for (Object value in valuesFromBundle) {
        expect(values.contains(value), true);
      }

      expect(valuesFromBundle.length, values.length);
    });

    testWidgets('bundleFromMapTest', (WidgetTester _) async {
      Log.info(_logTag, 'bundleFromMapTest');
      const String string = 'String';
      final List<String> strings = ['String 1', 'String 2'];
      final Uint8List bytes = Uint8List(1);
      bytes[0] = 0x03;
      final Map<String, Object> map = {
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

    testWidgets('addAllTest', (WidgetTester _) async {
      Log.info(_logTag, 'addAllTest');
      const String string = 'String';
      final List<String> strings = ['String 1', 'String 2'];
      final Uint8List bytes = Uint8List(1);
      bytes[0] = 0x02;
      final Map<String, Object> map = {
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

    testWidgets('addEntriesTest', (WidgetTester _) async {
      Log.info(_logTag, 'addEntriesTest');
      const String string = 'String';
      final List<String> strings = ['String 1', 'String 2'];
      final Uint8List bytes = Uint8List(2);
      bytes[0] = 0x01;
      bytes[1] = 0x02;
      final Map<String, Object> map = {
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

    testWidgets('clearTest', (WidgetTester _) async {
      Log.info(_logTag, 'clearTest');
      final Bundle bundle = Bundle();
      bundle['testName'] = 'clearTest';
      bundle['testName2'] = 'clearTest2';
      expect(bundle.length, 2);
      bundle.clear();
      expect(bundle.length, 0);
    });

    testWidgets('putIfAbsentTest', (WidgetTester _) async {
      Log.info(_logTag, 'putIfAbsentTest');
      final Bundle bundle = Bundle();
      bundle['testName'] = 'putIfAbsentTest';
      bundle.putIfAbsent('testName', () => 'putIfAbsentTest2');
      expect(bundle.length, 1);
      expect(bundle['testName'], 'putIfAbsentTest');
    });
  });
}
