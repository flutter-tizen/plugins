# package_info_tizen

The Tizen implementation of [`package_info`](https://github.com/flutter/plugins/tree/master/packages/package_info).

## Usage

This package is not an _endorsed_ implementation of `package_info`. Therefore, you have to include `package_info_tizen` alongside `package_info` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  package_info: ^2.0.0
  package_info_tizen: ^2.0.0
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

For detailed usage, see https://github.com/flutter/plugins/blob/master/packages/package_info/README.md#usage.

## Supported devices

This plugin is supported on these types of devices:

- Galaxy Watch (running Tizen 4.0 or later)
