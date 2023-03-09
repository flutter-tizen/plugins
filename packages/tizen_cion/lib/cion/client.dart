// Copyright 2022 - 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'dart:async';
import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';
import 'package:tizen_cion/common/callback.dart';
import 'package:tizen_cion/common/cion_method_channel.dart';
import 'package:tizen_cion/common/connection_result.dart';
import 'package:tizen_cion/common/payload.dart';
import 'package:tizen_cion/common/peer_info.dart';
import 'package:tizen_cion/common/security_info.dart';
import 'package:tizen_interop/6.5/tizen.dart';

/// Cion clinet class
class Client {
  /// The constructor for this class.
  Client(this.serviceName, {this.security}) {
    _handle = using((Arena arena) {
      final Pointer<cion_client_h> pHandle = arena();
      final int ret = tizen.cion_client_create(
          pHandle,
          serviceName.toNativeChar(),
          security?.handle ?? cion_security_h.fromAddress(0));
      if (ret != cion_error.CION_ERROR_NONE) {
        throw PlatformException(
          code: ret.toString(),
          message: tizen.get_error_message(ret).toDartString(),
        );
      }

      return pHandle.value;
    });

    _finalizer.attach(this, _handle, detach: this);
  }

  /// Connects with the server indicated by [peer].
  /// - [onDisconnected] is invoked when a disconnected event is received from the server.
  /// - [onReceived] is invoked when a payload is recieved from the server.
  ///
  /// The following privileges are required to use this API.
  /// - `http://tizen.org/privilege/internet`
  /// - `http://tizen.org/privilege/datasharing`
  Future<ConnectionResult> connect(PeerInfo peer,
      {OnDisconnected? onDisconnected, OnReceived? onReceived}) async {
    if (_isConnected || _connectCompleter.isCompleted) {
      throw Exception('Client $serviceName already connected to server');
    }

    _onDisconnected = onDisconnected;
    _onReceived = onReceived;
    print('connect');
    final CionMethodChannel instance = CionMethodChannel.instance;

    await instance.clientConnect(_handle.address, peer.handle.address);

    return _connectCompleter.future;
  }

  /// Disconnercts with the server.
  Future<void> disconnect() async {
    final int ret = tizen.cion_client_disconnect(_handle);
    if (ret != cion_error.CION_ERROR_NONE) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }

    return _disconnectCompleter.future;
  }

  /// Discovers listening server peers.
  /// - [onDiscovered] is invoked when the server peer info has been discovered.
  Future<void> tryDiscovery({required OnDiscovered onDiscovered}) async {
    if (_onDiscovered != null) {
      return;
    }

    _onDiscovered = onDiscovered;
    final CionMethodChannel instance = CionMethodChannel.instance;
    final Stream<dynamic> stream =
        await instance.clientTryDiscovery(_handle.address);
    print('tryDiscovery');
    _streamSubscription = stream.listen(_handleEvent);
  }

  /// Stops discovering server.
  void stopDiscovery() {
    if (_onDiscovered == null) {
      return;
    }

    final int ret = tizen.cion_client_stop_discovery(_handle);
    if (ret != cion_error.CION_ERROR_NONE) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }

    _onDiscovered = null;
  }

  /// Sends the payload to the server.
  void send(Payload payload) {
    final int ret = tizen.cion_client_send_payload_async(
        _handle, payload.handle, nullptr, nullptr);
    if (ret != cion_error.CION_ERROR_NONE) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Disposes self explicitly.
  void dispose() {
    _finalizer.detach(this);
    _streamSubscription?.cancel();
    _streamSubscription = null;
    tizen.cion_client_destroy(_handle);
  }

  Future<void> _handleEvent(dynamic event) async {
    if (event is Map) {
      final Map<dynamic, dynamic> map = event;
      if (map.containsKey('event')) {
        final int handle = map['handle'] as int;
        if (handle != _handle.address) {
          return;
        }

        final String event = map['event'] as String;
        final int peerHandle = map['peerHandle'] as int;
        final PeerInfo peer = PeerInfo.fromHandle(peerHandle);
        if (event == 'connectionResult') {
          final String reason = map['reason'] as String;
          final int statusVal = map['status'] as int;
          final ConnectionStatus status = ConnectionStatus.values
              .firstWhere((final ConnectionStatus e) => e.id == statusVal);
          return _onConnectionResultEvent(
              peer, ConnectionResult(reason, status));
        } else if (event == 'disconnected') {
          await _onDisconnectedEvent(peer);
          _streamSubscription?.cancel();
          _streamSubscription = null;
        } else if (event == 'received') {
          final int statusVal = map['status'] as int;
          final int typeVal = map['type'] as int;

          final PayloadTransferStatus status = PayloadTransferStatus.values
              .firstWhere((final PayloadTransferStatus e) => e.id == statusVal);
          final PayloadType type = PayloadType.values
              .firstWhere((final PayloadType e) => e.id == typeVal);
          late final Payload payload;
          switch (type) {
            case PayloadType.data:
              final Uint8List data = map['data'] as Uint8List;
              payload = DataPayload(data);
              break;
            case PayloadType.file:
              final String path = map['path'] as String;
              final int receivedBytes = map['receivedBytes'] as int;
              final int totalBytes = map['totalBytes'] as int;
              payload = FilePayload.internal(
                  filePath: path,
                  receivedBytes: receivedBytes,
                  totalBytes: totalBytes);
              break;
            default:
              throw Exception('Invalid payload Type: $type');
          }
          return _onReceivedEvent(peer, payload, status);
        } else if (event == 'discovered') {
          return _onDiscoveredEvent(peer);
        } else {
          throw Exception('Not supported event');
        }
      }
    }
  }

  Future<void> _onConnectionResultEvent(
      PeerInfo peer, ConnectionResult result) async {
    if (result.status == ConnectionStatus.ok) {
      _isConnected = true;
    } else {
      _isConnected = false;
    }

    if (_connectCompleter.isCompleted == false) {
      _connectCompleter.complete(result);
    }

    _connectCompleter = Completer<ConnectionResult>();
  }

  Future<void> _onDisconnectedEvent(PeerInfo peer) async {
    _isConnected = false;
    await _onDisconnected?.call(peer);
    _onDisconnected = null;
    _onReceived = null;

    if (_disconnectCompleter.isCompleted == false) {
      _disconnectCompleter.complete();
    }

    _disconnectCompleter = Completer<Void>();
  }

  Future<void> _onDiscoveredEvent(PeerInfo peer) async {
    return _onDiscovered?.call(peer);
  }

  Future<void> _onReceivedEvent(
      PeerInfo peer, Payload payload, PayloadTransferStatus status) async {
    return _onReceived?.call(peer, payload, status);
  }

  Completer<ConnectionResult> _connectCompleter = Completer<ConnectionResult>();
  Completer<dynamic> _disconnectCompleter = Completer<dynamic>();

  final Finalizer<cion_client_h> _finalizer =
      Finalizer<cion_client_h>((cion_client_h handle) {
    tizen.cion_client_destroy(handle);
  });

  /// The service name of the client.
  final String serviceName;

  /// The security info when cilent try to connect
  final SecurityInfo? security;

  bool _isConnected = false;

  OnDisconnected? _onDisconnected;
  OnReceived? _onReceived;
  OnDiscovered? _onDiscovered;

  StreamSubscription<dynamic>? _streamSubscription;
  late final cion_client_h _handle;
}
