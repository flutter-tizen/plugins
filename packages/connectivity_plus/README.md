# connectivity_plus_tizen

[![pub package](https://img.shields.io/pub/v/connectivity_plus_tizen.svg)](https://pub.dev/packages/connectivity_plus_tizen)

The Tizen implementation of [`connectivity_plus`](https://pub.dev/packages/connectivity_plus).

## Usage

This package is not an _endorsed_ implementation of `connectivity_plus`. Therefore, you have to include `connectivity_plus_tizen` alongside `connectivity_plus` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  connectivity_plus: ^4.0.1
  connectivity_plus_tizen: ^1.1.4
```

Then you can import `connectivity_plus` in your Dart code:

```dart
import 'package:connectivity_plus/connectivity_plus.dart';
```

For detailed usage, see https://pub.dev/packages/connectivity_plus#usage.

## Required privileges

To get connectivity information using this plugin, add below lines under the `<manifest>` section in your `tizen-manifest.xml` file,

```xml
<privileges>
  <privilege>http://tizen.org/privilege/network.get</privilege>
</privileges>
```
