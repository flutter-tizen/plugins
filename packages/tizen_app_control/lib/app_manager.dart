// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:tizen_app_manager/tizen_app_manager.dart';

/// Provides information about installed and running applications.
class AppManager {
  const AppManager._();

  /// Returns true if an application with the given [appId] is running,
  /// otherwise false.
  @Deprecated("Use tizen_app_manager's `AppManager.isRunning` instead")
  static bool isRunning(String appId) {
    final AppRunningContext context = AppRunningContext(appId: appId);
    try {
      return !context.isTerminated;
    } finally {
      context.dispose();
    }
  }

  /// Sends a terminate request to an application with the given [appId].
  /// UI applications that are in a paused state and service applications can
  /// be terminated by this API.
  ///
  /// The `http://tizen.org/privilege/appmanager.kill.bgapp` privilege is
  /// required to use this API.
  @Deprecated("Use tizen_app_manager's `AppRunningContext.terminate` instead")
  static void terminateBackgroundApplication(String appId) {
    final AppRunningContext context = AppRunningContext(appId: appId);
    try {
      context.terminate(background: true);
    } finally {
      context.dispose();
    }
  }
}
