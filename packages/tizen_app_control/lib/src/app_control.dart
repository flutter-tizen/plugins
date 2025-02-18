// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

import 'ffi.dart';

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
typedef AppControlReplyCallback =
    FutureOr<void> Function(
      AppControl request,
      AppControl reply,
      AppControlReplyResult result,
    );

/// Represents a control message exchanged between applications.
///
/// An explicit or implicit control request can be made by an application to
/// launch another application using this API. For detailed information on
/// Tizen application controls, see:
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
    if (_id == -1) {
      throw Exception('Could not create an instance of AppControl.');
    }
  }

  AppControl._fromMap(Map<String, dynamic> map)
    : _id = map['id'] as int,
      appId = map['appId'] as String?,
      operation = map['operation'] as String?,
      uri = map['uri'] as String?,
      mime = map['mime'] as String?,
      category = map['category'] as String?,
      launchMode = LaunchMode.values.byName(map['launchMode'] as String),
      extraData =
          (map['extraData'] as Map<dynamic, dynamic>).cast<String, dynamic>() {
    if (!nativeAttachAppControl(_id, this)) {
      throw Exception('Could not find an instance of AppControl with ID $_id.');
    }
  }

  /// The ID of the application to handle this request (applicable for explicit
  /// requests).
  ///
  /// Either `appId` or `operation` must be set to non-null before sending a
  /// request.
  String? appId;

  /// The operation to be performed by the callee application, such as
  /// `http://tizen.org/appcontrol/operation/view`.
  ///
  /// If null, defaults to `http://tizen.org/appcontrol/operation/default`.
  String? operation;

  /// The URI of the data to be handled by this request.
  ///
  /// If the URI points to a file (`file://`) in the caller's data directory,
  /// the callee process will be granted a read access to the file temporarily
  /// during its lifetime.
  String? uri;

  /// The MIME type of the data to be handled by this request.
  String? mime;

  /// The type of the application that should handle this request, such as
  /// `http://tizen.org/category/homeapp`.
  String? category;

  /// The launch mode, either [LaunchMode.single] or [LaunchMode.group].
  ///
  /// This value acts as a hint for the platform and cannot override the value
  /// set in the callee's manifest file.
  LaunchMode launchMode;

  /// Additional information contained by this application control. Each value
  /// must be either `String` or non-empty `List<String>`.
  Map<String, dynamic> extraData;

  /// The unique ID internally used for managing application control handles.
  late int _id;

  static const MethodChannel _methodChannel = MethodChannel(
    'tizen/internal/app_control_method',
  );

  static const EventChannel _eventChannel = EventChannel(
    'tizen/internal/app_control_event',
  );

  /// A stream of incoming application controls.
  static final Stream<ReceivedAppControl> onAppControl = _eventChannel
      .receiveBroadcastStream()
      .map(
        (dynamic event) => ReceivedAppControl._fromMap(
          (event as Map<dynamic, dynamic>).cast<String, dynamic>(),
        ),
      );

  /// Returns a list of installed applications that can handle this request.
  Future<List<String>> getMatchedAppIds() async {
    await _setAppControlData();

    final Map<String, dynamic> args = <String, dynamic>{'id': _id};
    final dynamic response = await _methodChannel.invokeMethod<dynamic>(
      'getMatchedAppIds',
      args,
    );
    return (response as List<dynamic>).cast<String>();
  }

  /// Sends a launch request to an application.
  ///
  /// The `http://tizen.org/privilege/appmanager.launch` privilege is required
  /// to use this API.
  ///
  /// If [replyCallback] is null, this call returns immediately after sending
  /// a request to the platform.
  ///
  /// If [replyCallback] is non-null, this call will not return until a reply
  /// is received from the callee and [replyCallback] is invoked. If the callee
  /// doesn't reply to the request or is terminated before replying, this call
  /// will never return and [replyCallback] will never be invoked, resulting in
  /// a memory leak.
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
      final dynamic response = await _methodChannel.invokeMethod<dynamic>(
        'sendLaunchRequest',
        args,
      );
      final Map<String, dynamic> responseMap =
          (response as Map<dynamic, dynamic>).cast<String, dynamic>();
      final AppControlReplyResult result = AppControlReplyResult.values.byName(
        responseMap['result'] as String,
      );
      final Map<String, dynamic> replyMap =
          (responseMap['reply'] as Map<dynamic, dynamic>)
              .cast<String, dynamic>();
      final AppControl reply = AppControl._fromMap(replyMap);
      await replyCallback(this, reply, result);
    }
  }

  /// Sends a terminate request to a running application.
  ///
  /// This API can be only used to terminate sub-applications launched by the
  /// caller application as a group. To terminate background applications not
  /// launched as a group, use [AppRunningContext.terminate] of the
  /// `tizen_app_manager` package instead.
  ///
  /// Applications that were launched by the callee application as a group will
  /// be terminated by this API as well.
  Future<void> sendTerminateRequest() async {
    await _setAppControlData();

    final Map<String, dynamic> args = <String, dynamic>{'id': _id};
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
      'launchMode': launchMode.name,
      'extraData': extraData,
    };
    await _methodChannel.invokeMethod<void>('setAppControlData', args);
  }

  /// Enables the auto restart setting.
  ///
  /// Calling this API makes the current app be automatically restarted upon
  /// termination. The given [appControl] is passed to the restarting app.
  ///
  /// This API is for platform developers only. The app must be signed with a
  /// platform-level certificate to use this API.
  static Future<void> setAutoRestart(AppControl appControl) async {
    await appControl._setAppControlData();

    final Map<String, dynamic> args = <String, dynamic>{'id': appControl._id};
    final int? handleAddress = await _methodChannel.invokeMethod<int>(
      'getHandle',
      args,
    );
    final Pointer<Void> handle = Pointer<Void>.fromAddress(handleAddress!);

    final int ret = appControlSetAutoRestart(handle);
    if (ret != 0) {
      final String message = getErrorMessage(ret).toDartString();
      throw PlatformException(code: ret.toString(), message: message);
    }
  }

  /// Disables the auto restart setting. See [setAutoRestart] for details.
  ///
  /// This API is for platform developers only. The app must be signed with a
  /// platform-level certificate to use this API.
  static Future<void> unsetAutoRestart() async {
    final int ret = appControlUnsetAutoRestart();
    if (ret != 0) {
      final String message = getErrorMessage(ret).toDartString();
      throw PlatformException(code: ret.toString(), message: message);
    }
  }
}

/// Represents a received [AppControl] message.
class ReceivedAppControl extends AppControl {
  ReceivedAppControl._fromMap(super.map)
    : callerAppId = map['callerAppId'] as String?,
      shouldReply = map['shouldReply'] as bool,
      super._fromMap();

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
      'id': _id,
      'replyId': reply._id,
      'result': result.name,
    };
    await AppControl._methodChannel.invokeMethod<void>('reply', args);
  }
}
