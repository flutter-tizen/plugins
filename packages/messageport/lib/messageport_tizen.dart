// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'src/messageport_manager.dart';

TizenMessagePortManager _manager = TizenMessagePortManager();

/// Signature for a callback receiving message on messageport.
///
/// This is used by [LocalPort.register].
typedef OnMessageReceived = Function(dynamic message, [RemotePort? remotePort]);

/// Local port to receive messages.
class LocalPort {
  LocalPort._(this.portName, this.trusted);

  /// Registers port and sets listener.
  ///
  /// Exception will be thrown when using on already registered port.
  void register(OnMessageReceived onMessage) {
    if (registered) {
      throw Exception('Port $portName is already registered');
    }

    final Stream<dynamic> stream = _manager.registerLocalPort(this);
    _streamSubscription = stream.listen((dynamic event) {
      if (event is Map) {
        final Map<dynamic, dynamic> map = event;
        final dynamic message = map['message'];
        if (map.containsKey('remotePort')) {
          final String remoteAppId = map['remoteAppId'] as String;
          final String remotePort = map['remotePort'] as String;
          final bool trusted = map['trusted'] as bool;
          onMessage(message, RemotePort._(remoteAppId, remotePort, trusted));
        } else {
          onMessage(message);
        }
      }
    });
    _registered = true;
  }

  /// Unregisters messageport. No operation for already unregistered port.
  Future<void> unregister() async {
    await _streamSubscription?.cancel();
    _registered = false;
  }

  /// Checks whether local port is trusted.
  final bool trusted;

  /// Returns port name.
  final String portName;

  /// Checks whether local port is registered.
  bool get registered {
    return _registered;
  }

  StreamSubscription<dynamic>? _streamSubscription;
  bool _registered = false;
}

/// Remote port to send messages.
class RemotePort {
  RemotePort._(this.remoteAppId, this.portName, this.trusted);

  /// Sends message through remote messageport.
  Future<void> send(dynamic message) async {
    return _manager.send(this, message);
  }

  /// Sends message through remote messageport with [localPort].
  ///
  /// Remote application can reply to the message by use of provided local port.
  Future<void> sendWithLocalPort(dynamic message, LocalPort localPort) async {
    return _manager.sendWithLocalPort(this, localPort, message);
  }

  // Checks whether remote port is registered in remote application.
  Future<bool> check() async {
    return _manager.checkForRemotePort(remoteAppId, portName, trusted);
  }

  /// Returns remote appId.
  final String remoteAppId;

  /// Returns port name.
  final String portName;

  /// Checks whether remote port is trusted.
  final bool trusted;
}

/// API for accessing MessagePorts in Tizen.
class TizenMessagePort {
  /// Creates Local Port
  ///
  /// By default trusted local port is created. Trusted port restricts communication
  /// through them to applications that share a signing certificate. Set [trusted] to false,
  /// to change this behaviour.
  ///
  /// Remember to call `register()` on local port to enable receiving messages.
  /// Multiple local ports with the same [portName] can be created and registered.
  /// Incoming messages will be delivered to all registered local ports with the same name.
  static Future<LocalPort> createLocalPort(String portName,
      {bool trusted = true}) async {
    await _manager.createLocalPort(portName, trusted);
    return LocalPort._(portName, trusted);
  }

  /// Connects to [portName] remote port in [remoteAppId].
  ///
  /// Corresponding local port has to be created and registered on the client side first.
  ///
  /// By default trusted remote port is created. Trusted port restricts communication
  /// through them to applications that share a signing certificate.
  /// Set [trusted] to false, to change this behaviour.
  ///
  /// Exception will be thrown if the remote port does not exist.
  static Future<RemotePort> connectToRemotePort(
      String remoteAppId, String portName,
      {bool trusted = true}) async {
    if (await _manager.checkForRemotePort(remoteAppId, portName, trusted)) {
      return RemotePort._(remoteAppId, portName, trusted);
    }
    throw Exception('Remote port not found');
  }
}
