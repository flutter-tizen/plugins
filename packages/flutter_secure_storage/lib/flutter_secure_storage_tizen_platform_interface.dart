import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'flutter_secure_storage_tizen_method_channel.dart';

abstract class FlutterSecureStorageTizenPlatform extends PlatformInterface {
  /// Constructs a FlutterSecureStorageTizenPlatform.
  FlutterSecureStorageTizenPlatform() : super(token: _token);

  static final Object _token = Object();

  static FlutterSecureStorageTizenPlatform _instance = MethodChannelFlutterSecureStorageTizen();

  /// The default instance of [FlutterSecureStorageTizenPlatform] to use.
  ///
  /// Defaults to [MethodChannelFlutterSecureStorageTizen].
  static FlutterSecureStorageTizenPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [FlutterSecureStorageTizenPlatform] when
  /// they register themselves.
  static set instance(FlutterSecureStorageTizenPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }
}
