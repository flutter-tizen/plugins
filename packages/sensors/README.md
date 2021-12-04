# sensors_tizen

[![pub package](https://img.shields.io/pub/v/sensors_tizen.svg)](https://pub.dev/packages/sensors_tizen)

The Tizen implementation of [`sensors`](https://github.com/flutter/plugins/tree/master/packages/sensors).

## Usage

This package is not an _endorsed_ implementation of `sensors`. Therefore, you have to include `sensors_tizen` alongside `sensors` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  sensors: ^2.0.0
  sensors_tizen: ^2.0.1
```

Then you can import `sensors` in your Dart code:

```dart
import 'package:sensors/sensors.dart';
```

For detailed usage, see https://github.com/flutter/plugins/tree/master/packages/sensors#usage.

## Supported devices

This plugin is supported on these types of devices:

- Galaxy Watch (running Tizen 4.0 or later)
