// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'src/app_manager.dart';

// Enumeration for application current state
enum AppState {
  // Undefined state
  undefined,

  // The UI Application is running in the foreground.
  foreground,

  // The UI Application is running in the background.
  background,

  // The service Application is running.
  service,

  // The service Application is terminated.
  terminated
}

class ApplicationRunningContext {
  late AppContext _context;

  final String applicationId;

  bool get isTerminated => !_context.isAppRunning();

  String get packageId => _context.getPackageId();

  int get processId => _context.getProcessId();

  AppState get appState => _stateConvert(_context.getAppState());

  ApplicationRunningContext(
      {required this.applicationId, int handleAddress = 0}) {
    _context = AppContext(applicationId, handleAddress);
  }

  static ApplicationRunningContext fromMap(dynamic map) {
    final appId = map['appId'] as String? ?? '';
    final handle = map['handle'] as int;
    return ApplicationRunningContext(
        applicationId: appId, handleAddress: handle);
  }

  void dispose() {
    _context.destroy();
  }

  void resume() {
    if (isTerminated) {
      _context.resume();
    }
  }

  void terminate() {
    if (!isTerminated) {
      _context.terminate();
    }
  }

  AppState _stateConvert(int index) => AppState.values[index];
}
