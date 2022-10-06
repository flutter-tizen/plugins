import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'flutter_secure_storage_tizen_platform_interface.dart';

/// An implementation of [FlutterSecureStorageTizenPlatform] that uses method channels.
class MethodChannelFlutterSecureStorageTizen extends FlutterSecureStorageTizenPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('flutter_secure_storage_tizen');

  @override
  Future<String?> getPlatformVersion() async {
    final version = await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }
}
