import 'dart:ffi';
import 'dart:typed_data';
import 'package:ffi/ffi.dart';

import 'package:tizen_interop/6.5/tizen.dart';

import 'bundle.dart';
import 'disposable.dart';

class Timestamp {
  int second;
  int nanoSecond;
  Timestamp(this.second, this.nanoSecond);
}

class ParcelHeader {
  ParcelHeader._fromHandle(this._header);

  late final Pointer<Void> _header;

  String get tag {
    return using((Arena arena) {
      final Pointer<Pointer<Int8>> rawTag = arena();
      final int ret = tizen.rpc_port_parcel_header_get_tag(_header, rawTag);
      arena.using(rawTag.value, malloc.free);
      if (ret != 0) {
        throw ret;
      }

      return rawTag.value.toDartString();
    });
  }

  set tag(String tag_) {
    using((Arena arena) {
      final Pointer<Int8> rawTag = tag_.toNativeInt8(allocator: arena);
      final int ret = tizen.rpc_port_parcel_header_set_tag(_header, rawTag);
      if (ret != 0) {
        throw ret;
      }
    });
  }

  int get sequenceNumber {
    return using((Arena arena) {
      Pointer<Int32> seqNum = arena();
      final int ret = tizen.rpc_port_parcel_header_get_seq_num(_header, seqNum);
      if (ret != 0) {
        throw ret;
      }

      return seqNum.value;
    });
  }

  set sequenceNumber(int seqNum) {
    final int ret = tizen.rpc_port_parcel_header_set_seq_num(_header, seqNum);
    if (ret != 0) {
      throw ret;
    }
  }

  Timestamp get timestamp {
    return using((Arena arena) {
      final Pointer<timespec> time = calloc<timespec>();
      arena.using(time, calloc.free);
      final int ret = tizen.rpc_port_parcel_header_get_timestamp(_header, time);
      if (ret != 0) {
        throw ret;
      }

      return Timestamp(time.ref.tv_sec, time.ref.tv_nsec);
    });
  }
}

class Parcel implements Disposable {
  Parcel() {
    _parcel = using((Arena arena) {
      final Pointer<Pointer<Void>> pParcel = arena();
      final int ret = tizen.rpc_port_parcel_create(pParcel);
      if (ret != 0) {
        throw ret;
      }

      isDisposed = false;
      return pParcel.value;
    });

    _finalizer.attach(this, _parcel, detach: this);
  }

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

  @override
  bool isDisposed = true;
  late final Pointer<Void> _parcel;

  static const int _byteMax = 0xff;
  static const int _int16Max = 0x7fff;
  static const int _int32Max = 0x7ffffffff;
  static const int _int64Max = 0x7FFFFFFFFFFFFFFF;

  static final Finalizer<Pointer<Void>> _finalizer =
      Finalizer<Pointer<Void>>((Pointer<Void> handle) {
    final int ret = tizen.rpc_port_parcel_destroy(handle);
    if (ret != 0) {
      throw ret;
    }
  });

  List<int> get raw {
    return using((Arena arena) {
      final Pointer<Pointer<Void>> rawData = arena();
      final Pointer<Uint32> size = arena();
      final int ret = tizen.rpc_port_parcel_get_raw(_parcel, rawData, size);
      if (ret != 0) {
        throw ret;
      }

      return rawData.value.cast<Uint8>().asTypedList(size.value);
    });
  }

  @override
  void dispose() {
    if (isDisposed) {
      return;
    }

    final int ret = tizen.rpc_port_parcel_destroy(_parcel);
    if (ret != 0) {
      throw ret;
    }

    _finalizer.detach(this);
    isDisposed = true;
  }

  void writeByte(int value) {
    final int ret = tizen.rpc_port_parcel_write_byte(_parcel, value & _byteMax);
    if (ret != 0) {
      throw ret;
    }
  }

  void writeInt16(int value) {
    final int ret =
        tizen.rpc_port_parcel_write_int16(_parcel, value & _int16Max);
    if (ret != 0) {
      throw ret;
    }
  }

  void writeInt32(int value) {
    final int ret =
        tizen.rpc_port_parcel_write_int32(_parcel, value & _int32Max);
    if (ret != 0) {
      throw ret;
    }
  }

  void writeInt64(int value) {
    final int ret =
        tizen.rpc_port_parcel_write_int64(_parcel, value & _int64Max);
    if (ret != 0) {
      throw ret;
    }
  }

  void writeDouble(double value) {
    final int ret = tizen.rpc_port_parcel_write_double(_parcel, value);
    if (ret != 0) {
      throw ret;
    }
  }

  void writeString(String value) {
    using((Arena arena) {
      final Pointer<Int8> str = value.toNativeInt8(allocator: arena);
      final int ret = tizen.rpc_port_parcel_write_string(_parcel, str);
      if (ret != 0) {
        throw ret;
      }
    });
  }

  void writeBool(bool value) {
    final int ret = tizen.rpc_port_parcel_write_bool(_parcel, value);
    if (ret != 0) {
      throw ret;
    }
  }

  void writeBundle(Bundle b) {
    dynamic bundleHandle = b.getHandle;
    final int ret = tizen.rpc_port_parcel_write_bundle(_parcel, bundleHandle);
    if (ret != 0) {
      throw ret;
    }
  }

  void writeArrayCount(int count) {
    final int ret =
        tizen.rpc_port_parcel_write_array_count(_parcel, count & _int32Max);

    if (ret != 0) {
      throw ret;
    }
  }

  int readByte() {
    return using((Arena arena) {
      Pointer<Int8> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_byte(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      return pV.value;
    });
  }

  int readInt16() {
    return using((Arena arena) {
      Pointer<Int16> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_int16(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      return pV.value;
    });
  }

  int readInt32() {
    return using((Arena arena) {
      Pointer<Int32> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_int32(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      return pV.value;
    });
  }

  int readInt64() {
    return using((Arena arena) {
      Pointer<Int64> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_int64(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      return pV.value;
    });
  }

  double readFloat() {
    return using((Arena arena) {
      final Pointer<Float> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_float(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      return pV.value;
    });
  }

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

  String readString() {
    return using((Arena arena) {
      Pointer<Pointer<Int8>> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_string(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      return pV.value.toDartString();
    });
  }

  bool readBool() {
    return using((Arena arena) {
      final Pointer<Uint8> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_bool(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      return pV.value != 0;
    });
  }

  Bundle readBundle() {
    return using((Arena arena) {
      Pointer<Pointer<Int8>> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_string(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      final String str = pV.value.toDartString();
      final BundleRaw raw = BundleRaw(str, str.length);
      return Bundle.fromBundleRaw(raw);
    });
  }

  int readArrayCount() {
    return using((Arena arena) {
      Pointer<Int32> pV = arena();
      final int ret = tizen.rpc_port_parcel_read_array_count(_parcel, pV);
      if (ret != 0) {
        throw ret;
      }

      return pV.value;
    });
  }

  List<int> burstRead(int size) {
    return using((Arena arena) {
      final Pointer<Uint8> buf = malloc.allocate(size).cast<Uint8>();
      arena.using(buf, malloc.free);
      final int ret = tizen.rpc_port_parcel_burst_read(_parcel, buf, size);
      if (ret != 0) {
        throw ret;
      }

      return List<int>.generate(size, (int index) => buf[index]);
    });
  }

  void burstWrite(List<int> buf) {
    using((Arena arena) {
      final Pointer<Uint8> rawBuf = malloc.allocate(buf.length).cast<Uint8>();
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

  ParcelHeader getHeader() {
    return using((Arena arena) {
      Pointer<Pointer<Void>> header = arena();
      final int ret = tizen.rpc_port_parcel_get_header(_parcel, header);
      if (ret != 0) {
        throw ret;
      }

      return ParcelHeader._fromHandle(header.value);
    });
  }
}
