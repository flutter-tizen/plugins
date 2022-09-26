import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';

import 'package:tizen_bundle/tizen_bundle.dart';
import 'package:tizen_interop/6.5/tizen.dart';

/// The header of parcel included information of the parcel.
class ParcelHeader {
  ParcelHeader._fromHandle(this._header);

  late final rpc_port_parcel_h _header;

  /// The tag of this header.
  String get tag {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> rawTag = arena();
      final int ret = tizen.rpc_port_parcel_header_get_tag(_header, rawTag);
      arena.using(rawTag.value, malloc.free);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return rawTag.value.toDartString();
    });
  }

  set tag(String value) {
    using((Arena arena) {
      final Pointer<Char> rawTag = value.toNativeChar(allocator: arena);
      final int ret = tizen.rpc_port_parcel_header_set_tag(_header, rawTag);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
    });
  }

  /// The sequence number of this header.
  int get sequenceNumber {
    return using((Arena arena) {
      final Pointer<Int> seqNum = arena();
      final int ret = tizen.rpc_port_parcel_header_get_seq_num(_header, seqNum);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return seqNum.value;
    });
  }

  set sequenceNumber(int seqNum) {
    final int ret = tizen.rpc_port_parcel_header_set_seq_num(_header, seqNum);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }
}

/// The parcel that can serialize & deserialize object data.
class Parcel {
  /// The constructor default parcel
  Parcel() {
    _parcel = using((Arena arena) {
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

    _finalizer.attach(this, _parcel, detach: this);
  }

  /// The constructor of parcel from the raw data.
  Parcel.fromRaw(Uint8List rawData) {
    using((Arena arena) {
      final Pointer<Uint8> nativeRaw = arena.allocate<Uint8>(rawData.length);
      final Pointer<rpc_port_parcel_h> parcelPointer = arena();
      for (int i = 0; i < rawData.length; ++i) {
        nativeRaw[i] = rawData[i] & 0xff;
      }

      final int ret = tizen.rpc_port_parcel_create_from_raw(
          parcelPointer, nativeRaw.cast<Void>(), rawData.length);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      _parcel = parcelPointer.value;
    });
  }

  late final Pointer<Void> _parcel;

  static const int _byteMax = 0xff;
  static const int _int16Max = 0xffff;
  static const int _int32Max = 0xffffffff;
  static const int _int64Max = 0xffffffffffffffff;

  static final Finalizer<Pointer<Void>> _finalizer =
      Finalizer<rpc_port_parcel_h>((rpc_port_parcel_h handle) {
    tizen.rpc_port_parcel_destroy(handle);
  });

  /// Gets the raw data of this parcel. (Byte array)
  Uint8List asRaw() {
    return using((Arena arena) {
      final Pointer<Pointer<Void>> nativeRaw = arena();
      final Pointer<UnsignedInt> size = arena();
      final int ret = tizen.rpc_port_parcel_get_raw(_parcel, nativeRaw, size);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return nativeRaw.value.cast<Uint8>().asTypedList(size.value);
    });
  }

  /// Writes a byte value to this parcel.
  void writeByte(int value) {
    final int ret = tizen.rpc_port_parcel_write_byte(_parcel, value & _byteMax);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Writes a int(16bit) value to this parcel.
  void writeInt16(int value) {
    final int ret =
        tizen.rpc_port_parcel_write_int16(_parcel, value & _int16Max);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Writes a int(32bit) value to this parcel.
  void writeInt32(int value) {
    final int ret =
        tizen.rpc_port_parcel_write_int32(_parcel, value & _int32Max);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Writes a int(64bit) value to this parcel.
  void writeInt64(int value) {
    final int ret =
        tizen.rpc_port_parcel_write_int64(_parcel, value & _int64Max);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Writes a double value to this parcel.
  void writeDouble(double value) {
    final int ret = tizen.rpc_port_parcel_write_double(_parcel, value);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Writes a String value to this parcel.
  void writeString(String value) {
    using((Arena arena) {
      final Pointer<Char> nativeString = value.toNativeChar(allocator: arena);
      final int ret = tizen.rpc_port_parcel_write_string(_parcel, nativeString);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }
    });
  }

  /// Writes a bool value to this parcel.
  void writeBool(bool value) {
    final int ret = tizen.rpc_port_parcel_write_bool(_parcel, value);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Writes a Bundle value to this parcel.
  void writeBundle(Bundle b) {
    final String raw = b.encode();
    writeString(raw);
  }

  /// Writes a array count to this parcel.
  void writeArrayCount(int count) {
    final int ret =
        tizen.rpc_port_parcel_write_array_count(_parcel, count & _int32Max);

    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Reads a byte value from the parcel.
  int readByte() {
    return using((Arena arena) {
      final Pointer<Char> valuePointer = arena();
      final int ret = tizen.rpc_port_parcel_read_byte(_parcel, valuePointer);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return valuePointer.value;
    });
  }

  /// Reads a int(16bit) value from the parcel.
  int readInt16() {
    return using((Arena arena) {
      final Pointer<Short> valuePointer = arena();
      final int ret = tizen.rpc_port_parcel_read_int16(_parcel, valuePointer);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return valuePointer.value;
    });
  }

  /// Reads a int(32bit) value from the parcel.
  int readInt32() {
    return using((Arena arena) {
      final Pointer<Int> valuePointer = arena();
      final int ret = tizen.rpc_port_parcel_read_int32(_parcel, valuePointer);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return valuePointer.value;
    });
  }

  /// Reads a int(64bit) value from the parcel.
  int readInt64() {
    return using((Arena arena) {
      final Pointer<LongLong> valuePointer = arena();
      final int ret = tizen.rpc_port_parcel_read_int64(_parcel, valuePointer);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return valuePointer.value;
    });
  }

  /// Reads a double value from the parcel.
  double readDouble() {
    return using((Arena arena) {
      final Pointer<Double> valuePointer = arena();
      final int ret = tizen.rpc_port_parcel_read_double(_parcel, valuePointer);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return valuePointer.value;
    });
  }

  /// Reads a String value from this parcel.
  String readString() {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> valuePointer = arena();
      final int ret = tizen.rpc_port_parcel_read_string(_parcel, valuePointer);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return valuePointer.value.toDartString();
    });
  }

  /// Reads a bool value from this parcel.
  bool readBool() {
    return using((Arena arena) {
      final Pointer<Bool> valuePointer = arena();
      final int ret = tizen.rpc_port_parcel_read_bool(_parcel, valuePointer);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return valuePointer.value;
    });
  }

  /// Reads a bundle value from this parcel.
  Bundle readBundle() {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> valuePointer = arena();
      final int ret = tizen.rpc_port_parcel_read_string(_parcel, valuePointer);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      final String str = valuePointer.value.toDartString();
      return Bundle.decode(str);
    });
  }

  /// Reads a array count from this parcel.
  int readArrayCount() {
    return using((Arena arena) {
      final Pointer<Int> valuePointer = arena();
      final int ret =
          tizen.rpc_port_parcel_read_array_count(_parcel, valuePointer);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return valuePointer.value;
    });
  }

  /// Reads a raw data from this parcel as much as size.
  List<int> burstRead(int size) {
    return using((Arena arena) {
      final Pointer<UnsignedChar> buf =
          malloc.allocate(size).cast<UnsignedChar>();
      arena.using(buf, malloc.free);
      final int ret = tizen.rpc_port_parcel_burst_read(_parcel, buf, size);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return List<int>.generate(size, (int index) => buf[index]);
    });
  }

  /// Writes a raw data to this parcel as much as size.
  void burstWrite(List<int> raw) {
    using((Arena arena) {
      final Pointer<UnsignedChar> nativeRaw =
          malloc.allocate(raw.length).cast<UnsignedChar>();
      arena.using(nativeRaw, malloc.free);
      for (int i = 0; i < raw.length; ++i) {
        nativeRaw[i] = raw[i];
      }

      final int ret =
          tizen.rpc_port_parcel_burst_write(_parcel, nativeRaw, raw.length);
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
      final Pointer<Pointer<Void>> header = arena();
      final int ret = tizen.rpc_port_parcel_get_header(_parcel, header);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return ParcelHeader._fromHandle(header.value);
    });
  }
}
