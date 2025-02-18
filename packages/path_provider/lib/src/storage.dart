// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';
import 'package:tizen_interop/6.0/tizen.dart';

export 'package:tizen_interop/6.0/tizen.dart' show storage_directory_e;

/// A cached [Storage] instance.
final Storage storage = Storage();

/// A Dart wrapper of Tizen's Storage module.
///
/// See: https://docs.tizen.org/application/native/api/wearable/latest/group__CAPI__SYSTEM__STORAGE__MODULE.html
class Storage {
  /// Creates an instance of [Storage].
  Storage() {
    if (_completer.isCompleted) {
      return;
    }
    final int ret = tizen.storage_foreach_device_supported(
      Pointer.fromFunction(_deviceSupportedCallback, false),
      nullptr,
    );
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// The unique storage device ID.
  final Future<int> storageId = _completer.future;

  /// A completer for [storageId].
  static final Completer<int> _completer = Completer<int>();

  static bool _deviceSupportedCallback(
    int storageId,
    int type,
    int state,
    Pointer<Char> path,
    Pointer<Void> userData,
  ) {
    // internal storage
    if (type == 0) {
      _completer.complete(storageId);
      return false;
    }
    return true;
  }

  /// Corresponds to `storage_get_directory()`.
  Future<String> getDirectory(int type) {
    return using((Arena arena) async {
      final Pointer<Pointer<Char>> path = arena();
      final int ret = tizen.storage_get_directory(await storageId, type, path);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return path.value.toDartString();
    });
  }
}
