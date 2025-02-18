// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';
import 'package:tizen_interop/6.0/tizen.dart';

typedef _TerminateAppNative = Int Function(app_context_h);
typedef _TerminateApp = int Function(app_context_h);

final DynamicLibrary _libAppMananger = DynamicLibrary.open(
  'libcapi-appfw-app-manager.so.0',
);
final _TerminateApp _terminateApp = _libAppMananger
    .lookupFunction<_TerminateAppNative, _TerminateApp>(
      'app_manager_terminate_app',
    );

class AppContext {
  AppContext(String appId, int handleAddress) : _appId = appId {
    if (appId.isEmpty) {
      throw ArgumentError('Must not be empty', 'appId');
    }
    using((Arena arena) {
      if (handleAddress == 0) {
        final Pointer<app_context_h> handle = arena();
        final Pointer<Char> id = appId.toNativeChar(allocator: arena);
        final int ret = tizen.app_manager_get_app_context(id, handle);
        if (ret != 0) {
          _throwPlatformException(ret);
        }
        _handle = handle.value;
      } else {
        _handle = app_context_h.fromAddress(handleAddress);
      }
    });
  }

  late app_context_h _handle;
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
    final int ret = _terminateApp(_handle);
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
