// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:collection';
import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart' hide Size;
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

  /// Creates an instance of [Bundle] with the encoded bundle raw.
  Bundle.decode(String raw) {
    _handle = tizen.bundle_decode(
        raw.toNativeInt8().cast<UnsignedChar>(), raw.length);
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

  late final Pointer<bundle> _handle;
  static final List<String> _keys = <String>[];
  static final Finalizer<Bundle> _finalizer =
      Finalizer<Bundle>((Bundle bundle) => tizen.bundle_free(bundle._handle));

  static void _bundleIteratorCallback(Pointer<Char> pKey, int type,
      Pointer<keyval_t> pKeyval, Pointer<Void> userData) {
    _keys.add(pKey.toDartString());
  }

  /// The keys of this.
  @override
  Iterable<String> get keys {
    _keys.clear();
    tizen.bundle_foreach(
        _handle, Pointer.fromFunction(_bundleIteratorCallback), nullptr);
    return _keys;
  }

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
    if (this[key] != null) {
      remove(key);
    }

    if (value is String) {
      _addString(key, value);
    } else if (value is List<String>) {
      _addStrings(key, value);
    } else if (value is Uint8List) {
      _addBytes(key, value);
    } else {
      throw ArgumentError('Not supported type: ${value.runtimeType}', 'value');
    }
  }

  /// Removes all entries from the [Bundle].
  @override
  void clear() {
    keys.forEach(remove);
  }

  /// Removes [key] and its associated value, if present, from the [Bundle].
  @override
  void remove(Object? key) {
    if (key == null) {
      return;
    }

    final int ret = using((Arena arena) {
      final String keyName = key as String;
      return tizen.bundle_del(_handle, keyName.toNativeChar(allocator: arena));
    });
    if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
      _throwException(ret);
    }
  }

  /// Encodes this object to String.
  String encode() {
    final String raw = using((Arena arena) {
      final Pointer<Pointer<Char>> rawPointer = arena<Pointer<Char>>();
      final Pointer<Int> lengthPointer = arena<Int>();
      final int ret = tizen.bundle_encode(
          _handle, rawPointer.cast<Pointer<UnsignedChar>>(), lengthPointer);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        _throwException(ret);
      }

      return rawPointer.value.toDartString();
    });

    return raw;
  }

  void _throwException(int ret) {
    throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString());
  }

  void _addString(String key, String value) {
    final int ret = using((Arena arena) {
      return tizen.bundle_add_str(_handle, key.toNativeChar(allocator: arena),
          value.toNativeChar(allocator: arena));
    });
    if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
      _throwException(ret);
    }
  }

  String _getString(String key) {
    final String value = using((Arena arena) {
      final Pointer<Pointer<Char>> pValue = arena<Pointer<Char>>();
      final int ret = tizen.bundle_get_str(
          _handle, key.toNativeChar(allocator: arena), pValue);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        _throwException(ret);
      }

      return pValue.value.toDartString();
    });

    return value;
  }

  void _addStrings(String key, List<String> values) {
    using((Arena arena) {
      final List<Pointer<Char>> pointerList = values
          .map((String str) => str.toNativeChar(allocator: arena))
          .toList();
      final Pointer<Pointer<Char>> pointerArray =
          arena<Pointer<Char>>(pointerList.length);
      values.asMap().forEach((int index, String value) {
        pointerArray[index] = pointerList[index];
      });

      final int ret = tizen.bundle_add_str_array(_handle,
          key.toNativeChar(allocator: arena), pointerArray, values.length);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        _throwException(ret);
      }
    });
  }

  List<String> _getStrings(String key) {
    final List<String> values = using((Arena arena) {
      final Pointer<Int> arraySize = arena<Int>();
      final Pointer<Pointer<Char>> stringArray = tizen.bundle_get_str_array(
          _handle, key.toNativeChar(allocator: arena), arraySize);
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
          key.toNativeChar(allocator: arena),
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
      final Pointer<Size> bytesSize = arena<Size>();
      final int ret = tizen.bundle_get_byte(
          _handle,
          key.toNativeChar(allocator: arena),
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
    return using((Arena arena) {
      return tizen.bundle_get_type(_handle, key.toNativeChar(allocator: arena));
    });
  }
}
