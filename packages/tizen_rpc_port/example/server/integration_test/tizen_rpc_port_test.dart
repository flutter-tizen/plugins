// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_bundle/tizen_bundle.dart';
import 'package:tizen_rpc_port/tizen_rpc_port.dart';

import 'package:tizen_rpc_port_server_example/message_server.dart';

// A minimal StubBase subclass used only to test single-app observable
// behaviour (construction, setTrusted, addPrivilege).
class _TestStub extends StubBase {
  _TestStub() : super('TestStubPort');

  @override
  Future<void> onConnectedEvent(String sender, String instance) async {}

  @override
  Future<void> onDisconnectedEvent(String sender, String instance) async {}

  @override
  Future<void> onReceivedEvent(
    String sender,
    String instance,
    Parcel parcel,
  ) async {}
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
      expect(
        PortType.values,
        containsAll(<PortType>[PortType.main, PortType.callback]),
      );
    });
  });

  // ---------------------------------------------------------------------------
  // StubBase — single-app observable behaviour
  // ---------------------------------------------------------------------------

  group('StubBase', () {
    testWidgets('portName is set from constructor', (WidgetTester _) async {
      final _TestStub stub = _TestStub();
      expect(stub.portName, 'TestStubPort');
    });

    testWidgets('setTrusted can be called without listen', (
      WidgetTester _,
    ) async {
      final _TestStub stub = _TestStub();
      stub.setTrusted(true);
      stub.setTrusted(false);
    });

    testWidgets('addPrivilege can be called without listen', (
      WidgetTester _,
    ) async {
      final _TestStub stub = _TestStub();
      stub.addPrivilege('http://tizen.org/privilege/datasharing');
    });
  });

  // ---------------------------------------------------------------------------
  // Message — server-side StubBase subclass
  // ---------------------------------------------------------------------------

  group('Message', () {
    testWidgets('portName is Message', (WidgetTester _) async {
      final Message server = Message(
        serviceBuilder: (String sender, String instance) =>
            _NoopService(sender, instance),
      );
      expect(server.portName, 'Message');
    });

    testWidgets('services list is initially empty', (WidgetTester _) async {
      final Message server = Message(
        serviceBuilder: (String sender, String instance) =>
            _NoopService(sender, instance),
      );
      expect(server.services, isEmpty);
    });
  });
}

class _NoopService extends ServiceBase {
  _NoopService(super.sender, super.instance);

  @override
  Future<void> onCreate() async {}

  @override
  Future<void> onTerminate() async {}

  @override
  Future<int> onRegister(String name, NotifyCallback callback) async => 0;

  @override
  Future<void> onUnregister() async {}

  @override
  Future<int> onSend(String message) async => 0;
}
