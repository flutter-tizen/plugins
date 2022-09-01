import 'dart:async';
import 'dart:typed_data';

import 'parcel.dart';
import 'rpc_port_platform_interface.dart';

String _logTag = "RPC_PORT";

enum PortType {
  main,
  callback,
}

class Port {
  final PortType portType;
  final String appid;
  final String portName;
  final String instance;

  Port.fromProxy(this.portName, this.instance, this.portType) : appid = "";
  Port.fromStub(this.appid, this.portName, this.portType) : instance = "";

  Future<void> send(Parcel parcel) async {
    final manager = RpcPortPlatform.instance;
    return manager.send(this, parcel.raw as Uint8List);
  }

  Future<Parcel> receive() async {
    final manager = RpcPortPlatform.instance;
    final raw = (await manager.receive(this));
    return Parcel.fromRaw(raw);
  }

  Future<void> setPrivateSharingList(List<String> paths) async {
    final manager = RpcPortPlatform.instance;
    await manager.setPrivateSharingArray(this, paths);
  }

  Future<void> setPrivateSharing(String path) async {
    final manager = RpcPortPlatform.instance;
    await manager.setPrivateSharing(this, path);
  }

  Future<void> unsetPrivateSharing() async {
    final manager = RpcPortPlatform.instance;
    await manager.unsetPrivateSharing(this);
  }

  Future<void> disconnect() async {
    final manager = RpcPortPlatform.instance;
    await manager.proxyDisconnect(this);
  }
}
