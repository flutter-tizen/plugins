# network_info_plus_tizen

[![pub package](https://img.shields.io/pub/v/network_info_plus_tizen.svg)](https://pub.dev/packages/network_info_plus_tizen)

The Tizen implementation of [`network_info_plus`](https://github.com/fluttercommunity/plus_plugins/tree/main/packages/network_info_plus).

## Usage

This package is not an _endorsed_ implementation of `network_info_plus`. Therefore, you have to include `network_info_plus_tizen` alongside `network_info_plus` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  network_info_plus: ^2.0.2
  network_info_plus_tizen: ^1.1.1
```

Then you can import `network_info_plus` in your Dart code:

```dart
import 'package:network_info_plus/network_info_plus.dart';
```

For detailed usage, see https://pub.dev/packages/network_info_plus#usage.

## Required privileges

To get network information using this plugin, add below lines under the `<manifest>` section in your `tizen-manifest.xml` file,

```xml
<privileges>
  <privilege>http://tizen.org/privilege/network.get</privilege>
</privileges>
```

For detailed information on Tizen privileges, see [Tizen Docs: API Privileges](https://docs.tizen.org/application/dotnet/get-started/api-privileges).
