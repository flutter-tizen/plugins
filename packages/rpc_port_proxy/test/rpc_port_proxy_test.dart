import 'package:flutter_test/flutter_test.dart';
import 'package:rpc_port_proxy/rpc_port_proxy.dart';
import 'package:rpc_port_proxy/rpc_port_proxy_platform_interface.dart';
import 'package:rpc_port_proxy/rpc_port_proxy_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockRpcPortProxyPlatform 
    with MockPlatformInterfaceMixin
    implements RpcPortProxyPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');
}

void main() {
  final RpcPortProxyPlatform initialPlatform = RpcPortProxyPlatform.instance;

  test('$MethodChannelRpcPortProxy is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelRpcPortProxy>());
  });

  test('getPlatformVersion', () async {
    RpcPortProxy rpcPortProxyPlugin = RpcPortProxy();
    MockRpcPortProxyPlatform fakePlatform = MockRpcPortProxyPlatform();
    RpcPortProxyPlatform.instance = fakePlatform;
  
    expect(await rpcPortProxyPlugin.getPlatformVersion(), '42');
  });
}
