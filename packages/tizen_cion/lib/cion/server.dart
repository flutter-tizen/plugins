// Copyright 2022 - 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'dart:async';
import 'dart:ffi';

import 'package:ffi/ffi.dart';
import 'package:flutter/services.dart';
import 'package:tizen_cion/common/callback.dart';
import 'package:tizen_cion/common/cion_method_channel.dart';
import 'package:tizen_cion/common/payload.dart';
import 'package:tizen_cion/common/peer_info.dart';
import 'package:tizen_cion/common/security_info.dart';
import 'package:tizen_interop/6.5/tizen.dart';

/// Cion server class
class Server {
  /// The constructor for this class.
  Server(this.serviceName, this.displayName, {this.security}) {
    _handle = using((Arena arena) {
      final Pointer<cion_server_h> pHandle = arena();
      final int ret = tizen.cion_server_create(
          pHandle,
          serviceName.toNativeChar(),
          displayName.toNativeChar(),
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

  /// Listens to the requests for connections.
  /// - [onConnectionRequest] is invoked when connection request is received from the client.
  /// - [onDisconnected] is invoked when a disconnected event is received from the client.
  /// - [onReceived] is invoked when a payload is recieved from the client.
  Future<void> listen(
      {required OnConnectionRequest onConnectionRequest,
      required OnDisconnected onDisconnected,
      required OnReceived onReceived}) async {
    _onConnectionRequest = onConnectionRequest;
    _onDisconnected = onDisconnected;
    _onReceived = onReceived;

    final CionMethodChannel instance = CionMethodChannel.instance;
    final Stream<dynamic> stream = await instance.serverListen(_handle.address);
    _streamSubscription = stream.listen(_handleEvent);
  }

  /// Stops listening the connection request.
  Future<void> stop() async {
    final int ret = tizen.cion_server_stop(_handle);
    if (ret != cion_error.CION_ERROR_NONE) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }

    await _streamSubscription?.cancel();
    _streamSubscription = null;
  }

  /// Disconnercts with the client indicated by [peer].
  void disconnect(PeerInfo peer) {
    final int ret = tizen.cion_server_disconnect(_handle, peer.handle);
    if (ret != cion_error.CION_ERROR_NONE) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Sends the [payload] to the client indicated by [peer].
  void send(PeerInfo peer, Payload payload) {
    final int ret = tizen.cion_server_send_payload_async(
        _handle, peer.handle, payload.handle, nullptr, nullptr);
    if (ret != cion_error.CION_ERROR_NONE) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  static List<PeerInfo> _result = List<PeerInfo>.empty();

  static bool _getConnectedPeerList(
      cion_peer_info_h peerHandle, Pointer<Void> _) {
    _result.add(PeerInfo.clone(
        PeerInfo.fromHandle(peerHandle.address, managed: false)));
    return true;
  }

  /// Gets connected peer info list.
  List<PeerInfo> getConnectedPeerList() {
    _result = List<PeerInfo>.empty(growable: true);
    final cion_server_connected_peer_info_cb pCallback =
        cion_server_connected_peer_info_cb
            .fromFunction<Bool Function(cion_peer_info_h, Pointer<Void>)>(
                _getConnectedPeerList, false);

    final int ret = tizen.cion_server_foreach_connected_peer_info(
        _handle, pCallback, Pointer<Void>.fromAddress(0));
    if (ret != cion_error.CION_ERROR_NONE) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }

    return _result;
  }

  /// Sends [payload] to every connected clients.
  void sendAll(Payload payload) {
    final List<PeerInfo> payloadList = getConnectedPeerList();
    for (final PeerInfo peer in payloadList) {
      send(peer, payload);
    }
  }

  /// Accepts a connection request indicated by [peer].
  void accept(PeerInfo peer) {
    final int ret = tizen.cion_server_accept(_handle, peer.handle);
    if (ret != cion_error.CION_ERROR_NONE) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }
  }

  /// Rejects a connection request indicated by [peer].
  void reject(PeerInfo peer, String reason) {
    final int ret =
        tizen.cion_server_reject(_handle, peer.handle, reason.toNativeChar());
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
        if (event == 'connectionRequest') {
          return _onConnectionRequestEvent(peer);
        } else if (event == 'disconnected') {
          await _onDisconnectedEvent(peer);
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
        } else {
          throw Exception('Not supported event');
        }
      }
    }
  }

  Future<void> _onConnectionRequestEvent(PeerInfo peer) async {
    return _onConnectionRequest?.call(peer);
  }

  Future<void> _onDisconnectedEvent(PeerInfo peer) async {
    return _onDisconnected?.call(peer);
  }

  Future<void> _onReceivedEvent(
      PeerInfo peer, Payload payload, PayloadTransferStatus status) async {
    return _onReceived?.call(peer, payload, status);
  }

  final Finalizer<cion_server_h> _finalizer =
      Finalizer<cion_server_h>((cion_server_h handle) {
    tizen.cion_server_destroy(handle);
  });

  /// The service name of the server.
  final String serviceName;

  /// The display name of the server.
  final String displayName;

  /// The security info when cilent try to connect
  final SecurityInfo? security;

  OnConnectionRequest? _onConnectionRequest;
  OnDisconnected? _onDisconnected;
  OnReceived? _onReceived;

  StreamSubscription<dynamic>? _streamSubscription;
  late final cion_server_h _handle;
}
