// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

import 'package:tizen_interop/6.5/tizen.dart';

export 'dart:typed_data';

/// The security info class.
class SecurityInfo {
  /// The constructor for this class.
  SecurityInfo() {
    handle = using((Arena arena) {
      final Pointer<cion_security_h> pHandle = arena();
      final int ret = tizen.cion_security_create(pHandle);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return pHandle.value;
    });
  }

  /// The CA cert path.
  ///
  /// http://tizen.org/privilege/mediastorage is needed if the file path is relevant to media storage.
  /// http://tizen.org/privilege/externalstorage is needed if the file path is relevant to external storage.
  String get caPath {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> pPath = arena();
      final int ret = tizen.cion_security_get_ca_path(handle, pPath);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      arena.using(pPath.value, malloc.free);
      return pPath.value.toDartString();
    });
  }

  set caPath(String value) {
    using((Arena arena) {
      final Pointer<Char> pPath = value.toNativeChar(allocator: arena);
      final int ret = tizen.cion_security_set_ca_path(handle, pPath);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
    });
  }

  /// The cert path.
  ///
  /// http://tizen.org/privilege/mediastorage is needed if the file path is relevant to media storage.
  /// http://tizen.org/privilege/externalstorage is needed if the file path is relevant to external storage.
  String get certPath {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> pPath = arena();
      final int ret = tizen.cion_security_get_cert_path(handle, pPath);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      arena.using(pPath.value, malloc.free);
      return pPath.value.toDartString();
    });
  }

  set certPath(String value) {
    using((Arena arena) {
      final Pointer<Char> pPath = value.toNativeChar(allocator: arena);
      final int ret = tizen.cion_security_set_cert_path(handle, pPath);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
    });
  }

  /// The private key path.
  ///
  /// http://tizen.org/privilege/mediastorage is needed if the file path is relevant to media storage.
  /// http://tizen.org/privilege/externalstorage is needed if the file path is relevant to external storage.
  String get privateKeyPath {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> pPath = arena();
      final int ret = tizen.cion_security_get_private_key_path(handle, pPath);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      arena.using(pPath.value, malloc.free);
      return pPath.value.toDartString();
    });
  }

  set privateKeyPath(String value) {
    using((Arena arena) {
      final Pointer<Char> pPath = value.toNativeChar(allocator: arena);
      final int ret = tizen.cion_security_set_private_key_path(handle, pPath);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
    });
  }

  // ignore: public_member_api_docs
  late final cion_security_h handle;
}
