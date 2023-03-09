// Copyright 2022 - 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

// ignore_for_file: public_member_api_docs

import 'package:flutter/services.dart';
import 'payload.dart';
import 'peer_info.dart';

class CionMethodChannel {
  static final CionMethodChannel instance = CionMethodChannel();

  final MethodChannel _channel = const MethodChannel('tizen/cion');

  Stream<dynamic>? _clientStream;
  Stream<dynamic>? _serverStream;
  Stream<dynamic>? _groupStream;

  Future<void> clientConnect(int clientHandle, int peerHandle) async {
    final Map<String, int> args = <String, int>{
      'handle': clientHandle,
      'peerInfoHandle': peerHandle,
    };
    await _channel.invokeMethod<dynamic>('clientConnect', args);
  }

  Future<Stream<dynamic>> clientTryDiscovery(int clientHandle) async {
    if (_clientStream == null) {
      const EventChannel eventChannel = EventChannel('tizen/cion_client');
      _clientStream = eventChannel.receiveBroadcastStream();
    }
    final Map<String, dynamic> args = <String, dynamic>{
      'handle': clientHandle,
    };

    _channel.invokeMethod<void>('clientTryDiscovery', args);
    return _clientStream!;
  }

  Future<void> clientSendPayload(int clientHandle, Payload payload) async {
    final Map<String, dynamic> args = <String, dynamic>{
      'handle': clientHandle,
      'payloadHandle': payload.handle.address,
    };

    return _channel.invokeMethod<void>('clientSendPayload', args);
  }

  Future<Stream<dynamic>> serverListen(int serverHandle) async {
    if (_serverStream == null) {
      const EventChannel eventChannel = EventChannel('tizen/cion_server');
      _serverStream = eventChannel.receiveBroadcastStream();
    }

    final Map<String, dynamic> args = <String, dynamic>{
      'handle': serverHandle,
    };

    await _channel.invokeMethod<dynamic>('serverListen', args);
    return _serverStream!;
  }

  Future<void> serverSendPayload(
      int serverHandle, PeerInfo peer, Payload payload) async {
    final Map<String, dynamic> args = <String, dynamic>{
      'handle': serverHandle,
      'peerInfoHandle': peer.handle.address,
      'payloadHandle': payload.handle.address,
    };

    return _channel.invokeMethod<void>('serverSendPayload', args);
  }

  Future<Stream<dynamic>> groupSubscribe(int groupHandle) async {
    if (_groupStream == null) {
      const EventChannel eventChannel = EventChannel('tizen/cion_group');
      _groupStream = eventChannel.receiveBroadcastStream();
    }

    final Map<String, dynamic> args = <String, dynamic>{
      'handle': groupHandle,
    };

    await _channel.invokeMethod<void>('groupSubscribe', args);
    return _groupStream!;
  }
}
