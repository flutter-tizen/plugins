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

  testWidgets('Create non trusted local port', (WidgetTester tester) async {
    final LocalPort port = await LocalPort.create(kTestPort, trusted: false);
    expect(port.trusted, isFalse);
  }, timeout: const Timeout(Duration(seconds: 5)));

  testWidgets('Create trusted local port', (WidgetTester tester) async {
    final LocalPort port = await LocalPort.create(kTestPort);
    expect(port.trusted, isTrue);
  }, timeout: const Timeout(Duration(seconds: 5)));

  testWidgets('Create remote port', (WidgetTester tester) async {
    final LocalPort localPort = await LocalPort.create(kTestPort);
    localPort.register((dynamic message, [RemotePort? remotePort]) => null);

    final RemotePort remotePort =
        await RemotePort.connect(kTestAppId, kTestPort);
    expect(remotePort.remoteAppId, equals(kTestAppId));
    expect(remotePort.portName, equals(kTestPort));

    await localPort.unregister();
  }, timeout: const Timeout(Duration(seconds: 5)));

  testWidgets('Create trusted remote port from not trusted',
      (WidgetTester tester) async {
    final LocalPort localPort =
        await LocalPort.create(kTestPort, trusted: false);
    localPort.register((dynamic message, [RemotePort? remotePort]) => null);

    await expectLater(
      () => RemotePort.connect(kTestAppId, kTestPort),
      throwsException,
    );

    await localPort.unregister();
  }, timeout: const Timeout(Duration(seconds: 5)));

  testWidgets('Check for remote', (WidgetTester tester) async {
    final LocalPort localPort = await LocalPort.create(kTestPort);
    localPort.register((dynamic message, [RemotePort? remotePort]) => null);

    final RemotePort remotePort =
        await RemotePort.connect(kTestAppId, kTestPort);
    expect(await remotePort.check(), isTrue);

    await localPort.unregister();
    expect(await remotePort.check(), isFalse);
  }, timeout: const Timeout(Duration(seconds: 5)));

  testWidgets('Send null message', (WidgetTester tester) async {
    final LocalPort localPort = await LocalPort.create(kTestPort);
    final Completer<dynamic> completer = Completer<dynamic>();
    localPort.register((dynamic message, [RemotePort? remotePort]) {
      expect(remotePort, isNull);
      completer.complete(message);
    });

    final RemotePort port = await RemotePort.connect(kTestAppId, kTestPort);
    await port.send(null);

    final dynamic message = await completer.future;
    expect(message, isNull);

    await localPort.unregister();
  }, timeout: const Timeout(Duration(seconds: 5)));

  testWidgets('Send string message', (WidgetTester tester) async {
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

  testWidgets('Send message with local port', (WidgetTester tester) async {
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

  group('Types test', () {
    late LocalPort localPort;
    late RemotePort remotePort;
    late Completer<List<dynamic>> completer;

    setUpAll(() async {
      localPort = await LocalPort.create(kTestPort);
      localPort.register((dynamic message, [RemotePort? remotePort]) {
        completer.complete(<dynamic>[message, remotePort]);
      });
      remotePort = await RemotePort.connect(kTestAppId, kTestPort);
    });

    setUp(() {
      completer = Completer<List<dynamic>>();
    });

    tearDownAll(() async {
      await localPort.unregister();
    });

    Future<void> checkForMessage<T>(T message) async {
      await remotePort.send(message);

      final List<dynamic> value = await completer.future;
      final dynamic receivedMessage = value[0];
      final RemotePort? receivedPort = value[1] as RemotePort?;
      expect(receivedMessage, equals(message));
      expect(receivedPort, isNull);
    }

    testWidgets('bool', (WidgetTester tester) async {
      const bool value = true;
      await checkForMessage<bool>(value);
    }, timeout: const Timeout(Duration(seconds: 5)));

    testWidgets('int', (WidgetTester tester) async {
      const int value = 834;
      await checkForMessage<int>(value);
    }, timeout: const Timeout(Duration(seconds: 5)));

    testWidgets('double', (WidgetTester tester) async {
      const double value = 12.847;
      await checkForMessage<double>(value);
    }, timeout: const Timeout(Duration(seconds: 5)));

    testWidgets('string', (WidgetTester tester) async {
      const String value = 'Short string message';
      await checkForMessage<String>(value);
    }, timeout: const Timeout(Duration(seconds: 5)));

    testWidgets('list', (WidgetTester tester) async {
      final List<int> value = <int>[1, 5, 8, 12, 0, 2];
      await checkForMessage<List<dynamic>>(value);
    }, timeout: const Timeout(Duration(seconds: 5)));

    testWidgets('map', (WidgetTester tester) async {
      final Map<String, int> value = <String, int>{'a': 5, 'b': 12};
      await checkForMessage<Map<dynamic, dynamic>>(value);
    }, timeout: const Timeout(Duration(seconds: 5)));
  });
}
