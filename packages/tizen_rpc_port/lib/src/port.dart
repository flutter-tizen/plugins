// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file

import 'dart:async';
import 'dart:typed_data';

import 'parcel.dart';
import 'rpc_port_method_channel.dart';

/// Enumeration for RPC port types.
enum PortType {
  /// Main channel to communicate.
  main,

  /// Sub channel for callbacks.
  callback,
}

/// The class that proxy and stub can use to communicate with each other.
class Port {
  /// Creates a port that represents a connection to a proxy.
  Port.fromProxy({
    required this.instance,
    required this.portName,
    required this.portType,
  }) : appid = null;

  /// Creates a port that represents a connection to a stub.
  Port.fromStub({
    required this.appid,
    required this.portName,
    required this.portType,
  }) : instance = null;

  static final MethodChannelRpcPort _methodChannel =
      MethodChannelRpcPort.instance;

  /// The type of this port.
  final PortType portType;

  /// The appid of the stub app. This member is used only proxy.
  final String? appid;

  /// The port name of the port connection.
  final String portName;

  /// The instance name of the proxy connection. This member is used only stub.
  final String? instance;

  /// Sends a parcel to the connected app.
  Future<void> send(Parcel parcel) async {
    await _methodChannel.portSend(this, parcel.asRaw());
  }

  /// Receives a parcel from connected app.
  /// This api should be used only guaranteed receive something after send().
  Future<Parcel> receive() async {
    final Uint8List raw = await _methodChannel.portReceive(this);
    return Parcel.fromRaw(raw);
  }

  /// Shares private files with other proxy applications.
  Future<void> shareFileList(List<String> paths) async {
    await _methodChannel.portSetPrivateSharingArray(this, paths);
  }

  /// Shares a private file with other proxy applications.
  Future<void> shareFile(String path) async {
    await _methodChannel.portSetPrivateSharing(this, path);
  }

  /// Unsets all shared private paths.
  Future<void> unshareFile() async {
    await _methodChannel.portUnsetPrivateSharing(this);
  }

  /// Disconnects the port.
  Future<void> disconnect() async {
    await _methodChannel.portDisconnect(this);
  }
}
