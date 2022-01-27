// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'package:flutter/services.dart';

import 'application_running_context.dart';

class TizenApplicationManager {
  TizenApplicationManager._();

  static const MethodChannel _channel =
      MethodChannel('tizen_application_manager');

  static const EventChannel _eventChannel =
      EventChannel('tizen_application_manager_events');

  static Future<String> get currentAppId async {
    final String appId = await _channel.invokeMethod('getCurrentAppId');
    return appId;
  }

  static Future<ApplicationInfo> getApplicationInfo(String appId) async {
    if (appId.isEmpty) {
      throw Exception('The appId can not be empty');
    }

    try {
      var map = await _channel.invokeMapMethod<String, dynamic>(
              'getAppInfo', <String, String>{'appId': appId}) ??
          <String, dynamic>{};

      return ApplicationInfo.fromMap(map);
    } catch (err) {
      throw Exception('getApplicationInfo fail!, $err');
    }
  }

  static Future<List<ApplicationInfo>> getInstalledApplications() async {
    try {
      final List<dynamic> apps =
          await _channel.invokeMethod('getInstalledApps');

      if (apps.isNotEmpty) {
        var list = <ApplicationInfo>[];
        for (var app in apps) {
          list.add(ApplicationInfo.fromMap(app));
        }
        return list;
      } else {
        return List<ApplicationInfo>.empty();
      }
    } catch (err) {
      throw Exception('getInstalledApplications fail!, $err');
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

  static Stream<ApplicationRunningContext> get onApplicationLaunched =>
      _launchedStreamController.stream;

  static Stream<ApplicationRunningContext> get onApplicationTerminated =>
      _terminatedStreamController.stream;

  static final Stream<ApplicationRunningContext> _applicationEvents =
      _eventChannel
          .receiveBroadcastStream()
          .map((dynamic map) => ApplicationRunningContext.fromMap(map));

  static StreamSubscription<ApplicationRunningContext>?
      _launchedStreamSubscription;

  static final StreamController<ApplicationRunningContext>
      _launchedStreamController =
      StreamController<ApplicationRunningContext>.broadcast(
    onListen: () {
      _launchedStreamSubscription = _applicationEvents
          .where((context) => context.isTerminated == false)
          .listen((context) => _launchedStreamController.add(context));
    },
    onCancel: () {
      _launchedStreamSubscription?.cancel();
    },
  );

  static StreamSubscription<ApplicationRunningContext>?
      _terminatedStreamSubscription;

  static final StreamController<ApplicationRunningContext>
      _terminatedStreamController =
      StreamController<ApplicationRunningContext>.broadcast(
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

class ApplicationInfo {
  ApplicationInfo({
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

  static ApplicationInfo fromMap(dynamic map) {
    return ApplicationInfo(
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
