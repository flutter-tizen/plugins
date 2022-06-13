// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

typedef _AppManagerGetAppContext = Int32 Function(
    Pointer<Utf8>, Pointer<Pointer<_ContextHandle>>);
typedef _AppManagerIsRunning = Int32 Function(Pointer<Utf8>, Pointer<Int8>);
typedef _AppContextGetPackageId = Int32 Function(
    Pointer<_ContextHandle>, Pointer<Pointer<Utf8>>);
typedef _AppContextGetPid = Int32 Function(
    Pointer<_ContextHandle>, Pointer<Int32>);
typedef _AppContextGetAppState = Int32 Function(
    Pointer<_ContextHandle>, Pointer<Int32>);
typedef _AppManagerTerminate = Int32 Function(Pointer<_ContextHandle>);
typedef _AppManagerRequestTerminateBgApp = Int32 Function(
    Pointer<_ContextHandle>);
typedef _AppManagerResumeApp = Int32 Function(Pointer<_ContextHandle>);
typedef _AppContextDestroy = Int32 Function(Pointer<_ContextHandle>);

class _ContextHandle extends Opaque {}

typedef _GetErrorMessageNative = Pointer<Utf8> Function(Int32);
final DynamicLibrary _libBaseCommon =
    DynamicLibrary.open('libcapi-base-common.so.0');

final Pointer<Utf8> Function(int) getErrorMessage = _libBaseCommon
    .lookup<NativeFunction<_GetErrorMessageNative>>('get_error_message')
    .asFunction();

_AppManager? _appManagerInstance;
_AppManager get _appManager => _appManagerInstance ??= _AppManager();

class _AppManager {
  _AppManager() {
    final DynamicLibrary libAppMananger =
        DynamicLibrary.open('libcapi-appfw-app-manager.so.0');
    getAppContext = libAppMananger
        .lookup<NativeFunction<_AppManagerGetAppContext>>(
            'app_manager_get_app_context')
        .asFunction();
    appIsRunning = libAppMananger
        .lookup<NativeFunction<_AppManagerIsRunning>>('app_manager_is_running')
        .asFunction();
    getPackageId = libAppMananger
        .lookup<NativeFunction<_AppContextGetPackageId>>(
            'app_context_get_package_id')
        .asFunction();
    getProcessId = libAppMananger
        .lookup<NativeFunction<_AppContextGetPid>>('app_context_get_pid')
        .asFunction();
    getAppState = libAppMananger
        .lookup<NativeFunction<_AppContextGetAppState>>(
            'app_context_get_app_state')
        .asFunction();
    terminateApp = libAppMananger
        .lookup<NativeFunction<_AppManagerTerminate>>(
            'app_manager_terminate_app')
        .asFunction();
    requestTerminateBgApp = libAppMananger
        .lookup<NativeFunction<_AppManagerRequestTerminateBgApp>>(
            'app_manager_request_terminate_bg_app')
        .asFunction();
    resumeApp = libAppMananger
        .lookup<NativeFunction<_AppManagerResumeApp>>('app_manager_resume_app')
        .asFunction();
    destroyContext = libAppMananger
        .lookup<NativeFunction<_AppContextDestroy>>('app_context_destroy')
        .asFunction();
  }

  late int Function(Pointer<Utf8>, Pointer<Pointer<_ContextHandle>>)
      getAppContext;
  late int Function(Pointer<Utf8>, Pointer<Int8>) appIsRunning;
  late int Function(Pointer<_ContextHandle>, Pointer<Pointer<Utf8>>)
      getPackageId;
  late int Function(Pointer<_ContextHandle>, Pointer<Int32>) getProcessId;
  late int Function(Pointer<_ContextHandle>, Pointer<Int32>) getAppState;
  late int Function(Pointer<_ContextHandle>) terminateApp;
  late int Function(Pointer<_ContextHandle>) requestTerminateBgApp;
  late int Function(Pointer<_ContextHandle>) resumeApp;
  late int Function(Pointer<_ContextHandle>) destroyContext;
}

class AppContext {
  AppContext(String appId, int handleAddress) : _appId = appId {
    if (appId.isEmpty) {
      throw ArgumentError('Must not be empty', 'appId');
    }

    if (handleAddress == 0) {
      final Pointer<Pointer<_ContextHandle>> pHandle = malloc();
      final Pointer<Utf8> pAppId = appId.toNativeUtf8();
      try {
        final int ret = _appManager.getAppContext(pAppId, pHandle);
        if (ret != 0) {
          throw PlatformException(
            code: ret.toString(),
            message: getErrorMessage(ret).toDartString(),
          );
        }
        _handle = pHandle.value;
      } finally {
        malloc.free(pAppId);
        malloc.free(pHandle);
      }
    } else {
      // if have app_context_h
      _handle = Pointer<_ContextHandle>.fromAddress(handleAddress);
    }
  }

  late Pointer<_ContextHandle> _handle;
  final String _appId;

  void destroy() {
    if (_handle != nullptr) {
      final int ret = _appManager.destroyContext(_handle);
      _handle = nullptr;
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: getErrorMessage(ret).toDartString(),
        );
      }
    }
  }

  bool isAppRunning() {
    final Pointer<Int8> pOut = malloc();
    final Pointer<Utf8> pAppId = _appId.toNativeUtf8();
    try {
      final int ret = _appManager.appIsRunning(pAppId, pOut);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: getErrorMessage(ret).toDartString(),
        );
      }

      return pOut.value != 0;
    } finally {
      malloc.free(pAppId);
      malloc.free(pOut);
    }
  }

  String getPackageId() {
    final Pointer<Pointer<Utf8>> pPackageId = malloc();
    try {
      _checkHandle();
      final int ret = _appManager.getPackageId(_handle, pPackageId);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: getErrorMessage(ret).toDartString(),
        );
      }

      return pPackageId.value.toDartString();
    } finally {
      malloc.free(pPackageId);
    }
  }

  int getProcessId() {
    final Pointer<Int32> pProcessId = malloc();
    try {
      _checkHandle();
      final int ret = _appManager.getProcessId(_handle, pProcessId);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: getErrorMessage(ret).toDartString(),
        );
      }
      return pProcessId.value;
    } finally {
      malloc.free(pProcessId);
    }
  }

  int getAppState() {
    final Pointer<Int32> pState = malloc();
    try {
      _checkHandle();
      final int ret = _appManager.getAppState(_handle, pState);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: getErrorMessage(ret).toDartString(),
        );
      }
      return pState.value;
    } finally {
      malloc.free(pState);
    }
  }

  void terminate() {
    _checkHandle();
    final int ret = _appManager.terminateApp(_handle);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: getErrorMessage(ret).toDartString(),
      );
    }
  }

  void requestTerminateBgApp() {
    _checkHandle();
    final int ret = _appManager.requestTerminateBgApp(_handle);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: getErrorMessage(ret).toDartString(),
      );
    }
  }

  void resume() {
    _checkHandle();
    final int ret = _appManager.resumeApp(_handle);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: getErrorMessage(ret).toDartString(),
      );
    }
  }

  void _checkHandle() {
    if (_handle == nullptr) {
      throw StateError('The handle object has been destroyed');
    }
  }
}
