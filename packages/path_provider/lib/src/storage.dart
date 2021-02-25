// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:ffi';
import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

// Native function signatures
typedef storage_get_directory = Int32 Function(
    Int32, Int32, Pointer<Pointer<Utf8>>);
typedef _storage_callback = Int32 Function(
    Int32, Int32, Int32, Pointer<Utf8>, Pointer<Void>);
typedef storage_foreach_device_supported = Int32 Function(
    Pointer<NativeFunction<_storage_callback>>, Pointer<Void>);

enum StorageDirectoryType {
  images,
  sounds,
  videos,
  camera,
  downloads,
  music,
  documents,
  others,
  system_ringtones,
}

Storage _storageInstance;
Storage get storage => _storageInstance ??= Storage();

/// A wrapper class for Tizen Storage APIs.
/// Not all functions or values are supported.
class Storage {
  Storage() {
    final DynamicLibrary libStorage = DynamicLibrary.open('libstorage.so.0.1');
    _storageGetDirectory = libStorage
        .lookup<NativeFunction<storage_get_directory>>('storage_get_directory')
        .asFunction();

    _storageForeachDeviceSupported = libStorage
        .lookup<NativeFunction<storage_foreach_device_supported>>(
            'storage_foreach_device_supported')
        .asFunction();

    if (_completer.isCompleted) {
      return;
    }

    final int ret = _storageForeachDeviceSupported(
        Pointer.fromFunction(_deviceSupportedCallback, 0), nullptr);
    if (ret != 0) {
      throw PlatformException(
        code: '$ret',
        message: 'Failed to execute storage_foreach_device_supported.',
      );
    }
  }

  // Bindings
  int Function(int, int, Pointer<Pointer<Utf8>>) _storageGetDirectory;
  int Function(Pointer<NativeFunction<_storage_callback>>, Pointer<Void>)
      _storageForeachDeviceSupported;

  /// The unique storage device id.
  final Future<int> storageId = _completer.future;

  /// A completer for [storageId].
  static final Completer<int> _completer = Completer<int>();

  static int _deviceSupportedCallback(
    int storageId,
    int type,
    int state,
    Pointer<Utf8> path,
    Pointer<Void> userData,
  ) {
    // internal storage
    if (type == 0) {
      _completer.complete(storageId);
      return 0;
    }
    return 1;
  }

  Future<String> getDirectory({
    StorageDirectoryType type,
  }) async {
    final Pointer<Pointer<Utf8>> path = allocate();
    try {
      final int ret = _storageGetDirectory(await storageId, type.index, path);
      if (ret != 0) {
        throw PlatformException(
          code: '$ret',
          message: 'Failed to execute storage_get_directory.',
        );
      }
      return Utf8.fromUtf8(path.value);
    } finally {
      free(path);
    }
  }
}
