// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:typed_data';

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

  testWidgets('Parcel test', (WidgetTester tester) async {
    final Parcel parcel = Parcel();
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
      testWidgets('round-trips positive value', (WidgetTester _) async {
        final Parcel parcel = Parcel();
        parcel.writeInt16(32767);
        expect(parcel.readInt16(), 32767);
      });

      testWidgets('round-trips zero', (WidgetTester _) async {
        final Parcel parcel = Parcel();
        parcel.writeInt16(0);
        expect(parcel.readInt16(), 0);
      });

      testWidgets('masks value to 16 bits', (WidgetTester _) async {
        final Parcel parcel = Parcel();
        // 0x10042 & 0xffff == 0x0042 == 66
        parcel.writeInt16(0x10042);
        expect(parcel.readInt16(), 66);
      });
    });

    group('writeInt64 / readInt64', () {
      testWidgets('round-trips large positive value', (WidgetTester _) async {
        final Parcel parcel = Parcel();
        const int value = 9007199254740992; // 2^53
        parcel.writeInt64(value);
        expect(parcel.readInt64(), value);
      });

      testWidgets('round-trips zero', (WidgetTester _) async {
        final Parcel parcel = Parcel();
        parcel.writeInt64(0);
        expect(parcel.readInt64(), 0);
      });
    });

    group('writeArrayCount / readArrayCount', () {
      testWidgets('round-trips non-zero count', (WidgetTester _) async {
        final Parcel parcel = Parcel();
        parcel.writeArrayCount(42);
        expect(parcel.readArrayCount(), 42);
      });

      testWidgets('round-trips zero count', (WidgetTester _) async {
        final Parcel parcel = Parcel();
        parcel.writeArrayCount(0);
        expect(parcel.readArrayCount(), 0);
      });
    });

    group('write / read (burst byte array)', () {
      testWidgets('round-trips byte array', (WidgetTester _) async {
        final Parcel parcel = Parcel();
        final Uint8List bytes = Uint8List.fromList(
          <int>[0x00, 0x01, 0x7f, 0x80, 0xff],
        );
        parcel.write(bytes);
        final Uint8List result = parcel.read(bytes.length);
        expect(result, equals(bytes));
      });

      testWidgets('round-trips single byte', (WidgetTester _) async {
        final Parcel parcel = Parcel();
        final Uint8List bytes = Uint8List.fromList(<int>[0xab]);
        parcel.write(bytes);
        expect(parcel.read(1), equals(bytes));
      });
    });

    group('asRaw', () {
      testWidgets('returns non-empty bytes after write', (WidgetTester _) async {
        final Parcel parcel = Parcel();
        parcel.writeInt32(1);
        final Uint8List raw = parcel.asRaw();
        expect(raw, isNotEmpty);
      });
    });

    group('Parcel.fromRaw', () {
      testWidgets('reconstructed parcel reads back original values', (
        WidgetTester _,
      ) async {
        final Parcel original = Parcel();
        original.writeInt32(99);
        original.writeString('raw');
        original.writeBool(true);

        final Uint8List raw = original.asRaw();
        final Parcel restored = Parcel.fromRaw(raw);

        expect(restored.readInt32(), 99);
        expect(restored.readString(), 'raw');
        expect(restored.readBool(), isTrue);
      });
    });

    group('writeBundle / readBundle', () {
      testWidgets('round-trips a Bundle with string entries', (
        WidgetTester _,
      ) async {
        final Bundle bundle = Bundle();
        bundle['key1'] = 'value1';
        bundle['key2'] = 'value2';

        final Parcel parcel = Parcel();
        parcel.writeBundle(bundle);

        final Bundle restored = parcel.readBundle();
        expect(restored['key1'], 'value1');
        expect(restored['key2'], 'value2');
      });

      testWidgets('round-trips an empty Bundle', (WidgetTester _) async {
        final Parcel parcel = Parcel();
        parcel.writeBundle(Bundle());
        final Bundle restored = parcel.readBundle();
        expect(restored.length, 0);
      });
    });

    group('header', () {
      testWidgets('tag can be set and retrieved', (WidgetTester _) async {
        final Parcel parcel = Parcel();
        final ParcelHeader header = parcel.header;
        header.tag = '1.2.3';
        expect(parcel.header.tag, '1.2.3');
      });

      testWidgets('sequenceNumber can be set and retrieved', (
        WidgetTester _,
      ) async {
        final Parcel parcel = Parcel();
        final ParcelHeader header = parcel.header;
        header.sequenceNumber = 7;
        expect(parcel.header.sequenceNumber, 7);
      });

      testWidgets('tag defaults to empty string', (WidgetTester _) async {
        final Parcel parcel = Parcel();
        expect(parcel.header.tag, isEmpty);
      });
    });

    group('Parcelable', () {
      testWidgets('custom Parcelable serializes and deserializes correctly', (
        WidgetTester _,
      ) async {
        final Parcel parcel = Parcel();
        final _Point original = _Point(x: 10, y: 20);
        original.serialize(parcel);

        final _Point restored = _Point();
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
    testWidgets('main and callback values are distinct', (
      WidgetTester _,
    ) async {
      expect(PortType.main, isNot(equals(PortType.callback)));
    });

    testWidgets('PortType.values contains both entries', (
      WidgetTester _,
    ) async {
      expect(PortType.values, containsAll(<PortType>[PortType.main, PortType.callback]));
    });
  });

  // ---------------------------------------------------------------------------
  // ProxyBase — single-app observable behaviour
  // ---------------------------------------------------------------------------

  group('ProxyBase', () {
    testWidgets('isConnected is false before connecting', (
      WidgetTester _,
    ) async {
      final _TestProxy proxy = _TestProxy();
      expect(proxy.isConnected, isFalse);
    });

    testWidgets('appid and portName are set from constructor', (
      WidgetTester _,
    ) async {
      final _TestProxy proxy = _TestProxy();
      expect(proxy.appid, 'org.tizen.nonexistent_app_for_rpc_test');
      expect(proxy.portName, 'TestPort');
    });
  });
}
