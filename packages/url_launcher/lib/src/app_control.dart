// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

import 'types.dart';

/// Defined in `app_control.h`.
const String APP_CONTROL_OPERATION_VIEW =
    'http://tizen.org/appcontrol/operation/view';

/// Dart wrapper of Tizen's `app_control`.
///
/// See: https://docs.tizen.org/application/native/api/wearable/latest/group__CAPI__APP__CONTROL__MODULE.html
class AppControl {
  AppControl() {
    final DynamicLibrary libAppControl =
        DynamicLibrary.open('libcapi-appfw-app-control.so.0');
    _appControlCreate = libAppControl
        .lookup<NativeFunction<AppControlCreateFunc>>('app_control_create')
        .asFunction();
    _appControlSetOperation = libAppControl
        .lookup<NativeFunction<AppControlSetOperationFunc>>(
            'app_control_set_operation')
        .asFunction();
    _appControlSetUri = libAppControl
        .lookup<NativeFunction<AppControlSetUriFunc>>('app_control_set_uri')
        .asFunction();
    _appControlSendLaunchRequest = libAppControl
        .lookup<NativeFunction<AppControlSendLaunchRequestFunc>>(
            'app_control_send_launch_request')
        .asFunction();
    _appControlDestroy = libAppControl
        .lookup<NativeFunction<AppControlDestroyFunc>>('app_control_destroy')
        .asFunction();

    final DynamicLibrary libBaseCommon =
        DynamicLibrary.open('libcapi-base-common.so.0');
    _getErrorMessage = libBaseCommon
        .lookup<NativeFunction<GetErrorMessageFunc>>('get_error_message')
        .asFunction();
  }

  late AppControlCreate _appControlCreate;
  late AppControlSetOperation _appControlSetOperation;
  late AppControlSetUri _appControlSetUri;
  late AppControlSendLaunchRequest _appControlSendLaunchRequest;
  late AppControlDestroy _appControlDestroy;
  late GetErrorMessage _getErrorMessage;

  AppControlHandle _handle = nullptr;

  void _throwOnError(int ret) {
    if (ret != 0) {
      final Pointer<Utf8> message = _getErrorMessage(ret);
      throw PlatformException(
        code: ret.toString(),
        message: message.toDartString(),
      );
    }
  }

  /// Corresponds to `app_control_create()`.
  void create() {
    final Pointer<AppControlHandle> pHandle = malloc();
    final int ret = _appControlCreate(pHandle);
    _handle = pHandle.value;
    malloc.free(pHandle);
    _throwOnError(ret);
  }

  /// Corresponds to `app_control_set_operation()`.
  void setOperation(String operation) {
    assert(_handle != nullptr);
    final Pointer<Utf8> operationNative = operation.toNativeUtf8();
    final int ret = _appControlSetOperation(_handle, operationNative);
    malloc.free(operationNative);
    _throwOnError(ret);
  }

  /// Corresponds to `app_control_set_uri()`.
  void setUri(String uri) {
    assert(_handle != nullptr);
    final Pointer<Utf8> uriNative = uri.toNativeUtf8();
    final int ret = _appControlSetUri(_handle, uriNative);
    malloc.free(uriNative);
    _throwOnError(ret);
  }

  /// Corresponds to `app_control_send_launch_request()`.
  void sendLaunchRequest() {
    assert(_handle != nullptr);
    _throwOnError(_appControlSendLaunchRequest(_handle, nullptr, nullptr));
  }

  /// Corresponds to `app_control_destroy()`.
  void destroy() {
    assert(_handle != nullptr);
    final int ret = _appControlDestroy(_handle);
    _handle = nullptr;
    _throwOnError(ret);
  }
}
