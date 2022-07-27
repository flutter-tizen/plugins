import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:rpc_port_proxy/rpc_port_proxy_method_channel.dart';

void main() {
  MethodChannelRpcPortProxy platform = MethodChannelRpcPortProxy();
  const MethodChannel channel = MethodChannel('rpc_port_proxy');

  TestWidgetsFlutterBinding.ensureInitialized();

  setUp(() {
    channel.setMockMethodCallHandler((MethodCall methodCall) async {
      return '42';
    });
  });

  tearDown(() {
    channel.setMockMethodCallHandler(null);
  });

  test('getPlatformVersion', () async {
    expect(await platform.getPlatformVersion(), '42');
  });
}
