# flutter_blue_plus_tizen

[![pub package](https://img.shields.io/pub/v/flutter_blue_plus_tizen.svg)](https://pub.dev/packages/flutter_blue_plus_tizen)

The Tizen implementation of [flutter_blue_plus](https://github.com/pauldemarco/flutter_blue_plus).


## Usage

This package is not an _endorsed_ implementation of `flutter_blue`. Therefore, you have to include `flutter_blue_plus_tizen` alongside flutter_blue as dependencies in your `pubspec.yaml` file.

```
dependencies:
  flutter_blue_plus: ^1.3.0
  flutter_blue_plus_tizen: 1.0.0
```

Then you can import `flutter_blue_plus` in your Dart code:

```dart
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
```


For detailed usage, see https://pub.dev/packages/flutter_blue_plus#usage.

## Required privileges

To use this plugin in a Tizen application, you need to declare the following privileges in your `tizen-manifest.xml` file.

```xml
<privileges>
    <privilege>http://tizen.org/privilege/bluetooth</privilege>
</privileges>
```

## Supported devices

Plugin is supported on Tizen 6.5 iot-headed armv7l and aarch64 (RPI4, RB5).
