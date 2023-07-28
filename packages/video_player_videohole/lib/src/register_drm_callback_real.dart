// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';
import 'dart:isolate';

import 'package:flutter/foundation.dart';

import 'drm_configs.dart';

typedef _InitDartApi = int Function(Pointer<Void>);
typedef _InitDartApiNative = IntPtr Function(Pointer<Void>);

typedef _RegisterSendPort = void Function(int, int);
typedef _RegisterSendPortNative = Void Function(Int64, Int64);

class _CppRequest {
  _CppRequest.fromList(List<Object?> message)
      : replyPort = message[0]! as SendPort,
        pendingCall = message[1]! as int,
        method = message[2]! as String,
        data = message[3]! as Uint8List;

  final SendPort replyPort;
  final int pendingCall;
  final String method;
  final Uint8List data;
}

class _CppResponse {
  _CppResponse(this.pendingCall, this.data);

  final int pendingCall;
  final Uint8List data;

  List<Object?> toList() => <Object?>[pendingCall, data];
}

/// Registers the DRM callback and communication channel for handling license challenges.
///
/// This function is used to register the necessary callback and communication channel
/// to handle DRM-related operations and license challenges for the video playback.
/// It establishes communication with native code using DynamicLibrary, and handles
/// incoming requests for license challenges from the platform side.
///
/// The [drmConfigs] parameter contains configuration details for DRM operations,
/// including a [licenseCallback] function that is responsible for acquiring licenses
/// based on the provided challenges.
///
/// It is important to call this function within the [initialize] method of the video
/// player to enable DRM capabilities and communicate with the native side properly.
///
/// Example usage:
/// ```dart
/// registerDrmCallback(licenseCallback, _playerId);
/// ```
///
/// Note: This function should not be called directly outside the [initialize] method,
/// and it requires proper configuration and setup of the native C/C++ code to handle
/// DRM operations.
void registerDrmCallback(LicenseCallback licenseCallback, int playerId) {
  final DynamicLibrary process = DynamicLibrary.process();
  final _InitDartApi initDartApi =
      process.lookupFunction<_InitDartApiNative, _InitDartApi>(
          'VideoPlayerTizenPluginInitDartApi');
  initDartApi(NativeApi.initializeApiDLData);

  final ReceivePort receivePort = ReceivePort();
  receivePort.listen((dynamic message) async {
    final _CppRequest request = _CppRequest.fromList(message as List<Object?>);

    if (request.method == 'onLicenseChallenge') {
      final Uint8List challenge = request.data;
      final Uint8List result = await licenseCallback(challenge);

      final _CppResponse response = _CppResponse(request.pendingCall, result);
      request.replyPort.send(response.toList());
    }
  });

  final _RegisterSendPort registerSendPort =
      process.lookupFunction<_RegisterSendPortNative, _RegisterSendPort>(
          'VideoPlayerTizenPluginRegisterSendPort');
  registerSendPort(playerId, receivePort.sendPort.nativePort);
}
