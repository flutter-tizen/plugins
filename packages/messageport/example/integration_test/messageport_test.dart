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
    final port =
        await TizenMessageport.createLocalPort('test_port', isTrusted: false);
    assert(port is TizenLocalPort, true);
    assert(!port.trusted, true);
  }, timeout: Timeout(Duration(seconds: 5)));

  testWidgets('Create trusted local port', (WidgetTester tester) async {
    final port = await TizenMessageport.createLocalPort('test_port');
    assert(port is TizenLocalPort, true);
    assert(port.trusted, true);
  }, timeout: Timeout(Duration(seconds: 5)));

  testWidgets('Create remote port', (WidgetTester tester) async {
    final localPort = await TizenMessageport.createLocalPort('test_port');
    localPort.register((message, [remotePort]) => null);
    final remotePort = await TizenMessageport.connectToRemotePort(
        'com.example.messageport_tizen_example', 'test_port');
    expect(remotePort is TizenRemotePort, true);
    expect(remotePort.remoteAppId,
        equals('com.example.messageport_tizen_example'));
    expect(remotePort.portName, equals('test_port'));
    await localPort.unregister();
  }, timeout: Timeout(Duration(seconds: 5)));

  testWidgets('Send simple message', (WidgetTester tester) async {
    final localPort = await TizenMessageport.createLocalPort('test_port');
    Completer completer = Completer();
    localPort.register(
        (message, [remotePort]) => completer.complete([message, remotePort]));
    final port = await TizenMessageport.connectToRemotePort(
        'com.example.messageport_tizen_example', 'test_port');
    port.send('Test message 1');
    final value = await completer.future;
    final message = value[0];
    final remotePort = value[1];
    expect(message, equals('Test message 1'));
    expect(remotePort, equals(null));
    await localPort.unregister();
  }, timeout: Timeout(Duration(seconds: 5)));

  testWidgets('Send message with local port', (WidgetTester tester) async {
    final localPort = await TizenMessageport.createLocalPort('test_port');
    Completer completer = Completer();
    localPort.register((message, [remotePort]) {
      if (completer.isCompleted) {
        print('WARNING: additional message received: $message');
      } else {
        completer.complete([message, remotePort]);
      }
    });
    final port = await TizenMessageport.connectToRemotePort(
        'com.example.messageport_tizen_example', 'test_port');
    port.sendWithLocalPort('Test message 2', localPort);
    final value = await completer.future;
    final message = value[0];
    final remotePort = value[1];
    expect(message, equals('Test message 2'));
    expect(remotePort is TizenRemotePort, true);
    expect(remotePort.remoteAppId,
        equals('com.example.messageport_tizen_example'));
    expect(remotePort.portName, equals('test_port'));
    await localPort.unregister();
  }, timeout: Timeout(Duration(seconds: 5)));

  group('Types test', () {
    TizenLocalPort localPort;
    TizenRemotePort remotePort;
    Completer completer;

    void onMessage(Object message, [TizenRemotePort remotePort]) {
      completer.complete([message, remotePort]);
    }

    setUpAll(() async {
      localPort = await TizenMessageport.createLocalPort('test_port');
      localPort.register(onMessage);
      remotePort = await TizenMessageport.connectToRemotePort(
          'com.example.messageport_tizen_example', 'test_port');
    });

    setUp(() {
      completer = Completer();
    });

    tearDownAll(() async {
      await localPort.unregister();
    });

    Future<void> checkForMessage<T>(Object message) async {
      remotePort.send(message);
      final ret = await completer.future;
      final receivedMessage = ret[0];
      final receivedPort = ret[1];
      expect(receivedMessage, message);
      expect(receivedMessage, isA<T>());
      expect(receivedPort, null);
    }

    testWidgets('bool', (WidgetTester tester) async {
      final bool value = true;
      await checkForMessage<bool>(value);
    }, timeout: Timeout(Duration(seconds: 5)));

    testWidgets('int', (WidgetTester tester) async {
      final int value = 834;
      await checkForMessage<int>(value);
    }, timeout: Timeout(Duration(seconds: 5)));

    testWidgets('double', (WidgetTester tester) async {
      final double value = 12.847;
      await checkForMessage<double>(value);
    }, timeout: Timeout(Duration(seconds: 5)));

    testWidgets('string', (WidgetTester tester) async {
      final String value = "Short string message";
      await checkForMessage<String>(value);
    }, timeout: Timeout(Duration(seconds: 5)));

    testWidgets('list', (WidgetTester tester) async {
      final List value = [1, 5, 8, 12, 0, 2];
      await checkForMessage<List>(value);
    }, timeout: Timeout(Duration(seconds: 5)));

    testWidgets('map', (WidgetTester tester) async {
      final Map<String, int> value = Map<String, int>();
      value['a'] = 5;
      value['b'] = 12;
      await checkForMessage<Map>(value);
    }, timeout: Timeout(Duration(seconds: 5)));
  });
}
