// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'package:flutter/services.dart';

import 'app_running_context.dart';

class TizenAppManager {
  TizenAppManager._();

  static const MethodChannel _channel = MethodChannel('tizen_app_manager');

  static const EventChannel _eventChannel =
      EventChannel('tizen_app_manager_events');

  static Future<String> get currentAppId async {
    final String appId = await _channel.invokeMethod('getCurrentAppId');
    return appId;
  }

  static Future<AppInfo> getAppInfo(String appId) async {
    if (appId.isEmpty) {
      throw Exception('The appId can not be empty');
    }

    try {
      var map = await _channel.invokeMapMethod<String, dynamic>(
              'getAppInfo', <String, String>{'appId': appId}) ??
          <String, dynamic>{};

      return AppInfo.fromMap(map);
    } catch (err) {
      throw Exception('getAppInfo fail!, $err');
    }
  }

  static Future<List<AppInfo>> getInstalledApps() async {
    try {
      final List<dynamic> apps =
          await _channel.invokeMethod('getInstalledApps');

      if (apps.isNotEmpty) {
        var list = <AppInfo>[];
        for (var app in apps) {
          list.add(AppInfo.fromMap(app));
        }
        return list;
      } else {
        return List<AppInfo>.empty();
      }
    } catch (err) {
      throw Exception('getInstalledApps fail!, $err');
    }
  }

  static Future<bool> isRunning(String appId) async {
    if (appId.isEmpty) {
      throw Exception('The appId can not be empty');
    }

    try {
      final bool isRun = await _channel
          .invokeMethod('isRunning', <String, String>{'appId': appId});
      return isRun;
    } catch (err) {
      throw Exception('isRunning check fail!, $err');
    }
  }

  static Stream<AppRunningContext> get onAppLaunched =>
      _launchedStreamController.stream;

  static Stream<AppRunningContext> get onAppTerminated =>
      _terminatedStreamController.stream;

  static final Stream<AppRunningContext> _applicationEvents = _eventChannel
      .receiveBroadcastStream()
      .map((dynamic map) => AppRunningContext.fromMap(map));

  static StreamSubscription<AppRunningContext>? _launchedStreamSubscription;

  static final StreamController<AppRunningContext> _launchedStreamController =
      StreamController<AppRunningContext>.broadcast(
    onListen: () {
      _launchedStreamSubscription = _applicationEvents
          .where((context) => context.isTerminated == false)
          .listen((context) => _launchedStreamController.add(context));
    },
    onCancel: () {
      _launchedStreamSubscription?.cancel();
    },
  );

  static StreamSubscription<AppRunningContext>? _terminatedStreamSubscription;

  static final StreamController<AppRunningContext> _terminatedStreamController =
      StreamController<AppRunningContext>.broadcast(
    onListen: () {
      _terminatedStreamSubscription = _applicationEvents
          .where((context) => context.isTerminated == true)
          .listen((context) => _terminatedStreamController.add(context));
    },
    onCancel: () {
      _terminatedStreamSubscription?.cancel();
    },
  );
}

class AppInfo {
  AppInfo({
    required this.appId,
    required this.packageId,
    required this.label,
    required this.applicationType,
    required this.iconPath,
    required this.executablePath,
    required this.sharedResourcePath,
    required this.isNoDisplay,
    required this.metaData,
  });

  final String appId;

  final String packageId;

  final String label;

  final String applicationType;

  final String iconPath;

  final String executablePath;

  final String sharedResourcePath;

  final bool isNoDisplay;

  final Map metaData;

  static AppInfo fromMap(dynamic map) {
    return AppInfo(
      appId: map['appId'] as String? ?? '',
      packageId: map['packageId'] as String? ?? '',
      label: map['label'] as String? ?? '',
      applicationType: map['type'] as String? ?? '',
      iconPath: map['iconPath'] as String? ?? '',
      executablePath: map['executablePath'] as String? ?? '',
      sharedResourcePath: map['sharedResourcePath'] as String? ?? '',
      isNoDisplay: map['isNoDisplay'] as bool? ?? false,
      metaData: map['metaData'] as Map,
    );
  }
}
