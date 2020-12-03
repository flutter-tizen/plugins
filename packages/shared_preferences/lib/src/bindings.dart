// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';

import 'types.dart';

class _PreferenceBindings {
  _PreferenceBindings() {
    _lib = DynamicLibrary.open('libcapi-appfw-preference.so.0');

    setInt = _lib.lookupFunction<preference_set_int_native_t, PreferenceSetInt>(
        'preference_set_int');
    getInt = _lib.lookupFunction<preference_get_int_native_t, PreferenceGetInt>(
        'preference_get_int');

    setDouble = _lib.lookupFunction<preference_set_double_native_t,
        PreferenceSetDouble>('preference_set_double');
    getDouble = _lib.lookupFunction<preference_get_double_native_t,
        PreferenceGetDouble>('preference_get_double');

    setString = _lib.lookupFunction<preference_set_string_native_t,
        PreferenceSetString>('preference_set_string');
    getString = _lib.lookupFunction<preference_get_string_native_t,
        PreferenceGetString>('preference_get_string');

    setBoolean = _lib.lookupFunction<preference_set_boolean_native_t,
        PreferenceSetBoolean>('preference_set_boolean');
    getBoolean = _lib.lookupFunction<preference_get_boolean_native_t,
        PreferenceGetBoolean>('preference_get_boolean');

    remove = _lib.lookupFunction<preference_remove_native_t, PreferenceRemove>(
        'preference_remove');

    isExisting = _lib.lookupFunction<preference_is_existing_native_t,
        PreferenceIsExisiting>('preference_is_existing');

    removeAll = _lib.lookupFunction<preference_remove_all_native_t,
        PreferenceRemoveAll>('preference_remove_all');

    foreachItem = _lib.lookupFunction<preference_foreach_item_native_t,
        PreferenceForeachItem>('preference_foreach_item');
  }

  DynamicLibrary _lib;
  PreferenceSetInt setInt;
  PreferenceGetInt getInt;
  PreferenceSetDouble setDouble;
  PreferenceGetDouble getDouble;
  PreferenceSetString setString;
  PreferenceGetString getString;
  PreferenceSetBoolean setBoolean;
  PreferenceGetBoolean getBoolean;
  PreferenceRemove remove;
  PreferenceIsExisiting isExisting;
  PreferenceRemoveAll removeAll;
  PreferenceForeachItem foreachItem;
}

_PreferenceBindings _cachedBindings;
_PreferenceBindings get bindings => _cachedBindings ??= _PreferenceBindings();
