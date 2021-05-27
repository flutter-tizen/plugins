// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter_test/flutter_test.dart';
import 'package:messageport_tizen/messageport_tizen.dart';
import 'package:integration_test/integration_test.dart';

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  testWidgets('Create non trusted local port', (WidgetTester tester) async {
    final LocalPort port =
        await TizenMessagePort.createLocalPort('test_port', isTrusted: false);
    assert(port is LocalPort, true);
    assert(!port.trusted, true);
  }, timeout: const Timeout(Duration(seconds: 5)));

  testWidgets('Create trusted local port', (WidgetTester tester) async {
    final LocalPort port = await TizenMessagePort.createLocalPort('test_port');
    assert(port is LocalPort, true);
    assert(port.trusted, true);
  }, timeout: const Timeout(Duration(seconds: 5)));

  testWidgets('Create remote port', (WidgetTester tester) async {
    final LocalPort localPort =
        await TizenMessagePort.createLocalPort('test_port');
    localPort.register((dynamic message, [RemotePort? remotePort]) => null);
    final RemotePort remotePort = await TizenMessagePort.connectToRemotePort(
        'com.example.messageport_tizen_example', 'test_port');
    expect(remotePort is RemotePort, true);
    expect(remotePort.remoteAppId,
        equals('com.example.messageport_tizen_example'));
    expect(remotePort.portName, equals('test_port'));
    await localPort.unregister();
  }, timeout: const Timeout(Duration(seconds: 5)));

  testWidgets('Create trusted remote port from not trusted',
      (WidgetTester tester) async {
    final LocalPort localPort =
        await TizenMessagePort.createLocalPort('test_port', isTrusted: false);
    localPort.register((dynamic message, [RemotePort? remotePort]) => null);
    expect(
        () async => await TizenMessagePort.connectToRemotePort(
            'com.example.messageport_tizen_example', 'test_port'),
        throwsA(isA<Exception>()));
    await localPort.unregister();
  }, timeout: const Timeout(Duration(seconds: 5)));

  testWidgets('Send simple message', (WidgetTester tester) async {
    final LocalPort localPort =
        await TizenMessagePort.createLocalPort('test_port');
    final Completer<List<dynamic>> completer = Completer<List<dynamic>>();
    localPort.register((dynamic message, [RemotePort? remotePort]) =>
        completer.complete(<dynamic>[message, remotePort]));
    final RemotePort port = await TizenMessagePort.connectToRemotePort(
        'com.example.messageport_tizen_example', 'test_port');
    port.send('Test message 1');
    final List<dynamic> value = await completer.future;
    final String message = value[0] as String;
    final RemotePort? remotePort = value[1] as RemotePort?;
    expect(message, equals('Test message 1'));
    expect(remotePort, equals(null));
    await localPort.unregister();
  }, timeout: const Timeout(Duration(seconds: 5)));

  testWidgets('Send message with local port', (WidgetTester tester) async {
    final LocalPort localPort =
        await TizenMessagePort.createLocalPort('test_port');
    final Completer<List<dynamic>> completer = Completer<List<dynamic>>();
    localPort.register((dynamic message, [RemotePort? remotePort]) {
      if (completer.isCompleted) {
        print('WARNING: additional message received: $message');
      } else {
        completer.complete(<dynamic>[message, remotePort]);
      }
    });
    final RemotePort port = await TizenMessagePort.connectToRemotePort(
        'com.example.messageport_tizen_example', 'test_port');
    port.sendWithLocalPort('Test message 2', localPort);
    final List<dynamic> value = await completer.future;
    final String message = value[0] as String;
    final RemotePort? remotePort = value[1] as RemotePort?;
    expect(message, equals('Test message 2'));
    expect(remotePort is RemotePort, true);
    expect(remotePort?.remoteAppId,
        equals('com.example.messageport_tizen_example'));
    expect(remotePort?.portName, equals('test_port'));
    await localPort.unregister();
  }, timeout: const Timeout(Duration(seconds: 5)));

  group('Types test', () {
    late LocalPort localPort;
    late RemotePort remotePort;
    late Completer<List<dynamic>> completer;

    void onMessage(dynamic message, [RemotePort? remotePort]) {
      completer.complete(<dynamic>[message, remotePort]);
    }

    setUpAll(() async {
      localPort = await TizenMessagePort.createLocalPort('test_port');
      localPort.register(onMessage);
      remotePort = await TizenMessagePort.connectToRemotePort(
          'com.example.messageport_tizen_example', 'test_port');
    });

    setUp(() {
      completer = Completer<List<dynamic>>();
    });

    tearDownAll(() async {
      await localPort.unregister();
    });

    Future<void> checkForMessage<T>(dynamic message) async {
      remotePort.send(message);
      final List<dynamic> ret = await completer.future;
      final dynamic receivedMessage = ret[0];
      final dynamic receivedPort = ret[1];
      expect(receivedMessage, message);
      expect(receivedMessage, isA<T>());
      expect(receivedPort, null);
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
      final Map<String, int> value = <String, int>{};
      value['a'] = 5;
      value['b'] = 12;
      await checkForMessage<Map<dynamic, dynamic>>(value);
    }, timeout: const Timeout(Duration(seconds: 5)));
  });
}
