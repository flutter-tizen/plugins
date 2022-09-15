import 'dart:async';
import 'dart:typed_data';

import 'parcel.dart';
import 'rpc_port_platform_interface.dart';

/// The port type.
enum PortType {
  /// The main port type.
  main,

  /// The callback port type.
  callback,
}

/// The port connection between proxy & stub.
class Port {
  /// Create port from proxy.
  Port.fromProxy(this.portName, this.instance, this.portType) : appid = '';

  /// Create port from stub.
  Port.fromStub(this.appid, this.portName, this.portType) : instance = '';

  /// The type of port[main|callback].
  final PortType portType;

  /// The appid of stub app. This member is used only proxy.
  final String appid;

  /// The port name of port connection.
  final String portName;

  /// The instance name of proxy. This member is used only stub.
  final String instance;

  /// Sends a parcel to connected app.
  Future<void> send(Parcel parcel) async {
    final RpcPortPlatform manager = RpcPortPlatform.instance;
    await manager.send(this, parcel.raw);
  }

  /// Receives a parcel from connected app.
  /// This api should be used only guaranteed receive something after send().
  Future<Parcel> receive() async {
    final RpcPortPlatform manager = RpcPortPlatform.instance;
    final Uint8List raw = await manager.receive(this);
    return Parcel.fromRaw(raw);
  }

  /// Sets private sharing paths.
  Future<void> setPrivateSharingList(List<String> paths) async {
    final RpcPortPlatform manager = RpcPortPlatform.instance;
    await manager.setPrivateSharingArray(this, paths);
  }

  /// Sets private sharing a path.
  Future<void> setPrivateSharing(String path) async {
    final RpcPortPlatform manager = RpcPortPlatform.instance;
    await manager.setPrivateSharing(this, path);
  }

  /// Unsets all setted private sharing paths.
  Future<void> unsetPrivateSharing() async {
    final RpcPortPlatform manager = RpcPortPlatform.instance;
    await manager.unsetPrivateSharing(this);
  }

  /// Disconnects port connection.
  Future<void> disconnect() async {
    final RpcPortPlatform manager = RpcPortPlatform.instance;
    await manager.proxyDisconnect(this);
  }
}
