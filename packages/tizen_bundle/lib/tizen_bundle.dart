// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';
import 'package:tizen_interop/4.0/tizen.dart';
import 'package:tizen_log/tizen_log.dart';

/// Bundle is a string based Dictionary ADT.
/// A dictionary is an ordered or unordered list of key element pairs,
/// where keys are used to locate elements in the list.
class Bundle {
  /// Creates an instance of [Bundle].
  Bundle() {
    _handle = tizen.bundle_create();
    _finalizer.attach(this, this, detach: this);
  }

  /// Creates an instance of [Bundle] with the given raw data.
  Bundle.fromRaw(String raw) {
    _handle = tizen.bundle_decode(raw.toNativeInt8().cast<Uint8>(), raw.length);
    _finalizer.attach(this, this, detach: this);
  }

  /// Creates an instance of [Bundle] with the given bundle object.
  Bundle.fromBundle(Bundle bundle) {
    _handle = tizen.bundle_dup(bundle._handle);
    _finalizer.attach(this, this, detach: this);
  }

  /// Creates an instance of [Bundle] with the given map object.
  factory Bundle.fromMap(Map<String, Object> map) {
    final Bundle bundle = Bundle();

    map.forEach((String key, Object value) => bundle[key] = value);
    return bundle;
  }

  static const String _logTag = 'BUNDLE';
  late final _handle;
  static final List<String> _keys = <String>[];
  bool _isDisposed = false;
  static final Finalizer<Bundle> _finalizer =
      Finalizer<Bundle>((Bundle bundle) => bundle.dispose());

  /// The map entries of this.
  Iterable<MapEntry<String, Object>> get entries {
    return toMap().entries;
  }

  /// The keys of this.
  Iterable<String> get keys {
    final List<String> keys = <String>[];
    toMap().forEach((String key, Object value) {
      keys.add(key);
    });
    return keys;
  }

  /// The values of this.
  Iterable<Object> get values {
    final List<Object> values = <Object>[];
    toMap().forEach((String key, Object value) {
      values.add(value);
    });
    return values;
  }

  /// Gets the number of items in the bundle object.
  int get length => toMap().length;

  /// Checks whether the bundle object is empty or not.
  bool get isEmpty => length == 0;

  /// Checks whether the bundle object is not empty or not.
  bool get isNotEmpty => length > 0;

  /// The value for the given key, or null if key is not in the Bundle.
  Object? operator [](String key) {
    Object? value;
    final int type = _getType(key);
    if (type == bundle_type.BUNDLE_TYPE_STR) {
      value = _getString(key);
    } else if (type == bundle_type.BUNDLE_TYPE_STR_ARRAY) {
      value = _getStrings(key);
    } else if (type == bundle_type.BUNDLE_TYPE_BYTE) {
      value = _getBytes(key);
    }

    return value;
  }

  /// Associates the key with the given value.
  void operator []=(String key, Object value) {
    if (containsKey(key)) {
      remove(key);
    }

    if (value is String) {
      _addString(key, value);
    } else if (value is List<String>) {
      _addStrings(key, value);
    } else if (value is Uint8List) {
      _addBytes(key, value);
    } else {
      Log.error(_logTag, 'No such type: ${value.runtimeType}');
      _throwException(bundle_error_e.BUNDLE_ERROR_INVALID_PARAMETER);
    }
  }

  /// Adds all key/value pairs [other] to this Bundle.
  void addAll(Map<String, Object> other) {
    other.forEach((String key, Object value) {
      if (containsKey(key)) {
        remove(key);
      }

      this[key] = value;
    });
  }

  /// Adds all key/value pairs of [newEntries] to this [Bundle].
  void addEntries(Iterable<MapEntry<String, Object>> newEntries) {
    for (final MapEntry<String, Object> element in newEntries) {
      this[element.key] = element.value;
    }
  }

  /// Removes all entries from the [Bundle].
  void clear() {
    keys.forEach((String key) => remove(key));
  }

  /// Applies [action] to each key/value pair of the [Bundle].
  void forEach(void Function(String key, Object value) action) {
    toMap().forEach((String key, Object value) => action(key, value));
  }

  /// Whether this [Bundle] contains the given [key].
  bool containsKey(String key) {
    return toMap().containsKey(key);
  }

  /// Whether this [Bundle] contains the given [value].
  bool containsValue(Object value) {
    return toMap().containsValue(value);
  }

  static void _bundleIteratorCallback(Pointer<Int8> pKey, int type,
      Pointer<keyval_t> pKeyval, Pointer<Void> userData) {
    _keys.add(pKey.toDartString());
  }

  /// Creates a [Map] containing the elements of this [Bundle].
  Map<String, Object> toMap() {
    _keys.clear();
    tizen.bundle_foreach(
        _handle, Pointer.fromFunction(_bundleIteratorCallback), nullptr);
    final Map<String, Object> map = <String, Object>{};
    for (final String element in _keys) {
      map[element] = this[element]!;
    }
    return map;
  }

  /// Returns a new map where all entries of this [Bundle] are transformed by the given [convert] function.
  Map<K, V> map<K, V>(
      MapEntry<K, V> Function(String key, Object value) convert) {
    final Map<K, V> convertedMap = <K, V>{};
    toMap().forEach((String key, Object value) {
      final List<MapEntry<K, V>> iterable = <MapEntry<K, V>>[
        convert(key, value)
      ];
      convertedMap.addEntries(iterable);
    });
    return convertedMap;
  }

  /// Look up the value of the [key], or add a new entry if it isn't there.
  Object putIfAbsent(String key, Object Function() ifAbsent) {
    final Object? value = this[key];
    if (value == null) {
      this[key] = ifAbsent();
    }

    return this[key]!;
  }

  /// Removes [key] and its associated value, if present, from the [Bundle].
  void remove(String key) {
    final int ret = using((Arena arena) {
      return tizen.bundle_del(_handle, key.toNativeInt8(allocator: arena));
    });
    if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
      Log.error(_logTag, 'Failed to delete key/value. key: $key, error: $ret');
      _throwException(ret);
    }
  }

  /// Removes all entries of this [Bundle] that satisfy the given [test].
  void removeWhere(bool Function(String key, Object value) test) {
    forEach((String key, Object value) {
      if (test(key, value)) {
        remove(key);
      }
    });
  }

  /// Updates the value for the provided [key].
  Object update(String key, Object Function(Object value) update,
      {Object Function()? ifAbsent}) {
    final Object? value = this[key];
    if (value == null) {
      if (ifAbsent == null) {
        throw Exception('ifAbsent should not be null');
      } else {
        this[key] = ifAbsent();
      }
    } else {
      this[key] = update(value);
    }

    return this[key]!;
  }

  /// Updates all values.
  void updateAll(Object Function(String key, Object value) update) {
    forEach((String key, Object value) {
      this[key] = update(key, value);
    });
  }

  /// Converts this object to String type.
  String toRaw() {
    final String raw = using((Arena arena) {
      final Pointer<Pointer<Int8>> rawPointer = arena<Pointer<Int8>>();
      final Pointer<Int32> lengthPointer = arena<Int32>();
      final int ret = tizen.bundle_encode(
          _handle, rawPointer.cast<Pointer<Uint8>>(), lengthPointer);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(_logTag, 'Failed to encode bundle to raw data. error: $ret');
        _throwException(ret);
      }

      return rawPointer.value.toDartString();
    });

    return raw;
  }

  /// Releases all resources associated with this object.
  void dispose() {
    if (_isDisposed) {
      return;
    }

    tizen.bundle_free(_handle);
    _isDisposed = true;
    _finalizer.detach(this);
  }

  static void _throwException(int ret) {
    throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString());
  }

  void _addString(String key, String value) {
    final int ret = using((Arena arena) {
      return tizen.bundle_add_str(_handle, key.toNativeInt8(allocator: arena),
          value.toNativeInt8(allocator: arena));
    });
    if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
      Log.error(_logTag,
          'Failed to add string. key: $key, value: $value, error: $ret');
      _throwException(ret);
    }
  }

  String _getString(String key) {
    final String value = using((Arena arena) {
      final Pointer<Pointer<Int8>> pValue = arena<Pointer<Int8>>();
      final int ret = tizen.bundle_get_str(
          _handle, key.toNativeInt8(allocator: arena), pValue);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(_logTag, 'Failed to get string. key: $key, error: $ret');
        _throwException(ret);
      }

      return pValue.value.toDartString();
    });

    return value;
  }

  void _addStrings(String key, List<String> values) {
    using((Arena arena) {
      final List<Pointer<Int8>> pointerList = values
          .map((String str) => str.toNativeInt8(allocator: arena))
          .toList();
      final Pointer<Pointer<Int8>> pointerArray =
          arena<Pointer<Int8>>(pointerList.length);
      values.asMap().forEach((int index, String value) {
        pointerArray[index] = pointerList[index];
      });

      final int ret = tizen.bundle_add_str_array(_handle,
          key.toNativeInt8(allocator: arena), pointerArray, values.length);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(
            _logTag, 'Failed to add string array. key: $key, error: $ret');
        _throwException(ret);
      }
    });
  }

  List<String> _getStrings(String key) {
    final List<String> values = using((Arena arena) {
      final Pointer<Int32> arraySize = arena<Int32>();
      final Pointer<Pointer<Int8>> stringArray = tizen.bundle_get_str_array(
          _handle, key.toNativeInt8(allocator: arena), arraySize);
      final int ret = tizen.get_last_result();
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(
            _logTag, 'Failed to get string array. key: $key, error: $ret');
        _throwException(ret);
      }

      final List<String> strings = <String>[];
      for (int index = 0; index < arraySize.value; ++index) {
        strings.add(stringArray[index].toDartString());
      }

      return strings;
    });

    return values;
  }

  void _addBytes(String key, Uint8List bytes) {
    using((Arena arena) {
      final Pointer<Uint8> bytesArray = arena<Uint8>(bytes.length);
      for (int index = 0; index < bytes.length; ++index) {
        bytesArray[index] = bytes[index] & 0xff;
      }

      final int ret = tizen.bundle_add_byte(
          _handle,
          key.toNativeInt8(allocator: arena),
          bytesArray.cast<Void>(),
          bytes.length);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(_logTag, 'Failed to add byte. key: $key, error: $ret');
        _throwException(ret);
      }
    });
  }

  Uint8List _getBytes(String key) {
    final Uint8List values = using((Arena arena) {
      final Pointer<Pointer<Uint8>> bytes = arena<Pointer<Uint8>>();
      final Pointer<Int32> bytesSize = arena<Int32>();
      final int ret = tizen.bundle_get_byte(
          _handle,
          key.toNativeInt8(allocator: arena),
          bytes.cast<Pointer<Void>>(),
          bytesSize);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(_logTag, 'Failed to get byte. key: $key, error: $ret');
        _throwException(ret);
      }

      final Uint8List byteList = Uint8List(bytesSize.value);
      for (int index = 0; index < bytesSize.value; ++index) {
        byteList[index] = bytes.value[index];
      }

      return byteList;
    });

    return values;
  }

  int _getType(String key) {
    final int type = using((Arena arena) {
      return tizen.bundle_get_type(_handle, key.toNativeInt8(allocator: arena));
    });

    return type;
  }
}
