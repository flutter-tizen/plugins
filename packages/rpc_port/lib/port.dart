import 'dart:async';
import 'dart:typed_data';

import 'parcel.dart';
import 'rpc_port_platform_interface.dart';

String _logTag = "RPC_PORT";

enum PortType {
  main,
  callback,
}

abstract class Port {
  final PortType _portType;

  Port(this._portType);
  PortType get portType => _portType;

  Future<void> send(Parcel parcel);

  Future<Parcel> receive();

  Future<void> setPrivateSharing(String path);

  Future<void> setPrivateSharingList(List<String> paths);

  Future<void> unsetPrivateSharing();

  Future<void> disconnect();
}
