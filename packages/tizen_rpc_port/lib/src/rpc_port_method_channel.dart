// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

// ignore_for_file: public_member_api_docs

import 'package:flutter/services.dart';

class MethodChannelRpcPort {
  static final MethodChannelRpcPort instance = MethodChannelRpcPort();

  static const MethodChannel _channel = MethodChannel('tizen/rpc_port');

  static const EventChannel _proxyEventChannel =
      EventChannel('tizen/rpc_port_proxy');
  static const EventChannel _stubEventChannel =
      EventChannel('tizen/rpc_port_stub');

  static final Stream<Map<String, dynamic>> _proxyEvents = _proxyEventChannel
      .receiveBroadcastStream()
      .map((dynamic event) =>
          Map<String, dynamic>.from(event as Map<dynamic, dynamic>));
  static final Stream<Map<String, dynamic>> _stubEvents = _stubEventChannel
      .receiveBroadcastStream()
      .map((dynamic event) =>
          Map<String, dynamic>.from(event as Map<dynamic, dynamic>));

  Future<Stream<Map<String, dynamic>>> proxyConnect(
      int handle, String appid, String portName) async {
    await _channel.invokeMethod<void>(
      'proxyConnect',
      <String, Object>{
        'handle': handle,
        'appid': appid,
        'portName': portName,
      },
    );
    return _proxyEvents;
  }

  Future<Stream<Map<String, dynamic>>> stubListen(int handle) async {
    await _channel.invokeMethod<void>(
      'stubListen',
      <String, int>{'handle': handle},
    );
    return _stubEvents;
  }
}
