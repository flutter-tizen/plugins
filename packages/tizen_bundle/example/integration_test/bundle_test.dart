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
      bundle.addString('testName', 'bundleTest');
      expect(bundle.getString('testName'), 'bundleTest');
    });

    testWidgets('bundleFromRawTest', (WidgetTester _) async {
      Log.info(_logTag, 'bundleFromRawTest');
      final Bundle bundle = Bundle();
      bundle.addString('testName', 'bundleFromRawTest');
      final String bundleRaw = bundle.toRaw();

      final Bundle newBundle = Bundle.fromRaw(bundleRaw);
      expect(newBundle.getString('testName'), 'bundleFromRawTest');
    });

    testWidgets('bundleFromBundleTest', (WidgetTester _) async {
      Log.info(_logTag, 'bundleFromBundleTest');
      final Bundle bundle = Bundle();
      bundle.addString('testName', 'bundleFromBundleTest');

      final Bundle newBundle = Bundle.fromBundle(bundle);
      expect(newBundle.getString('testName'), 'bundleFromBundleTest');
    });

    testWidgets('addStringAndGetString', (WidgetTester _) async {
      Log.info(_logTag, 'addStringAndGetStringTest');
      final Bundle bundle = Bundle();
      bundle.addString('testKey', 'testValue');
      expect(bundle.length, 1);
      expect(bundle.getString('testKey'), 'testValue');
    });

    testWidgets('addBytesAndGetBytes', (WidgetTester _) async {
      Log.info(_logTag, 'addBytesAndGetBytesTest');
      final Bundle bundle = Bundle();
      final List<int> bytes = [0x01, 0x02, 0x03, 0x04, 0x05];
      bundle.addBytes('testKey', bytes);
      expect(bundle.length, 1);
      final List<int> resultBytes = bundle.getBytes('testKey');
      expect(bytes, resultBytes);
      expect(bytes.length, resultBytes.length);
    });

    testWidgets('addStringsAndGetStrings', (WidgetTester _) async {
      Log.info(_logTag, 'addStringsAndGetStringsTest');
      final Bundle bundle = Bundle();
      final List<String> strings = ['Hello', 'Tizen', 'Bundle'];
      bundle.addStrings('testKey', strings);
      bundle.addString('testName', 'addStringsAndGetStringsTest');
      expect(bundle.length, 2);
      expect(bundle.getString('testName'), 'addStringsAndGetStringsTest');
      final List<String> resultStrings = bundle.getStrings('testKey');
      expect(resultStrings.length, strings.length);
      expect(resultStrings, strings);
    });

    testWidgets('toRawTest', (WidgetTester _) async {
      Log.info(_logTag, 'toRawTest');
      final Bundle bundle = Bundle();
      bundle.addString('testName', 'toRawTest');
      expect(bundle.length, 1);

      final String bundleRaw = bundle.toRaw();
      expect(bundleRaw.isNotEmpty, true);
      expect(bundleRaw.isEmpty, false);

      final Bundle newBundle = Bundle.fromRaw(bundleRaw);
      expect(newBundle.getString('testName'), 'toRawTest');
    });

    testWidgets('lengthTest', (WidgetTester _) async {
      Log.info(_logTag, 'lengthTest');
      final Bundle bundle = Bundle();
      expect(bundle.length, 0);
      bundle.addString('key1', 'value1');
      expect(bundle.length, 1);
      bundle.addString('key2', 'value2');
      expect(bundle.length, 2);
      bundle.addString('key3', 'value3');
      expect(bundle.length, 3);
      bundle.addString('key4', 'value4');
      bundle.addString('testName', 'getCountTest');
      expect(bundle.length, 5);
    });

    testWidgets('deleteTest', (WidgetTester _) async {
      Log.info(_logTag, 'deleteTest');
      final Bundle bundle = Bundle();
      bundle.addString('testName', 'deleteTest');
      expect(bundle.length, 1);
      expect(bundle.getString('testName'), 'deleteTest');
      bundle.delete('testName');
      expect(bundle.length, 0);
    });

    testWidgets('isEmptyTest', (WidgetTester _) async {
      Log.info(_logTag, 'isEmptyTest');
      final Bundle bundle = Bundle();
      bundle.addString('testName', 'isEmptyTest');
      expect(bundle.isEmpty, false);
      bundle.delete('testName');
      expect(bundle.isEmpty, true);
    });

    testWidgets('isNotEmptyTest', (WidgetTester _) async {
      Log.info(_logTag, 'isNotEmptyTest');
      final Bundle bundle = Bundle();
      bundle.addString('testName', 'isNotEmptyTest');
      expect(bundle.isNotEmpty, true);
      bundle.delete('testName');
      expect(bundle.isNotEmpty, false);
    });

    testWidgets('getTypeTest', (WidgetTester _) async {
      Log.info(_logTag, 'getTypeTest');
      final Bundle bundle = Bundle();
      bundle.addString('testName', 'getTypeTest');
      BundleType type = bundle.getType('testName');
      expect(type, BundleType.string);

      final List<int> bytes = [0x01, 0x02, 0x03];
      bundle.addBytes('byteKey', bytes);
      type = bundle.getType('byteKey');
      expect(type, BundleType.bytes);
    });

    testWidgets('containsTest', (WidgetTester _) async {
      Log.info(_logTag, 'containsTest');
      final Bundle bundle = Bundle();
      bundle.addString('testName', 'containsTest');
      expect(bundle.contains('testName'), true);
    });

    testWidgets('getKeysTest', (WidgetTester _) async {
      Log.info(_logTag, 'getKeysTest');
      final Bundle bundle = Bundle();
      final List<String> keys = ['testKey', 'key1', 'key2', 'key3'];
      final List<String> values = ['getKeyTest', 'value1', 'value2', 'value3'];
      for (int index = 0; index < keys.length; ++index) {
        bundle.addString(keys[index], values[index]);
      }

      final List<KeyInfo> keyInfos = bundle.getKeys();
      keyInfos.asMap().forEach((int index, KeyInfo keyInfo) {
        bool matched = false;
        for (int index = 0; index < keys.length; ++index) {
          if (matched) {
            break;
          }

          matched =
              keyInfo.name == keys[index] && keyInfo.type == BundleType.string;
        }

        expect(matched, true);
      });

      expect(keyInfos.length, keys.length);
    });

    testWidgets('bundleFromMapTest', (WidgetTester _) async {
      Log.info(_logTag, 'bundleFromMapTest');
      const String string = 'String';
      final List<String> strings = ['String 1', 'String 2'];
      final List<int> bytes = [0x01, 0x02, 0x03];
      final Map<String, dynamic> map = {};
      map['keyString'] = string;
      map['keyListString'] = strings;
      map['keyBytes'] = bytes;

      final Bundle bundle = Bundle.fromMap(map);
      expect(bundle.length, 3);
      expect(bundle.getString('keyString'), string);
      expect(bundle.getStrings('keyListString'), strings);
      expect(bundle.getBytes('keyBytes'), bytes);
    });
  });
}
