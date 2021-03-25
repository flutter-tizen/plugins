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

    final Pointer<Int8> pBool = malloc();
    ret = bindings.getBoolean(pKey, pBool);
    final bool boolValue = pBool.value == 1;
    malloc.free(pBool);
    if (ret == 0) {
      _cachedPreferences![key] = boolValue;
      return 1;
    }

    final Pointer<Double> pDouble = malloc();
    ret = bindings.getDouble(pKey, pDouble);
    final double doubleValue = pDouble.value;
    malloc.free(pDouble);
    if (ret == 0) {
      _cachedPreferences![key] = doubleValue;
      return 1;
    }

    final Pointer<Int32> pInt = malloc();
    ret = bindings.getInt(pKey, pInt);
    final int intValue = pInt.value;
    malloc.free(pInt);
    if (ret == 0) {
      _cachedPreferences![key] = intValue;
      return 1;
    }

    final Pointer<Pointer<Utf8>> ppString = malloc();
    ret = bindings.getString(pKey, ppString);
    final Pointer<Utf8> pString = ppString.value;
    malloc.free(ppString);
    if (ret == 0) {
      // The value could be either String or StringList. For details, see
      // setValue().
      final String stringValue = pString.toDartString();
      if (stringValue == _separator) {
        _cachedPreferences![key] = <String>[];
      } else if (stringValue.contains(_separator)) {
        final List<String> list = stringValue.split(_separator);
        _cachedPreferences![key] = list.getRange(1, list.length - 1);
      } else {
        _cachedPreferences![key] = stringValue;
      }
      return 1;
    }

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
    _preferences.remove(key);

    return bindings.remove(key.toNativeUtf8()) == 0;
  }

  @override
  Future<bool> setValue(String valueType, String key, Object value) async {
    _preferences[key] = value;

    final Pointer<Utf8> pKey = key.toNativeUtf8();
    int ret;
    if (valueType == 'Bool') {
      ret = bindings.setBoolean(pKey, (value as bool) ? 1 : 0);
    } else if (valueType == 'Double') {
      ret = bindings.setDouble(pKey, value as double);
    } else if (valueType == 'Int') {
      ret = bindings.setInt(pKey, value as int);
    } else if (valueType == 'String') {
      ret = bindings.setString(pKey, (value as String).toNativeUtf8());
    } else if (valueType == 'StringList') {
      // Tizen Preference API doesn't support arrays.
      final List<String> list = value as List<String>;
      String joined;
      if (list.isEmpty) {
        joined = _separator;
      } else {
        joined = _separator + list.join(_separator) + _separator;
      }
      ret = bindings.setString(pKey, joined.toNativeUtf8());
    } else {
      print('Not implemented : valueType[' + valueType + ']');
      ret = -1;
    }
    return ret == 0;
  }
}
