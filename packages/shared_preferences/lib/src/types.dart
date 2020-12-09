// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';
import 'package:ffi/ffi.dart';

// int preference_set_int (const char *key, int value)
typedef preference_set_int_native_t = Int32 Function(
    Pointer<Utf8> key, Int32 value);
typedef PreferenceSetInt = int Function(Pointer<Utf8> key, int value);

// int preference_get_int (const char *key, int *value)
typedef preference_get_int_native_t = Int32 Function(
    Pointer<Utf8> key, Pointer<Int32> value);
typedef PreferenceGetInt = int Function(
    Pointer<Utf8> key, Pointer<Int32> value);

// int preference_set_double (const char *key, double value)
typedef preference_set_double_native_t = Int32 Function(
    Pointer<Utf8> key, Double value);
typedef PreferenceSetDouble = int Function(Pointer<Utf8> key, double value);

// int preference_get_double (const char *key, double *value)
typedef preference_get_double_native_t = Int32 Function(
    Pointer<Utf8> key, Pointer<Double> value);
typedef PreferenceGetDouble = int Function(
    Pointer<Utf8> key, Pointer<Double> value);

// int preference_set_string (const char *key, const char *value)
typedef preference_set_string_native_t = Int32 Function(
    Pointer<Utf8> key, Pointer<Utf8> value);
typedef PreferenceSetString = int Function(
    Pointer<Utf8> key, Pointer<Utf8> value);

// int preference_get_string (const char *key, char **value)
typedef preference_get_string_native_t = Int32 Function(
    Pointer<Utf8> key, Pointer<Pointer<Utf8>> value);
typedef PreferenceGetString = int Function(
    Pointer<Utf8> key, Pointer<Pointer<Utf8>> value);

// int preference_set_boolean (const char *key, bool value)
typedef preference_set_boolean_native_t = Int32 Function(
    Pointer<Utf8> key, Int8 value);
typedef PreferenceSetBoolean = int Function(Pointer<Utf8> key, int value);

// int preference_get_boolean (const char *key, bool *value)
typedef preference_get_boolean_native_t = Int32 Function(
    Pointer<Utf8> key, Pointer<Int8> value);
typedef PreferenceGetBoolean = int Function(
    Pointer<Utf8> key, Pointer<Int8> value);

// int preference_remove (const char *key)
typedef preference_remove_native_t = Int32 Function(Pointer<Utf8> key);
typedef PreferenceRemove = int Function(Pointer<Utf8> key);

// int preference_is_existing (const char *key, bool *existing)
typedef preference_is_existing_native_t = Int32 Function(
    Pointer<Utf8> key, Pointer<Int8> existing);
typedef PreferenceIsExisiting = int Function(
    Pointer<Utf8> key, Pointer<Int8> existing);

// int preference_remove_all (void)
typedef preference_remove_all_native_t = Int32 Function();
typedef PreferenceRemoveAll = int Function();

// typedef bool(* preference_item_cb )(const char *key, void *user_data)
typedef preference_item_cb_native_t = Int8 Function(
    Pointer<Utf8> key, Pointer<Void> userData);

// int preference_foreach_item (preference_item_cb callback, void *user_data)
typedef preference_foreach_item_native_t = Int32 Function(
    Pointer<NativeFunction<preference_item_cb_native_t>> callback,
    Pointer<Void> userData);
typedef PreferenceForeachItem = int Function(
    Pointer<NativeFunction<preference_item_cb_native_t>> callback,
    Pointer<Void> userData);
