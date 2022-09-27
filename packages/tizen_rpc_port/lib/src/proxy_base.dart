// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'dart:async';
import 'dart:typed_data';

import 'package:tizen_log/tizen_log.dart';

import 'parcel.dart';
import 'port.dart';
import 'rpc_port_method_channel.dart';

/// The abstract class for creating a proxy class for RPC.
abstract class ProxyBase {
  /// The constructor for this class.
  ProxyBase(this.appid, this.portName) {
    _methodChannel.proxyCreate(appid, portName);
    _finalizer.attach(this, this);
  }

  static const String _logTag = 'RPC_PORT';

  static final MethodChannelRpcPort _methodChannel =
      MethodChannelRpcPort.instance;

  final Finalizer<ProxyBase> _finalizer =
      Finalizer<ProxyBase>((ProxyBase proxy) {
    _methodChannel.proxyDestroy(proxy);
  });

  /// The target stub application id.
  final String appid;

  /// The port name of the connection with the stub.
  final String portName;

  bool _isConnected = false;
  StreamSubscription<dynamic>? _streamSubscription;

  Future<void> _handleEvent(dynamic event) async {
    if (event is Map) {
      final Map<dynamic, dynamic> map = event;
      if (map.containsKey('event')) {
        final String event = map['event'] as String;
        final String appid = map['receiver'] as String;
        final String portName = map['portName'] as String;
        Log.info(_logTag, 'event: $event, appid:$appid, portName:$portName');
        if (event == 'connected') {
          _isConnected = true;
          await onConnectedEvent();
        } else if (event == 'disconnected') {
          _isConnected = false;
          await onDisconnectedEvent();
          _streamSubscription?.cancel();
          _streamSubscription = null;
        } else if (event == 'rejected') {
          _isConnected = false;
          final String error = map['error'] as String;
          await onRejectedEvent(error);
          _streamSubscription?.cancel();
          _streamSubscription = null;
        } else if (event == 'received') {
          final Uint8List rawData = map['rawData'] as Uint8List;
          final Parcel parcel = Parcel.fromRaw(rawData);
          await onReceivedEvent(parcel);
        } else {
          throw Exception('Not supported event');
        }
      }
    }
  }

  /// Connects to the stub.
  Future<void> connect() async {
    if (_isConnected) {
      throw Exception('Proxy $appid/$portName already connected to stub');
    }

    final Stream<dynamic> stream = await _methodChannel.proxyConnect(this);
    _streamSubscription = stream.listen(_handleEvent);
  }

  /// Gets a port.
  Port getPort(PortType portType) {
    return Port.fromStub(appid: appid, portName: portName, portType: portType);
  }

  /// The abstract method for receiving connected event.
  Future<void> onConnectedEvent();

  /// The abstract method for receiving disconnected event.
  Future<void> onDisconnectedEvent();

  /// The abstract method for receiving rejected event.
  Future<void> onRejectedEvent(String errorMessage);

  /// The abstract method called when the proxy receives data from stub.
  Future<void> onReceivedEvent(Parcel parcel);
}
