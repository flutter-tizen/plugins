// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

library tizen_app_control;

import 'dart:async';

import 'package:flutter/services.dart';

import 'app_manager.dart';
import 'src/ffi.dart';
import 'src/utils.dart';

/// Enumeration for the application control launch mode.
///
/// For detailed information on Tizen's launch modes, see:
/// https://docs.tizen.org/application/native/guides/app-management/app-controls/#application-group-management
enum LaunchMode {
  /// Prefer to launch the application as a main application in a new group.
  single,

  /// Prefer to launch the application as a sub application in the same group.
  group,
}

/// Enumeration for the application control result.
enum AppControlReplyResult {
  /// Reserved for platform developers.
  appStarted,

  /// The callee application launched successfully.
  succeeded,

  /// The launch failed.
  failed,

  /// The operation has been canceled.
  canceled,
}

/// Callback to be called when a reply to a launch request is delivered.
///
/// [request] represents the launch request that has been sent.
/// [reply] represents the reply message sent by the callee application.
/// [result] represents the result of launch.
typedef AppControlReplyCallback = FutureOr<void> Function(
  AppControl request,
  AppControl reply,
  AppControlReplyResult result,
);

/// Represents a control message exchanged between applications.
///
/// An explicit or implicit control request may be sent by an application to
/// launch another application. For detailed information, see:
/// https://docs.tizen.org/application/native/guides/app-management/app-controls
///
/// For a list of common operation types and examples, see:
/// https://docs.tizen.org/application/native/guides/app-management/common-appcontrols
class AppControl {
  /// Creates an instance of [AppControl] with the given parameters.
  AppControl({
    this.appId,
    this.operation,
    this.uri,
    this.mime,
    this.category,
    this.launchMode = LaunchMode.single,
    this.extraData = const <String, dynamic>{},
  }) {
    _id = nativeCreateAppControl(this);
  }

  AppControl._fromMap(dynamic map)
      : _id = map['id'] as int,
        appId = map['appId'] as String?,
        operation = map['operation'] as String?,
        uri = map['uri'] as String?,
        mime = map['mime'] as String?,
        category = map['category'] as String?,
        launchMode =
            enumFromString(LaunchMode.values, map['launchMode'] as String),
        extraData = Map<String, dynamic>.from(
            map['extraData'] as Map<dynamic, dynamic>);

  /// The ID of the application to handle this request (applicable for explicit
  /// requests).
  String? appId;

  /// The operation to be performed by the callee, such as
  /// `http://tizen.org/appcontrol/operation/view`.
  String? operation;

  /// The URI of the data to be handled by this request.
  String? uri;

  /// The MIME type of the data to be handled by this request.
  String? mime;

  /// The type of the application that should handle this request, such as
  /// `http://tizen.org/category/homeapp`.
  String? category;

  /// The launch mode, either [LaunchMode.single] or [LaunchMode.group].
  LaunchMode launchMode;

  /// Additional information contained by this application control. Each value
  /// must be either `String` or `List<String>`.
  Map<String, dynamic> extraData;

  /// The unique ID internally used for managing application control handles.
  late int _id;

  static const MethodChannel _methodChannel =
      MethodChannel('tizen/internal/app_control_method');

  static const EventChannel _eventChannel =
      EventChannel('tizen/internal/app_control_event');

  /// A stream of incoming application controls.
  static Stream<ReceivedAppControl> get onAppControl => _eventChannel
      .receiveBroadcastStream()
      .map((dynamic event) => ReceivedAppControl._fromMap(
          Map<String, dynamic>.from(event as Map<dynamic, dynamic>)));

  /// Sends a launch request to an application.
  ///
  /// The `http://tizen.org/privilege/appmanager.launch` privilege is required
  /// to use this API.
  ///
  /// If [replyCallback] is null, this call returns immediately after sending
  /// a request to the platform.
  ///
  /// If [replyCallback] is non-null, this call will not return until a reply
  /// is received from the callee and [replyCallback] is invoked.
  Future<void> sendLaunchRequest({
    AppControlReplyCallback? replyCallback,
  }) async {
    await _setAppControlData();

    final Map<String, dynamic> args = <String, dynamic>{
      'id': _id,
      'waitForReply': replyCallback != null,
    };
    if (replyCallback == null) {
      await _methodChannel.invokeMethod<void>('sendLaunchRequest', args);
    } else {
      final dynamic response =
          await _methodChannel.invokeMethod<dynamic>('sendLaunchRequest', args);
      final Map<String, dynamic> responseMap =
          Map<String, dynamic>.from(response as Map<dynamic, dynamic>);
      final AppControlReplyResult result = enumFromString(
        AppControlReplyResult.values,
        responseMap['result'] as String,
        AppControlReplyResult.failed,
      );
      final Map<String, dynamic> replyMap = Map<String, dynamic>.from(
          responseMap['reply'] as Map<dynamic, dynamic>);
      final AppControl reply = AppControl._fromMap(replyMap);
      await replyCallback(this, reply, result);
    }
  }

  /// Sends a terminate request to a running application.
  ///
  /// This API can be only used to terminate sub applications launched by the
  /// caller application as a group. To terminate background applications not
  /// launched as a group, use [AppManager.terminateBackgroundApplication]
  /// instead.
  Future<void> sendTerminateRequest() async {
    await _setAppControlData();

    final Map<String, dynamic> args = <String, dynamic>{
      'id': _id,
    };
    await _methodChannel.invokeMethod<void>('sendTerminateRequest', args);
  }

  Future<void> _setAppControlData() async {
    final Map<String, dynamic> args = <String, dynamic>{
      'id': _id,
      'appId': appId,
      'operation': operation,
      'uri': uri,
      'mime': mime,
      'category': category,
      'launchMode': enumToString(launchMode),
      'extraData': extraData,
    };
    await _methodChannel.invokeMethod<void>('setAppControlData', args);
  }
}

/// Represents a received [AppControl] message.
class ReceivedAppControl extends AppControl {
  ReceivedAppControl._fromMap(dynamic map)
      : callerAppId = map['callerAppId'] as String?,
        shouldReply = map['shouldReply'] as bool,
        super._fromMap(map);

  /// The caller application ID.
  final String? callerAppId;

  /// Whether a reply is requested.
  ///
  /// This is true when the caller application provided non-null
  /// `replyCallback` for [AppControl.sendLaunchRequest].
  final bool shouldReply;

  /// Replies to a launch request.
  ///
  /// [reply] and [result] are sent back to the caller application and set as
  /// arguments of [AppControlReplyCallback].
  Future<void> reply(AppControl reply, AppControlReplyResult result) async {
    await reply._setAppControlData();

    final Map<String, dynamic> args = <String, dynamic>{
      'id': reply._id,
      'requestId': _id,
      'result': enumToString(result),
    };
    await AppControl._methodChannel.invokeMethod<void>('reply', args);
  }
}
