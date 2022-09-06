import 'dart:typed_data';

import 'package:rpc_port/rpc_port.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:rpc_port/rpc_port_platform_interface.dart';
import 'package:rpc_port/rpc_port_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockRpcPortPlatform
    with MockPlatformInterfaceMixin
    implements RpcPortPlatform {
  @override
  Future<String?> getPlatformVersion() => Future.value('42');

  @override
  Future<void> create(String portName) async {
    return;
  }

  @override
  Future<void> destroy(String portName) async {
    return;
  }

  @override
  Stream<dynamic> connect(ProxyBase proxy) => const Stream.empty();

  @override
  Stream<dynamic> connectSync(ProxyBase proxy) => const Stream.empty();

  @override
  Future<void> proxyDisconnect(Port port) async {
    return;
  }

  @override
  Future<dynamic> send(Port port, Uint8List raw) async {
    throw UnimplementedError('proxySend() has not been implemented.');
  }

  @override
  Future<Uint8List> receive(Port port) async {
    throw UnimplementedError('proxyReceive() has not been implemented.');
  }

  @override
  Future<void> setPrivateSharingArray(Port port, List<String> paths) async {
    throw UnimplementedError(
        'proxySetPrivateSharingArray() has not been implemented.');
  }

  @override
  Future<void> setPrivateSharing(Port port, String path) async {
    throw UnimplementedError(
        'proxySetPrivateSharing() has not been implemented.');
  }

  @override
  Future<void> unsetPrivateSharing(Port port) async {
    throw UnimplementedError(
        'proxyUnsetPrivateSharing() has not been implemented.');
  }

  @override
  Future<void> disconnect(Port port) async {
    throw UnimplementedError('disconnect() has not been implemented.');
  }

  @override
  Future<void> setTrusted(String portName, bool trusted) async {
    throw UnimplementedError('setTrusted() has not been implemented.');
  }

  @override
  Future<void> addPrivilege(String portName, String privilege) async {
    throw UnimplementedError('addPrivilege() has not been implemented.');
  }

  @override
  Stream<dynamic> listen(String portName) {
    throw UnimplementedError('listen() has not been implemented.');
  }
}

void main() {
  final RpcPortPlatform initialPlatform = RpcPortPlatform.instance;

  test('$MethodChannelRpcPort is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelRpcPort>());
  });

  test('getPlatformVersion', () async {
    Port rpcPortPlugin = Port();
    MockRpcPortPlatform fakePlatform = MockRpcPortPlatform();
    RpcPortPlatform.instance = fakePlatform;

    expect(await rpcPortPlugin.getPlatformVersion(), '42');
  });
}
