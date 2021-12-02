// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:ffi';

import 'types.dart';

class _PreferenceBindings {
  _PreferenceBindings() {
    final DynamicLibrary lib =
        DynamicLibrary.open('libcapi-appfw-preference.so.0');

    setInt = lib.lookupFunction<PreferenceSetIntNative, PreferenceSetInt>(
        'preference_set_int');
    getInt = lib.lookupFunction<PreferenceGetIntNative, PreferenceGetInt>(
        'preference_get_int');
    setDouble =
        lib.lookupFunction<PreferenceSetDoubleNative, PreferenceSetDouble>(
            'preference_set_double');
    getDouble =
        lib.lookupFunction<PreferenceGetDoubleNative, PreferenceGetDouble>(
            'preference_get_double');
    setString =
        lib.lookupFunction<PreferenceSetStringNative, PreferenceSetString>(
            'preference_set_string');
    getString =
        lib.lookupFunction<PreferenceGetStringNative, PreferenceGetString>(
            'preference_get_string');
    setBoolean =
        lib.lookupFunction<PreferenceSetBooleanNative, PreferenceSetBoolean>(
            'preference_set_boolean');
    getBoolean =
        lib.lookupFunction<PreferenceGetBooleanNative, PreferenceGetBoolean>(
            'preference_get_boolean');
    remove = lib.lookupFunction<PreferenceRemoveNative, PreferenceRemove>(
        'preference_remove');
    removeAll =
        lib.lookupFunction<PreferenceRemoveAllNative, PreferenceRemoveAll>(
            'preference_remove_all');
    foreachItem =
        lib.lookupFunction<PreferenceForeachItemNative, PreferenceForeachItem>(
            'preference_foreach_item');
  }

  late PreferenceSetInt setInt;
  late PreferenceGetInt getInt;
  late PreferenceSetDouble setDouble;
  late PreferenceGetDouble getDouble;
  late PreferenceSetString setString;
  late PreferenceGetString getString;
  late PreferenceSetBoolean setBoolean;
  late PreferenceGetBoolean getBoolean;
  late PreferenceRemove remove;
  late PreferenceRemoveAll removeAll;
  late PreferenceForeachItem foreachItem;
}

_PreferenceBindings? _cachedBindings;
_PreferenceBindings get bindings => _cachedBindings ??= _PreferenceBindings();
