// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';
import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

// Native function signatures
typedef app_control_create = Int32 Function(Pointer<Pointer<_AppControl>>);
typedef app_control_set_operation = Int32 Function(
    Pointer<_AppControl>, Pointer<Utf8>);
typedef app_control_set_uri = Int32 Function(
    Pointer<_AppControl>, Pointer<Utf8>);
typedef app_control_send_launch_request = Int32 Function(
    Pointer<_AppControl>, Pointer<Void>, Pointer<Void>);
typedef app_control_destroy = Int32 Function(Pointer<_AppControl>);

// Constants from [app_control_result_e] in `app_control.h`
const int APP_CONTROL_RESULT_APP_STARTED = 1;
const int APP_CONTROL_RESULT_SUCCEEDED = 0;
const int APP_CONTROL_RESULT_FAILED = -1;
const int APP_CONTROL_RESULT_CANCELED = -2;

// Constant from `app_control.h`
const String APP_CONTROL_OPERATION_VIEW =
    'http://tizen.org/appcontrol/operation/view';

// Constants from `tizen_error.h`
const int TIZEN_ERROR_NONE = 0;
const int TIZEN_ERROR_INVALID_PARAMETER = -22;
const int TIZEN_ERROR_OUT_OF_MEMORY = -12;
const int TIZEN_ERROR_APPLICATION = -0x01100000;
const int TIZEN_ERROR_KEY_NOT_AVAILABLE = -126;
const int TIZEN_ERROR_KEY_REJECTED = -129;
const int TIZEN_ERROR_PERMISSION_DENIED = -13;
const int TIZEN_ERROR_TIMED_OUT = -1073741824 + 1;
const int TIZEN_ERROR_IO_ERROR = -5;

class _AppControl extends Struct {}

// Constants from [app_control_error_e] in `app_control.h`
const Map<int, String> errorCodes = <int, String>{
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

String getErrorCode(int returnCode) =>
    errorCodes.containsKey(returnCode) ? errorCodes[returnCode] : '$returnCode';

/// A wrapper class of the native App Control API.
/// Not all functions or values are supported.
class AppControl {
  AppControl() {
    final DynamicLibrary lib =
        DynamicLibrary.open('libcapi-appfw-app-control.so.0');
    _create = lib
        .lookup<NativeFunction<app_control_create>>('app_control_create')
        .asFunction();
    _setOperation = lib
        .lookup<NativeFunction<app_control_set_operation>>(
            'app_control_set_operation')
        .asFunction();
    _setUri = lib
        .lookup<NativeFunction<app_control_set_uri>>('app_control_set_uri')
        .asFunction();
    _sendLaunchRequest = lib
        .lookup<NativeFunction<app_control_send_launch_request>>(
            'app_control_send_launch_request')
        .asFunction();
    _destroy = lib
        .lookup<NativeFunction<app_control_destroy>>('app_control_destroy')
        .asFunction();
  }

  // Bindings
  int Function(Pointer<Pointer<_AppControl>>) _create;
  int Function(Pointer<_AppControl>, Pointer<Utf8>) _setOperation;
  int Function(Pointer<_AppControl>, Pointer<Utf8>) _setUri;
  int Function(Pointer<_AppControl>, Pointer<Void>, Pointer<Void>)
      _sendLaunchRequest;
  int Function(Pointer<_AppControl>) _destroy;

  Pointer<_AppControl> _handle;

  bool get isValid => _handle != null;

  void create() {
    final Pointer<Pointer<_AppControl>> pHandle = allocate();
    final int ret = _create(pHandle);
    _handle = pHandle.value;
    free(pHandle);

    if (ret != 0) {
      throw PlatformException(
        code: getErrorCode(ret),
        message: 'Failed to execute app_control_create.',
      );
    }
  }

  void setOperation(String operation) {
    assert(isValid);

    final int ret = _setOperation(_handle, Utf8.toUtf8(operation));
    if (ret != 0) {
      throw PlatformException(
        code: getErrorCode(ret),
        message: 'Failed to execute app_control_set_operation.',
      );
    }
  }

  void setUri(String uri) {
    assert(isValid);

    final int ret = _setUri(_handle, Utf8.toUtf8(uri));
    if (ret != 0) {
      throw PlatformException(
        code: getErrorCode(ret),
        message: 'Failed to execute app_control_set_uri.',
      );
    }
  }

  void sendLaunchRequest() {
    assert(isValid);

    final int ret = _sendLaunchRequest(_handle, nullptr, nullptr);
    if (ret != 0) {
      throw PlatformException(
        code: getErrorCode(ret),
        message: 'Failed to execute app_control_send_launch_request.',
      );
    }
  }

  /// This method must be called after use to release the underlying handle.
  /// `dart:ffi` doesn't support finalizers (the disposal pattern) as of now.
  void destroy() {
    if (isValid) {
      _destroy(_handle);
      _handle = null;
    }
  }
}
