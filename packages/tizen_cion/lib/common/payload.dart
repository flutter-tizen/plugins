// Copyright 2022 - 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'dart:ffi';
import 'dart:io';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';
import 'package:tizen_interop/6.5/tizen.dart';

export 'dart:typed_data';

/// The payload type enum.
enum PayloadType {
  /// The DataPayload type.
  data(0),

  /// The FilePayload type.
  file(1);

  // ignore: public_member_api_docs
  const PayloadType(this.id);
  // ignore: public_member_api_docs
  final int id;
}

/// The cion data transmission unit class.
class Payload {
  Payload._(PayloadType type) {
    handle = using((Arena arena) {
      final Pointer<cion_peer_info_h> pHandle = arena();
      final int ret = tizen.cion_payload_create(pHandle, type.id);
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

  // ignore: public_member_api_docs
  Payload.fromHandle(int pointer, {bool managed = true}) {
    handle = cion_payload_h.fromAddress(pointer);

    if (managed) {
      _finalizer.attach(this, handle, detach: this);
    }
  }

  /// The type of the payload.
  PayloadType get type {
    final int typeNative = using((Arena arena) {
      final Pointer<Int32> pType = arena();
      final int ret = tizen.cion_payload_get_type(handle, pType);
      if (ret != cion_error.CION_ERROR_NONE) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return pType.value;
    });

    return PayloadType.values
        .firstWhere((final PayloadType e) => e.id == typeNative);
  }

  /// The id of the payload.
  String get id {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> pId = arena();
      final int ret = tizen.cion_payload_get_payload_id(handle, pId);
      if (ret != cion_error.CION_ERROR_NONE) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      arena.using(pId.value, malloc.free);
      return pId.value.toDartString();
    });
  }

  // ignore: public_member_api_docs
  late final cion_payload_h handle;

  static final Finalizer<cion_payload_h> _finalizer =
      Finalizer<cion_payload_h>((cion_payload_h handle) {
    tizen.cion_payload_destroy(handle);
  });
}

/// The data payload class.
class DataPayload extends Payload {
  /// The constructor for this class by [data].
  DataPayload(Uint8List data) : super._(PayloadType.data) {
    this.data = data;
  }

  /// The raw data of the DataPayload.
  Uint8List get data {
    return using((Arena arena) {
      final Pointer<Pointer<UnsignedChar>> pData = arena();
      final Pointer<UnsignedInt> pDataSize = arena();
      final int ret = tizen.cion_payload_get_data(handle, pData, pDataSize);
      if (ret != cion_error.CION_ERROR_NONE) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      arena.using(pData.value, malloc.free);
      final Uint8List result = Uint8List.fromList(
          pData.value.cast<Uint8>().asTypedList(pDataSize.value));
      return result;
    });
  }

  set data(Uint8List rawData) {
    using((Arena arena) {
      final Pointer<UnsignedChar> pRaw =
          arena.allocate<UnsignedChar>(rawData.length);
      for (int i = 0; i < rawData.length; ++i) {
        pRaw[i] = rawData[i] & 0xff;
      }

      final int ret = tizen.cion_payload_set_data(handle, pRaw, rawData.length);
      if (ret != cion_error.CION_ERROR_NONE) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
    });
  }
}

/// The file payload class.
class FilePayload extends Payload {
  /// The constructor for this class by [filePath].
  FilePayload(String filePath) : super._(PayloadType.file) {
    using((Arena arena) {
      final int ret =
          tizen.cion_payload_set_file_path(handle, filePath.toNativeChar());
      if (ret != cion_error.CION_ERROR_NONE) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
    });
  }

  // ignore: public_member_api_docs
  FilePayload.internal(
      {required String filePath,
      required int receivedBytes,
      required int totalBytes})
      : super._(PayloadType.file) {
    _filePath = filePath;
    _receivedBytes = receivedBytes;
    _totalBytes = totalBytes;
  }

  /// The file name of the FilePayload.
  String get fileName {
    return _filePath ??
        using((Arena arena) {
          final Pointer<Pointer<Char>> pFileName = arena();
          final int ret =
              tizen.cion_payload_get_received_file_name(handle, pFileName);
          if (ret != cion_error.CION_ERROR_NONE) {
            throw PlatformException(
              code: ret.toString(),
              message: tizen.get_error_message(ret).toDartString(),
            );
          }

          arena.using(pFileName.value, malloc.free);
          return pFileName.value.toDartString();
        });
  }

  /// The bytes of received data size.
  int get receivedBytes {
    return _receivedBytes ??
        using((Arena arena) {
          final Pointer<Uint64> pReceivedBytes = arena();
          final int ret =
              tizen.cion_payload_get_received_bytes(handle, pReceivedBytes);
          if (ret != cion_error.CION_ERROR_NONE) {
            throw PlatformException(
              code: ret.toString(),
              message: tizen.get_error_message(ret).toDartString(),
            );
          }

          return pReceivedBytes.value;
        });
  }

  /// The bytes of total size of the file.
  int get totalBytes {
    return _totalBytes ??
        using((Arena arena) {
          final Pointer<Uint64> pTotalBytes = arena();
          final int ret =
              tizen.cion_payload_get_total_bytes(handle, pTotalBytes);
          if (ret != cion_error.CION_ERROR_NONE) {
            throw PlatformException(
              code: ret.toString(),
              message: tizen.get_error_message(ret).toDartString(),
            );
          }

          return pTotalBytes.value;
        });
  }

  /// Saves as the file to [path].
  void saveAsFile(String path) {
    if (_filePath != null) {
      if (_totalBytes != _receivedBytes) {
        throw PlatformException(
          code: cion_error.CION_ERROR_INVALID_OPERATION.toString(),
          message: 'File receiving is not completed yet',
        );
      }

      late final File file;
      file = File(_filePath!);
      try {
        file.renameSync(path);
      } on FileSystemException catch (_) {
        file.copySync(path);
        file.deleteSync();
      }

      return;
    }

    using((Arena arena) {
      final int ret =
          tizen.cion_payload_save_as_file(handle, path.toNativeChar());
      if (ret != cion_error.CION_ERROR_NONE) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
    });
  }

  String? _filePath;
  int? _receivedBytes;
  int? _totalBytes;
}
