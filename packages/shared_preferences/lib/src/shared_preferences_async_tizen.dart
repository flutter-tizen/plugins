// Copyright 2024 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:shared_preferences_platform_interface/shared_preferences_async_platform_interface.dart';
import 'package:shared_preferences_platform_interface/types.dart';
import 'package:tizen_interop/6.0/tizen.dart';

/// The Tizen implementation of [SharedPreferencesAsyncPlatform].
///
/// This class implements the `package:shared_preferences` functionality for Tizen.
base class SharedPreferencesAsyncTizen extends SharedPreferencesAsyncPlatform {
  /// Registers this class as the default instance of [SharedPreferencesAsyncPlatform].
  static void register() {
    SharedPreferencesAsyncPlatform.instance = SharedPreferencesAsyncTizen();
  }

  static const String _separator = '␞';

  @override
  Future<void> setString(
    String key,
    String value,
    SharedPreferencesOptions options,
  ) async {
    using((Arena arena) {
      final Pointer<Char> pKey = key.toNativeChar(allocator: arena);
      _checkResult(
        tizen.preference_set_string(pKey, value.toNativeChar(allocator: arena)),
      );
    });
  }

  @override
  Future<void> setInt(
    String key,
    int value,
    SharedPreferencesOptions options,
  ) async {
    using((Arena arena) {
      final Pointer<Char> pKey = key.toNativeChar(allocator: arena);
      _checkResult(tizen.preference_set_int(pKey, value));
    });
  }

  @override
  Future<void> setStringList(
    String key,
    List<String> value,
    SharedPreferencesOptions options,
  ) async {
    using((Arena arena) {
      final Pointer<Char> pKey = key.toNativeChar(allocator: arena);
      _checkResult(
        tizen.preference_set_string(
          pKey,
          _joinStringList(value).toNativeChar(allocator: arena),
        ),
      );
    });
  }

  String _joinStringList(List<String> list) {
    return list.isEmpty
        ? _separator
        : _separator + list.join(_separator) + _separator;
  }

  @override
  Future<void> setBool(
    String key,
    bool value,
    SharedPreferencesOptions options,
  ) async {
    using((Arena arena) {
      final Pointer<Char> pKey = key.toNativeChar(allocator: arena);
      _checkResult(tizen.preference_set_boolean(pKey, value));
    });
  }

  @override
  Future<void> setDouble(
    String key,
    double value,
    SharedPreferencesOptions options,
  ) async {
    using((Arena arena) {
      final Pointer<Char> pKey = key.toNativeChar(allocator: arena);
      _checkResult(tizen.preference_set_double(pKey, value));
    });
  }

  @override
  Future<String?> getString(
    String key,
    SharedPreferencesOptions options,
  ) async {
    String? value;
    using((Arena arena) {
      final Pointer<Char> pKey = key.toNativeChar(allocator: arena);
      final Pointer<Pointer<Char>> ppString = arena();
      if (_checkResult(tizen.preference_get_string(pKey, ppString))) {
        final Pointer<Char> pString = ppString.value;
        final String stringValue = pString.toDartString();
        arena.using(pString, calloc.free);
        value = stringValue;
      }
    });
    return value;
  }

  @override
  Future<bool?> getBool(String key, SharedPreferencesOptions options) async {
    bool? value;
    using((Arena arena) {
      final Pointer<Char> pKey = key.toNativeChar(allocator: arena);
      final Pointer<Bool> pBool = arena();
      if (_checkResult(tizen.preference_get_boolean(pKey, pBool))) {
        value = pBool.value;
      }
    });
    return value;
  }

  @override
  Future<double?> getDouble(
    String key,
    SharedPreferencesOptions options,
  ) async {
    double? value;
    using((Arena arena) {
      final Pointer<Char> pKey = key.toNativeChar(allocator: arena);
      final Pointer<Double> pDouble = arena();
      if (_checkResult(tizen.preference_get_double(pKey, pDouble))) {
        value = pDouble.value;
      }
    });
    return value;
  }

  @override
  Future<int?> getInt(String key, SharedPreferencesOptions options) async {
    int? value;
    using((Arena arena) {
      final Pointer<Char> pKey = key.toNativeChar(allocator: arena);
      final Pointer<Int> pInt = arena();
      if (_checkResult(tizen.preference_get_int(pKey, pInt))) {
        value = pInt.value;
      }
    });
    return value;
  }

  @override
  Future<List<String>?> getStringList(
    String key,
    SharedPreferencesOptions options,
  ) async {
    List<String>? value;
    using((Arena arena) {
      final Pointer<Char> pKey = key.toNativeChar(allocator: arena);
      final Pointer<Pointer<Char>> ppString = arena();
      if (_checkResult(tizen.preference_get_string(pKey, ppString))) {
        final Pointer<Char> pString = ppString.value;
        final String stringValue = pString.toDartString();
        arena.using(pString, calloc.free);
        if (stringValue == _separator) {
          value = <String>[];
        } else if (stringValue.contains(_separator)) {
          final List<String> list = stringValue.split(_separator);
          value = list.getRange(1, list.length - 1).toList();
        } else {
          throw TypeError();
        }
      }
    });
    return value;
  }

  @override
  Future<void> clear(
    ClearPreferencesParameters parameters,
    SharedPreferencesOptions options,
  ) async {
    final PreferencesFilters filter = parameters.filter;

    List<String> keyList = <String>[];
    await getKeys(
      GetPreferencesParameters(filter: parameters.filter),
      options,
    ).then((Set<String> keys) {
      keyList = keys.toList();
    });

    for (final String key in keyList) {
      if (filter.allowList == null || filter.allowList!.contains(key)) {
        if (!(await _remove(key))) {
          return;
        }
      }
    }
  }

  Future<bool> _remove(String key) async {
    return using((Arena arena) {
      final bool ret =
          tizen.preference_remove(key.toNativeChar(allocator: arena)) == 0;
      return ret;
    });
  }

  static late PreferencesFilters _keysFilters;
  static late Set<String>? _keys;

  static bool _getKeysCallback(Pointer<Char> pKey, Pointer<Void> data) {
    final String key = pKey.toDartString();
    if (_keysFilters.allowList == null ||
        _keysFilters.allowList!.contains(key)) {
      _keys!.add(key);
    }
    return true;
  }

  @override
  Future<Set<String>> getKeys(
    GetPreferencesParameters parameters,
    SharedPreferencesOptions options,
  ) async {
    _keysFilters = parameters.filter;
    _keys = <String>{};

    final int ret = tizen.preference_foreach_item(
      Pointer.fromFunction(_getKeysCallback, false),
      nullptr,
    );
    if (ret != 0) {
      return <String>{};
    }
    return _keys!;
  }

  static late PreferencesFilters _preferencesFilters;
  static late Map<String, Object>? _preferences;

  static bool _getPreferenceCallback(Pointer<Char> pKey, Pointer<Void> data) {
    final String key = pKey.toDartString();
    if (_preferencesFilters.allowList == null ||
        _preferencesFilters.allowList!.contains(key)) {
      using((Arena arena) {
        final Pointer<Bool> pBool = arena();
        if (tizen.preference_get_boolean(pKey, pBool) == 0) {
          _preferences![key] = pBool.value;
          return;
        }

        final Pointer<Double> pDouble = arena();
        if (tizen.preference_get_double(pKey, pDouble) == 0) {
          _preferences![key] = pDouble.value;
          return;
        }

        final Pointer<Int> pInt = arena();
        if (tizen.preference_get_int(pKey, pInt) == 0) {
          _preferences![key] = pInt.value;
          return;
        }

        final Pointer<Pointer<Char>> ppString = arena();
        if (tizen.preference_get_string(pKey, ppString) == 0) {
          final Pointer<Char> pString = ppString.value;
          final String stringValue = pString.toDartString();
          if (stringValue == _separator) {
            _preferences![key] = <String>[];
          } else if (stringValue.contains(_separator)) {
            final List<String> list = stringValue.split(_separator);
            _preferences![key] = list.getRange(1, list.length - 1).toList();
          } else {
            _preferences![key] = stringValue;
          }
          arena.using(pString, calloc.free);
          return;
        }
      });
    }
    return true;
  }

  @override
  Future<Map<String, Object>> getPreferences(
    GetPreferencesParameters parameters,
    SharedPreferencesOptions options,
  ) async {
    _preferencesFilters = parameters.filter;
    _preferences = <String, Object>{};

    final int ret = tizen.preference_foreach_item(
      Pointer.fromFunction(_getPreferenceCallback, false),
      nullptr,
    );
    if (ret != 0) {
      return <String, Object>{};
    }
    return _preferences!;
  }

  bool _checkResult(int error) {
    if (error == preference_error_e.PREFERENCE_ERROR_NONE) {
      return true;
    } else if (error == preference_error_e.PREFERENCE_ERROR_INVALID_PARAMETER) {
      throw TypeError();
    } else if (error == preference_error_e.PREFERENCE_ERROR_OUT_OF_MEMORY) {
      throw const OutOfMemoryError();
    }
    return false;
  }
}
