// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';
import 'package:tizen_interop/6.5/tizen.dart';

/// Enumeration for RPC port types.
enum PortType {
  /// Main channel for data transfer.
  main,

  /// Sub channel for callbacks.
  callback,
}

/// Represents a connection between remote endpoints (proxy and stub).
class Port {
  /// Creates a [Port] from an existing native handle.
  Port.fromNativeHandle(this.handle);

  /// The native handle.
  final rpc_port_h handle;

  /// Temporarily grants an access to files at [paths] to the remote
  /// application.
  void shareFiles(List<String> paths) {
    using((Arena arena) {
      final Pointer<Pointer<Char>> pPaths = arena.allocate<Pointer<Char>>(
        paths.length,
      );

      for (int i = 0; i < paths.length; ++i) {
        pPaths[i] = paths[i].toNativeChar();
      }

      final int ret = tizen.rpc_port_set_private_sharing_array(
        handle,
        pPaths,
        paths.length,
      );

      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
    });
  }

  /// Temporarily grants an access to a file at [path] to the remote
  /// application.
  void shareFile(String path) {
    using((Arena arena) {
      final Pointer<Char> pPath = path.toNativeChar();
      final int ret = tizen.rpc_port_set_private_sharing(handle, pPath);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
    });
  }

  /// Revoke file access permissions.
  void unshareFile() {
    final int ret = tizen.rpc_port_unset_private_sharing(handle);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Disconnects the port.
  void disconnect() {
    final int ret = tizen.rpc_port_disconnect(handle);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }
}
