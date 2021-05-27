// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';

const MethodChannel _channel = MethodChannel('tizen/messageport');

/// Signature for a callback receiving message on messageport.
///
/// This is used by [LocalPort.register].
typedef OnMessageReceived = Function(dynamic message, [RemotePort? remotePort]);

/// Local port to receive messages
class LocalPort {
  LocalPort._(this._id, this._portName, this._isTrusted);

  /// Register messageport and sets listener
  ///
  /// Names of already registered ports has to be unique.
  /// Exception will be thrown when using on already registered port
  void register(OnMessageReceived onMessage) {
    if (_registeredPorts.contains(portName)) {
      throw Exception('Port $portName is already registered');
    }

    _eventChannel = EventChannel('tizen/messageport$_id');
    _streamSubscription =
        _eventChannel.receiveBroadcastStream().listen((dynamic event) {
      if (event is Map) {
        final Map<dynamic, dynamic> map = event;
        final dynamic message = map['message'];
        if (map.containsKey('remotePort')) {
          final String remoteAppId = map['remoteAppId'] as String;
          final String remotePort = map['remotePort'] as String;
          final bool isTrusted = map['isTrusted'] as bool;
          onMessage(message, RemotePort._(remoteAppId, remotePort, isTrusted));
        } else {
          onMessage(message);
        }
      }
    });

    _registeredPorts.add(portName);
  }

  /// Unregisters messageport
  Future<void> unregister() async {
    await _streamSubscription.cancel();
    _registeredPorts.remove(portName);
  }

  /// Checks whether local port is trusted
  bool get trusted {
    return _isTrusted;
  }

  // Returns port name
  String get portName {
    return _portName;
  }

  /// Checks whether local port is registered
  bool get registered {
    return _registeredPorts.contains(_portName);
  }

  final int _id;
  final bool _isTrusted;
  final String _portName;
  late StreamSubscription<dynamic> _streamSubscription;
  late EventChannel _eventChannel;

  static final Set<String> _registeredPorts = <String>{};
}

/// Remote port to send messages
class RemotePort {
  RemotePort._(this._remoteAppId, this._portName, this._isTrusted);

  /// Sends message through remote messageport
  Future<void> send(dynamic message) async {
    final Map<String, dynamic> args = <String, dynamic>{};
    args['isTrusted'] = _isTrusted;
    args['remoteAppId'] = _remoteAppId;
    args['portName'] = _portName;
    args['message'] = message;

    return _channel.invokeMethod('send', args);
  }

  /// Sends message through remote messageport with local port
  ///
  /// Remote application can reply to the message by use of provided local port
  Future<void> sendWithLocalPort(dynamic message, LocalPort localPort) async {
    final Map<String, dynamic> args = <String, dynamic>{};
    args['isTrusted'] = _isTrusted;
    args['remoteAppId'] = _remoteAppId;
    args['portName'] = _portName;
    args['localPort'] = localPort._id;
    args['message'] = message;

    return _channel.invokeMethod('send', args);
  }

  /// Returns remote appId
  String get remoteAppId {
    return _remoteAppId;
  }

  /// Returns port name
  String get portName {
    return _portName;
  }

  /// Checks whether remote port is trusted
  bool get trusted {
    return _isTrusted;
  }

  final String _remoteAppId;
  final String _portName;
  final bool _isTrusted;
}

/// API for accessing MessagePorts in Tizen
class TizenMessagePort {
  /// Creates Local Port
  ///
  /// By default trusted local port is created. Set isTrusted to false,
  /// to change this behaviour.
  ///
  /// Remember to call `register()` on local port to enable receiving messages
  static Future<LocalPort> createLocalPort(String portName,
      {bool isTrusted = true}) async {
    final Map<String, dynamic> args = <String, dynamic>{};
    args['portName'] = portName;
    args['isTrusted'] = isTrusted;
    final int id = await _channel.invokeMethod<int>('createLocal', args) as int;
    return LocalPort._(id, portName, isTrusted);
  }

  /// Connects to `portName` remote port in `remoteAppId`
  ///
  /// Remote port has to be created and registered on client side first.
  ///
  /// Exception will be thrown if remote port does not exist.
  static Future<RemotePort> connectToRemotePort(
      String remoteAppId, String portName,
      {bool isTrusted = true}) async {
    final Map<String, dynamic> args = <String, dynamic>{};
    args['remoteAppId'] = remoteAppId;
    args['portName'] = portName;
    args['isTrusted'] = isTrusted;
    final bool check =
        await _channel.invokeMethod<bool>('createRemote', args) as bool;
    if (check) {
      return RemotePort._(remoteAppId, portName, isTrusted);
    }
    throw Exception('Remote port not found');
  }
}
