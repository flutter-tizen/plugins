// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:ffi';

import 'package:ffi/ffi.dart';

typedef _app_common_get_data_path = Pointer<Utf8> Function();
typedef _app_common_get_cache_path = Pointer<Utf8> Function();
typedef _app_common_get_external_data_path = Pointer<Utf8> Function();
typedef _app_common_get_external_cache_path = Pointer<Utf8> Function();

AppCommon? _appCommonInstance;
AppCommon get appCommon => _appCommonInstance ??= AppCommon();

/// Dart wrapper of Tizen's `app_common`.
///
/// See: https://docs.tizen.org/application/native/api/wearable/latest/group__CAPI__APP__COMMON__MODULE.html
class AppCommon {
  AppCommon() {
    final DynamicLibrary libAppCommon =
        DynamicLibrary.open('libcapi-appfw-app-common.so.0');
    _getDataPath = libAppCommon
        .lookup<NativeFunction<_app_common_get_data_path>>('app_get_data_path')
        .asFunction();
    _getCachePath = libAppCommon
        .lookup<NativeFunction<_app_common_get_cache_path>>(
            'app_get_cache_path')
        .asFunction();
    _getExternalDataPath = libAppCommon
        .lookup<NativeFunction<_app_common_get_external_data_path>>(
            'app_get_external_data_path')
        .asFunction();
    _getExternalCachePath = libAppCommon
        .lookup<NativeFunction<_app_common_get_external_cache_path>>(
            'app_get_external_cache_path')
        .asFunction();
  }

  late Pointer<Utf8> Function() _getDataPath;
  late Pointer<Utf8> Function() _getCachePath;
  late Pointer<Utf8> Function() _getExternalDataPath;
  late Pointer<Utf8> Function() _getExternalCachePath;

  /// Corresponds to `app_get_cache_path()`.
  String getCachePath() {
    final Pointer<Utf8> path = _getCachePath();
    final String str = path.toDartString();
    if (path != nullptr) {
      malloc.free(path);
    }
    return str;
  }

  /// Corresponds to `app_get_data_path()`.
  String getDataPath() {
    final Pointer<Utf8> path = _getDataPath();
    final String str = path.toDartString();
    if (path != nullptr) {
      malloc.free(path);
    }
    return str;
  }

  /// Corresponds to `app_get_external_data_path()`.
  String getExternalDataPath() {
    final Pointer<Utf8> path = _getExternalDataPath();
    final String str = path.toDartString();
    if (path != nullptr) {
      malloc.free(path);
    }
    return str;
  }

  /// Corresponds to `app_get_external_cache_path()`.
  String getExternalCachePath() {
    final Pointer<Utf8> path = _getExternalCachePath();
    final String str = path.toDartString();
    if (path != nullptr) {
      malloc.free(path);
    }
    return str;
  }
}
