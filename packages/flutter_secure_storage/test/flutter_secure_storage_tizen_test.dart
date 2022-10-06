import 'package:flutter_test/flutter_test.dart';
import 'package:flutter_secure_storage_tizen/flutter_secure_storage_tizen.dart';
import 'package:flutter_secure_storage_tizen/flutter_secure_storage_tizen_platform_interface.dart';
import 'package:flutter_secure_storage_tizen/flutter_secure_storage_tizen_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockFlutterSecureStorageTizenPlatform
    with MockPlatformInterfaceMixin
    implements FlutterSecureStorageTizenPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');
}

void main() {
  final FlutterSecureStorageTizenPlatform initialPlatform = FlutterSecureStorageTizenPlatform.instance;

  test('$MethodChannelFlutterSecureStorageTizen is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelFlutterSecureStorageTizen>());
  });

  test('getPlatformVersion', () async {
    FlutterSecureStorageTizen flutterSecureStorageTizenPlugin = FlutterSecureStorageTizen();
    MockFlutterSecureStorageTizenPlatform fakePlatform = MockFlutterSecureStorageTizenPlatform();
    FlutterSecureStorageTizenPlatform.instance = fakePlatform;

    expect(await flutterSecureStorageTizenPlugin.getPlatformVersion(), '42');
  });
}
