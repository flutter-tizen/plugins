import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'rpc_port_proxy_method_channel.dart';

abstract class RpcPortProxyPlatform extends PlatformInterface {
  /// Constructs a RpcPortProxyPlatform.
  RpcPortProxyPlatform() : super(token: _token);

  static final Object _token = Object();

  static RpcPortProxyPlatform _instance = MethodChannelRpcPortProxy();

  /// The default instance of [RpcPortProxyPlatform] to use.
  ///
  /// Defaults to [MethodChannelRpcPortProxy].
  static RpcPortProxyPlatform get instance => _instance;
  
  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [RpcPortProxyPlatform] when
  /// they register themselves.
  static set instance(RpcPortProxyPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }
}
