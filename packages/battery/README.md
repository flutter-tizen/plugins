# battery_tizen

The Tizen implementation of [`battery`](https://github.com/flutter/plugins/tree/master/packages/battery).

## Usage

This package is not an _endorsed_ implementation of `battery`. Therefore, you have to include `battery_tizen` alongside `battery` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  battery: ^1.0.7
  battery_tizen: ^1.0.0
```

Then you can import `battery` in your Dart code:

```dart
import 'package:battery/battery.dart';
```

For detailed usage, see https://github.com/flutter/plugins/tree/master/packages/battery/battery#usage.

## Supported devices

This plugin is available on these types of devices:

- Galaxy Watch (running Tizen 5.5 or later)
