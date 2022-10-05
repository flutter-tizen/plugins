// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'package:flutter/services.dart';

// ignore_for_file: public_member_api_docs

class MethodChannelRpcPort {
  static final MethodChannelRpcPort instance = MethodChannelRpcPort();

  final MethodChannel _channel = const MethodChannel('tizen/rpc_port');

  Stream<dynamic>? _proxyStream;
  Stream<dynamic>? _stubStream;

  Future<Stream<dynamic>> proxyConnect(
      int proxyPtr, String appid, String portName) async {
    if (_proxyStream == null) {
      const EventChannel eventChannel = EventChannel('tizen/rpc_port_proxy');
      _proxyStream = eventChannel.receiveBroadcastStream();
    }

    final Map<String, dynamic> args = <String, dynamic>{
      'handle': proxyPtr,
      'appid': appid,
      'portName': portName,
    };

    await _channel.invokeMethod<dynamic>('proxyConnect', args);
    return _proxyStream!;
  }

  Future<Stream<dynamic>> stubListen(int stubPtr) async {
    if (_stubStream == null) {
      const EventChannel eventChannel = EventChannel('tizen/rpc_port_stub');
      _stubStream = eventChannel.receiveBroadcastStream();
    }

    final Map<String, int> args = <String, int>{'handle': stubPtr};
    await _channel.invokeMethod<dynamic>('stubListen', args);
    return _stubStream!;
  }
}
