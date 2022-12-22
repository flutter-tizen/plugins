// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';
import 'package:tizen_bundle/tizen_bundle.dart';
import 'package:tizen_interop/6.5/tizen.dart';

import 'port.dart';

export 'dart:typed_data';

/// An interface for marshaling and unmarshaling object data.
abstract class Parcelable {
  /// Serializes the object data to the [parcel].
  void serialize(Parcel parcel);

  /// Desrializes the object data from the [parcel].
  void deserialize(Parcel parcel);
}

/// A parcel header that contains various metadata about a [Parcel].
class ParcelHeader {
  ParcelHeader._fromHandle(this._header);

  final rpc_port_parcel_header_h _header;

  /// The tag name.
  String get tag {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> pTag = arena();
      final int ret = tizen.rpc_port_parcel_header_get_tag(_header, pTag);
      arena.using(pTag.value, malloc.free);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return pTag.value.toDartString();
    });
  }

  set tag(String value) {
    using((Arena arena) {
      final Pointer<Char> pTag = value.toNativeChar(allocator: arena);
      final int ret = tizen.rpc_port_parcel_header_set_tag(_header, pTag);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
    });
  }

  /// The sequence number.
  int get sequenceNumber {
    return using((Arena arena) {
      final Pointer<Int> pNumber = arena();
      final int ret =
          tizen.rpc_port_parcel_header_get_seq_num(_header, pNumber);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return pNumber.value;
    });
  }

  set sequenceNumber(int value) {
    final int ret = tizen.rpc_port_parcel_header_set_seq_num(_header, value);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }
}

/// Contains serialized and deserialized object data to be transferred for
/// RPC communication.
class Parcel {
  /// Creates a new [Parcel].
  Parcel() {
    _handle = using((Arena arena) {
      final Pointer<rpc_port_parcel_h> pParcel = arena();
      final int ret = tizen.rpc_port_parcel_create(pParcel);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return pParcel.value;
    });

    _finalizer.attach(this, _handle, detach: this);
  }

  /// Creates a [Parcel] from an existing port.
  Parcel.fromPort(Port port) {
    _handle = using((Arena arena) {
      final Pointer<rpc_port_parcel_h> pParcel = arena();
      final int ret =
          tizen.rpc_port_parcel_create_from_port(pParcel, port.handle);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return pParcel.value;
    });

    _finalizer.attach(this, _handle, detach: this);
  }

  /// Creates a [Parcel] from the given [rawData].
  Parcel.fromRaw(Uint8List rawData) {
    _handle = using((Arena arena) {
      final Pointer<Uint8> pRaw = arena.allocate<Uint8>(rawData.length);
      for (int i = 0; i < rawData.length; ++i) {
        pRaw[i] = rawData[i] & _byteMax;
      }

      final Pointer<rpc_port_parcel_h> pParcel = arena();
      final int ret = tizen.rpc_port_parcel_create_from_raw(
          pParcel, pRaw.cast<Void>(), rawData.length);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return pParcel.value;
    });

    _finalizer.attach(this, _handle, detach: this);
  }

  late final rpc_port_parcel_h _handle;

  static const int _byteMax = 0xff;
  static const int _int16Max = 0xffff;
  static const int _int32Max = 0xffffffff;

  static final Finalizer<rpc_port_parcel_h> _finalizer =
      Finalizer<rpc_port_parcel_h>((rpc_port_parcel_h handle) {
    tizen.rpc_port_parcel_destroy(handle);
  });

  /// Gets a byte array backed by the raw data of this parcel.
  Uint8List asRaw() {
    return using((Arena arena) {
      final Pointer<Pointer<Void>> pRaw = arena();
      final Pointer<UnsignedInt> pSize = arena();
      final int ret = tizen.rpc_port_parcel_get_raw(_handle, pRaw, pSize);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return pRaw.value.cast<Uint8>().asTypedList(pSize.value);
    });
  }

  /// Sends this parcel over the [port].
  void send(Port port) {
    final int ret = tizen.rpc_port_parcel_send(_handle, port.handle);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Writes a byte value to this parcel.
  void writeByte(int value) {
    final int ret = tizen.rpc_port_parcel_write_byte(_handle, value & _byteMax);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Writes a 16-bit integer value to this parcel.
  void writeInt16(int value) {
    final int ret =
        tizen.rpc_port_parcel_write_int16(_handle, value & _int16Max);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Writes a 32-bit integer value to this parcel.
  void writeInt32(int value) {
    final int ret =
        tizen.rpc_port_parcel_write_int32(_handle, value & _int32Max);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Writes a 64-bit integer value to this parcel.
  void writeInt64(int value) {
    final int ret = tizen.rpc_port_parcel_write_int64(_handle, value);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Writes a double value to this parcel.
  void writeDouble(double value) {
    final int ret = tizen.rpc_port_parcel_write_double(_handle, value);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Writes a string value to this parcel.
  void writeString(String value) {
    using((Arena arena) {
      final Pointer<Char> pString = value.toNativeChar(allocator: arena);
      final int ret = tizen.rpc_port_parcel_write_string(_handle, pString);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
    });
  }

  /// Writes a boolean value to this parcel.
  void writeBool(bool value) {
    final int ret = tizen.rpc_port_parcel_write_bool(_handle, value);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Writes a [Bundle] to this parcel.
  void writeBundle(Bundle bundle) {
    writeString(bundle.encode());
  }

  /// Writes an array count to this parcel.
  void writeArrayCount(int count) {
    final int ret =
        tizen.rpc_port_parcel_write_array_count(_handle, count & _int32Max);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Reads a byte value from this parcel.
  int readByte() {
    return using((Arena arena) {
      final Pointer<Char> pValue = arena();
      final int ret = tizen.rpc_port_parcel_read_byte(_handle, pValue);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return pValue.value;
    });
  }

  /// Reads a 16-bit integer value from this parcel.
  int readInt16() {
    return using((Arena arena) {
      final Pointer<Short> pValue = arena();
      final int ret = tizen.rpc_port_parcel_read_int16(_handle, pValue);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return pValue.value;
    });
  }

  /// Reads a 32-bit integer value from this parcel.
  int readInt32() {
    return using((Arena arena) {
      final Pointer<Int> pValue = arena();
      final int ret = tizen.rpc_port_parcel_read_int32(_handle, pValue);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return pValue.value;
    });
  }

  /// Reads a 64-bit integer value from this parcel.
  int readInt64() {
    return using((Arena arena) {
      final Pointer<LongLong> pValue = arena();
      final int ret = tizen.rpc_port_parcel_read_int64(_handle, pValue);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return pValue.value;
    });
  }

  /// Reads a double value from this parcel.
  double readDouble() {
    return using((Arena arena) {
      final Pointer<Double> pValue = arena();
      final int ret = tizen.rpc_port_parcel_read_double(_handle, pValue);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return pValue.value;
    });
  }

  /// Reads a string value from this parcel.
  String readString() {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> pValue = arena();
      final int ret = tizen.rpc_port_parcel_read_string(_handle, pValue);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return pValue.value.toDartString();
    });
  }

  /// Reads a boolean value from this parcel.
  bool readBool() {
    return using((Arena arena) {
      final Pointer<Bool> pValue = arena();
      final int ret = tizen.rpc_port_parcel_read_bool(_handle, pValue);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return pValue.value;
    });
  }

  /// Reads a [Bundle] from this parcel.
  Bundle readBundle() {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> pValue = arena();
      final int ret = tizen.rpc_port_parcel_read_string(_handle, pValue);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return Bundle.decode(pValue.value.toDartString());
    });
  }

  /// Reads an array count from this parcel.
  int readArrayCount() {
    return using((Arena arena) {
      final Pointer<Int> pValue = arena();
      final int ret = tizen.rpc_port_parcel_read_array_count(_handle, pValue);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return pValue.value;
    });
  }

  /// Reads a byte array of length [size] from this parcel.
  Uint8List read(int size) {
    return using((Arena arena) {
      final Pointer<UnsignedChar> pBuffer = arena(size);
      final int ret = tizen.rpc_port_parcel_burst_read(_handle, pBuffer, size);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return Uint8List.fromList(
          List<int>.generate(size, (int index) => pBuffer[index]));
    });
  }

  /// Writes a byte array to this parcel.
  void write(Uint8List bytes) {
    using((Arena arena) {
      final Pointer<UnsignedChar> pBuffer = arena(bytes.length);
      for (int i = 0; i < bytes.length; ++i) {
        pBuffer[i] = bytes[i];
      }
      final int ret =
          tizen.rpc_port_parcel_burst_write(_handle, pBuffer, bytes.length);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
    });
  }

  /// The header of this parcel.
  ParcelHeader get header {
    return using((Arena arena) {
      final Pointer<rpc_port_parcel_header_h> pHeader = arena();
      final int ret = tizen.rpc_port_parcel_get_header(_handle, pHeader);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
      return ParcelHeader._fromHandle(pHeader.value);
    });
  }
}
