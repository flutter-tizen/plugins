// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:shared_preferences_platform_interface/shared_preferences_platform_interface.dart';
import 'package:shared_preferences_platform_interface/types.dart';
import 'package:tizen_interop/4.0/tizen.dart';

/// The Tizen implementation of [SharedPreferencesStorePlatform].
class SharedPreferencesPlugin extends SharedPreferencesStorePlatform {
  /// Registers this class as the default instance of [SharedPreferencesStorePlatform].
  static void register() {
    SharedPreferencesStorePlatform.instance = SharedPreferencesPlugin();
  }

  static Map<String, Object>? _cachedPreferences;
  static const String _separator = '␞';

  static bool _preferenceItemCallback(Pointer<Char> pKey, Pointer<Void> data) {
    final String key = pKey.toDartString();

    using((Arena arena) {
      final Pointer<Bool> pBool = arena();
      if (tizen.preference_get_boolean(pKey, pBool) == 0) {
        _cachedPreferences![key] = pBool.value;
        return;
      }

      final Pointer<Double> pDouble = arena();
      if (tizen.preference_get_double(pKey, pDouble) == 0) {
        _cachedPreferences![key] = pDouble.value;
        return;
      }

      final Pointer<Int> pInt = arena();
      if (tizen.preference_get_int(pKey, pInt) == 0) {
        _cachedPreferences![key] = pInt.value;
        return;
      }

      final Pointer<Pointer<Char>> ppString = arena();
      if (tizen.preference_get_string(pKey, ppString) == 0) {
        final Pointer<Char> pString = ppString.value;
        final String stringValue = pString.toDartString();
        if (stringValue == _separator) {
          _cachedPreferences![key] = <String>[];
        } else if (stringValue.contains(_separator)) {
          final List<String> list = stringValue.split(_separator);
          _cachedPreferences![key] = list.getRange(1, list.length - 1);
        } else {
          _cachedPreferences![key] = stringValue;
        }
        arena.using(pString, calloc.free);
        return;
      }
    });

    return true;
  }

  Map<String, Object> get _preferences {
    if (_cachedPreferences != null) {
      return _cachedPreferences!;
    }
    _cachedPreferences = <String, Object>{};

    final int ret = tizen.preference_foreach_item(
        Pointer.fromFunction(_preferenceItemCallback, false), nullptr);
    if (ret == 0) {
      return _cachedPreferences!;
    }
    return <String, Object>{};
  }

  @override
  Future<bool> clear() async {
    _preferences.clear();

    return tizen.preference_remove_all() == 0;
  }

  @override
  Future<bool> clearWithParameters(ClearParameters parameters) async {
    final PreferencesFilter filter = parameters.filter;
    final List<String> keys = List<String>.of(_preferences.keys);
    bool failed = false;
    for (final String key in keys) {
      if (key.startsWith(filter.prefix) &&
          (filter.allowList == null || filter.allowList!.contains(key))) {
        failed |= !(await remove(key));
      }
    }
    return failed;
  }

  @override
  Future<Map<String, Object>> getAll() async => _preferences;

  @override
  Future<Map<String, Object>> getAllWithParameters(
      GetAllParameters parameters) async {
    final PreferencesFilter filter = parameters.filter;
    final Map<String, Object> withPrefix =
        Map<String, Object>.from(_preferences);
    withPrefix.removeWhere((String key, _) => !(key.startsWith(filter.prefix) &&
        (filter.allowList?.contains(key) ?? true)));
    return withPrefix;
  }

  @override
  Future<bool> remove(String key) async {
    return using((Arena arena) {
      final bool ret =
          tizen.preference_remove(key.toNativeChar(allocator: arena)) == 0;
      if (ret) {
        _preferences.remove(key);
      }
      return ret;
    });
  }

  @override
  Future<bool> setValue(String valueType, String key, Object value) async {
    final int ret = using((Arena arena) {
      final Pointer<Char> pKey = key.toNativeChar(allocator: arena);
      switch (valueType) {
        case 'Bool':
          return tizen.preference_set_boolean(pKey, value as bool);
        case 'Double':
          return tizen.preference_set_double(pKey, value as double);
        case 'Int':
          return tizen.preference_set_int(pKey, value as int);
        case 'String':
          return tizen.preference_set_string(
            pKey,
            (value as String).toNativeChar(allocator: arena),
          );
        case 'StringList':
          return tizen.preference_set_string(
            pKey,
            _joinStringList(value as List<String>)
                .toNativeChar(allocator: arena),
          );
        default:
          throw UnimplementedError('Not supported type: $valueType');
      }
    });
    if (ret == 0) {
      _preferences[key] = value;
      return true;
    }
    return false;
  }

  String _joinStringList(List<String> list) {
    return list.isEmpty
        ? _separator
        : _separator + list.join(_separator) + _separator;
  }
}
