import 'package:flutter_test/flutter_test.dart';
import 'package:rpcport_proxy/rpcport_proxy.dart';
import 'package:rpcport_proxy/rpcport_proxy_platform_interface.dart';
import 'package:rpcport_proxy/rpcport_proxy_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockRpcportProxyPlatform 
    with MockPlatformInterfaceMixin
    implements RpcportProxyPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');
}

void main() {
  final RpcportProxyPlatform initialPlatform = RpcportProxyPlatform.instance;

  test('$MethodChannelRpcportProxy is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelRpcportProxy>());
  });

  test('getPlatformVersion', () async {
    RpcportProxy rpcportProxyPlugin = RpcportProxy();
    MockRpcportProxyPlatform fakePlatform = MockRpcportProxyPlatform();
    RpcportProxyPlatform.instance = fakePlatform;
  
    expect(await rpcportProxyPlugin.getPlatformVersion(), '42');
  });
}
