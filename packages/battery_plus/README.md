# battery_plus_tizen

[![pub package](https://img.shields.io/pub/v/battery_plus_tizen.svg)](https://pub.dev/packages/battery_plus_tizen)

The Tizen implementation of [`battery_plus`](https://github.com/fluttercommunity/plus_plugins/tree/main/packages/battery_plus).

## Usage

This package is not an _endorsed_ implementation of `battery_plus`. Therefore, you have to include `battery_plus_tizen` alongside `battery_plus` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  battery_plus: ^2.0.1
  battery_plus_tizen: ^1.0.2
```

Then you can import `battery_plus` in your Dart code:

```dart
import 'package:battery_plus/battery_plus.dart';
```

For detailed usage, see https://pub.dev/packages/battery_plus#usage.

## Supported APIs

- [x] `Battery.batteryLevel`
- [ ] `Battery.isInBatterySaveMode`
- [x] `Battery.onBatteryStateChanged`

## Supported devices

- Galaxy Watch series (running Tizen 4.0 or later)
