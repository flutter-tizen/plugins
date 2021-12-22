# battery_tizen

---

## Deprecation Notice

The `battery` plugin has been replaced by the [Flutter Community Plus Plugins](https://plus.fluttercommunity.dev) version, [`battery_plus`](https://pub.dev/packages/battery_plus). Consider migrating to `battery_plus` and its Tizen implementation [`battery_plus_tizen`](https://pub.dev/packages/battery_plus_tizen).

---

[![pub package](https://img.shields.io/pub/v/battery_tizen.svg)](https://pub.dev/packages/battery_tizen)

The Tizen implementation of [`battery`](https://pub.dev/packages/battery).

## Usage

This package is not an _endorsed_ implementation of `battery`. Therefore, you have to include `battery_tizen` alongside `battery` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  battery: ^2.0.1
  battery_tizen: ^2.0.2
```

Then you can import `battery` in your Dart code:

```dart
import 'package:battery/battery.dart';
```

For detailed usage, see https://pub.dev/packages/battery#usage.

## Supported devices

- Galaxy Watch series (running Tizen 4.0 or later)
