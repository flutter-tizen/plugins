import 'dart:ffi';
import 'dart:typed_data';
import 'package:ffi/ffi.dart';

import 'package:tizen_bundle/tizen_bundle.dart';
import 'package:tizen_interop/6.5/tizen.dart';

class _Timespec extends Struct {
  @Int64()
  external int tv_sec;

  @Int64()
  external int tv_nsec;
}

/// The timestamp when parcel created.
class Timestamp {
  Timestamp._(this.second, this.nanoSecond);

  /// The second of timestamp.
  int second;

  /// THe nano second of timestamp. (10^-9 second)
  int nanoSecond;
}

/// The header of parcel that is included information of the parcel.
class ParcelHeader {
  ParcelHeader._fromHandle(this._header);

  late final Pointer<Void> _header;

  /// The tag is used version naming from TIDL.

  /// Gets the tag of header.
  String get tag {
    return using((Arena arena) {
      final Pointer<Pointer<Char>> rawTag = arena();
      final int ret = tizen.rpc_port_parcel_header_get_tag(_header, rawTag);
      arena.using(rawTag.value, malloc.free);
      if (ret != 0) {
        throw ret;
      }

      return rawTag.value.toDartString();
    });
  }

  /// Sets the tag of header.
  set tag(String tag_) {
    using((Arena arena) {
      final Pointer<Char> rawTag = tag_.toNativeChar(allocator: arena);
      final int ret = tizen.rpc_port_parcel_header_set_tag(_header, rawTag);
      if (ret != 0) {
        throw ret;
      }
    });
  }

  /// The sequence number is used when distinguish parcel pair from TIDL.

  /// Gets the sequence number of header.
  int get sequenceNumber {
    return using((Arena arena) {
      final Pointer<Int> seqNum = arena();
      final int ret = tizen.rpc_port_parcel_header_get_seq_num(_header, seqNum);
      if (ret != 0) {
        throw ret;
      }

      return seqNum.value;
    });
  }

  /// Sets the sequence number of header.
  set sequenceNumber(int seqNum) {
    final int ret = tizen.rpc_port_parcel_header_set_seq_num(_header, seqNum);
    if (ret != 0) {
      throw ret;
    }
  }

  /// Gets the timestamp of header.
  Timestamp get timestamp {
    return using((Arena arena) {
      final Pointer<_Timespec> time = calloc<_Timespec>();
      arena.using(time, calloc.free);
      final int ret = tizen.rpc_port_parcel_header_get_timestamp(
          _header, time as Pointer<timespec>);
      if (ret != 0) {
        throw ret;
      }

      return Timestamp._(time.ref.tv_sec, time.ref.tv_nsec);
    });
  }
}

/// The parcel that can serialize & deserialize.
/// It can only sequentially read & write.(like FIFO)
class Parcel {
  /// The constructor default parcel
  Parcel() {
    _parcel = using((Arena arena) {
      final Pointer<Pointer<Void>> pParcel = arena();
      final int ret = tizen.rpc_port_parcel_create(pParcel);
      if (ret != 0) {
        throw ret;
      }

      return pParcel.value;
    });

    _finalizer.attach(this, _parcel, detach: this);
  }

  /// The constructor of parcel from raw data.
  Parcel.fromRaw(Uint8List rawData) {
    using((Arena arena) {
      final Pointer<Uint8> pRaw = arena.allocate<Uint8>(rawData.length);
      final Pointer<Pointer<Void>> pH = arena();
      for (final int i in Iterable<int>.generate(rawData.length)) {
        pRaw[i] = rawData[i] & 0xff;
      }

      final int ret = tizen.rpc_port_parcel_create_from_raw(
          pH, pRaw.cast<Void>(), rawData.length);
      if (ret != 0) {
        throw ret;
      }

      _parcel = pH.value;
    });
  }

  late final Pointer<Void> _parcel;

  static const int _byteMax = 0xff;
  static const int _int16Max = 0xffff;
  static const int _int32Max = 0xffffffff;
  static const int _int64Max = 0xffffffffffffffff;

  static final Finalizer<Pointer<Void>> _finalizer =
      Finalizer<Pointer<Void>>((Pointer<Void> handle) {
    final int ret = tizen.rpc_port_parcel_destroy(handle);
    if (ret != 0) {
      throw ret;
    }
  });

  /// Gets the raw data. (Byte array)
  Uint8List get raw {
    return using((Arena arena) {
      final Pointer<Pointer<Void>> rawData = arena();
      final Pointer<UnsignedInt> size = arena();
      final int ret = tizen.rpc_port_parcel_get_raw(_parcel, rawData, size);
      if (ret != 0) {
        throw ret;
      }

      return rawData.value.cast<Uint8>().asTypedList(size.value);
    });
  }

  /// Writes a byte value to the parcel.
  void writeByte(int value) {
    final int ret = tizen.rpc_port_parcel_write_byte(_parcel, value & _byteMax);
    if (ret != 0) {
      throw ret;
    }
  }

  /// Writes a int(16bit) value to the parcel.
  void writeInt16(int value) {
    final int ret =
        tizen.rpc_port_parcel_write_int16(_parcel, value & _int16Max);
    if (ret != 0) {
      throw ret;
    }
  }

  /// Writes a int(32bit) value to the parcel.
  void writeInt32(int value) {
    final int ret =
        tizen.rpc_port_parcel_write_int32(_parcel, value & _int32Max);
    if (ret != 0) {
      throw ret;
    }
  }

  /// Writes a int(64bit) value to the parcel.
  void writeInt64(int value) {
    final int ret =
        tizen.rpc_port_parcel_write_int64(_parcel, value & _int64Max);
    if (ret != 0) {
      throw ret;
    }
  }

  /// Writes a double value to the parcel.
  void writeDouble(double value) {
    final int ret = tizen.rpc_port_parcel_write_double(_parcel, value);
    if (ret != 0) {
      throw ret;
    }
  }

  /// Writes a String value to the parcel.
  void writeString(String value) {
    using((Arena arena) {
      final Pointer<Char> str = value.toNativeChar(allocator: arena);
      final int ret = tizen.rpc_port_parcel_write_string(_parcel, str);
      if (ret != 0) {
        throw ret;
      }
    });
  }

  /// Writes a bool value to the parcel.
  void writeBool(bool value) {
    final int ret = tizen.rpc_port_parcel_write_bool(_parcel, value);
    if (ret != 0) {
      throw ret;
    }
  }

  /// Writes a Bundle value to the parcel.
  void writeBundle(Bundle b) {
    final String raw = b.encode();
    writeString(raw);
  }

  /// Writes the array count to the parcel.
  void writeArrayCount(int count) {
    final int ret =
        tizen.rpc_port_parcel_write_array_count(_parcel, count & _int32Max);

    if (ret != 0) {
      throw ret;
    }
  }

  /// Reads a byte value from the parcel.
  int readByte() {
    return using((Arena arena) {
      Pointer<Char> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_byte(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      return pV.value;
    });
  }

  /// Reads a int(16bit) value from the parcel.
  int readInt16() {
    return using((Arena arena) {
      Pointer<Short> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_int16(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      return pV.value;
    });
  }

  /// Reads a int(32bit) value from the parcel.
  int readInt32() {
    return using((Arena arena) {
      Pointer<Int> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_int32(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      return pV.value;
    });
  }

  /// Reads a int(64bit) value from the parcel.
  int readInt64() {
    return using((Arena arena) {
      Pointer<LongLong> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_int64(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      return pV.value;
    });
  }

  /// Reads a double value from the parcel.
  double readDouble() {
    return using((Arena arena) {
      Pointer<Double> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_double(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      return pV.value;
    });
  }

  /// Reads a String value from the parcel.
  String readString() {
    return using((Arena arena) {
      Pointer<Pointer<Char>> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_string(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      return pV.value.toDartString();
    });
  }

  /// Reads a bool value from the parcel.
  bool readBool() {
    return using((Arena arena) {
      final Pointer<Bool> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_bool(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      return pV.value;
    });
  }

  /// Reads a bundle value from the parcel.
  Bundle readBundle() {
    return using((Arena arena) {
      Pointer<Pointer<Char>> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_string(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      final String str = pV.value.toDartString();
      return Bundle.decode(str);
    });
  }

  /// Reads a array count from the parcel.
  int readArrayCount() {
    return using((Arena arena) {
      Pointer<Int> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_array_count(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      return pV.value;
    });
  }

  /// Reads a raw data from the parcel by size.
  List<int> burstRead(int size) {
    return using((Arena arena) {
      final Pointer<UnsignedChar> buf =
          malloc.allocate(size).cast<UnsignedChar>();
      arena.using(buf, malloc.free);
      final int ret = tizen.rpc_port_parcel_burst_read(_parcel, buf, size);
      if (ret != 0) {
        throw ret;
      }

      return List<int>.generate(size, (int index) => buf[index]);
    });
  }

  /// Writes a raw data to the parcel by size.
  void burstWrite(List<int> buf) {
    using((Arena arena) {
      final Pointer<UnsignedChar> rawBuf =
          malloc.allocate(buf.length).cast<UnsignedChar>();
      arena.using(rawBuf, malloc.free);
      for (int i = 0; i < buf.length; ++i) {
        rawBuf[i] = buf[i];
      }

      final int ret =
          tizen.rpc_port_parcel_burst_write(_parcel, rawBuf, buf.length);
      if (ret != 0) {
        throw ret;
      }
    });
  }

  /// Gets the header of parcel.
  ParcelHeader get header {
    return using((Arena arena) {
      final Pointer<Pointer<Void>> header = arena();
      final int ret = tizen.rpc_port_parcel_get_header(_parcel, header);
      if (ret != 0) {
        throw ret;
      }

      return ParcelHeader._fromHandle(header.value);
    });
  }
}
