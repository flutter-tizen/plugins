# wifi_info_flutter_tizen

[![pub package](https://img.shields.io/pub/v/wifi_info_flutter_tizen.svg)](https://pub.dev/packages/wifi_info_flutter_tizen)

The Tizen implementation of [`wifi_info_flutter`](https://github.com/flutter/plugins/tree/master/packages/wifi_info_flutter).

## Usage

This package is not an _endorsed_ implementation of `wifi_info_flutter`. Therefore, you have to include `wifi_info_flutter_tizen` alongside `wifi_info_flutter` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  wifi_info_flutter: ^2.0.0
  wifi_info_flutter_tizen: ^2.0.0
```

Then you can import `wifi_info_flutter` in your Dart code:

```dart
import 'package:wifi_info_flutter/wifi_info_flutter.dart';
```

For detailed usage, see https://github.com/flutter/plugins/tree/master/packages/wifi_info_flutter/wifi_info_flutter#usage.

## Required privileges

To get network information using this plugin, add below lines under the `<manifest>` section in your `tizen-manifest.xml` file,

```xml
<privileges>
  <privilege>http://tizen.org/privilege/network.get</privilege>
</privileges>
```

For details, see [Security and API Privileges](https://docs.tizen.org/application/dotnet/tutorials/sec-privileges).
