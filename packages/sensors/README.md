# sensors_tizen

---

## Deprecation Notice

The `sensors` plugin has been replaced by the [Flutter Community Plus Plugins](https://plus.fluttercommunity.dev) version, [`sensors_plus`](https://pub.dev/packages/sensors_plus). Consider migrating to `sensors_plus` and its Tizen implementation [`sensors_plus_tizen`](https://pub.dev/packages/sensors_plus_tizen).

---

[![pub package](https://img.shields.io/pub/v/sensors_tizen.svg)](https://pub.dev/packages/sensors_tizen)

The Tizen implementation of [`sensors`](https://github.com/flutter/plugins/tree/master/packages/sensors).

## Usage

This package is not an _endorsed_ implementation of `sensors`. Therefore, you have to include `sensors_tizen` alongside `sensors` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  sensors: ^2.0.0
  sensors_tizen: ^2.0.2
```

Then you can import `sensors` in your Dart code:

```dart
import 'package:sensors/sensors.dart';
```

For detailed usage, see https://pub.dev/packages/sensors#usage.

## Supported devices

- Galaxy Watch series (running Tizen 4.0 or later)
