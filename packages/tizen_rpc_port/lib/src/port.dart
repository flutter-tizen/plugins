// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';
import 'package:tizen_interop/6.5/tizen.dart';

import 'parcel.dart';

/// Enumeration for RPC port types.
enum PortType {
  /// Main channel to communicate.
  main,

  /// Sub channel for callbacks.
  callback,
}

/// The class that proxy and stub can use to communicate with each other.
class Port {
  /// Constructor of this class.
  Port(this._handle);

  late final rpc_port_h _handle;

  /// The native handle of this port.
  rpc_port_h get handle => _handle;

  /// Sends a parcel to the connected app.
  void send(Parcel parcel) {
    final int ret = tizen.rpc_port_parcel_send(parcel.handle, _handle);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Receives a parcel from connected app.
  /// This api should be used only guaranteed receive something after send().
  // Future<Parcel> receive() async {
  //   final Uint8List raw = await _methodChannel.portReceive(this);
  //   return Parcel.fromRaw(raw);
  // }

  /// Shares private files with other applications.
  void shareFileList(List<String> paths) {
    using((Arena arena) {
      final Pointer<Pointer<Char>> pPaths =
          arena.allocate<Pointer<Char>>(paths.length);

      for (int i = 0; i < paths.length; ++i) {
        pPaths[i] = paths[i].toNativeChar();
      }

      final int ret = tizen.rpc_port_set_private_sharing_array(
          _handle, pPaths, paths.length);

      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
    });
  }

  /// Shares a private file with other applications.
  void shareFile(String path) {
    using((Arena arena) {
      final Pointer<Char> pPath = path.toNativeChar();
      final int ret = tizen.rpc_port_set_private_sharing(_handle, pPath);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
    });
  }

  /// Unsets all shared files.
  void unshareFile() {
    final int ret = tizen.rpc_port_unset_private_sharing(_handle);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Disconnects the port.
  void disconnect() {
    final int ret = tizen.rpc_port_disconnect(_handle);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }
}
