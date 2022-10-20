# flutter_reactive_ble_tizen

[![pub package](https://img.shields.io/pub/v/flutter_reactive_ble_tizen.svg)](https://pub.dev/packages/flutter_reactive_ble_tizen)

The Tizen implementation of [`flutter_reactive_ble`](https://pub.dev/packages/flutter_reactive_ble).

## Usage

This package is not an _endorsed_ implementation of `flutter_reactive_ble`. Therefore, you have to include `flutter_reactive_ble_tizen` alongside `flutter_reactive_ble` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  flutter_reactive_ble: ^5.0.3
  flutter_reactive_ble_tizen: ^0.1.0
```

Then you can import `flutter_reactive_ble` in your Dart code:

```dart
import 'package:flutter_reactive_ble/flutter_reactive_ble.dart';
```

For detailed usage, see https://pub.dev/packages/flutter_reactive_ble#usage.

## Required privileges

The bluetooth privilege must be added to your `tizen-manifest.xml` file to use this plugin.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/bluetooth</privilege>
</privileges>
```

## Known issues

- The following parameters are not supported and will be ignored on Tizen.
  - `connectionTimeout` of `FlutterReactiveBle.connectToDevice()` and `FlutterReactiveBle.connectToAdvertisingDevice()`
  - `scanMode` of `FlutterReactiveBle.scanForDevices()`
- The plugin sometimes doesn't respond to characteristic read or notify/indicate requests, requiring the user to press the same button twice in the example app. This looks like a bug in the frontend package.
- The plugin often fails to retrieve the names of discovered devices on Tizen more frequently than on other platforms.
