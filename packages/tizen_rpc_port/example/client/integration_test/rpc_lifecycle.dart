// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter_test/flutter_test.dart';
import 'package:integration_test/integration_test.dart';
import 'package:tizen_rpc_port/tizen_rpc_port.dart';

import 'package:tizen_rpc_port_client_example/message_client.dart' show Message;

// Manual pair test, not run by CI. Run in two terminals:
//   example/server $ flutter-tizen run -t integration_test/rpc_server.dart
//   example/client $ flutter-tizen test integration_test/rpc_lifecycle.dart
// Restart the server before re-running the client suite.
const String _appId = 'org.tizen.tizen_rpc_port_server_example';
const Duration _timeout = Duration(seconds: 10);

class _EchoProxy extends ProxyBase {
  _EchoProxy(String portName) : super(_appId, portName);

  late Completer<String> _reply;
  void Function(String)? replyHandler;

  Future<String> request(String value) {
    _reply = Completer<String>();
    (Parcel()..writeString(value)).send(getPort(PortType.main));
    return _reply.future.timeout(_timeout);
  }

  @override
  Future<void> onReceivedEvent(Parcel parcel) async {
    final String value = parcel.readString();
    if (replyHandler != null) {
      replyHandler!(value);
    } else {
      _reply.complete(value);
    }
  }
}

void main() {
  IntegrationTestWidgetsFlutterBinding.ensureInitialized();

  test('multiple stubs answer the first request and closing one preserves the other', () async {
    final errors = <Object>[];
    final a = _EchoProxy('LifecycleA');
    final b = _EchoProxy('LifecycleB');
    addTearDown(() => unawaited(a.disconnect()));
    addTearDown(() => unawaited(b.disconnect()));
    final disconnected = Completer<void>();
    await a
        .connect(
            onError: errors.add,
            onDisconnected: () async {
              disconnected.complete();
            })
        .timeout(_timeout);
    await b.connect(onError: errors.add).timeout(_timeout);
    expect(await a.request('first A'), 'first A');
    expect(await b.request('first B'), 'first B');
    expect(await b.request('close A'), 'closed');
    await disconnected.future.timeout(_timeout);
    expect(await b.request('still B'), 'still B');
    expect(await b.request('errors'), '0');
    expect(errors, isEmpty);
  });

  test('close does not wait for callbacks and can be awaited inside one', () async {
    final client = _EchoProxy('LifecycleClose');
    final control = _EchoProxy('LifecycleB');
    final selfClosing = _EchoProxy('LifecycleSelfClose');
    addTearDown(() => unawaited(client.disconnect()));
    addTearDown(() => unawaited(control.disconnect()));
    addTearDown(() => unawaited(selfClosing.disconnect()));
    final disconnected = Completer<void>();
    final selfDisconnected = Completer<void>();
    await client.connect(onDisconnected: () async => disconnected.complete()).timeout(_timeout);
    await control.connect().timeout(_timeout);
    (Parcel()..writeString('in flight')).send(client.getPort(PortType.main));
    expect(await control.request('close pending'), 'closed');
    await disconnected.future.timeout(_timeout);
    await selfClosing
        .connect(onDisconnected: () async => selfDisconnected.complete())
        .timeout(_timeout);
    (Parcel()..writeString('close self')).send(selfClosing.getPort(PortType.main));
    await selfDisconnected.future.timeout(_timeout);
    expect(await control.request('errors'), '0');
  });

  test('generated Message supports calls and callbacks', () async {
    final client = Message(_appId);
    final notification = Completer<String>();
    addTearDown(() => unawaited(client.disconnect()));
    await client.connect().timeout(_timeout);
    expect(
        client.register('client', (String sender, String message) {
          notification.complete('$sender:$message');
        }),
        0);
    expect(client.send('hello'), 0);
    expect(await notification.future.timeout(_timeout), 'client:hello');
    await client.unregister();
  });

  test('a stub request can await later input from the same client', () async {
    final client = _EchoProxy('LifecycleConcurrent');
    addTearDown(() => unawaited(client.disconnect()));
    final replies = <String>[];
    final completed = Completer<void>();
    client.replyHandler = (String value) {
      replies.add(value);
      if (replies.length == 2) {
        completed.complete();
      }
    };
    await client.connect().timeout(_timeout);
    for (final value in <String>['wait', 'release']) {
      (Parcel()..writeString(value)).send(client.getPort(PortType.main));
    }
    await completed.future.timeout(_timeout);
    expect(replies, unorderedEquals(<String>['wait', 'release']));
  });

  test('a waiting request does not block another client of the same stub', () async {
    final waiting = _EchoProxy('LifecycleClients');
    final other = _EchoProxy('LifecycleClients');
    addTearDown(() => unawaited(waiting.disconnect()));
    addTearDown(() => unawaited(other.disconnect()));
    await waiting.connect().timeout(_timeout);
    await other.connect().timeout(_timeout);
    final Future<String> response = waiting.request('wait');
    expect(await other.request('release'), 'release');
    expect(await response, 'wait');
  });

  test('client disconnection can cancel a pending server request', () async {
    final client = _EchoProxy('LifecycleCancel');
    final control = _EchoProxy('LifecycleB');
    addTearDown(() => unawaited(client.disconnect()));
    addTearDown(() => unawaited(control.disconnect()));
    await client.connect().timeout(_timeout);
    await control.connect().timeout(_timeout);
    (Parcel()..writeString('pending')).send(client.getPort(PortType.main));
    expect(await control.request('cancel started'), 'started');
    unawaited(client.disconnect());
    expect(await control.request('cancel done'), 'cancelled');
    expect(await control.request('errors'), '0');
  });
}
