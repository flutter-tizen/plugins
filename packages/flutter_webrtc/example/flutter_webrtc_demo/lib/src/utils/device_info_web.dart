import 'package:web/web.dart' as web;

class DeviceInfo {
  static String get label {
    return 'Flutter Web';
  }

  static String get userAgent {
    return 'flutter-webrtc/web-plugin 0.0.1 ( ${web.window.navigator.userAgent} )';
  }
}
