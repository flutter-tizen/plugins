// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';

import 'package:ffi/ffi.dart';

class AppControlStruct extends Opaque {}

typedef AppControlHandle = Pointer<AppControlStruct>;

// Native functions.
typedef AppControlCreateFunc = Int32 Function(Pointer<AppControlHandle>);
typedef AppControlSetOperationFunc = Int32 Function(
    AppControlHandle, Pointer<Utf8>);
typedef AppControlSetUriFunc = Int32 Function(AppControlHandle, Pointer<Utf8>);
typedef AppControlSendLaunchRequestFunc = Int32 Function(
    AppControlHandle, Pointer<Void>, Pointer<Void>);
typedef AppControlDestroyFunc = Int32 Function(AppControlHandle);
typedef GetErrorMessageFunc = Pointer<Utf8> Function(Int32);

// Dart functions.
typedef AppControlCreate = int Function(Pointer<AppControlHandle>);
typedef AppControlSetOperation = int Function(AppControlHandle, Pointer<Utf8>);
typedef AppControlSetUri = int Function(AppControlHandle, Pointer<Utf8>);
typedef AppControlSendLaunchRequest = int Function(
    AppControlHandle, Pointer<Void>, Pointer<Void>);
typedef AppControlDestroy = int Function(AppControlHandle);
typedef GetErrorMessage = Pointer<Utf8> Function(int);
