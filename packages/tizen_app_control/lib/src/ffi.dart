// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:ffi';

import 'package:ffi/ffi.dart';

typedef _InitializeDartApiNative = IntPtr Function(Pointer<Void>);
typedef _InitializeDartApi = int Function(Pointer<Void>);
typedef _CreateAppControlNative = Int32 Function(Handle);
typedef CreateAppControl = int Function(Object);
typedef _AttachAppControlNative = Bool Function(Int32, Handle);
typedef AttachAppControl = bool Function(int, Object);

final DynamicLibrary _processLib = () {
  final DynamicLibrary processLib = DynamicLibrary.process();
  final _InitializeDartApi initFunction =
      processLib.lookupFunction<_InitializeDartApiNative, _InitializeDartApi>(
    'NativeInitializeDartApi',
  );
  initFunction(NativeApi.initializeApiDLData);
  return processLib;
}();

final CreateAppControl nativeCreateAppControl =
    _processLib.lookupFunction<_CreateAppControlNative, CreateAppControl>(
  'NativeCreateAppControl',
);
final AttachAppControl nativeAttachAppControl =
    _processLib.lookupFunction<_AttachAppControlNative, AttachAppControl>(
  'NativeAttachAppControl',
);

typedef _SetAutoRestartNative = Int32 Function(Pointer<Void>);
typedef SetAutoRestart = int Function(Pointer<Void>);
typedef _UnsetAutoRestartNative = Int32 Function();
typedef UnsetAutoRestart = int Function();

final SetAutoRestart appControlSetAutoRestart =
    _processLib.lookupFunction<_SetAutoRestartNative, SetAutoRestart>(
  'app_control_set_auto_restart',
);
final UnsetAutoRestart appControlUnsetAutoRestart =
    _processLib.lookupFunction<_UnsetAutoRestartNative, UnsetAutoRestart>(
  'app_control_unset_auto_restart',
);

typedef _GetErrorMessageNative = Pointer<Utf8> Function(Int);
typedef GetErrorMessage = Pointer<Utf8> Function(int);

final GetErrorMessage getErrorMessage =
    _processLib.lookupFunction<_GetErrorMessageNative, GetErrorMessage>(
  'get_error_message',
);
