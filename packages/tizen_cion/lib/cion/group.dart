// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
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

/// Cion group class
class Group {
  /// The constructor for this class.
  Group(this.topicName, {this.security}) {
    _handle = using((Arena arena) {
      final Pointer<cion_group_h> pHandle = arena();
      final int ret = tizen.cion_group_create(pHandle, topicName.toNativeChar(),
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

  /// Subscribes the topic.
  /// - [onJoined] is invoked when a peer is joined to the topic.
  /// - [onLeft] is invoked when a peer is left from the topic.
  /// - [onReceived] is invoked when a payload is recieved.
  ///
  /// The following privileges are required to use this API.
  /// - `http://tizen.org/privilege/d2d.datasharing`
  /// - `http://tizen.org/privilege/internet`
  ///
  Future<void> subscribe(
      {required OnJoined onJoined,
      required OnLeft onLeft,
      required OnGroupReceived onReceived}) async {
    print('subscribe');
    _onJoined = onJoined;
    _onLeft = onLeft;
    _onReceived = onReceived;
    final CionMethodChannel instance = CionMethodChannel.instance;

    final Stream<dynamic> stream =
        await instance.groupSubscribe(_handle.address);

    print('subscribe');
    _streamSubscription = stream.listen(_handleEvent);
  }

  /// Unsubscribes the topic.
  void unsubscribe() {
    final int ret = tizen.cion_group_unsubscribe(_handle);
    if (ret != cion_error.CION_ERROR_NONE) {
      throw PlatformException(
        code: ret.toString(),
        message: tizen.get_error_message(ret).toDartString(),
      );
    }

    _onJoined = null;
    _onLeft = null;
    _onReceived = null;

    _streamSubscription?.cancel();
    _streamSubscription = null;
  }

  /// Publishes the payload to the topic.
  /// It's possible to publish only data type payload.
  Future<void> publish(Payload payload) async {
    final int ret = tizen.cion_group_publish(_handle, payload.handle);
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
    tizen.cion_group_destroy(_handle);
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

        if (event == 'joined') {
          return _onJoinedEvent(peer);
        } else if (event == 'left') {
          return _onLeftEvent(peer);
        } else if (event == 'received') {
          final Uint8List data = map['data'] as Uint8List;
          late final Payload payload = DataPayload(data);
          return _onReceivedEvent(peer, payload as DataPayload);
        } else {
          throw Exception('Not supported event');
        }
      }
    }
  }

  Future<void> _onJoinedEvent(PeerInfo peer) async {
    return _onJoined?.call(peer);
  }

  Future<void> _onLeftEvent(PeerInfo peer) async {
    return _onLeft?.call(peer);
  }

  Future<void> _onReceivedEvent(PeerInfo peer, DataPayload payload) async {
    return _onReceived?.call(peer, payload);
  }

  final Finalizer<cion_group_h> _finalizer =
      Finalizer<cion_group_h>((cion_group_h handle) {
    tizen.cion_group_destroy(handle);
  });

  /// The topic name of the group.
  final String topicName;

  /// The security info when cilent try to subscribe.
  final SecurityInfo? security;

  OnJoined? _onJoined;
  OnLeft? _onLeft;
  OnGroupReceived? _onReceived;

  StreamSubscription<dynamic>? _streamSubscription;
  late final cion_group_h _handle;
}
