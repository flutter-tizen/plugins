# package_info_plus_tizen

[![pub package](https://img.shields.io/pub/v/package_info_plus_tizen.svg)](https://pub.dev/packages/package_info_plus_tizen)

The Tizen implementation of [`package_info_plus`](https://pub.dev/packages/package_info_plus).

## Usage

This package is not an _endorsed_ implementation of `package_info_plus`. Therefore, you have to include `package_info_plus_tizen` alongside `package_info_plus` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  package_info_plus: ^4.0.1
  package_info_plus_tizen: ^1.0.3
```

Then you can import `package_info_plus` in your Dart code.

```dart
import 'package:package_info_plus/package_info_plus.dart';
```

For detailed usage, see https://pub.dev/packages/package_info_plus#usage.

## Supported properties

- [x] `PackageInfo.appName`
- [x] `PackageInfo.packageName`
- [x] `PackageInfo.version`
- [ ] `PackageInfo.buildNumber`
- [ ] `PackageInfo.buildSignature`
- [ ] `PackageInfo.installerStore`
