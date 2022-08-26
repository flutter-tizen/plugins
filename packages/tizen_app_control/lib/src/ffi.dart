// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:ffi';

import 'package:ffi/ffi.dart';

typedef _InitializeDartApiNative = IntPtr Function(Pointer<Void>);
typedef _InitializeDartApi = int Function(Pointer<Void>);
typedef _CreateAppControlNative = Uint32 Function(Handle);
typedef CreateAppControl = int Function(Object);
typedef _AttachAppControlNative = Int8 Function(Int32, Handle);
typedef _AttachAppControl = int Function(int, Object);

final DynamicLibrary _processLib = () {
  final DynamicLibrary processLib = DynamicLibrary.process();
  final _InitializeDartApi initFunction =
      processLib.lookupFunction<_InitializeDartApiNative, _InitializeDartApi>(
          'NativeInitializeDartApi');
  initFunction(NativeApi.initializeApiDLData);
  return processLib;
}();

final CreateAppControl nativeCreateAppControl =
    _processLib.lookupFunction<_CreateAppControlNative, CreateAppControl>(
        'NativeCreateAppControl');
final _AttachAppControl _nativeAttachAppControl =
    _processLib.lookupFunction<_AttachAppControlNative, _AttachAppControl>(
        'NativeAttachAppControl');

bool nativeAttachAppControl(int id, Object dartObject) {
  return _nativeAttachAppControl(id, dartObject) > 0;
}

class _AppContext extends Opaque {}

typedef AppContextHandle = Pointer<_AppContext>;

typedef _AppContextDestroyNative = Int32 Function(AppContextHandle);
typedef AppContextDestroy = int Function(AppContextHandle);

typedef _AppManagerGetAppContextNative = Int32 Function(
    Pointer<Utf8>, Pointer<AppContextHandle>);
typedef AppManagerGetAppContext = int Function(
    Pointer<Utf8>, Pointer<AppContextHandle>);

final DynamicLibrary _libAppManager =
    DynamicLibrary.open('libcapi-appfw-app-manager.so.0');

final AppManagerGetAppContext appManagerGetAppContext = _libAppManager
    .lookupFunction<_AppManagerGetAppContextNative, AppManagerGetAppContext>(
        'app_manager_get_app_context');

final AppContextDestroy appContextDestroy =
    _libAppManager.lookupFunction<_AppContextDestroyNative, AppContextDestroy>(
        'app_context_destroy');

typedef _GetErrorMessageNative = Pointer<Utf8> Function(Int32);
typedef GetErrorMessage = Pointer<Utf8> Function(int);

final DynamicLibrary _libBaseCommon =
    DynamicLibrary.open('libcapi-base-common.so.0');

final GetErrorMessage getErrorMessage =
    _libBaseCommon.lookupFunction<_GetErrorMessageNative, GetErrorMessage>(
        'get_error_message');
