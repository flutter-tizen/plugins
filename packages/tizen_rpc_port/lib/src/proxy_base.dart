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

export 'package:meta/meta.dart' show nonVirtual, visibleForOverriding;

const String _logTag = 'RPC_PORT';

/// A signature for callbacks to be invoked on stream errors.
typedef OnError = void Function(Object error);

/// A signature for callbacks to be invoked on disconnected events.
typedef OnDisconnected = Future<void> Function();

/// The base class for creating a custom proxy object.
abstract class ProxyBase {
  /// Creates a [ProxyBase] instance with a stub application ID and port name.
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

  Completer<void> _connectCompleter = Completer<void>();
  Completer<void> _disconnectCompleter = Completer<void>();

  /// The target stub application ID.
  final String appid;

  /// A port name to use when connecting to the remote stub.
  final String portName;

  bool _isConnected = false;

  /// Whether the proxy is connected to the remote stub.
  bool get isConnected => _isConnected;

  StreamSubscription<Map<String, dynamic>>? _streamSubscription;
  OnDisconnected? _onDisconnected;

  Future<void> _connectInternal(OnError? onError) async {
    if (_isConnected) {
      // TODO(swift-kim): Immediately return without throwing an error when
      // already connected.
      throw StateError('Proxy $appid/$portName already connected.');
    }

    final Stream<Map<String, dynamic>> stream = await MethodChannelRpcPort
        .instance
        .proxyConnect(_handle.address, appid, portName);
    _streamSubscription = stream.listen((Map<String, dynamic> map) async {
      final int handle = map['handle'] as int;
      if (handle == _handle.address && map.containsKey('event')) {
        final String event = map['event'] as String;
        final String appid = map['receiver'] as String;
        final String portName = map['portName'] as String;
        Log.info(_logTag, 'event: $event, appid: $appid, portName: $portName');

        if (event == 'connected') {
          _isConnected = true;
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
          Log.error(_logTag, 'Unknown event: $event');
        }
      }
    }, onError: onError);
    return _connectCompleter.future;
  }

  /// Connects to the remote stub.
  ///
  /// If [onDisconnected] is provided, it is later called when this proxy is
  /// disconnected from the remote stub.
  ///
  /// The following privileges are required to use this API.
  /// - `http://tizen.org/privilege/appmanager.launch`
  /// - `http://tizen.org/privilege/datasharing`
  Future<void> connect({
    OnError? onError,
    OnDisconnected? onDisconnected,
  }) async {
    await _connectInternal(onError);
    _onDisconnected = onDisconnected;
  }

  /// Disconnects from the remote stub.
  Future<void> disconnect() async {
    if (!_isConnected) {
      throw StateError('Not connected.');
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

  /// Gets a [Port] associated with this proxy.
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
    if (!_disconnectCompleter.isCompleted) {
      _disconnectCompleter.complete();
      _disconnectCompleter = Completer<void>();
    }
  }

  /// Called when data are received from the remote stub.
  Future<void> onReceivedEvent(Parcel parcel);
}
