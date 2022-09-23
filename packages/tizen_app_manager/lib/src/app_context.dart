// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';
import 'package:tizen_interop/4.0/tizen.dart';

_AppManager? _appManagerInstance;
_AppManager get _appManager => _appManagerInstance ??= _AppManager();

class _AppManager {
  _AppManager() {
    final DynamicLibrary libAppMananger =
        DynamicLibrary.open('libcapi-appfw-app-manager.so.0');
    // Since app_manager_terminate_app is non public API, load it using ffi directly.
    terminateApp = libAppMananger
        .lookup<NativeFunction<Int32 Function(Pointer<app_context_s>)>>(
            'app_manager_terminate_app')
        .asFunction();
  }
  late int Function(Pointer<app_context_s>) terminateApp;
}

class AppContext {
  AppContext(String appId, int handleAddress) : _appId = appId {
    if (appId.isEmpty) {
      throw ArgumentError('Must not be empty', 'appId');
    }
    using((Arena arena) {
      if (handleAddress == 0) {
        final Pointer<Pointer<app_context_s>> handle = arena();
        final Pointer<Char> pAppId = appId.toNativeChar(allocator: arena);
        final int ret = tizen.app_manager_get_app_context(pAppId, handle);
        if (ret != 0) {
          _throwPlatformException(ret);
        }
        _handle = handle.value;
      } else {
        _handle = Pointer<app_context_s>.fromAddress(handleAddress);
      }
    });
  }

  late Pointer<app_context_s> _handle;
  final String _appId;

  void destroy() {
    if (_handle != nullptr) {
      final int ret = tizen.app_context_destroy(_handle);
      _handle = nullptr;
      if (ret != 0) {
        _throwPlatformException(ret);
      }
    }
  }

  bool isAppRunning() {
    return using((Arena arena) {
      final Pointer<Bool> isRunning = arena();
      final Pointer<Char> id = _appId.toNativeChar(allocator: arena);
      final int ret = tizen.app_manager_is_running(id, isRunning);
      if (ret != 0) {
        _throwPlatformException(ret);
      }
      return isRunning.value;
    });
  }

  String getPackageId() {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> packageId = arena();
      final int ret = tizen.app_context_get_package_id(_handle, packageId);
      if (ret != 0) {
        _throwPlatformException(ret);
      }
      return packageId.value.toDartString();
    });
  }

  int getProcessId() {
    return using((Arena arena) {
      final Pointer<Int> pid = arena();
      final int ret = tizen.app_context_get_pid(_handle, pid);
      if (ret != 0) {
        _throwPlatformException(ret);
      }
      return pid.value;
    });
  }

  int getAppState() {
    return using((Arena arena) {
      final Pointer<Int32> state = arena();
      final int ret = tizen.app_context_get_app_state(_handle, state);
      if (ret != 0) {
        _throwPlatformException(ret);
      }
      return state.value;
    });
  }

  void terminate() {
    final int ret = _appManager.terminateApp(_handle);
    if (ret != 0) {
      _throwPlatformException(ret);
    }
  }

  void requestTerminateBgApp() {
    final int ret = tizen.app_manager_request_terminate_bg_app(_handle);
    if (ret != 0) {
      _throwPlatformException(ret);
    }
  }

  void resume() {
    final int ret = tizen.app_manager_resume_app(_handle);
    if (ret != 0) {
      _throwPlatformException(ret);
    }
  }

  void _throwPlatformException(int error) {
    throw PlatformException(
      code: error.toString(),
      message: tizen.get_error_message(error).toDartString(),
    );
  }
}
