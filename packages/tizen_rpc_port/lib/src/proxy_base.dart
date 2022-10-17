// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'dart:async';
import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';
import 'package:tizen_interop/6.5/tizen.dart';
import 'package:tizen_log/tizen_log.dart';

import 'parcel.dart';
import 'port.dart';
import 'rpc_port_method_channel.dart';

export 'package:meta/meta.dart' show nonVirtual;

/// The method type for receiving disconnected event.
typedef OnDisconnected = Future<void> Function();

/// The abstract class for creating a proxy class for RPC.
abstract class ProxyBase {
  /// The constructor for this class.
  ProxyBase(this.appid, this.portName) {
    _handle = using((Arena arena) {
      final Pointer<rpc_port_proxy_h> pProxy = arena();
      final int ret = tizen.rpc_port_proxy_create(pProxy);
      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return pProxy.value;
    });

    _finalizer.attach(this, this);
  }

  late final rpc_port_proxy_h _handle;

  static const String _logTag = 'RPC_PORT';

  static final MethodChannelRpcPort _methodChannel =
      MethodChannelRpcPort.instance;

  final Finalizer<ProxyBase> _finalizer =
      Finalizer<ProxyBase>((ProxyBase proxy) {
    proxy._streamSubscription?.cancel();
    tizen.rpc_port_proxy_destroy(proxy._handle);
  });

  final Completer<dynamic> _connectCompleter = Completer<dynamic>();
  final Completer<dynamic> _disconnectCompleter = Completer<dynamic>();

  /// The target stub application id.
  final String appid;

  /// The port name of the connection with the stub.
  final String portName;

  bool _isConnected = false;

  /// The flag of whether the port is connected.
  bool get isConnected => _isConnected;

  StreamSubscription<dynamic>? _streamSubscription;
  OnDisconnected? _onDisconnected;

  Future<void> _handleEvent(dynamic event) async {
    if (event is Map) {
      final Map<dynamic, dynamic> map = event;
      final int handle = map['handle'] as int;
      if (handle == _handle.address && map.containsKey('event')) {
        final String event = map['event'] as String;
        final String appid = map['receiver'] as String;
        final String portName = map['portName'] as String;
        Log.info(_logTag, 'event: $event, appid:$appid, portName:$portName');
        if (event == 'connected') {
          _isConnected = true;
          await _onConnectedEvent();
        } else if (event == 'disconnected') {
          _isConnected = false;
          await onDisconnectedEvent();
          _streamSubscription?.cancel();
          _streamSubscription = null;
        } else if (event == 'rejected') {
          _isConnected = false;
          final String error = map['error'] as String;
          await _onRejectedEvent(error);
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

  Future<void> _connectInternal() async {
    if (_isConnected) {
      throw Exception('Proxy $appid/$portName already connected to stub');
    }

    final Stream<dynamic> stream =
        await _methodChannel.proxyConnect(_handle.address, appid, portName);
    _streamSubscription = stream.listen(_handleEvent);
    return _connectCompleter.future;
  }

  /// Connects to the stub asynchronously.
  ///
  /// The following privileges are required to use this API.
  /// - `http://tizen.org/privilege/appmanager.launch`
  /// - `http://tizen.org/privilege/datasharing`
  Future<void> connect({OnDisconnected? onDisconnected}) async {
    await _connectInternal();
    _onDisconnected = onDisconnected;
  }

  /// Disconnect to the stub.
  Future<void> disconnect() async {
    if (!_isConnected) {
      throw Exception('Not connected');
    }

    final Port port = getPort(PortType.main);
    final int ret = tizen.rpc_port_disconnect(port.handle);
    if (ret != 0) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }

    return _disconnectCompleter.future;
  }

  /// Gets a port.
  Port getPort(PortType portType) {
    return using((Arena arena) {
      final Pointer<rpc_port_h> pPort = arena();
      final int ret =
          tizen.rpc_port_proxy_get_port(_handle, portType.index, pPort);

      if (ret != 0) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return Port.fromNativeHandle(pPort.value);
    });
  }

  Future<void> _onConnectedEvent() async {
    _connectCompleter.complete();
  }

  Future<void> _onRejectedEvent(String errorMessage) async {
    _connectCompleter.completeError(errorMessage);
  }

  /// The method for receiving disconnected event.
  Future<void> onDisconnectedEvent() async {
    await _onDisconnected?.call();
    if (_disconnectCompleter.isCompleted == false) {
      _disconnectCompleter.complete();
    }
  }

  /// The abstract method called when the proxy receives data from stub.
  Future<void> onReceivedEvent(Parcel parcel);
}
