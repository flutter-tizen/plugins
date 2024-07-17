// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:tizen_interop/6.0/tizen.dart';

/// A cached [AppCommon] instance.
final AppCommon appCommon = AppCommon();

/// A Dart wrapper of Tizen's App Common module.
///
/// See: https://docs.tizen.org/application/native/api/wearable/latest/group__CAPI__APP__COMMON__MODULE.html
class AppCommon {
  /// Corresponds to `app_get_data_path()`.
  String getDataPath() {
    return using((Arena arena) {
      final Pointer<Char> path = tizen.app_get_data_path();
      arena.using(path, calloc.free);
      return path.toDartString();
    });
  }

  /// Corresponds to `app_get_cache_path()`.
  String getCachePath() {
    return using((Arena arena) {
      final Pointer<Char> path = tizen.app_get_cache_path();
      arena.using(path, calloc.free);
      return path.toDartString();
    });
  }

  /// Corresponds to `app_get_external_data_path()`.
  String getExternalDataPath() {
    return using((Arena arena) {
      final Pointer<Char> path = tizen.app_get_external_data_path();
      arena.using(path, calloc.free);
      return path.toDartString();
    });
  }

  /// Corresponds to `app_get_external_cache_path()`.
  String getExternalCachePath() {
    return using((Arena arena) {
      final Pointer<Char> path = tizen.app_get_external_cache_path();
      arena.using(path, calloc.free);
      return path.toDartString();
    });
  }
}
