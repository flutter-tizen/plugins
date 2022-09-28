// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'package:flutter/services.dart';
import 'package:tizen_log/tizen_log.dart';

const String _logTag = 'RpcPortMethodChannel';

// ignore_for_file: public_member_api_docs
class MethodChannelRpcPort {
  static final MethodChannelRpcPort instance = MethodChannelRpcPort();

  final MethodChannel _channel = const MethodChannel('tizen/rpc_port');

  Stream<dynamic>? _stream;

  Future<Stream<dynamic>> proxyConnect(
      int proxyPtr, String appid, String portName) async {
    Log.info(_logTag,
        'connect. ptr: ${proxyPtr.toRadixString(16)}, $appid/$portName');
    if (_stream == null) {
      const EventChannel eventChannel = EventChannel('tizen/rpc_port_proxy');
      _stream = eventChannel.receiveBroadcastStream();
    }

    final Map<String, dynamic> args = <String, dynamic>{
      'handle': proxyPtr,
      'appid': appid,
      'portName': portName,
    };

    await _channel.invokeMethod<dynamic>('proxyConnect', args);
    return _stream!;
  }

  Future<Stream<dynamic>> stubListen(int stubPtr) async {
    Log.info(_logTag, 'listen. ptr: ${stubPtr.toRadixString(16)}');
    if (_stream == null) {
      const EventChannel eventChannel = EventChannel('tizen/rpc_port_stub');
      _stream = eventChannel.receiveBroadcastStream();
    }

    final Map<String, int> args = <String, int>{'handle': stubPtr};
    await _channel.invokeMethod<dynamic>('stubListen', args);
    return _stream!;
  }
}
