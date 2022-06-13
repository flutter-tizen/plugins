// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';

import 'src/app_context.dart';

/// Enumeration for the application's current state.
enum AppState {
  /// Undefined state.
  undefined,

  /// The UI application is running in the foreground.
  foreground,

  /// The UI application is running in the background.
  background,

  /// The service application is running.
  service,

  /// The service application is terminated.
  terminated,
}

/// The application manager provides information about installed and running
/// applications. It provides functions for obtaining the current
/// application ID and [AppInfo] instance of the specific application and
/// the event of an application launched and terminated.
///
/// For detailed information on Tizen's application manager, see:
/// https://docs.tizen.org/application/dotnet/guides/app-management/app-manager/
class AppManager {
  AppManager._();

  static const MethodChannel _channel = MethodChannel('tizen/app_manager');

  static const EventChannel _launchEventChannel =
      EventChannel('tizen/app_manager/launch_event');

  static const EventChannel _terminateEventChannel =
      EventChannel('tizen/app_manager/terminate_event');

  /// The app ID of the currently running app.
  static Future<String> get currentAppId async {
    final String appId =
        await _channel.invokeMethod<String>('getCurrentAppId') ?? '';
    return appId;
  }

  /// Gets the information of app specified by the [appId].
  static Future<AppInfo> getAppInfo(String appId) async {
    if (appId.isEmpty) {
      throw ArgumentError('Must not be empty', 'appId');
    }

    final dynamic map = await _channel.invokeMapMethod<String, dynamic>(
            'getAppInfo', <String, String>{'appId': appId}) ??
        <String, dynamic>{};

    return AppInfo.fromMap(map);
  }

  /// Gets the information of all apps installed on a device.
  static Future<List<AppInfo>> getInstalledApps() async {
    final List<dynamic>? apps =
        await _channel.invokeMethod<List<dynamic>>('getInstalledApps');

    final List<AppInfo> list = <AppInfo>[];
    if (apps != null) {
      for (final dynamic app in apps) {
        list.add(AppInfo.fromMap(app));
      }
    }
    return list;
  }

  /// Gets the running state of the specific app.
  static Future<bool> isRunning(String appId) async {
    if (appId.isEmpty) {
      throw ArgumentError('Must not be empty', 'appId');
    }

    final bool isRunning = await _channel.invokeMethod<bool>(
            'isRunning', <String, String>{'appId': appId}) ??
        false;
    return isRunning;
  }

  /// A stream of events occurring when any application is launched.
  static Stream<AppRunningContext> get onAppLaunched => _launchEventChannel
      .receiveBroadcastStream()
      .map((dynamic map) => AppRunningContext.fromMap(map));

  /// A stream of events occurring when any application is terminated.
  static Stream<AppRunningContext> get onAppTerminated => _terminateEventChannel
      .receiveBroadcastStream()
      .map((dynamic map) => AppRunningContext.fromMap(map));
}

/// Represents general information on an installed application.
class AppInfo {
  /// Creates an instance of [AppInfo] with the given parameters.
  AppInfo({
    required this.appId,
    required this.packageId,
    required this.label,
    required this.appType,
    required this.iconPath,
    required this.executablePath,
    required this.sharedResourcePath,
    required this.isNoDisplay,
    required this.metadata,
  });

  /// The ID of the application.
  final String appId;

  /// The ID of the application package.
  final String packageId;

  /// The label of the application.
  final String label;

  /// The type of the application, such as `capp`, `dotnet`.
  final String appType;

  /// The path to the icon image.
  final String? iconPath;

  /// The path to the application executable directory.
  final String executablePath;

  /// The path to the shared resource directory.
  final String sharedResourcePath;

  /// Whether the app icon is displayed in the app tray.
  final bool isNoDisplay;

  /// The metadata of the application.
  final Map<dynamic, dynamic> metadata;

  /// Creates an instance of [AppInfo] with the map parameter.
  static AppInfo fromMap(dynamic map) {
    return AppInfo(
      appId: map['appId'] as String,
      packageId: map['packageId'] as String,
      label: map['label'] as String,
      appType: map['type'] as String,
      iconPath: map['iconPath'] as String?,
      executablePath: map['executablePath'] as String,
      sharedResourcePath: map['sharedResourcePath'] as String,
      isNoDisplay: map['isNoDisplay'] as bool,
      metadata: map['metadata'] as Map<dynamic, dynamic>,
    );
  }
}

/// Represents the running application's detail information.
class AppRunningContext {
  /// Creates an instance of [AppRunningContext] with application ID and handle address.
  AppRunningContext({required this.appId, int handleAddress = 0}) {
    _context = AppContext(appId, handleAddress);
  }

  late AppContext _context;

  /// The ID of the application.
  final String appId;

  /// Returns `true` if the application is not running.
  bool get isTerminated => !_context.isAppRunning();

  /// Gets the package ID of the application.
  String get packageId => _context.getPackageId();

  /// Gets the process ID of the application.
  int get processId => _context.getProcessId();

  /// Gets the current app state of the application.
  AppState get appState => AppState.values[_context.getAppState()];

  /// Creates an instance of [AppRunningContext] with map.
  static AppRunningContext fromMap(dynamic map) {
    final String appId = map['appId'] as String;
    final int handle = map['handle'] as int;
    return AppRunningContext(appId: appId, handleAddress: handle);
  }

  /// Releases all resources associated with this object.
  ///
  /// Note that [dispose] must be called on an instance of [AppRunningContext]
  /// after use.
  void dispose() {
    _context.destroy();
  }

  /// Sends a resume request to the application if it is not running.
  ///
  /// The `http://tizen.org/privilege/appmanager.launch` privilege is required
  /// to use this API.
  void resume() {
    if (isTerminated) {
      _context.resume();
    }
  }

  /// Sends a terminate request to the application.
  ///
  /// Set [background] to true if the application is running in background or
  /// is a service application.
  ///
  /// The `http://tizen.org/privilege/appmanager.kill.bgapp` privilege is
  /// required if [background] is true. Otherwise, the
  /// `http://tizen.org/privilege/appmanager.kill` platform privilege is
  /// required.
  void terminate({bool background = false}) {
    if (!isTerminated) {
      if (background) {
        _context.requestTerminateBgApp();
      } else {
        _context.terminate();
      }
    }
  }
}
