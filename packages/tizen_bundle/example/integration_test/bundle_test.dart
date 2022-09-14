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

    testWidgets('entriesPropertyTest', (WidgetTester _) async {
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

    testWidgets('keysPropertyTest', (WidgetTester _) async {
      Log.info(_logTag, 'keysPropertyTest');
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
    });

    testWidgets('valuesPropertyTest', (WidgetTester _) async {
      Log.info(_logTag, 'valuesPropertyTest');
      final Bundle bundle = Bundle();
      final List<String> keys = ['testKey', 'key1', 'key2', 'key3'];
      final List<String> values = ['getKeyTest', 'value1', 'value2', 'value3'];
      for (int index = 0; index < keys.length; ++index) {
        bundle[keys[index]] = values[index];
      }

      final Iterable<Object> valuesFromBundle = bundle.values;
      for (Object value in valuesFromBundle) {
        expect(values.contains(value), true);
      }

      expect(valuesFromBundle.length, values.length);
    });

    testWidgets('lengthPropertyTest', (WidgetTester _) async {
      Log.info(_logTag, 'lengthPropertyTest');
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

    testWidgets('isEmptyPropertyTest', (WidgetTester _) async {
      Log.info(_logTag, 'isEmptyPropertyTest');
      final Bundle bundle = Bundle();
      bundle['testName'] = 'isEmptyPropertyTest';
      expect(bundle.isEmpty, false);
      bundle.remove('testName');
      expect(bundle.isEmpty, true);
    });

    testWidgets('isNotEmptyPropertyTest', (WidgetTester _) async {
      Log.info(_logTag, 'isNotEmptyPropertyTest');
      final Bundle bundle = Bundle();
      bundle['testName'] = 'isNotEmptyPropertyTest';
      expect(bundle.isNotEmpty, true);
      bundle.remove('testName');
      expect(bundle.isNotEmpty, false);
    });

    testWidgets('operatorTestForString', (WidgetTester _) async {
      Log.info(_logTag, 'operatorTestForString');
      final Bundle bundle = Bundle();
      bundle['testKey'] = 'testValue';
      expect(bundle.length, 1);
      expect(bundle['testKey'], 'testValue');
    });

    testWidgets('operatorTestForBytes', (WidgetTester _) async {
      Log.info(_logTag, 'operatorTestForBytes');
      final Bundle bundle = Bundle();
      final Uint8List bytes = Uint8List(10);
      bundle['testKey'] = bytes;
      expect(bundle.length, 1);
      final Uint8List resultBytes = bundle['testKey']! as Uint8List;
      expect(bytes, resultBytes);
      expect(bytes.length, resultBytes.length);
    });

    testWidgets('operatorTestForStrings', (WidgetTester _) async {
      Log.info(_logTag, 'operatorTestForStrings');
      final Bundle bundle = Bundle();
      final List<String> strings = ['Hello', 'Tizen', 'Bundle'];
      bundle['testKey'] = strings;
      bundle['testName'] = 'operatorTestForStrings';
      expect(bundle.length, 2);
      expect(bundle['testName'], 'operatorTestForStrings');
      final List<String> resultStrings = bundle['testKey']! as List<String>;
      expect(resultStrings.length, strings.length);
      expect(resultStrings, strings);
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

    testWidgets('forEachTest', (WidgetTester _) async {
      Log.info(_logTag, 'forEachTest');
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

    testWidgets('toMapTest', (WidgetTester _) async {
      Log.info(_logTag, 'toMapTest');
      final Bundle bundle = Bundle();
      bundle['testName'] = 'toMapTest';
      bundle['testName2'] = 'toMapTest2';
      bundle['testName3'] = 'toMapTest3';
      bundle.toMap().forEach((String key, Object value) {
        bool matched = false;
        if (key == 'testName' && value == 'toMapTest') {
          matched = true;
        } else if (key == 'testName2' && value == 'toMapTest2') {
          matched = true;
        } else if (key == 'testName3' && value == 'toMapTest3') {
          matched = true;
        }

        expect(matched, true);
      });
    });

    testWidgets('mapTest', (WidgetTester _) async {
      Log.info(_logTag, 'mapTest');
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

    testWidgets('putIfAbsentTest', (WidgetTester _) async {
      Log.info(_logTag, 'putIfAbsentTest');
      final Bundle bundle = Bundle();
      bundle['testName'] = 'putIfAbsentTest';
      bundle.putIfAbsent('testName', () => 'putIfAbsentTest2');
      expect(bundle.length, 1);
      expect(bundle['testName'], 'putIfAbsentTest');
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
  });
}
