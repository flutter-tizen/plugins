// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/widgets.dart';
import 'package:tizen_rpc_port/tizen_rpc_port.dart';

import 'package:tizen_rpc_port_server_example/main.dart' show EchoService;
import 'package:tizen_rpc_port_server_example/message_server.dart' show Message;

class _EchoStub extends StubBase {
  _EchoStub(super.portName, {this.handleRequest, this.handleDisconnect});

  final Future<String> Function(String)? handleRequest;
  final void Function()? handleDisconnect;
  final Set<String> _clients = <String>{};

  @override
  Future<void> onConnectedEvent(String sender, String instance) async {
    _clients.add(instance);
  }

  @override
  Future<void> onDisconnectedEvent(String sender, String instance) async {
    _clients.remove(instance);
    handleDisconnect?.call();
  }

  @override
  Future<void> onReceivedEvent(String sender, String instance, Parcel parcel) async {
    if (!_clients.contains(instance)) {
      throw StateError('Request arrived without a connection');
    }
    final String request = parcel.readString();
    if (request == 'close self') {
      await close();
      return;
    }
    final String response = await handleRequest?.call(request) ?? request;
    if (_clients.contains(instance)) {
      (Parcel()..writeString(response)).send(getPort(instance, PortType.callback));
    }
  }
}

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  final errors = <Object>[];
  final a = _EchoStub('LifecycleA');
  final started = Completer<void>();
  final cancelStarted = Completer<void>();
  final cancelRelease = Completer<void>();
  final cancelled = Completer<void>();
  final pending = _EchoStub('LifecycleClose', handleRequest: (String request) async {
    started.complete();
    await Completer<void>().future;
    return request;
  });
  final b = _EchoStub('LifecycleB', handleRequest: (String request) async {
    if (request == 'close A') {
      await a.close();
      await a.close();
      return 'closed';
    }
    if (request == 'close pending') {
      await started.future;
      await pending.close();
      return 'closed';
    }
    if (request == 'cancel started') {
      await cancelStarted.future;
      return 'started';
    }
    if (request == 'cancel done') {
      await cancelled.future;
      return 'cancelled';
    }
    return request == 'errors' ? '${errors.length}' : request;
  });
  for (final stub in <StubBase>[
    a,
    b,
    pending,
    _EchoStub('LifecycleSelfClose'),
    _EchoStub('LifecycleCancel', handleRequest: (String request) async {
      cancelStarted.complete();
      await cancelRelease.future;
      cancelled.complete();
      return request;
    }, handleDisconnect: cancelRelease.complete),
    Message(serviceBuilder: EchoService.new),
  ]) {
    await stub.listen(onError: errors.add);
  }
  for (final name in <String>['LifecycleConcurrent', 'LifecycleClients']) {
    final started = Completer<void>();
    final release = Completer<void>();
    await _EchoStub(name, handleRequest: (String request) async {
      if (request == 'wait') {
        started.complete();
        await release.future;
      } else if (request == 'release') {
        await started.future;
        release.complete();
      }
      return request;
    }).listen(onError: errors.add);
  }
  runApp(const Directionality(
    textDirection: TextDirection.ltr,
    child: Center(child: Text('RPC lifecycle test server ready')),
  ));
}
