import 'dart:typed_data';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import "proxy_base.dart";
import "stub_base.dart";
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

  Future<void> destroy(String portName) async {
    throw UnimplementedError('destroy() has not been implemented.');
  }

  Stream<dynamic> connect(ProxyBase proxy) {
    throw UnimplementedError('connect() has not been implemented.');
  }

  Stream<dynamic> connectSync(ProxyBase proxy) {
    throw UnimplementedError('connectSync() has not been implemented.');
  }

  Future<void> proxyDisconnect(ProxyPort port) async {
    throw UnimplementedError('proxyDisconnect() has not been implemented.');
  }

  Future<dynamic> proxySend(ProxyPort port, Uint8List raw) async {
    throw UnimplementedError('proxySend() has not been implemented.');
  }

  Future<Uint8List> proxyReceive(ProxyPort port) async {
    throw UnimplementedError('proxyReceive() has not been implemented.');
  }

  Future<void> proxySetPrivateSharingArray(
      ProxyPort port, List<String> paths) async {
    throw UnimplementedError(
        'proxySetPrivateSharingArray() has not been implemented.');
  }

  Future<void> proxySetPrivateSharing(ProxyPort port, String path) async {
    throw UnimplementedError(
        'proxySetPrivateSharing() has not been implemented.');
  }

  Future<void> proxyUnsetPrivateSharing(ProxyPort port) async {
    throw UnimplementedError(
        'proxyUnsetPrivateSharing() has not been implemented.');
  }

  Future<void> stubDisconnect(StubPort port) async {
    throw UnimplementedError('disconnect() has not been implemented.');
  }

  Future<dynamic> stubSend(StubPort port, Uint8List raw) async {
    throw UnimplementedError('send() has not been implemented.');
  }

  Future<Uint8List> stubReceive(StubPort port) async {
    throw UnimplementedError('receive() has not been implemented.');
  }

  Future<void> stubSetPrivateSharingArray(
      StubPort port, List<String> paths) async {
    throw UnimplementedError(
        'setPrivateSharingArray() has not been implemented.');
  }

  Future<void> stubSetPrivateSharing(StubPort port, String path) async {
    throw UnimplementedError('setPrivateSharing() has not been implemented.');
  }

  Future<void> stubUnsetPrivateSharing(StubPort port) async {
    throw UnimplementedError('unsetPrivateSharing() has not been implemented.');
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
