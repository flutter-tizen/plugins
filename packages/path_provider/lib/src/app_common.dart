// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';
import 'package:ffi/ffi.dart';

typedef app_common_get_data_path = Pointer<Utf8> Function();
typedef app_common_get_cache_path = Pointer<Utf8> Function();
typedef app_common_get_external_data_path = Pointer<Utf8> Function();
typedef app_common_get_external_cache_path = Pointer<Utf8> Function();

AppCommon _appCommonInstance;
AppCommon get appCommon => _appCommonInstance ??= AppCommon();

/// A wrapper class for Tizen App Common APIs.
/// Not all functions or values are supported.
class AppCommon {
  AppCommon() {
    final DynamicLibrary libAppCommon =
        DynamicLibrary.open('libcapi-appfw-app-common.so.0');
    _getDataPath = libAppCommon
        .lookup<NativeFunction<app_common_get_data_path>>('app_get_data_path')
        .asFunction();
    _getCachePath = libAppCommon
        .lookup<NativeFunction<app_common_get_cache_path>>('app_get_cache_path')
        .asFunction();
    _getExternalDataPath = libAppCommon
        .lookup<NativeFunction<app_common_get_external_data_path>>(
            'app_get_external_data_path')
        .asFunction();
    _getExternalCachePath = libAppCommon
        .lookup<NativeFunction<app_common_get_external_cache_path>>(
            'app_get_external_cache_path')
        .asFunction();
  }

  Pointer<Utf8> Function() _getDataPath;
  Pointer<Utf8> Function() _getCachePath;
  Pointer<Utf8> Function() _getExternalDataPath;
  Pointer<Utf8> Function() _getExternalCachePath;

  String getCachePath() {
    final Pointer<Utf8> path = _getCachePath();
    final String str = (path != null) ? Utf8.fromUtf8(path) : '';
    free(path);
    return str;
  }

  String getDataPath() {
    final Pointer<Utf8> path = _getDataPath();
    final String str = (path != null) ? Utf8.fromUtf8(path) : '';
    free(path);
    return str;
  }

  String getExternalDataPath() {
    final Pointer<Utf8> path = _getExternalDataPath();
    final String str = (path != null) ? Utf8.fromUtf8(path) : '';
    free(path);
    return str;
  }

  String getExternalCachePath() {
    final Pointer<Utf8> path = _getExternalCachePath();
    final String str = (path != null) ? Utf8.fromUtf8(path) : '';
    free(path);
    return str;
  }
}
