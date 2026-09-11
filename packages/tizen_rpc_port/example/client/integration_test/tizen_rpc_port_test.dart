// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_bundle/tizen_bundle.dart';
import 'package:tizen_rpc_port/tizen_rpc_port.dart';

// A minimal Parcelable implementation used only in tests.
class _Point implements Parcelable {
  _Point({this.x = 0, this.y = 0});

  int x;
  int y;

  @override
  void serialize(Parcel parcel) {
    parcel.writeInt32(x);
    parcel.writeInt32(y);
  }

  @override
  void deserialize(Parcel parcel) {
    x = parcel.readInt32();
    y = parcel.readInt32();
  }
}

// A minimal ProxyBase subclass used only to test single-app observable
// behaviour (isConnected initial state, connection rejection).
class _TestProxy extends ProxyBase {
  _TestProxy() : super('org.tizen.nonexistent_app_for_rpc_test', 'TestPort');

  @override
  Future<void> onReceivedEvent(Parcel parcel) async {}
}

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  // ---------------------------------------------------------------------------
  // Existing tests — preserved unchanged
  // ---------------------------------------------------------------------------

  test('Parcel test', () async {
    final parcel = Parcel();
    parcel.writeBool(false);
    parcel.writeInt32(123);
    parcel.writeString('Hello');
    parcel.writeByte(0x3f);
    parcel.writeDouble(123.4);

    expect(parcel.readBool(), false);
    expect(parcel.readInt32(), 123);
    expect(parcel.readString(), 'Hello');
    expect(parcel.readByte(), 0x3f);
    expect(parcel.readDouble(), 123.4);
  });

  // ---------------------------------------------------------------------------
  // Parcel — additional primitive types
  // ---------------------------------------------------------------------------

  group('Parcel', () {
    group('writeInt16 / readInt16', () {
      test('round-trips positive value', () async {
        final parcel = Parcel();
        parcel.writeInt16(32767);
        expect(parcel.readInt16(), 32767);
      });

      test('round-trips negative value', () async {
        final parcel = Parcel();
        parcel.writeInt16(-1);
        expect(parcel.readInt16(), -1);
      });

      test('round-trips minimum value', () async {
        final parcel = Parcel();
        parcel.writeInt16(-32768);
        expect(parcel.readInt16(), -32768);
      });

      test('masks value to 16 bits', () async {
        final parcel = Parcel();
        // 0x10042 & 0xffff == 0x0042 == 66
        parcel.writeInt16(0x10042);
        expect(parcel.readInt16(), 66);
      });
    });

    group('writeInt64 / readInt64', () {
      test('round-trips large positive value', () async {
        final parcel = Parcel();
        const value = 9007199254740992; // 2^53
        parcel.writeInt64(value);
        expect(parcel.readInt64(), value);
      });

      test('round-trips maximum value', () async {
        final parcel = Parcel();
        const value = 9223372036854775807; // int64 max
        parcel.writeInt64(value);
        expect(parcel.readInt64(), value);
      });

      test('round-trips minimum value', () async {
        final parcel = Parcel();
        const value = -9223372036854775808; // int64 min
        parcel.writeInt64(value);
        expect(parcel.readInt64(), value);
      });
    });

    group('writeByte / readByte', () {
      test('round-trips a value', () async {
        final parcel = Parcel();
        parcel.writeByte(0xab);
        expect(parcel.readByte(), 0xab);
      });

      test('round-trips maximum unsigned value', () async {
        final parcel = Parcel();
        parcel.writeByte(255);
        expect(parcel.readByte(), 255);
      });

      test('reads a negative input back as an unsigned byte', () async {
        final parcel = Parcel();
        // -1 is written as 0xff and must read back as the unsigned byte 255.
        parcel.writeByte(-1);
        expect(parcel.readByte(), 255);
      });

      test('masks value to 8 bits', () async {
        final parcel = Parcel();
        // 0x142 & 0xff == 0x42 == 66
        parcel.writeByte(0x142);
        expect(parcel.readByte(), 66);
      });
    });

    group('writeArrayCount / readArrayCount', () {
      test('round-trips a count', () async {
        final parcel = Parcel();
        parcel.writeArrayCount(42);
        expect(parcel.readArrayCount(), 42);
      });
    });

    group('write / read (burst byte array)', () {
      test('round-trips byte array', () async {
        final parcel = Parcel();
        final bytes = Uint8List.fromList(<int>[
          0x00,
          0x01,
          0x7f,
          0x80,
          0xff,
        ]);
        parcel.write(bytes);
        final Uint8List result = parcel.read(bytes.length);
        expect(result, equals(bytes));
      });
    });

    group('Parcel.fromRaw', () {
      test('reconstructed parcel reads back original values', () async {
        final original = Parcel();
        original.writeInt32(99);
        original.writeString('raw');
        original.writeBool(true);

        final Uint8List raw = original.asRaw();
        final restored = Parcel.fromRaw(raw);

        expect(restored.readInt32(), 99);
        expect(restored.readString(), 'raw');
        expect(restored.readBool(), isTrue);
      });
    });

    group('writeBundle / readBundle', () {
      test('round-trips a Bundle with string entries', () async {
        final bundle = Bundle();
        bundle['key1'] = 'value1';
        bundle['key2'] = 'value2';

        final parcel = Parcel();
        parcel.writeBundle(bundle);

        final Bundle restored = parcel.readBundle();
        expect(restored['key1'], 'value1');
        expect(restored['key2'], 'value2');
      });

      test('round-trips an empty Bundle', () async {
        final parcel = Parcel();
        parcel.writeBundle(Bundle());
        final Bundle restored = parcel.readBundle();
        expect(restored.length, 0);
      });
    });

    group('header', () {
      test('tag can be set and retrieved', () async {
        final parcel = Parcel();
        final ParcelHeader header = parcel.header;
        header.tag = '1.2.3';
        expect(parcel.header.tag, '1.2.3');
      });

      test('sequenceNumber can be set and retrieved', () async {
        final parcel = Parcel();
        final ParcelHeader header = parcel.header;
        header.sequenceNumber = 7;
        expect(parcel.header.sequenceNumber, 7);
      });

      test('tag defaults to empty string', () async {
        final parcel = Parcel();
        expect(parcel.header.tag, isEmpty);
      });
    });

    group('Parcelable', () {
      test('custom Parcelable serializes and deserializes correctly', () async {
        final parcel = Parcel();
        final original = _Point(x: 10, y: 20);
        original.serialize(parcel);

        final restored = _Point();
        restored.deserialize(parcel);
        expect(restored.x, 10);
        expect(restored.y, 20);
      });
    });
  });

  // ---------------------------------------------------------------------------
  // PortType enum
  // ---------------------------------------------------------------------------

  group('PortType', () {
    test('has exactly the main and callback values', () async {
      expect(
        PortType.values,
        unorderedEquals(<PortType>[PortType.main, PortType.callback]),
      );
    });
  });

  // ---------------------------------------------------------------------------
  // ProxyBase — single-app observable behaviour
  // ---------------------------------------------------------------------------

  group('ProxyBase', () {
    test('isConnected is false before connecting', () async {
      final proxy = _TestProxy();
      expect(proxy.isConnected, isFalse);
    });

    test('appid and portName are set from constructor', () async {
      final proxy = _TestProxy();
      expect(proxy.appid, 'org.tizen.nonexistent_app_for_rpc_test');
      expect(proxy.portName, 'TestPort');
    });
  });

  // ---------------------------------------------------------------------------
  // Tizen 10.0+ Parcel APIs (reader, dataSize, reserve) with safety guards
  // ---------------------------------------------------------------------------

  group('Tizen 10.0+ Parcel APIs', () {
    test('reader getter/setter', () async {
      final parcel = Parcel();
      try {
        parcel.reader = 10;
        expect(parcel.reader, 10);
      } on UnsupportedError catch (e) {
        expect(e.message, contains('not supported on this version of Tizen'));
      }
    });

    test('dataSize getter/setter', () async {
      final parcel = Parcel();
      try {
        parcel.dataSize = 20;
        expect(parcel.dataSize, 20);
      } on UnsupportedError catch (e) {
        expect(e.message, contains('not supported on this version of Tizen'));
      }
    });

    test('reserve', () async {
      final parcel = Parcel();
      try {
        parcel.reserve(100);
      } on UnsupportedError catch (e) {
        expect(e.message, contains('not supported on this version of Tizen'));
      }
    });
  });
}
