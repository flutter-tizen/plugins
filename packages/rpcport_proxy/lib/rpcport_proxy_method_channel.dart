import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'rpcport_proxy_platform_interface.dart';

/// An implementation of [RpcportProxyPlatform] that uses method channels.
class MethodChannelRpcportProxy extends RpcportProxyPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('tizen/rpcport_proxy');

  @override
  Future<String?> getPlatformVersion() async {
    final version = await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }
}
