// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:collection';
import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart' hide Size;
import 'package:tizen_interop/4.0/tizen.dart';

/// A string-based dictionary data type.
///
/// A dictionary is a collection of key-value pairs from which you can locate
/// a value using its associated key. The key is always a [String]. The value
/// must be either a [String], a list of [String]s, or a [Uint8List].
class Bundle extends MapMixin<String, Object> {
  /// Creates an empty [Bundle].
  Bundle() {
    _handle = tizen.bundle_create();
    _finalizer.attach(this, this, detach: this);
  }

  /// Creates a [Bundle] from the encoded bundle string.
  Bundle.decode(String raw) {
    _handle = tizen.bundle_decode(
        raw.toNativeInt8().cast<UnsignedChar>(), raw.length);
    _finalizer.attach(this, this, detach: this);
  }

  /// Creates a copy of the given [bundle].
  Bundle.fromBundle(Bundle bundle) {
    _handle = tizen.bundle_dup(bundle._handle);
    _finalizer.attach(this, this, detach: this);
  }

  /// Creates a [Bundle] from the given [map].
  factory Bundle.fromMap(Map<String, Object> map) {
    final Bundle bundle = Bundle();
    map.forEach((String key, Object value) => bundle[key] = value);
    return bundle;
  }

  late final Pointer<bundle> _handle;
  static final Finalizer<Bundle> _finalizer =
      Finalizer<Bundle>((Bundle bundle) => tizen.bundle_free(bundle._handle));

  static final List<String> _keys = <String>[];

  static void _bundleIteratorCallback(
    Pointer<Char> key,
    int type,
    Pointer<keyval_t> keyval,
    Pointer<Void> userData,
  ) {
    _keys.add(key.toDartString());
  }

  /// The keys of this.
  @override
  Iterable<String> get keys {
    _keys.clear();
    tizen.bundle_foreach(
        _handle, Pointer.fromFunction(_bundleIteratorCallback), nullptr);
    return _keys;
  }

  @override
  int get length => tizen.bundle_get_count(_handle);

  @override
  bool get isEmpty => length == 0;

  @override
  bool get isNotEmpty => length != 0;

  /// The value for the given [key], or null if [key] is not in the bundle.
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

  /// Associates the [key] with the given [value].
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

  /// Removes all entries from the bundle.
  @override
  void clear() {
    keys.forEach(remove);
  }

  /// Removes [key] and its associated value, if present, from the bundle.
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
    return using((Arena arena) {
      final Pointer<Pointer<Char>> raw = arena<Pointer<Char>>();
      final Pointer<Int> length = arena<Int>();
      final int ret = tizen.bundle_encode(
          _handle, raw.cast<Pointer<UnsignedChar>>(), length);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        _throwException(ret);
      }

      return raw.value.toDartString();
    });
  }

  void _throwException(int ret) {
    throw PlatformException(
      code: ret.toString(),
      message: tizen.get_error_message(ret).toDartString(),
    );
  }

  void _addString(String key, String value) {
    using((Arena arena) {
      final int ret = tizen.bundle_add_str(
        _handle,
        key.toNativeChar(allocator: arena),
        value.toNativeChar(allocator: arena),
      );
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        _throwException(ret);
      }
    });
  }

  String _getString(String key) {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> string = arena<Pointer<Char>>();
      final int ret = tizen.bundle_get_str(
          _handle, key.toNativeChar(allocator: arena), string);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        _throwException(ret);
      }

      return string.value.toDartString();
    });
  }

  void _addStrings(String key, List<String> values) {
    using((Arena arena) {
      final List<Pointer<Char>> stringList = values
          .map((String str) => str.toNativeChar(allocator: arena))
          .toList();
      final Pointer<Pointer<Char>> stringArray =
          arena<Pointer<Char>>(stringList.length);
      values.asMap().forEach((int index, String value) {
        stringArray[index] = stringList[index];
      });

      final int ret = tizen.bundle_add_str_array(_handle,
          key.toNativeChar(allocator: arena), stringArray, values.length);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        _throwException(ret);
      }
    });
  }

  List<String> _getStrings(String key) {
    return using((Arena arena) {
      final Pointer<Int> length = arena<Int>();
      final Pointer<Pointer<Char>> stringArray = tizen.bundle_get_str_array(
          _handle, key.toNativeChar(allocator: arena), length);
      final int ret = tizen.get_last_result();
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        _throwException(ret);
      }

      final List<String> strings = <String>[];
      for (int index = 0; index < length.value; ++index) {
        strings.add(stringArray[index].toDartString());
      }
      return strings;
    });
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
        bytes.length,
      );
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        _throwException(ret);
      }
    });
  }

  Uint8List _getBytes(String key) {
    return using((Arena arena) {
      final Pointer<Pointer<Uint8>> bytes = arena<Pointer<Uint8>>();
      final Pointer<Size> size = arena<Size>();
      final int ret = tizen.bundle_get_byte(
        _handle,
        key.toNativeChar(allocator: arena),
        bytes.cast<Pointer<Void>>(),
        size,
      );
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        _throwException(ret);
      }

      final Uint8List byteList = Uint8List(size.value);
      for (int index = 0; index < size.value; ++index) {
        byteList[index] = bytes.value[index];
      }
      return byteList;
    });
  }

  int _getType(String key) {
    return using((Arena arena) {
      return tizen.bundle_get_type(_handle, key.toNativeChar(allocator: arena));
    });
  }
}
