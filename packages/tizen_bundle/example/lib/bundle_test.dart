import 'package:tizen_log/tizen_log.dart';
import 'package:test/test.dart';
import 'package:tizen_bundle/tizen_bundle.dart';

class BundleTest {
  static final BundleTest _instance = BundleTest._internal();
  final Map<String, Function> _testcases = {};
  static const String _logTag = "BundleTest";

  factory BundleTest() {
    return _instance;
  }

  void addTestcase(Function func) {
    _testcases[func.toString()] = func;
  }

  BundleTest._internal() {
    addTestcase(bundleTest);
    addTestcase(bundleFromBundleRawTest);
    addTestcase(bundleFromBundleTest);
    addTestcase(addStringAndGetStringTest);
    addTestcase(addBytesAndGetBytesTest);
    addTestcase(addStringsAndGetStringsTest);
    addTestcase(toRawTest);
    addTestcase(getCountTest);
    addTestcase(deleteTest);
    addTestcase(isEmptyTest);
    addTestcase(getTypeTest);
    addTestcase(getKeysTest);
    addTestcase(bundleFromMapTest);
  }

  void setUp() {}

  void tearDown() {}

  void run() {
    _testcases.forEach((key, testFunction) {
      Log.info(_logTag, 'BundleTest: $key');
      setUp();
      testFunction();
      tearDown();
    });
  }

  void bundleTest() {
    test('bundleTest', () {
      Log.info(_logTag, 'bundleTest');
      var bundle = Bundle();
      bundle.addString("testName", "bundleTest");
      expect(bundle.getString("testName"), "bundleTest");
      bundle.dispose();
    });
  }

  void bundleFromBundleRawTest() {
    test('bundleFromBundleRawTest', () {
      Log.info(_logTag, 'bundleFromBundleRawTest');
      var bundle = Bundle();
      bundle.addString("testName", "bundleFromBundleRawTest");
      BundleRaw bundleRaw = bundle.toRaw();
      bundle.dispose();

      var newBundle = Bundle.fromBundleRaw(bundleRaw);
      expect(newBundle.getString("testName"), "bundleFromBundleRawTest");
      newBundle.dispose();
    });
  }

  void bundleFromBundleTest() {
    test('bundleFromBundleTest', () {
      Log.info(_logTag, 'bundleFromBundleTest');
      var bundle = Bundle();
      bundle.addString("testName", "bundleFromBundleTest");

      var newBundle = Bundle.fromBundle(bundle);
      bundle.dispose();
      expect(newBundle.getString("testName"), "bundleFromBundleTest");
      newBundle.dispose();
    });
  }

  void addStringAndGetStringTest() {
    test('addStringAndGetString', () {
      Log.info(_logTag, 'addStringAndGetStringTest');
      var bundle = Bundle();
      bundle.addString("testKey", "testValue");
      expect(bundle.getCount(), 1);
      expect(bundle.getString("testKey"), "testValue");
      bundle.dispose();
    });
  }

  void addBytesAndGetBytesTest() {
    test('addBytesAndGetBytes', () {
      Log.info(_logTag, 'addBytesAndGetBytesTest');
      var bundle = Bundle();
      List<int> bytes = [0x01, 0x02, 0x03, 0x04, 0x05];
      bundle.addBytes("testKey", bytes);
      expect(bundle.getCount(), 1);
      var resultBytes = bundle.getBytes("testKey");
      expect(bytes, resultBytes);
      expect(bytes.length, resultBytes.length);
      bundle.dispose();
    });
  }

  void addStringsAndGetStringsTest() {
    test('addStringsAndGetStrings', () {
      Log.info(_logTag, 'addStringsAndGetStringsTest');
      var bundle = Bundle();
      List<String> strings = ["Hello", "Tizen", "Bundle"];
      bundle.addStrings("testKey", strings);
      bundle.addString("testName", "addStringsAndGetStringsTest");
      expect(bundle.getCount(), 2);
      expect(bundle.getString("testName"), "addStringsAndGetStringsTest");
      var resultStrings = bundle.getStrings("testKey");
      expect(resultStrings.length, strings.length);
      expect(resultStrings, strings);
      bundle.dispose();
    });
  }

  void toRawTest() {
    test('toRawTest', () {
      Log.info(_logTag, 'toRawTest');
      var bundle = Bundle();
      bundle.addString("testName", "toRawTest");
      expect(bundle.getCount(), 1);

      var bundleRaw = bundle.toRaw();
      bundle.dispose();
      expect(bundleRaw.raw.isNotEmpty, true);
      expect(bundleRaw.length != 0, true);

      var newBundle = Bundle.fromBundleRaw(bundleRaw);
      expect(newBundle.getString("testName"), "toRawTest");
      newBundle.dispose();
    });
  }

  void getCountTest() {
    test('getCountTest', () {
      Log.info(_logTag, 'getCountTest');
      var bundle = Bundle();
      expect(bundle.getCount(), 0);
      bundle.addString("key1", "value1");
      expect(bundle.getCount(), 1);
      bundle.addString("key2", "value2");
      expect(bundle.getCount(), 2);
      bundle.addString("key3", "value3");
      expect(bundle.getCount(), 3);
      bundle.addString("key4", "value4");
      bundle.addString("testName", "getCountTest");
      expect(bundle.getCount(), 5);
      bundle.dispose();
    });
  }

  void deleteTest() {
    test('deleteTest', () {
      Log.info(_logTag, 'deleteTest');
      var bundle = Bundle();
      bundle.addString("testName", "deleteTest");
      expect(bundle.getCount(), 1);
      expect(bundle.getString("testName"), "deleteTest");
      bundle.delete("testName");
      expect(bundle.getCount(), 0);
      bundle.dispose();
    });
  }

  void isEmptyTest() {
    test('isEmptyTest', () {
      Log.info(_logTag, 'isEmptyTest');
      var bundle = Bundle();
      bundle.addString('testName', 'isEmptyTest');
      expect(bundle.isEmpty(), false);
      bundle.delete('testName');
      expect(bundle.isEmpty(), true);
      bundle.dispose();
    });
  }

  void getTypeTest() {
    test('getTypeTest', () {
      Log.info(_logTag, 'getTypeTest');
      var bundle = Bundle();
      bundle.addString('testName', 'getTypeTest');
      int type = bundle.getType('testName');
      expect(type, BundleType.string);

      List<int> bytes = [0x01, 0x02, 0x03];
      bundle.addBytes('byteKey', bytes);
      type = bundle.getType('byteKey');
      expect(type, BundleType.byte);
      bundle.dispose();
    });
  }

  void getKeysTest() {
    test('getKeysTest', () {
      Log.info(_logTag, 'getKeysTest');
      var bundle = Bundle();
      List<String> keys = ['testKey', 'key1', 'key2', 'key3'];
      List<String> values = ['getKeyTest', 'value1', 'value2', 'value3'];
      for (var index = 0; index < keys.length; ++index) {
        bundle.addString(keys[index], values[index]);
      }

      final keyInfos = bundle.getKeys();
      keyInfos?.asMap().forEach((index, keyInfo) {
        bool matched = false;
        for (var index = 0; index < keys.length; ++index) {
          if (matched) break;

          matched =
              keyInfo.name == keys[index] && keyInfo.type == BundleType.string;
        }

        expect(matched, true);
      });

      expect(keyInfos?.length, keys.length);
      bundle.dispose();
    });
  }

  void bundleFromMapTest() {
    test('bundleFromMapTest', () {
      Log.info(_logTag, 'bundleFromMapTest');
      String string = 'String';
      List<String> strings = ['String 1', 'String 2'];
      List<int> bytes = [0x01, 0x02, 0x03];
      Map<String, dynamic> map = {};
      map['keyString'] = string;
      map['keyListString'] = strings;
      map['keyBytes'] = bytes;

      var bundle = Bundle.fromMap(map);
      expect(bundle.getCount(), 3);
      expect(bundle.getString('keyString'), string);
      expect(bundle.getStrings('keyListString'), strings);
      expect(bundle.getBytes('keyBytes'), bytes);
      bundle.dispose();
    });
  }
}
