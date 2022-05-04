<br>
<h1 align="left"> flutter_blue_tizen </h1>
<br>

[![pub package](https://img.shields.io/pub/v/flutter_blue_tizen.svg)](https://pub.dev/packages/flutter_blue_tizen)

The Tizen implementation of <a href="https://github.com/pauldemarco/flutter_blue">flutter_blue</a>.


## Usage

This package is not an endorsed implementation of `flutter_blue`. Therefore, you have to include `flutter_blue_tizen` alongside flutter_blue as dependencies in your `pubspec.yaml` file.

```
dependencies:
  flutter_blue: ^0.8.0
  flutter_blue_tizen: ^1.0.0
```

Then you can import `flutter_blue` in your Dart code:

```dart
import 'package:flutter_blue/flutter_blue.dart';
```


For detailed usage, see https://pub.dev/packages/flutter_blue#usage.

## Required privileges

To use this plugin in a Tizen application, you need to declare the following privileges in your `tizen-manifest.xml` file.

```xml
<privileges>
    <privilege>http://tizen.org/privilege/bluetooth</privilege>
</privileges>
```

## Supported devices

Plugin is supported on Tizen 6.5 iot-headed armv7l and aarch64 (RPI4, RB5).
