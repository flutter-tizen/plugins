import 'dart:async';
import 'dart:typed_data';

import 'parcel.dart';
import 'rpc_port_method_channel.dart';

/// The port type.
enum PortType {
  /// The main port type.
  main,

  /// The callback port type.
  callback,
}

/// The port connection between proxy & stub.
class Port {
  /// Creates a port that represents a connection to a proxy.
  Port.fromProxy({
    required this.instance,
    required this.portName,
    required this.portType,
  }) : appid = '';

  /// Creates a port that represents a connection to a stub.
  Port.fromStub({
    required this.appid,
    required this.portName,
    required this.portType,
  }) : instance = '';

  static final MethodChannelRpcPort _methodChannel =
      MethodChannelRpcPort.instance;

  /// The type of port.
  final PortType portType;

  /// The appid of stub app. This member is used only proxy.
  final String appid;

  /// The port name of port connection.
  final String portName;

  /// The instance name of proxy. This member is used only stub.
  final String instance;

  /// Sends a parcel to connected app.
  Future<void> send(Parcel parcel) async {
    await _methodChannel.portSend(this, parcel.raw);
  }

  /// Receives a parcel from connected app.
  /// This api should be used only guaranteed receive something after send().
  Future<Parcel> receive() async {
    final Uint8List raw = await _methodChannel.portReceive(this);
    return Parcel.fromRaw(raw);
  }

  /// Sets private sharing paths.
  Future<void> setPrivateSharingList(List<String> paths) async {
    await _methodChannel.portSetPrivateSharingArray(this, paths);
  }

  /// Sets private sharing a path.
  Future<void> setPrivateSharing(String path) async {
    await _methodChannel.portSetPrivateSharing(this, path);
  }

  /// Unsets all setted private sharing paths.
  Future<void> unsetPrivateSharing() async {
    await _methodChannel.portUnsetPrivateSharing(this);
  }

  /// Disconnects port connection.
  Future<void> disconnect() async {
    await _methodChannel.portDisconnect(this);
  }
}
