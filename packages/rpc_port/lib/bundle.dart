import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:tizen_interop/6.5/tizen.dart';
import 'package:tizen_log/tizen_log.dart';

import 'disposable.dart';

const String _logTag = "DART_BUNDLE";

class BundleTypeProperty {
  static const int array = 0x0100;
  static const int primitive = 0x0200;
  static const int measurable = 0x0400;
}

class BundleType {
  static const int none = -1;
  static const int any = 0;
  static const int string = 1 | BundleTypeProperty.measurable;
  static const int stringArray = string | BundleTypeProperty.array;
  static const int byte = 2;
  static const int byteArray = byte | BundleTypeProperty.array;
}

class KeyInfo {
  final String _name;
  final int _type;

  KeyInfo(this._name, this._type);

  bool isArray() {
    return (_type & BundleTypeProperty.array) as bool;
  }

  int get getType => _type;

  String get getName => _name;
}

class BundleRaw {
  final String _raw;
  final int _length;

  BundleRaw(this._raw, this._length);

  String get getRaw => _raw;
  int get getLength => _length;
}

class Bundle implements Disposable {
  final dynamic _handle;
  static int _iteratorCount = 0;
  static final Map<int, List<KeyInfo>> _iteratorMap = <int, List<KeyInfo>>{};

  @override
  bool isDisposed = false;

  Bundle() : _handle = tizen.bundle_create();

  Bundle.fromBundleRaw(BundleRaw bundleRaw)
      : _handle = tizen.bundle_decode(
            bundleRaw.getRaw.toNativeInt8().cast<Uint8>(), bundleRaw.getLength);

  Bundle.fromBundle(Bundle bundle) : _handle = tizen.bundle_dup(bundle._handle);

  Bundle.fromBundleHandle(dynamic handle) : _handle = tizen.bundle_dup(handle);

  Bundle.withBundleHandle(dynamic handle) : _handle = handle;

  Bundle.fromMap(Map<String, dynamic> map) : _handle = tizen.bundle_create() {
    map.forEach((key, value) {
      if (value is String) {
        addString(key, value);
      } else if (value is List<String>) {
        addStrings(key, value);
      } else if (value is List<int>) {
        addBytes(key, value);
      } else {
        Log.error(_logTag, "No such type. type: ${value.runtimeType}");
      }
    });
  }

  void addString(String key, String value) {
    int ret =
        tizen.bundle_add_str(_handle, key.toNativeInt8(), value.toNativeInt8());
    if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
      Log.error(_logTag,
          "Failed to add string. key: $key, value: $value, error: $ret");
      throw Exception("bundle_add_str() is failed. error: $ret");
    }
  }

  String getString(String key) {
    String value = using((Arena arena) {
      final pValue = arena<Pointer<Int8>>();
      int ret = tizen.bundle_get_str(_handle, key.toNativeInt8(), pValue);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(_logTag, "Failed to get string. key: $key, error: $ret");
        throw Exception("bundle_get_str() is failed. error: $ret");
      }

      return pValue.value.toDartString();
    });

    return value;
  }

  void addStrings(String key, List<String> values) {
    using((Arena arena) {
      final pointerList = values.map((str) => str.toNativeInt8()).toList();
      final pointerArray = arena<Pointer<Int8>>(pointerList.length);
      values.asMap().forEach((index, value) {
        pointerArray[index] = pointerList[index];
      });

      int ret = tizen.bundle_add_str_array(
          _handle, key.toNativeInt8(), pointerArray, values.length);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(
            _logTag, "Failed to add string array. key: $key, error: $ret");
        throw Exception("bundle_add_str_array() is failed. error: $ret");
      }
    });
  }

  List<String> getStrings(String key) {
    List<String> values = using((Arena arena) {
      final arraySize = arena<Int32>();
      final stringArray =
          tizen.bundle_get_str_array(_handle, key.toNativeInt8(), arraySize);
      int ret = tizen.get_last_result();
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(
            _logTag, "Failed to get string array. key: $key, error: $ret");
        throw Exception("bundle_get_str_array() is failed. error: $ret");
      }

      List<String> strings = [];
      for (int index = 0; index < arraySize.value; ++index) {
        strings.add(stringArray[index].toDartString());
      }

      return strings;
    });

    return values;
  }

  static void _bundleIteratorCallback(Pointer<Int8> pKey, int type,
      Pointer<keyval_t> pKeyval, Pointer<Void> userData) {
    final pCount = userData.cast<Int32>();
    final String key = pKey.toDartString();
    _iteratorMap[pCount.value]?.add(KeyInfo(key, type));
  }

  List<KeyInfo>? getKeys() {
    int count = _iteratorCount++;
    _iteratorMap[count] = [];
    List<KeyInfo>? keys = using((Arena arena) {
      final pCount = arena<Int32>();
      pCount.value = count;
      tizen.bundle_foreach(_handle,
          Pointer.fromFunction(_bundleIteratorCallback), pCount.cast<Void>());
      return _iteratorMap[count];
    });
    _iteratorMap.remove(count);

    return keys;
  }

  void delete(String key) {
    int ret = tizen.bundle_del(_handle, key.toNativeInt8());
    if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
      Log.error(_logTag, "Failed to delete key/value. key: $key, error: $ret");
      throw Exception("bundle_del() is failed. error: $ret");
    }
  }

  bool isEmpty() {
    return tizen.bundle_get_count(_handle) == 0;
  }

  void addBytes(String key, List<int> bytes) {
    using((Arena arena) {
      final byteArray = arena<Uint8>(bytes.length);
      for (int index = 0; index < bytes.length; ++index) {
        byteArray[index] = bytes[index] & 0xff;
      }

      int ret = tizen.bundle_add_byte(
          _handle, key.toNativeInt8(), byteArray.cast<Void>(), bytes.length);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(_logTag, "Failed to add bytes. key: $key, error: $ret");
        throw Exception("bundle_add_bytes() is failed. error: $ret");
      }
    });
  }

  List<int> getBytes(String key) {
    List<int> values = using((Arena arena) {
      final bytes = arena<Pointer<Uint8>>();
      final bytesSize = arena<Int32>();
      int ret = tizen.bundle_get_byte(
          _handle, key.toNativeInt8(), bytes.cast<Pointer<Void>>(), bytesSize);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(_logTag, "Failed to get bytes. key: $key, error: $ret");
        throw Exception("bundle_get_byte() is failed. error: $ret");
      }

      List<int> byteList = [];
      for (int index = 0; index < bytesSize.value; ++index) {
        byteList.add(bytes.value[index]);
      }

      return byteList;
    });

    return values;
  }

  BundleRaw toRaw() {
    BundleRaw raw = using((Arena arena) {
      final rawPointer = arena<Pointer<Int8>>();
      final lengthPointer = arena<Int32>();
      int ret = tizen.bundle_encode(
          _handle, rawPointer.cast<Pointer<Uint8>>(), lengthPointer);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(_logTag, "Failed to encode bundle to raw data. error: $ret");
        throw Exception("bundle_encode() is failed. error: $ret");
      }

      return BundleRaw(rawPointer.value.toDartString(), lengthPointer.value);
    });

    return raw;
  }

  int getCount() {
    return tizen.bundle_get_count(_handle);
  }

  int getType(String key) {
    return tizen.bundle_get_type(_handle, key.toNativeInt8());
  }

  dynamic get getHandle => _handle;

  @override
  void dispose() {
    if (isDisposed) return;

    tizen.bundle_free(_handle);
    isDisposed = true;
  }
}
