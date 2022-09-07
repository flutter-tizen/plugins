// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';
import 'package:tizen_interop/4.0/tizen.dart';
import 'package:tizen_log/tizen_log.dart';

/// The class for the type of the bundle.
enum BundleType {
  /// The none type.
  none(bundle_type.BUNDLE_TYPE_NONE),

  /// The string type.
  string(bundle_type.BUNDLE_TYPE_STR),

  /// The string list Type.
  strings(bundle_type.BUNDLE_TYPE_STR_ARRAY),

  /// The bytes type.
  bytes(bundle_type.BUNDLE_TYPE_BYTE_ARRAY);

  /// Creates an instance of [BundleType] with the given argument.
  const BundleType(this.value);

  /// The value of the bundle type.
  final int value;
}

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

    map.forEach((String key, Object value) {
      if (value is String) {
        bundle.addString(key, value);
      } else if (value is List<String>) {
        bundle.addStrings(key, value);
      } else if (value is List<int>) {
        bundle.addBytes(key, value);
      } else {
        Log.error(_logTag, 'No such type. type: ${value.runtimeType}');
        throw Exception('Invalid parameter');
      }
    });
    return bundle;
  }

  static const String _logTag = 'BUNDLE';
  late final _handle;
  static final List<String> _keys = [];
  bool _isDisposed = false;
  static final Finalizer<Bundle> _finalizer =
      Finalizer<Bundle>((Bundle bundle) => bundle.dispose());

  void _throwException(int ret) {
    throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString());
  }

  /// Adds a string ino the bundle object.
  void addString(String key, String value) {
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

  /// Gets a string from the bundle object with the specific key.
  String getString(String key) {
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

  /// Adds string ino the bundle object.
  void addStrings(String key, List<String> values) {
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

  /// Gets strings from the bundle object with the specific key.
  List<String> getStrings(String key) {
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

      final List<String> strings = [];
      for (int index = 0; index < arraySize.value; ++index) {
        strings.add(stringArray[index].toDartString());
      }

      return strings;
    });

    return values;
  }

  static void _bundleIteratorCallback(Pointer<Int8> pKey, int type,
      Pointer<keyval_t> pKeyval, Pointer<Void> userData) {
    _keys.add(pKey.toDartString());
  }

  /// Gets the KeyInfo items from the bundle object.
  List<String> getKeys() {
    _keys.clear();
    tizen.bundle_foreach(
        _handle, Pointer.fromFunction(_bundleIteratorCallback), nullptr);
    return _keys;
  }

  /// Deletes the item from the bundle object with the specific key.
  void delete(String key) {
    final int ret = using((Arena arena) {
      return tizen.bundle_del(_handle, key.toNativeInt8(allocator: arena));
    });
    if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
      Log.error(_logTag, 'Failed to delete key/value. key: $key, error: $ret');
      _throwException(ret);
    }
  }

  /// Checks whether the bundle object is empty or not.
  bool get isEmpty => length == 0;

  /// Checks whether the bundle object is not empty or not.
  bool get isNotEmpty => length > 0;

  /// Adds bytes type key-value pair into the bundle object.
  void addBytes(String key, List<int> bytes) {
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

  /// Gets bytes from the bundle object with the given specific key.
  List<int> getBytes(String key) {
    final List<int> values = using((Arena arena) {
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

      final List<int> byteList = [];
      for (int index = 0; index < bytesSize.value; ++index) {
        byteList.add(bytes.value[index]);
      }

      return byteList;
    });

    return values;
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

  /// Gets the number of items in the bundle object.
  int get length => tizen.bundle_get_count(_handle);

  /// Gets a type of the item of the key.
  BundleType getType(String key) {
    final int type = using((Arena arena) {
      return tizen.bundle_get_type(_handle, key.toNativeInt8(allocator: arena));
    });

    BundleType bundleType;
    if (type == BundleType.string.value) {
      bundleType = BundleType.string;
    } else if (type == BundleType.strings.value) {
      bundleType = BundleType.strings;
    } else if (type == BundleType.bytes.value) {
      bundleType = BundleType.bytes;
    } else {
      bundleType = BundleType.none;
    }

    return bundleType;
  }

  /// Checks whether the key exists or not.
  bool contains(String key) {
    return getType(key) != BundleType.none;
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
}
