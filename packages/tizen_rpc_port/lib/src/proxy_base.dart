// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'dart:async';
import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';
import 'package:tizen_interop/6.5/tizen.dart';

import 'parcel.dart';
import 'port.dart';

export 'package:meta/meta.dart' show nonVirtual, visibleForOverriding;

/// A signature for callbacks to be invoked on stream errors.
typedef OnError = void Function(Object error);

/// A signature for callbacks to be invoked on disconnected events.
typedef OnDisconnected = Future<void> Function();

/// The base class for proxy objects used by RPC clients.
abstract class ProxyBase {
  /// Creates a [ProxyBase] instance with a server application ID and
  /// port name.
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

  final Finalizer<ProxyBase> _finalizer =
      Finalizer<ProxyBase>((ProxyBase proxy) {
    proxy._streamSubscription?.cancel();
    tizen.rpc_port_proxy_destroy(proxy._handle);
  });

  /// The server application ID.
  final String appid;

  /// A port name to use when connecting to the server.
  final String portName;

  bool _isConnected = false;

  /// Whether the proxy is connected to the server.
  bool get isConnected => _isConnected;

  Completer<void> _connectCompleter = Completer<void>();
  Completer<void> _disconnectCompleter = Completer<void>();

  static const EventChannel _eventChannel =
      EventChannel('tizen/rpc_port_proxy');

  StreamSubscription<dynamic>? _streamSubscription;
  OnDisconnected? _onDisconnected;

  /// Connects to the server.
  ///
  /// This internally creates an event [Stream] and manages until the proxy
  /// is disconnected from the server.
  ///
  /// The [onError] handler is invoked when a platform error is reported by
  /// the stream. If omitted, the error is considered unhandled.
  ///
  /// The [onDisconnected] handler is invoked when the proxy is disconnected
  /// from the server. If omitted, nothing happens.
  ///
  /// The following privileges are required to use this API.
  /// - `http://tizen.org/privilege/appmanager.launch`
  /// - `http://tizen.org/privilege/datasharing`
  Future<void> connect({
    OnError? onError,
    OnDisconnected? onDisconnected,
  }) async {
    if (_isConnected) {
      throw StateError('Proxy $appid/$portName already connected');
    }

    final Stream<dynamic> stream =
        _eventChannel.receiveBroadcastStream(<String, Object>{
      'handle': _handle.address,
      'appid': appid,
      'portName': portName,
    });
    _streamSubscription = stream.listen((dynamic map) async {
      final int handle = map['handle'] as int;
      if (handle != _handle.address) {
        return;
      }
      final String event = map['event'] as String;
      if (event == 'connected') {
        _isConnected = true;
        _onDisconnected = onDisconnected;
        await _onConnectedEvent();
      } else if (event == 'disconnected') {
        _isConnected = false;
        await _onDisconnectedEvent();
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
        print('Unknown event: $event');
      }
    }, onError: onError);

    return _connectCompleter.future;
  }

  /// Disconnects from the connected server.
  Future<void> disconnect() async {
    if (!_isConnected) {
      return;
    }

    getPort(PortType.main).disconnect();
    getPort(PortType.callback).disconnect();
    return _disconnectCompleter.future;
  }

  /// Gets a [Port] used to connect to the server.
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
    if (!_connectCompleter.isCompleted) {
      _connectCompleter.complete();
      _connectCompleter = Completer<void>();
    }
  }

  Future<void> _onRejectedEvent(String errorMessage) async {
    if (!_connectCompleter.isCompleted) {
      _connectCompleter.completeError(PlatformException(
        code: 'Connection rejected',
        message: errorMessage,
      ));
      _connectCompleter = Completer<void>();
    }
  }

  Future<void> _onDisconnectedEvent() async {
    await _onDisconnected?.call();
    _onDisconnected = null;

    getPort(PortType.main).disconnect();
    getPort(PortType.callback).disconnect();

    if (!_disconnectCompleter.isCompleted) {
      _disconnectCompleter.complete();
      _disconnectCompleter = Completer<void>();
    }
  }

  /// Called when data are received from the connected server.
  Future<void> onReceivedEvent(Parcel parcel);
}
