// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs
// ignore_for_file: always_specify_types

import 'dart:ffi';

import 'package:ffi/ffi.dart';

typedef _InitializeDartApiNative = IntPtr Function(Pointer<Void>);
typedef _InitializeDartApi = int Function(Pointer<Void>);
typedef _CreateAppControlNative = Uint32 Function(Handle);
typedef CreateAppControl = int Function(Object);
typedef _AttachAppControlNative = Int8 Function(Int32, Handle);
typedef _AttachAppControl = int Function(int, Object);

DynamicLibrary? _libEmbedderCache;

DynamicLibrary get _libEmbedder {
  if (_libEmbedderCache == null) {
    const List<String> embedderPaths = <String>[
      'libflutter_tizen.so',
      'libflutter_tizen_common.so',
      'libflutter_tizen_mobile.so',
      'libflutter_tizen_tv.so',
      'libflutter_tizen_wearable.so',
    ];
    for (final String path in embedderPaths) {
      try {
        _libEmbedderCache = DynamicLibrary.open(path);
        break;
      } on ArgumentError {
        continue;
      }
    }
    if (_libEmbedderCache == null) {
      throw Exception('Failed to load the embedder library.');
    }
    final _InitializeDartApi initFunction = _libEmbedder.lookupFunction<
        _InitializeDartApiNative,
        _InitializeDartApi>('NativeInitializeDartApi');
    initFunction(NativeApi.initializeApiDLData);
  }
  return _libEmbedderCache!;
}

final CreateAppControl nativeCreateAppControl =
    _libEmbedder.lookupFunction<_CreateAppControlNative, CreateAppControl>(
        'NativeCreateAppControl');
final _AttachAppControl _nativeAttachAppControl =
    _libEmbedder.lookupFunction<_AttachAppControlNative, _AttachAppControl>(
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
typedef _AppManagerIsRunningNative = Int32 Function(
    Pointer<Utf8>, Pointer<Uint8>);
typedef AppManagerIsRunning = int Function(Pointer<Utf8>, Pointer<Uint8>);
typedef _AppManagerRequestTerminateBgAppNative = Int32 Function(
    AppContextHandle);
typedef AppManagerRequestTerminateBgApp = int Function(AppContextHandle);

final DynamicLibrary _libAppManager =
    DynamicLibrary.open('libcapi-appfw-app-manager.so.0');

final AppManagerGetAppContext appManagerGetAppContext = _libAppManager
    .lookupFunction<_AppManagerGetAppContextNative, AppManagerGetAppContext>(
        'app_manager_get_app_context');

final AppContextDestroy appContextDestroy =
    _libAppManager.lookupFunction<_AppContextDestroyNative, AppContextDestroy>(
        'app_context_destroy');

final AppManagerRequestTerminateBgApp appManagerRequestTerminateBgApp =
    _libAppManager.lookupFunction<_AppManagerRequestTerminateBgAppNative,
            AppManagerRequestTerminateBgApp>(
        'app_manager_request_terminate_bg_app');

final AppManagerIsRunning appManagerIsRunning = _libAppManager.lookupFunction<
    _AppManagerIsRunningNative, AppManagerIsRunning>('app_manager_is_running');

typedef _GetErrorMessageNative = Pointer<Utf8> Function(Int32);
typedef GetErrorMessage = Pointer<Utf8> Function(int);

final DynamicLibrary _libBaseCommon =
    DynamicLibrary.open('libcapi-base-common.so.0');

final GetErrorMessage getErrorMessage =
    _libBaseCommon.lookupFunction<_GetErrorMessageNative, GetErrorMessage>(
        'get_error_message');
