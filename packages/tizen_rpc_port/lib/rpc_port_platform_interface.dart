import 'dart:typed_data';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'port.dart';
import 'proxy_base.dart';
import 'rpc_port_method_channel.dart';

abstract class RpcPortPlatform extends PlatformInterface {
  RpcPortPlatform() : super(token: _token);

  static final Object _token = Object();

  static RpcPortPlatform _instance = MethodChannelRpcPort();

  /// The default instance of [RpcPortPlatform] to use.
  ///
  /// Defaults to [MethodChannelRpcPort].
  static RpcPortPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [RpcPortPlatform] when
  /// they register themselves.
  static set instance(RpcPortPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<void> create(String portName) async {
    throw UnimplementedError('create() has not been implemented.');
  }

  Future<void> destroyStub(String portName) async {
    throw UnimplementedError('destroyStub() has not been implemented.');
  }

  Stream<dynamic> connect(ProxyBase proxy) {
    throw UnimplementedError('connect() has not been implemented.');
  }

  Future<void> destoryProxy(ProxyBase proxy) async {
    throw UnimplementedError('destoryProxy() has not been implemented.');
  }

  Stream<dynamic> connectSync(ProxyBase proxy) {
    throw UnimplementedError('connectSync() has not been implemented.');
  }

  Future<void> proxyDisconnect(Port port) async {
    throw UnimplementedError('proxyDisconnect() has not been implemented.');
  }

  Future<dynamic> send(Port port, Uint8List raw) async {
    throw UnimplementedError('proxySend() has not been implemented.');
  }

  Future<Uint8List> receive(Port port) async {
    throw UnimplementedError('proxyReceive() has not been implemented.');
  }

  Future<void> setPrivateSharingArray(Port port, List<String> paths) async {
    throw UnimplementedError(
        'proxySetPrivateSharingArray() has not been implemented.');
  }

  Future<void> setPrivateSharing(Port port, String path) async {
    throw UnimplementedError(
        'proxySetPrivateSharing() has not been implemented.');
  }

  Future<void> unsetPrivateSharing(Port port) async {
    throw UnimplementedError(
        'proxyUnsetPrivateSharing() has not been implemented.');
  }

  Future<void> disconnect(Port port) async {
    throw UnimplementedError('disconnect() has not been implemented.');
  }

  Future<void> setTrusted(String portName, bool trusted) async {
    throw UnimplementedError('setTrusted() has not been implemented.');
  }

  Future<void> addPrivilege(String portName, String privilege) async {
    throw UnimplementedError('addPrivilege() has not been implemented.');
  }

  Stream<dynamic> listen(String portName) {
    throw UnimplementedError('listen() has not been implemented.');
  }
}
