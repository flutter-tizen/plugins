// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';

final MethodChannel _channel = MethodChannel('tizen/messageport');

/// Signature for a callback receiving message on messageport.
///
/// This is used by [TizenLocalPort.register].
typedef OnMessageReceived = Function(Object message,
    [TizenRemotePort remotePort]);

/// Local port to receive messages
class TizenLocalPort {
  TizenLocalPort._(this._id, this._portName, this._isTrusted);

  /// Register messageport and sets listener
  ///
  /// Names of already registered ports has to be unique.
  /// Exception will be thrown when using on already registered port
  void register(OnMessageReceived onMessage) {
    if (_registeredPorts.contains(portName)) {
      throw Exception('Port $portName is already registered');
    }

    _eventChannel = EventChannel('tizen/messageport$_id');
    _streamSubscription = _eventChannel.receiveBroadcastStream().listen((map) {
      if (map is Map) {
        Object message = map['message'];
        if (map.containsKey('remotePort')) {
          String remoteAppId = map['remoteAppId'];
          String remotePort = map['remotePort'];
          bool isTrusted = map["isTrusted"];
          onMessage(
              message, TizenRemotePort._(remoteAppId, remotePort, isTrusted));
        } else {
          onMessage(message);
        }
      }
    });

    _registeredPorts.add(portName);
  }

  /// Unregisters messageport
  Future<void> unregister() async {
    await _streamSubscription?.cancel();
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
  bool _isRegistered = false;
  String _portName;
  StreamSubscription<Object> _streamSubscription;
  EventChannel _eventChannel;

  static Set<String> _registeredPorts = Set<String>();
}

/// Remote port to send messages
class TizenRemotePort {
  TizenRemotePort._(this._remoteAppId, this._portName, this._isTrusted);

  /// Sends message through remote messageport
  Future<void> send(Object message) async {
    Map<String, Object> args = new Map<String, Object>();
    args["isTrusted"] = _isTrusted;
    args["remoteAppId"] = _remoteAppId;
    args["portName"] = _portName;
    args["message"] = message;

    return _channel.invokeMethod("send", args);
  }

  /// Sends message through remote messageport with local port
  ///
  /// Remote application can reply to the message by use of provided local port
  Future<void> sendWithLocalPort(
      Object message, TizenLocalPort localPort) async {
    Map<String, Object> args = new Map<String, Object>();
    args["isTrusted"] = _isTrusted;
    args["remoteAppId"] = _remoteAppId;
    args["portName"] = _portName;
    args["localPort"] = localPort._id;
    args["message"] = message;

    return _channel.invokeMethod("send", args);
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
class TizenMessageport {
  /// Creates Local Port
  ///
  /// By default trusted local port is created. Set isTrusted to false,
  /// to change this behaviour.
  ///
  /// Remember to call `register()` on local port to enable receiving messages
  static Future<TizenLocalPort> createLocalPort(String portName,
      {bool isTrusted = true}) async {
    Map<String, Object> args = new Map<String, Object>();
    args['portName'] = portName;
    args['isTrusted'] = isTrusted;
    int id = await _channel.invokeMethod('createLocal', args);
    return TizenLocalPort._(id, portName, isTrusted);
  }

  /// Connects to `portName` remote port in `remoteAppId`
  ///
  /// Remote port has to be created and registered on client side first.
  ///
  /// Exception will be thrown if remote port does not exist.
  static Future<TizenRemotePort> connectToRemotePort(
      String remoteAppId, String portName,
      {bool isTrusted = true}) async {
    Map<String, Object> args = new Map<String, Object>();
    args['remoteAppId'] = remoteAppId;
    args['portName'] = portName;
    args['isTrusted'] = isTrusted;
    bool check = await _channel.invokeMethod('createRemote', args);
    if (check) {
      return TizenRemotePort._(remoteAppId, portName, isTrusted);
    }
    throw Exception('Remote port not found');
  }
}
