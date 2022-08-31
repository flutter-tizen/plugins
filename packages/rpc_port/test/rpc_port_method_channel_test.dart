import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:rpc_port/rpc_port_method_channel.dart';

void main() {
  MethodChannelRpcPort platform = MethodChannelRpcPort();
  const MethodChannel channel = MethodChannel('rpc_port');

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
