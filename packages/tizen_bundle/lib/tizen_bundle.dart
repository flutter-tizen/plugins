// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:tizen_interop/4.0/tizen.dart';
import 'package:tizen_log/tizen_log.dart';

/// The class for the property type of the bundle.
class _BundleTypeProperty {
  /// The array type.
  static const int array = 0x0100;

  /// The primitive type.
  static const int primitive = 0x0200;

  /// The measurable type.
  static const int measurable = 0x0400;
}

/// The class for the type of the bundle.
class BundleType {
  /// The none type.
  static const int none = 0;

  /// The string type.
  static const int string = 1 | _BundleTypeProperty.measurable;

  /// The string list Type.
  static const int strings = string | _BundleTypeProperty.array;

  /// The byte type.
  static const int byte = 2;

  /// The byte list type.
  static const int bytes = byte | _BundleTypeProperty.array;
}

/// The class for information of the key of the bundle object.
class KeyInfo {
  /// The name of the key.
  final String name;

  /// The type of the key.
  final int type;

  /// Creates an instance of [KeyInfo] with the given arguments.
  KeyInfo(this.name, this.type);

  /// Check whether the type of the key is array or not.
  bool get isArray => (type & _BundleTypeProperty.array) as bool;
}

class _BundleErrorFactory {
  static final _instance = _BundleErrorFactory._internal();
  final Map<int, String> _errorMessages = {
    bundle_error_e.BUNDLE_ERROR_OUT_OF_MEMORY: 'Out of memory',
    bundle_error_e.BUNDLE_ERROR_INVALID_PARAMETER: 'Invalid parameter',
    bundle_error_e.BUNDLE_ERROR_KEY_EXISTS: 'Key already exists',
    bundle_error_e.BUNDLE_ERROR_KEY_NOT_AVAILABLE: 'Key does not exists',
  };

  factory _BundleErrorFactory() {
    return _instance;
  }

  _BundleErrorFactory._internal();

  void throwException(int error) {
    if (_errorMessages.containsKey(error)) {
      throw Exception(_errorMessages[error]);
    } else {
      throw Exception('Unknown error: $error');
    }
  }
}

/// A bundle object represents a bundle.
/// A bundle holds items (key-value pairs) and can be used with other Tizen APIs.
/// Keys can be used to access values.
/// This class is accessed by using a constructor to create a new instance of this object.
/// A bundle instance is not guaranteed to be thread safe if the instance is modified by multiple threads.
class Bundle {
  static const String _logTag = 'BUNDLE';
  final dynamic _handle;
  static final List<KeyInfo> _keyInfos = [];
  bool _isDisposed = false;

  static final Finalizer<Bundle> _finalizer =
      Finalizer<Bundle>((Bundle bundle) => bundle.dispose());

  /// Creates an instance of [Bundle].
  Bundle() : _handle = tizen.bundle_create() {
    _finalizer.attach(this, this, detach: this);
  }

  /// Creates an instance of [Bundle] with the given raw data.
  Bundle.fromRaw(String raw)
      : _handle =
            tizen.bundle_decode(raw.toNativeInt8().cast<Uint8>(), raw.length) {
    _finalizer.attach(this, this, detach: this);
  }

  /// Creates an instance of [Bundle] with the given bundle object.
  Bundle.fromBundle(Bundle bundle)
      : _handle = tizen.bundle_dup(bundle._handle) {
    _finalizer.attach(this, this, detach: this);
  }

  /// Creates an instance of [Bundle] with the given map object.
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
        throw Exception('Invalid parameter');
      }
    });

    _finalizer.attach(this, this, detach: this);
  }

  /// Adds a string ino the bundle object.
  void addString(String key, String value) {
    int ret = using((Arena arena) {
      return tizen.bundle_add_str(_handle, key.toNativeInt8(allocator: arena),
          value.toNativeInt8(allocator: arena));
    });
    if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
      Log.error(_logTag,
          "Failed to add string. key: $key, value: $value, error: $ret");
      _BundleErrorFactory().throwException(ret);
    }
  }

  /// Gets a string from the bundle object with the specific key.
  String getString(String key) {
    String value = using((Arena arena) {
      final pValue = arena<Pointer<Int8>>();
      int ret = tizen.bundle_get_str(
          _handle, key.toNativeInt8(allocator: arena), pValue);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(_logTag, "Failed to get string. key: $key, error: $ret");
        _BundleErrorFactory().throwException(ret);
      }

      return pValue.value.toDartString();
    });

    return value;
  }

  /// Adds string ino the bundle object.
  void addStrings(String key, List<String> values) {
    using((Arena arena) {
      final pointerList =
          values.map((str) => str.toNativeInt8(allocator: arena)).toList();
      final pointerArray = arena<Pointer<Int8>>(pointerList.length);
      values.asMap().forEach((index, value) {
        pointerArray[index] = pointerList[index];
      });

      int ret = tizen.bundle_add_str_array(_handle,
          key.toNativeInt8(allocator: arena), pointerArray, values.length);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(
            _logTag, "Failed to add string array. key: $key, error: $ret");
        _BundleErrorFactory().throwException(ret);
      }
    });
  }

  /// Gets strings from the bundle object with the specific key.
  List<String> getStrings(String key) {
    List<String> values = using((Arena arena) {
      final arraySize = arena<Int32>();
      final stringArray = tizen.bundle_get_str_array(
          _handle, key.toNativeInt8(allocator: arena), arraySize);
      int ret = tizen.get_last_result();
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(
            _logTag, "Failed to get string array. key: $key, error: $ret");
        _BundleErrorFactory().throwException(ret);
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
    final String key = pKey.toDartString();
    _keyInfos.add(KeyInfo(key, type));
  }

  /// Gets the KeyInfo items from the bundle object.
  List<KeyInfo> getKeys() {
    _keyInfos.clear();
    tizen.bundle_foreach(
        _handle, Pointer.fromFunction(_bundleIteratorCallback), nullptr);
    return _keyInfos;
  }

  /// Deletes the item from the bundle object with the specific key.
  void delete(String key) {
    int ret = using((Arena arena) {
      return tizen.bundle_del(_handle, key.toNativeInt8(allocator: arena));
    });
    if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
      Log.error(_logTag, "Failed to delete key/value. key: $key, error: $ret");
      _BundleErrorFactory().throwException(ret);
    }
  }

  /// Checks whether the bundle object is empty or not.
  bool get isEmpty => length == 0;

  /// Checks whether the bundle object is not empty or not.
  bool get isNotEmpty => length > 0;

  /// Adds a bytes type key-value pair into the bundle object.
  void addBytes(String key, List<int> bytes) {
    using((Arena arena) {
      final byteArray = arena<Uint8>(bytes.length);
      for (int index = 0; index < bytes.length; ++index) {
        byteArray[index] = bytes[index] & 0xff;
      }

      int ret = tizen.bundle_add_byte(
          _handle,
          key.toNativeInt8(allocator: arena),
          byteArray.cast<Void>(),
          bytes.length);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(_logTag, "Failed to add bytes. key: $key, error: $ret");
        _BundleErrorFactory().throwException(ret);
      }
    });
  }

  /// Gets bytes from the bundle object with the given specific key.
  List<int> getBytes(String key) {
    List<int> values = using((Arena arena) {
      final bytes = arena<Pointer<Uint8>>();
      final bytesSize = arena<Int32>();
      int ret = tizen.bundle_get_byte(
          _handle,
          key.toNativeInt8(allocator: arena),
          bytes.cast<Pointer<Void>>(),
          bytesSize);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(_logTag, "Failed to get bytes. key: $key, error: $ret");
        _BundleErrorFactory().throwException(ret);
      }

      List<int> byteList = [];
      for (int index = 0; index < bytesSize.value; ++index) {
        byteList.add(bytes.value[index]);
      }

      return byteList;
    });

    return values;
  }

  /// Converts this object to String type.
  String toRaw() {
    String raw = using((Arena arena) {
      final rawPointer = arena<Pointer<Int8>>();
      final lengthPointer = arena<Int32>();
      int ret = tizen.bundle_encode(
          _handle, rawPointer.cast<Pointer<Uint8>>(), lengthPointer);
      if (ret != bundle_error_e.BUNDLE_ERROR_NONE) {
        Log.error(_logTag, "Failed to encode bundle to raw data. error: $ret");
        _BundleErrorFactory().throwException(ret);
      }

      return rawPointer.value.toDartString();
    });

    return raw;
  }

  /// Gets the number of items in the bundle object.
  int get length => tizen.bundle_get_count(_handle);

  /// Gets a type of the item of the key.
  int getType(String key) {
    return using((Arena arena) {
      return tizen.bundle_get_type(_handle, key.toNativeInt8(allocator: arena));
    });
  }

  /// Checks whether the key exists or not.
  bool contains(String key) {
    return using((Arena arena) {
      return tizen.bundle_get_type(
              _handle, key.toNativeInt8(allocator: arena)) !=
          BundleType.none;
    });
  }

  /// Releases all resources associated with this object.
  void dispose() {
    if (_isDisposed) return;

    tizen.bundle_free(_handle);
    _isDisposed = true;
    _finalizer.detach(this);
  }
}
