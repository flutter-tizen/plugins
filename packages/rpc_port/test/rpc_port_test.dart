import 'package:flutter_test/flutter_test.dart';
import 'package:rpc_port/proxy_base.dart';
import 'package:rpc_port/rpc_port_platform_interface.dart';
import 'package:rpc_port/rpc_port_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockRpcPortPlatform
    with MockPlatformInterfaceMixin
    implements RpcPortPlatform {
  @override
  Future<String?> getPlatformVersion() => Future.value('42');
}

void main() {
  final RpcPortPlatform initialPlatform = RpcPortPlatform.instance;

  test('$MethodChannelRpcPort is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelRpcPort>());
  });

  test('getPlatformVersion', () async {
    ProxyPort rpcPortPlugin = ProxyPort();
    MockRpcPortPlatform fakePlatform = MockRpcPortPlatform();
    RpcPortPlatform.instance = fakePlatform;

    expect(await rpcPortPlugin.getPlatformVersion(), '42');
  });
}
