// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:messageport_tizen/messageport_tizen.dart';

const String kTestPort = 'test_port';
const String kTestAppId = 'org.tizen.messageport_tizen_example';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  test('Create non trusted local port', () async {
    final LocalPort port = await LocalPort.create(kTestPort, trusted: false);
    expect(port.trusted, isFalse);
  }, timeout: const Timeout(Duration(seconds: 5)));

  test('Create trusted local port', () async {
    final LocalPort port = await LocalPort.create(kTestPort);
    expect(port.trusted, isTrue);
  }, timeout: const Timeout(Duration(seconds: 5)));

  test('Create remote port', () async {
    final LocalPort localPort = await LocalPort.create(kTestPort);
    localPort.register((dynamic message, [RemotePort? remotePort]) {});

    final RemotePort remotePort = await RemotePort.connect(
      kTestAppId,
      kTestPort,
    );
    expect(remotePort.remoteAppId, equals(kTestAppId));
    expect(remotePort.portName, equals(kTestPort));

    await localPort.unregister();
  }, timeout: const Timeout(Duration(seconds: 5)));

  test(
    'Create trusted remote port from not trusted',
    () async {
      final LocalPort localPort = await LocalPort.create(
        kTestPort,
        trusted: false,
      );
      localPort.register((dynamic message, [RemotePort? remotePort]) {});

      await expectLater(
        () => RemotePort.connect(kTestAppId, kTestPort),
        throwsException,
      );

      await localPort.unregister();
    },
    timeout: const Timeout(Duration(seconds: 5)),
  );

  test('Check for remote', () async {
    final LocalPort localPort = await LocalPort.create(kTestPort);
    localPort.register((dynamic message, [RemotePort? remotePort]) {});

    final RemotePort remotePort = await RemotePort.connect(
      kTestAppId,
      kTestPort,
    );
    expect(await remotePort.check(), isTrue);

    await localPort.unregister();
    expect(await remotePort.check(), isFalse);
  }, timeout: const Timeout(Duration(seconds: 5)));

  test('Send simple message', () async {
    final LocalPort localPort = await LocalPort.create(kTestPort);
    final Completer<dynamic> completer = Completer<dynamic>();
    localPort.register((dynamic message, [RemotePort? remotePort]) {
      expect(remotePort, isNull);
      completer.complete(message);
    });

    final RemotePort port = await RemotePort.connect(kTestAppId, kTestPort);
    await port.send('Test message 1');

    final dynamic message = await completer.future;
    expect(message, equals('Test message 1'));

    await localPort.unregister();
  }, timeout: const Timeout(Duration(seconds: 5)));

  test('Send message with local port', () async {
    final LocalPort localPort = await LocalPort.create(kTestPort);
    final Completer<List<dynamic>> completer = Completer<List<dynamic>>();
    localPort.register((dynamic message, [RemotePort? remotePort]) {
      if (!completer.isCompleted) {
        completer.complete(<dynamic>[message, remotePort]);
      }
    });

    final RemotePort port = await RemotePort.connect(kTestAppId, kTestPort);
    await port.sendWithLocalPort('Test message 2', localPort);

    final List<dynamic> value = await completer.future;
    final String message = value[0] as String;
    final RemotePort? remotePort = value[1] as RemotePort?;
    expect(message, equals('Test message 2'));
    expect(remotePort?.remoteAppId, equals(kTestAppId));
    expect(remotePort?.portName, equals(kTestPort));

    await localPort.unregister();
  }, timeout: const Timeout(Duration(seconds: 5)));

  group('LocalPort lifecycle', () {
    test('registered reflects the register/unregister lifecycle', (
      WidgetTester tester,
    ) async {
      final LocalPort localPort = await LocalPort.create(kTestPort);
      // Ensure the port is unregistered even if an assertion below fails.
      addTearDown(localPort.unregister);
      expect(localPort.portName, equals(kTestPort));
      expect(localPort.registered, isFalse);

      localPort.register((dynamic message, [RemotePort? remotePort]) {});
      expect(localPort.registered, isTrue);

      await localPort.unregister();
      expect(localPort.registered, isFalse);
    }, timeout: const Timeout(Duration(seconds: 5)));

    test('register throws when the port is already registered', (
      WidgetTester tester,
    ) async {
      final LocalPort localPort = await LocalPort.create(kTestPort);
      addTearDown(localPort.unregister);
      localPort.register((dynamic message, [RemotePort? remotePort]) {});

      expect(
        () =>
            localPort.register((dynamic message, [RemotePort? remotePort]) {}),
        throwsException,
      );
    }, timeout: const Timeout(Duration(seconds: 5)));

    test('unregister is safe when not registered and when repeated', (
      WidgetTester tester,
    ) async {
      final LocalPort localPort = await LocalPort.create(kTestPort);
      addTearDown(localPort.unregister);

      // Unregistering before any registration must not throw.
      await localPort.unregister();
      expect(localPort.registered, isFalse);

      // Unregister is safe to call repeatedly after a registration.
      localPort.register((dynamic message, [RemotePort? remotePort]) {});
      await localPort.unregister();
      await localPort.unregister();
      expect(localPort.registered, isFalse);
    }, timeout: const Timeout(Duration(seconds: 5)));
  });

  group('Types test', () {
    late LocalPort localPort;
    late RemotePort remotePort;
    late Completer<dynamic> completer;

    setUpAll(() async {
      localPort = await LocalPort.create(kTestPort);
      localPort.register((dynamic message, [RemotePort? remotePort]) {
        expect(remotePort, isNull);
        completer.complete(message);
      });
      remotePort = await RemotePort.connect(kTestAppId, kTestPort);
    });

    setUp(() {
      completer = Completer<dynamic>();
    });

    tearDownAll(() async {
      await localPort.unregister();
    });

    Future<void> checkForMessage<T>(T message) async {
      await remotePort.send(message);

      final T receivedMessage = await completer.future as T;
      expect(receivedMessage, equals(message));
    }

    test('null', () async {
      await checkForMessage(null);
    }, timeout: const Timeout(Duration(seconds: 5)));

    test('bool', () async {
      const bool value = true;
      await checkForMessage<bool>(value);
    }, timeout: const Timeout(Duration(seconds: 5)));

    test('int', () async {
      const int value = 834;
      await checkForMessage<int>(value);
    }, timeout: const Timeout(Duration(seconds: 5)));

    test('double', () async {
      const double value = 12.847;
      await checkForMessage<double>(value);
    }, timeout: const Timeout(Duration(seconds: 5)));

    test('string', () async {
      const String value = 'Short string message';
      await checkForMessage<String>(value);
    }, timeout: const Timeout(Duration(seconds: 5)));

    test('list', () async {
      final List<int> value = <int>[1, 5, 8, 12, 0, 2];
      await checkForMessage<List<dynamic>>(value);
    }, timeout: const Timeout(Duration(seconds: 5)));

    test('map', () async {
      final Map<String, int> value = <String, int>{'a': 5, 'b': 12};
      await checkForMessage<Map<dynamic, dynamic>>(value);
    }, timeout: const Timeout(Duration(seconds: 5)));

    test('empty string', () async {
      const String value = '';
      await checkForMessage<String>(value);
    }, timeout: const Timeout(Duration(seconds: 5)));

    test('nested collection', () async {
      final Map<String, dynamic> value = <String, dynamic>{
        'numbers': <int>[1, 2, 3],
        'nested': <String, String>{'key': 'value'},
      };
      await checkForMessage<Map<dynamic, dynamic>>(value);
    }, timeout: const Timeout(Duration(seconds: 5)));
  });
}
