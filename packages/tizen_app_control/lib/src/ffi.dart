// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:ffi';

import 'package:ffi/ffi.dart';

typedef _InitializeDartApiNative = IntPtr Function(Pointer<Void>);
typedef _InitializeDartApi = int Function(Pointer<Void>);
typedef _CreateAppControlNative = Uint32 Function(Handle);
typedef _CreateAppControl = int Function(Object);
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

final _CreateAppControl nativeCreateAppControl =
    _libEmbedder.lookupFunction<_CreateAppControlNative, _CreateAppControl>(
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
typedef _AppContextDestroy = int Function(AppContextHandle);

typedef _AppManagerGetAppContextNative = Int32 Function(
    Pointer<Utf8>, Pointer<AppContextHandle>);
typedef _AppManagerGetAppContext = int Function(
    Pointer<Utf8>, Pointer<AppContextHandle>);
typedef _AppManagerIsRunningNative = Int32 Function(
    Pointer<Utf8>, Pointer<Uint8>);
typedef _AppManagerIsRunning = int Function(Pointer<Utf8>, Pointer<Uint8>);
typedef _AppManagerRequestTerminateBgAppNative = Int32 Function(
    AppContextHandle);
typedef _AppManagerRequestTerminateBgApp = int Function(AppContextHandle);

final DynamicLibrary _libAppManager =
    DynamicLibrary.open('libcapi-appfw-app-manager.so.0');

final _AppManagerGetAppContext appManagerGetAppContext = _libAppManager
    .lookupFunction<_AppManagerGetAppContextNative, _AppManagerGetAppContext>(
        'app_manager_get_app_context');

final _AppContextDestroy appContextDestroy =
    _libAppManager.lookupFunction<_AppContextDestroyNative, _AppContextDestroy>(
        'app_context_destroy');

final _AppManagerRequestTerminateBgApp appManagerRequestTerminateBgApp =
    _libAppManager.lookupFunction<_AppManagerRequestTerminateBgAppNative,
            _AppManagerRequestTerminateBgApp>(
        'app_manager_request_terminate_bg_app');

final _AppManagerIsRunning appManagerIsRunning = _libAppManager.lookupFunction<
    _AppManagerIsRunningNative, _AppManagerIsRunning>('app_manager_is_running');

typedef _GetErrorMessageNative = Pointer<Utf8> Function(Int32);
typedef _GetErrorMessage = Pointer<Utf8> Function(int);

final DynamicLibrary _libBaseCommon =
    DynamicLibrary.open('libcapi-base-common.so.0');

final _GetErrorMessage getErrorMessage =
    _libBaseCommon.lookupFunction<_GetErrorMessageNative, _GetErrorMessage>(
        'get_error_message');
