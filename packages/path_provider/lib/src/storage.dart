// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:async';
import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

typedef _StorageGetDirectory = Int32 Function(
    Int32, Int32, Pointer<Pointer<Utf8>>);
typedef _StorageCallback = Int32 Function(
    Int32, Int32, Int32, Pointer<Utf8>, Pointer<Void>);
typedef _StorageForeachDeviceSupported = Int32 Function(
    Pointer<NativeFunction<_StorageCallback>>, Pointer<Void>);

/// Corresponds to `storage_directory_e`.
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

Storage? _storageInstance;
Storage get storage => _storageInstance ??= Storage();

/// Dart wrapper of Tizen's `storage`.
///
/// See: https://docs.tizen.org/application/native/api/wearable/latest/group__CAPI__SYSTEM__STORAGE__MODULE.html
class Storage {
  Storage() {
    final DynamicLibrary libStorage = DynamicLibrary.open('libstorage.so.0.1');
    _storageGetDirectory = libStorage
        .lookup<NativeFunction<_StorageGetDirectory>>('storage_get_directory')
        .asFunction();

    _storageForeachDeviceSupported = libStorage
        .lookup<NativeFunction<_StorageForeachDeviceSupported>>(
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

  late int Function(int, int, Pointer<Pointer<Utf8>>) _storageGetDirectory;
  late int Function(Pointer<NativeFunction<_StorageCallback>>, Pointer<Void>)
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

  /// Corresponds to `storage_get_directory()`.
  Future<String> getDirectory({
    required StorageDirectoryType type,
  }) async {
    final Pointer<Pointer<Utf8>> path = malloc();
    try {
      final int ret = _storageGetDirectory(await storageId, type.index, path);
      if (ret != 0) {
        throw PlatformException(
          code: '$ret',
          message: 'Failed to execute storage_get_directory.',
        );
      }
      return path.value.toDartString();
    } finally {
      malloc.free(path);
    }
  }
}
