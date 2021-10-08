// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'dart:async';

import 'package:flutter/services.dart';
import 'package:messageport_tizen/messageport_tizen.dart';

const MethodChannel _channel = MethodChannel('tizen/messageport');

class TizenMessagePortManager {
  TizenMessagePortManager();

  Future<void> createLocalPort(String portName, bool trusted) async {
    final Map<String, dynamic> args = <String, dynamic>{};
    args['portName'] = portName;
    args['trusted'] = trusted;
    return _channel.invokeMethod('createLocal', args);
  }

  Future<bool> checkForRemotePort(
      String remoteAppId, String portName, bool trusted) async {
    final Map<String, dynamic> args = <String, dynamic>{};
    args['remoteAppId'] = remoteAppId;
    args['portName'] = portName;
    args['trusted'] = trusted;
    final bool? status =
        await _channel.invokeMethod<bool>('checkForRemote', args);
    return status ?? false;
  }

  Future<void> send(RemotePort remotePort, dynamic message) async {
    final Map<String, dynamic> args = <String, dynamic>{};
    args['trusted'] = remotePort.trusted;
    args['remoteAppId'] = remotePort.remoteAppId;
    args['portName'] = remotePort.portName;
    args['message'] = message;

    return _channel.invokeMethod('send', args);
  }

  Future<void> sendWithLocalPort(
      RemotePort remotePort, LocalPort localPort, dynamic message) async {
    final Map<String, dynamic> args = <String, dynamic>{};
    args['trusted'] = remotePort.trusted;
    args['remoteAppId'] = remotePort.remoteAppId;
    args['portName'] = remotePort.portName;
    args['localPort'] = localPort.portName;
    args['localPortTrusted'] = localPort.trusted;
    args['message'] = message;

    return _channel.invokeMethod('send', args);
  }

  Stream<dynamic> registerLocalPort(LocalPort localPort) {
    if (localPort.trusted) {
      if (!_trustedLocalPorts.containsKey(localPort.portName)) {
        final EventChannel eventChannel =
            EventChannel('tizen/messageport/${localPort.portName}_trusted');
        _trustedLocalPorts[localPort.portName] =
            eventChannel.receiveBroadcastStream();
      }
      return _trustedLocalPorts[localPort.portName]!;
    }
    if (!_localPorts.containsKey(localPort.portName)) {
      final EventChannel eventChannel =
          EventChannel('tizen/messageport/${localPort.portName}');
      _localPorts[localPort.portName] = eventChannel.receiveBroadcastStream();
    }

    return _localPorts[localPort.portName]!;
  }

  final Map<String, Stream<dynamic>> _localPorts = <String, Stream<dynamic>>{};
  final Map<String, Stream<dynamic>> _trustedLocalPorts =
      <String, Stream<dynamic>>{};
}
