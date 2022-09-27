// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'dart:async';
import 'dart:typed_data';

import 'package:tizen_log/tizen_log.dart';
import 'parcel.dart';
import 'port.dart';
import 'rpc_port_method_channel.dart';

/// The abstract class for creating a stub class for RPC.
abstract class StubBase {
  /// The constructor for this class.
  StubBase(this.portName) {
    _methodChannel.stubCreate(portName);
    _finalizer.attach(this, this);
  }

  static const String _logTag = 'RPC_PORT';

  static final MethodChannelRpcPort _methodChannel =
      MethodChannelRpcPort.instance;

  final Finalizer<StubBase> _finalizer = Finalizer<StubBase>((StubBase stub) {
    stub._streamSubscription?.cancel();
    _methodChannel.stubDestroy(stub.portName);
  });

  /// The port name of the connection with the proxy.
  final String portName;

  /// ignore: cancel_subscriptions
  StreamSubscription<dynamic>? _streamSubscription;

  /// Sets a trusted proxy to the stub.
  Future<void> setTrusted(bool trusted) async {
    await _methodChannel.stubSetTrusted(portName, trusted);
  }

  /// Adds a privilege which is required when the proxy connects this stub.
  Future<void> addPrivilege(String privilege) async {
    await _methodChannel.stubAddPrivilege(portName, privilege);
  }

  /// Listens to the requests for connections.
  Future<void> listen() async {
    if (_streamSubscription != null) {
      return;
    }

    final Stream<dynamic> stream = await _methodChannel.stubListen(portName);
    _streamSubscription = stream.listen((dynamic event) async {
      if (event is Map) {
        final Map<dynamic, dynamic> map = event;
        final String eventName = map['event'] as String;
        final String sender = map['sender'] as String;
        final String instance = map['instance'] as String;

        Log.info(
            _logTag, 'event: $eventName, sender: $sender, instance: $instance');
        if (eventName == 'connected') {
          await onConnectedEvent(sender, instance);
        } else if (eventName == 'disconnected') {
          await onDisconnectedEvent(sender, instance);
        } else if (eventName == 'received') {
          final Uint8List rawData = map['rawData'] as Uint8List;
          final Parcel parcel = Parcel.fromRaw(rawData);
          await onReceivedEvent(sender, instance, parcel);
        } else {
          Log.error(_logTag, 'Unknown event; $eventName');
        }
      }
    });
  }

  /// Gets a port connected with proxy that has instance id.
  Port getPort(String instance, PortType portType) {
    return Port.fromProxy(
        portName: portName, instance: instance, portType: portType);
  }

  /// The abstract method for receiving connected event.
  Future<void> onConnectedEvent(String sender, String instance);

  /// The abstract method for receiving disconnected event.
  Future<void> onDisconnectedEvent(String sender, String instance);

  /// The abstract method called when the stub receives data from proxy.
  Future<void> onReceivedEvent(String sender, String instance, Parcel parcel);
}
