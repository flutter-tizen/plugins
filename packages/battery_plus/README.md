# Battery Plus Tizen

The Tizen implementation of [`battery_plus`](https://github.com/fluttercommunity/plus_plugins/tree/main/packages/battery_plus).

## Usage

This package is not an _endorsed_ implementation of `battery_plus`. Therefore, you have to include `battery_plus_tizen` alongside `battery_plus` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  battery_plus: ^1.0.1
  battery_plus_tizen: ^1.0.0
```

Then you can import `battery_plus` in your Dart code:

```dart
import 'package:battery_plus/battery_plus.dart';
```

For detailed usage, see https://pub.dev/packages/battery_plus#usage.

## Supported devices

This plugin is supported on Galaxy Watch (running Tizen 4.0 or later).
