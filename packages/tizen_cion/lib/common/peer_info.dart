// Copyright 2022 - 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

import 'package:tizen_interop/6.5/tizen.dart';

export 'dart:typed_data';

/// The transfer status of the receiving payload.
enum PayloadTransferStatus {
  /// Success
  success(0),

  /// Failure
  failure(1),

  /// In progress
  inProgress(2);

  // ignore: public_member_api_docs
  const PayloadTransferStatus(this.id);
  // ignore: public_member_api_docs
  final int id;
}

/// The peer info class that represents the cion endpoint.
class PeerInfo {
  // ignore: public_member_api_docs
  PeerInfo.fromHandle(int pointer, {bool managed = true}) {
    handle = cion_peer_info_h.fromAddress(pointer);
    if (managed) {
      _finalizer.attach(this, handle, detach: this);
    }
  }

  /// The clone constructor for this class.
  PeerInfo.clone(PeerInfo peer) {
    handle = using((Arena arena) {
      final Pointer<cion_peer_info_h> pHandle = arena();
      final int ret = tizen.cion_peer_info_clone(peer.handle, pHandle);
      if (ret != cion_error.CION_ERROR_NONE) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return pHandle.value;
    });

    _finalizer.attach(this, handle, detach: this);
  }

  /// The device id of this PeerInfo's target device.
  String get deviceId {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> pDeviceId = arena();
      final int ret = tizen.cion_peer_info_get_device_id(handle, pDeviceId);
      if (ret != cion_error.CION_ERROR_NONE) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      arena.using(pDeviceId.value, malloc.free);
      return pDeviceId.value.toDartString();
    });
  }

  /// The device name of this PeerInfo's target device.
  String get deviceName {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> pDeviceName = arena();
      final int ret = tizen.cion_peer_info_get_device_name(handle, pDeviceName);
      if (ret != cion_error.CION_ERROR_NONE) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      arena.using(pDeviceName.value, malloc.free);
      return pDeviceName.value.toDartString();
    });
  }

  /// The device platform of this PeerInfo's target device.
  String get devicePlatform {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> pDevicePlatform = arena();
      final int ret =
          tizen.cion_peer_info_get_device_platform(handle, pDevicePlatform);
      if (ret != cion_error.CION_ERROR_NONE) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      arena.using(pDevicePlatform.value, malloc.free);
      return pDevicePlatform.value.toDartString();
    });
  }

  /// The device platform version of this PeerInfo's target device.
  String get devicePlatformVersion {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> pDevicePlatformVersion = arena();
      final int ret = tizen.cion_peer_info_get_device_platform_version(
          handle, pDevicePlatformVersion);
      if (ret != cion_error.CION_ERROR_NONE) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      arena.using(pDevicePlatformVersion.value, malloc.free);
      return pDevicePlatformVersion.value.toDartString();
    });
  }

  /// The device type of this PeerInfo's target device.
  String get deviceType {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> pDeviceType = arena();
      final int ret = tizen.cion_peer_info_get_device_type(handle, pDeviceType);
      if (ret != cion_error.CION_ERROR_NONE) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      arena.using(pDeviceType.value, malloc.free);
      return pDeviceType.value.toDartString();
    });
  }

  /// The application id of this PeerInfo's application.
  String get appid {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> pAppid = arena();
      final int ret = tizen.cion_peer_info_get_app_id(handle, pAppid);
      if (ret != cion_error.CION_ERROR_NONE) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      arena.using(pAppid.value, malloc.free);
      return pAppid.value.toDartString();
    });
  }

  /// The application version of this PeerInfo's application.
  String get appVersion {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> pAppVersion = arena();
      final int ret = tizen.cion_peer_info_get_app_version(handle, pAppVersion);
      if (ret != cion_error.CION_ERROR_NONE) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      arena.using(pAppVersion.value, malloc.free);
      return pAppVersion.value.toDartString();
    });
  }

  /// The uuid of this PeerInfo.
  String get uuid {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> pUuid = arena();
      final int ret = tizen.cion_peer_info_get_uuid(handle, pUuid);
      if (ret != cion_error.CION_ERROR_NONE) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      arena.using(pUuid.value, malloc.free);
      return pUuid.value.toDartString();
    });
  }

  /// The display name of this PeerInfo.
  /// This is set by server constructor.
  String get displayName {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> pDisplayName = arena();
      final int ret =
          tizen.cion_peer_info_get_display_name(handle, pDisplayName);
      if (ret != cion_error.CION_ERROR_NONE) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      arena.using(pDisplayName.value, malloc.free);
      return pDisplayName.value.toDartString();
    });
  }

  // ignore: public_member_api_docs
  late final cion_peer_info_h handle;

  static final Finalizer<cion_peer_info_h> _finalizer =
      Finalizer<cion_peer_info_h>((cion_peer_info_h handle) {
    tizen.cion_peer_info_destroy(handle);
  });
}
