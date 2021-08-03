// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:shared_preferences_platform_interface/shared_preferences_platform_interface.dart';

import 'src/bindings.dart';

/// The Tizen implementation of [SharedPreferencesStorePlatform].
///
/// This class implements the `package:shared_preferences` functionality for Tizen.
class SharedPreferencesPlugin extends SharedPreferencesStorePlatform {
  /// Registers this class as the default instance of [SharedPreferencesStorePlatform].
  static void register() {
    SharedPreferencesStorePlatform.instance = SharedPreferencesPlugin();
  }

  static Map<String, Object>? _cachedPreferences;
  static const String _separator = '␞';

  static int _preferenceItemCallback(Pointer<Utf8> pKey, Pointer<Void> data) {
    final String key = pKey.toDartString();
    int ret;

    using((Arena arena) {
      final Pointer<Int8> pBool = arena();
      if (bindings.getBoolean(pKey, pBool) == 0) {
        _cachedPreferences![key] = pBool.value == 1;
        return;
      }

      final Pointer<Double> pDouble = arena();
      if (bindings.getDouble(pKey, pDouble) == 0) {
        _cachedPreferences![key] = pDouble.value;
        return;
      }

      final Pointer<Int32> pInt = arena();
      if (bindings.getInt(pKey, pInt) == 0) {
        _cachedPreferences![key] = pInt.value;
        return;
      }

      final Pointer<Pointer<Utf8>> ppString = arena();
      if (bindings.getString(pKey, ppString) == 0) {
        final Pointer<Utf8> pString = ppString.value;
        final String stringValue = pString.toDartString();
        if (stringValue == _separator) {
          _cachedPreferences![key] = <String>[];
        } else if (stringValue.contains(_separator)) {
          final List<String> list = stringValue.split(_separator);
          _cachedPreferences![key] = list.getRange(1, list.length - 1);
        } else {
          _cachedPreferences![key] = stringValue;
        }
        malloc.free(pString);
        return;
      }
    });

    return 1;
  }

  Map<String, Object> get _preferences {
    if (_cachedPreferences != null) {
      return _cachedPreferences!;
    }
    _cachedPreferences = <String, Object>{};

    final int ret = bindings.foreachItem(
        Pointer.fromFunction(_preferenceItemCallback, 0), nullptr);
    if (ret == 0) {
      return _cachedPreferences!;
    }
    return <String, Object>{};
  }

  @override
  Future<bool> clear() async {
    _preferences.clear();

    return bindings.removeAll() == 0;
  }

  @override
  Future<Map<String, Object>> getAll() async => _preferences;

  @override
  Future<bool> remove(String key) async {
    return using((Arena arena) {
      final bool ret = bindings.remove(key.toNativeUtf8(allocator: arena)) == 0;
      if (ret) {
        _preferences.remove(key);
      }
      return ret;
    });
  }

  @override
  Future<bool> setValue(String valueType, String key, Object value) async {
    _preferences[key] = value;

    return using((Arena arena) {
      int ret;
      final Pointer<Utf8> pKey = key.toNativeUtf8(allocator: arena);
      switch (valueType) {
        case 'Bool':
          ret = bindings.setBoolean(pKey, (value as bool) ? 1 : 0);
          break;
        case 'Double':
          ret = bindings.setDouble(pKey, value as double);
          break;
        case 'Int':
          ret = bindings.setInt(pKey, value as int);
          break;
        case 'String':
          ret = bindings.setString(
            pKey,
            (value as String).toNativeUtf8(allocator: arena),
          );
          break;
        case 'StringList':
          ret = bindings.setString(
            pKey,
            _joinStringList(value as List<String>)
                .toNativeUtf8(allocator: arena),
          );
          break;
        default:
          print('Not implemented : valueType[' + valueType + ']');
          ret = -1;
      }
      return ret == 0;
    });
  }

  String _joinStringList(List<String> list) {
    return list.isEmpty
        ? _separator
        : _separator + list.join(_separator) + _separator;
  }
}
