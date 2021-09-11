// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: always_specify_types

library tizen_app_control;

import 'dart:ffi';

import 'package:ffi/ffi.dart';

import 'src/ffi.dart';
import 'src/utils.dart';

/// Provides information about installed and running applications.
class AppManager {
  const AppManager._();

  /// Returns true if an application with the given [appId] is running,
  /// otherwise false.
  static bool isRunning(String appId) {
    return using((Arena arena) {
      final Pointer<Uint8> running = arena();
      throwOnError(
          appManagerIsRunning(appId.toNativeUtf8(allocator: arena), running));
      return running.value > 0;
    });
  }

  /// Sends a terminate request to an application with the given [appId].
  /// UI applications that are in a paused state and service applications can
  /// be terminated by this API.
  ///
  /// The `http://tizen.org/privilege/appmanager.kill.bgapp` privilege is
  /// required to use this API.
  static void terminateBackgroundApplication(String appId) {
    using((Arena arena) {
      final Pointer<AppContextHandle> appContext = arena();
      throwOnError(appManagerGetAppContext(
          appId.toNativeUtf8(allocator: arena), appContext));
      throwOnError(appManagerRequestTerminateBgApp(appContext.value));
      throwOnError(appContextDestroy(appContext.value));
    });
  }
}
