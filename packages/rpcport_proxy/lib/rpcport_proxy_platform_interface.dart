import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'rpcport_proxy_method_channel.dart';

abstract class RpcportProxyPlatform extends PlatformInterface {
  /// Constructs a RpcportProxyPlatform.
  RpcportProxyPlatform() : super(token: _token);

  static final Object _token = Object();

  static RpcportProxyPlatform _instance = MethodChannelRpcportProxy();

  /// The default instance of [RpcportProxyPlatform] to use.
  ///
  /// Defaults to [MethodChannelRpcportProxy].
  static RpcportProxyPlatform get instance => _instance;
  
  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [RpcportProxyPlatform] when
  /// they register themselves.
  static set instance(RpcportProxyPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }
}
