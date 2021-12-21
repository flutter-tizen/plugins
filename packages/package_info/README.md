# package_info_tizen

---

## Deprecation Notice

The `package_info` plugin has been replaced by the [Flutter Community Plus Plugins](https://plus.fluttercommunity.dev) version, [`package_info_plus`](https://pub.dev/packages/package_info_plus). Consider migrating to `package_info_plus` and its Tizen implementation [`package_info_plus_tizen`](https://pub.dev/packages/package_info_plus_tizen).

---

[![pub package](https://img.shields.io/pub/v/package_info_tizen.svg)](https://pub.dev/packages/package_info_tizen)

The Tizen implementation of [`package_info`](https://pub.dev/packages/package_info).

## Usage

This package is not an _endorsed_ implementation of `package_info`. Therefore, you have to include `package_info_tizen` alongside `package_info` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  package_info: ^2.0.0
  package_info_tizen: ^2.0.1
```

Then you can import `package_info` in your Dart code:

```dart
import 'package:package_info/package_info.dart';

PackageInfo packageInfo = await PackageInfo.fromPlatform();

String appName = packageInfo.appName;
String packageName = packageInfo.packageName;
String version = packageInfo.version;
String buildNumber = packageInfo.buildNumber;
```

Or in async mode:

```dart
PackageInfo.fromPlatform().then((PackageInfo packageInfo) {
  String appName = packageInfo.appName;
  String packageName = packageInfo.packageName;
  String version = packageInfo.version;
  String buildNumber = packageInfo.buildNumber;
});
```

For detailed usage, see https://pub.dev/packages/package_info#usage.

## Supported devices

- Galaxy Watch series (running Tizen 4.0 or later)
