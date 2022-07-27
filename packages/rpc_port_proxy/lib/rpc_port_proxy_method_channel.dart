import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'rpc_port_proxy_platform_interface.dart';

/// An implementation of [RpcPortProxyPlatform] that uses method channels.
class MethodChannelRpcPortProxy extends RpcPortProxyPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('rpc_port_proxy');

  @override
  Future<String?> getPlatformVersion() async {
    final version = await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }
}
