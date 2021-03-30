// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:ffi';
import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

typedef _app_control_create = Int32 Function(Pointer<Pointer<_AppControl>>);
typedef _app_control_set_operation = Int32 Function(
    Pointer<_AppControl>, Pointer<Utf8>);
typedef _app_control_set_uri = Int32 Function(
    Pointer<_AppControl>, Pointer<Utf8>);
typedef _app_control_send_launch_request = Int32 Function(
    Pointer<_AppControl>, Pointer<Void>, Pointer<Void>);
typedef _app_control_destroy = Int32 Function(Pointer<_AppControl>);

// Corresponds to `app_control_result_e` in `app_control.h`
const int APP_CONTROL_RESULT_APP_STARTED = 1;
const int APP_CONTROL_RESULT_SUCCEEDED = 0;
const int APP_CONTROL_RESULT_FAILED = -1;
const int APP_CONTROL_RESULT_CANCELED = -2;

// Corresponds to constants defined in `app_control.h`
const String APP_CONTROL_OPERATION_VIEW =
    'http://tizen.org/appcontrol/operation/view';

// Corresponds to constants defined in `tizen_error.h`
const int TIZEN_ERROR_NONE = 0;
const int TIZEN_ERROR_INVALID_PARAMETER = -22;
const int TIZEN_ERROR_OUT_OF_MEMORY = -12;
const int TIZEN_ERROR_APPLICATION = -0x01100000;
const int TIZEN_ERROR_KEY_NOT_AVAILABLE = -126;
const int TIZEN_ERROR_KEY_REJECTED = -129;
const int TIZEN_ERROR_PERMISSION_DENIED = -13;
const int TIZEN_ERROR_TIMED_OUT = -1073741824 + 1;
const int TIZEN_ERROR_IO_ERROR = -5;

class _AppControl extends Opaque {}

/// Corresponds to `app_control_error_e` in `app_control.h`
const Map<int, String> _errorCodes = <int, String>{
  TIZEN_ERROR_NONE: 'APP_CONTROL_ERROR_NONE',
  TIZEN_ERROR_INVALID_PARAMETER: 'APP_CONTROL_ERROR_INVALID_PARAMETER',
  TIZEN_ERROR_OUT_OF_MEMORY: 'APP_CONTROL_ERROR_OUT_OF_MEMORY',
  TIZEN_ERROR_APPLICATION | 0x21: 'APP_CONTROL_ERROR_APP_NOT_FOUND',
  TIZEN_ERROR_KEY_NOT_AVAILABLE: 'APP_CONTROL_ERROR_KEY_NOT_FOUND',
  TIZEN_ERROR_KEY_REJECTED: 'APP_CONTROL_ERROR_KEY_REJECTED',
  TIZEN_ERROR_APPLICATION | 0x22: 'APP_CONTROL_ERROR_INVALID_DATA_TYPE',
  TIZEN_ERROR_APPLICATION | 0x23: 'APP_CONTROL_ERROR_LAUNCH_REJECTED',
  TIZEN_ERROR_PERMISSION_DENIED: 'APP_CONTROL_ERROR_PERMISSION_DENIED',
  TIZEN_ERROR_APPLICATION | 0x24: 'APP_CONTROL_ERROR_LAUNCH_FAILED',
  TIZEN_ERROR_TIMED_OUT: 'APP_CONTROL_ERROR_TIMED_OUT',
  TIZEN_ERROR_IO_ERROR: 'APP_CONTROL_ERROR_IO_ERROR',
};

String _getErrorCode(int returnCode) => _errorCodes.containsKey(returnCode)
    ? _errorCodes[returnCode]!
    : '$returnCode';

/// Dart wrapper of Tizen's `app_control`.
///
/// See: https://docs.tizen.org/application/native/api/wearable/latest/group__CAPI__APP__CONTROL__MODULE.html
class AppControl {
  AppControl() {
    final DynamicLibrary lib =
        DynamicLibrary.open('libcapi-appfw-app-control.so.0');
    _create = lib
        .lookup<NativeFunction<_app_control_create>>('app_control_create')
        .asFunction();
    _setOperation = lib
        .lookup<NativeFunction<_app_control_set_operation>>(
            'app_control_set_operation')
        .asFunction();
    _setUri = lib
        .lookup<NativeFunction<_app_control_set_uri>>('app_control_set_uri')
        .asFunction();
    _sendLaunchRequest = lib
        .lookup<NativeFunction<_app_control_send_launch_request>>(
            'app_control_send_launch_request')
        .asFunction();
    _destroy = lib
        .lookup<NativeFunction<_app_control_destroy>>('app_control_destroy')
        .asFunction();
  }

  late int Function(Pointer<Pointer<_AppControl>>) _create;
  late int Function(Pointer<_AppControl>, Pointer<Utf8>) _setOperation;
  late int Function(Pointer<_AppControl>, Pointer<Utf8>) _setUri;
  late int Function(Pointer<_AppControl>, Pointer<Void>, Pointer<Void>)
      _sendLaunchRequest;
  late int Function(Pointer<_AppControl>) _destroy;

  Pointer<_AppControl> _handle = nullptr;

  /// Corresponds to `app_control_create()`.
  void create() {
    final Pointer<Pointer<_AppControl>> pHandle = malloc();
    final int ret = _create(pHandle);
    _handle = pHandle.value;
    malloc.free(pHandle);

    if (ret != 0) {
      throw PlatformException(
        code: _getErrorCode(ret),
        message: 'Failed to execute app_control_create.',
      );
    }
  }

  /// Corresponds to `app_control_set_operation()`.
  void setOperation(String operation) {
    assert(_handle != nullptr);

    final int ret = _setOperation(_handle, operation.toNativeUtf8());
    if (ret != 0) {
      throw PlatformException(
        code: _getErrorCode(ret),
        message: 'Failed to execute app_control_set_operation.',
      );
    }
  }

  /// Corresponds to `app_control_set_uri()`.
  void setUri(String uri) {
    assert(_handle != nullptr);

    final int ret = _setUri(_handle, uri.toNativeUtf8());
    if (ret != 0) {
      throw PlatformException(
        code: _getErrorCode(ret),
        message: 'Failed to execute app_control_set_uri.',
      );
    }
  }

  /// Corresponds to `app_control_send_launch_request()`.
  void sendLaunchRequest() {
    assert(_handle != nullptr);

    final int ret = _sendLaunchRequest(_handle, nullptr, nullptr);
    if (ret != 0) {
      throw PlatformException(
        code: _getErrorCode(ret),
        message: 'Failed to execute app_control_send_launch_request.',
      );
    }
  }

  /// Corresponds to `app_control_destroy()`.
  void destroy() {
    if (_handle != nullptr) {
      _destroy(_handle);
      _handle = nullptr;
    }
  }
}
