// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_rpc_port/tizen_rpc_port.dart';

import 'package:tizen_rpc_port_server_example/message_server.dart';

// The core Parcel and PortType tests are context-independent, so they are
// exercised only in the client example to avoid duplicating them here. This
// suite covers the server-specific StubBase behaviour.

// A minimal StubBase subclass used only to test single-app observable
// behaviour (construction, setTrusted, addPrivilege, close).
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
  // StubBase — single-app observable behaviour
  // ---------------------------------------------------------------------------

  group('StubBase', () {
    test('portName is set from constructor', () async {
      final stub = _TestStub();
      addTearDown(stub.close);
      expect(stub.portName, 'TestStubPort');
    });

    test('setTrusted can be called without listen', () async {
      final stub = _TestStub();
      addTearDown(stub.close);
      stub.setTrusted(true);
      stub.setTrusted(false);
    });

    test('addPrivilege can be called without listen', () async {
      final stub = _TestStub();
      addTearDown(stub.close);
      stub.addPrivilege('http://tizen.org/privilege/datasharing');
    });

    test('operations on a closed stub throw StateError', () async {
      final stub = _TestStub();
      await stub.close();
      expect(() => stub.setTrusted(true), throwsStateError);
      expect(() => stub.addPrivilege('http://tizen.org/privilege/datasharing'), throwsStateError);
      expect(() => stub.getPort('instance', PortType.main), throwsStateError);
      await expectLater(stub.listen(), throwsStateError);
    });

    test('close returns the same future when called twice', () async {
      final stub = _TestStub();
      final Future<void> first = stub.close();
      expect(identical(stub.close(), first), isTrue);
      await first;
    });
  });

  // ---------------------------------------------------------------------------
  // Message — server-side StubBase subclass
  // ---------------------------------------------------------------------------

  group('Message', () {
    test('portName is Message', () async {
      final server = Message(
        serviceBuilder: (String sender, String instance) => _NoopService(sender, instance),
      );
      addTearDown(server.close);
      expect(server.portName, 'Message');
    });

    test('services list is initially empty', () async {
      final server = Message(
        serviceBuilder: (String sender, String instance) => _NoopService(sender, instance),
      );
      addTearDown(server.close);
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
