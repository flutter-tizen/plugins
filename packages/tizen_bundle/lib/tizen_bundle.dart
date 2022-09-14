// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:collection';
import 'dart:ffi';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';
import 'package:tizen_interop/4.0/tizen.dart';

/// Bundle is a string based Dictionary ADT.
/// A dictionary is an ordered or unordered list of key element pairs,
/// where keys are used to locate elements in the list.
class Bundle extends MapMixin<String, Object> {
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

  /// Creates a copy of [Bundle] with the given bundle object.
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

  late final _handle;
  static final List<String> _keys = <String>[];
  bool _isDisposed = false;
  static final Finalizer<Bundle> _finalizer =
      Finalizer<Bundle>((Bundle bundle) => bundle.dispose());

  /// The keys of this.
  @override
  Iterable<String> get keys => toMap().keys;

  /// The value for the given key, or null if key is not in the Bundle.
  @override
  Object? operator [](Object? key) {
    if (key == null) {
      return null;
    }

    Object? value;
    final int type = _getType(key as String);
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
  @override
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
      _throwException(bundle_error_e.BUNDLE_ERROR_INVALID_PARAMETER);
    }
  }

  /// Removes all entries from the [Bundle].
  @override
  void clear() {
    keys.forEach(remove);
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

  /// Removes [key] and its associated value, if present, from the [Bundle].
  @override
  void remove(Object? key) {
    if (key == null) {
      return;
    }

    final int ret = using((Arena arena) {
      final String keyName = key as String;
      return tizen.bundle_del(_handle, keyName.toNativeInt8(allocator: arena));
    });
    if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
      _throwException(ret);
    }
  }

  /// Converts this object to String type.
  String toRaw() {
    final String raw = using((Arena arena) {
      final Pointer<Pointer<Int8>> rawPointer = arena<Pointer<Int8>>();
      final Pointer<Int32> lengthPointer = arena<Int32>();
      final int ret = tizen.bundle_encode(
          _handle, rawPointer.cast<Pointer<Uint8>>(), lengthPointer);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
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

  void _throwException(int ret) {
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
      _throwException(ret);
    }
  }

  String _getString(String key) {
    final String value = using((Arena arena) {
      final Pointer<Pointer<Int8>> pValue = arena<Pointer<Int8>>();
      final int ret = tizen.bundle_get_str(
          _handle, key.toNativeInt8(allocator: arena), pValue);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
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
