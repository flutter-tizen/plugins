// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'src/messageport_manager.dart';

TizenMessagePortManager _manager = TizenMessagePortManager();

/// Called when a message is received on message port.
///
/// This is used by [LocalPort.register].
typedef OnMessageReceived = Function(dynamic message, [RemotePort? remotePort]);

/// Local message port for receiving messages.
class LocalPort {
  LocalPort._(this.portName, this.trusted);

  /// Creates a local port.
  ///
  /// By default a trusted local port is created. Trusted message ports
  /// restrict communication through them to applications that share a signing
  /// certificate. Set [trusted] to false to change this behaviour.
  ///
  /// Remember to call [register] on the created port to enable receiving
  /// messages.
  ///
  /// Multiple local ports with the same [portName] can be created and
  /// registered. Incoming messages will be delivered to all registered local
  /// ports with the same name.
  static Future<LocalPort> create(
    String portName, {
    bool trusted = true,
  }) async {
    await _manager.createLocalPort(portName, trusted);
    return LocalPort._(portName, trusted);
  }

  /// Registers the local port and sets a listener.
  ///
  /// An [Exception] is thrown if the port is already registered.
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

  /// Unregisters the local port. No operation for already unregistered port.
  Future<void> unregister() async {
    await _streamSubscription?.cancel();
    _registered = false;
  }

  /// The local port name.
  final String portName;

  /// Whether the local port is trusted.
  final bool trusted;

  /// Whether the local port is registered.
  bool get registered {
    return _registered;
  }

  StreamSubscription<dynamic>? _streamSubscription;
  bool _registered = false;
}

/// Remote message port for sending messages.
class RemotePort {
  RemotePort._(this.remoteAppId, this.portName, this.trusted);

  /// Connects to a remote port named [portName].
  ///
  /// The corresponding local port should first be created and registered by
  /// the remote app [remoteAppId].
  ///
  /// By default a trusted remote port is created. Trusted message ports
  /// restrict communication through them to applications that share a signing
  /// certificate. Set [trusted] to false to change this behaviour.
  ///
  /// An [Exception] is thrown if the remote port does not exist.
  static Future<RemotePort> connect(
    String remoteAppId,
    String portName, {
    bool trusted = true,
  }) async {
    if (await _manager.checkForRemotePort(remoteAppId, portName, trusted)) {
      return RemotePort._(remoteAppId, portName, trusted);
    }
    throw Exception('Remote port not found');
  }

  /// Sends a message through the remote port.
  Future<void> send(dynamic message) async {
    return _manager.send(this, message);
  }

  /// Sends a message through the remote port with [localPort] information.
  ///
  /// The remote app can reply to the message through the provided [localPort].
  Future<void> sendWithLocalPort(dynamic message, LocalPort localPort) async {
    return _manager.sendWithLocalPort(this, localPort, message);
  }

  /// Checks whether the remote port is registered by the remote app.
  Future<bool> check() async {
    return _manager.checkForRemotePort(remoteAppId, portName, trusted);
  }

  /// The remote application ID.
  final String remoteAppId;

  /// The remote port name.
  final String portName;

  /// Whether the remote port is trusted.
  final bool trusted;
}

// ignore: avoid_classes_with_only_static_members
/// API for accessing MessagePorts in Tizen.
class TizenMessagePort {
  /// Creates a local port.
  @Deprecated('Use LocalPort.create instead.')
  static Future<LocalPort> createLocalPort(
    String portName, {
    bool trusted = true,
  }) async {
    return LocalPort.create(portName, trusted: trusted);
  }

  /// Connects to a remote port named [portName].
  @Deprecated('Use RemotePort.connect instead.')
  static Future<RemotePort> connectToRemotePort(
    String remoteAppId,
    String portName, {
    bool trusted = true,
  }) async {
    return RemotePort.connect(remoteAppId, portName, trusted: trusted);
  }
}
