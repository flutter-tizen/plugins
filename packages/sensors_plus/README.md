# sensors_plus_tizen

The Tizen implementation of ['sensors_plus'](https://github.com/fluttercommunity/plus_plugins/tree/main/packages/sensors_plus).

## Usage

This package is not an _endorsed_ implementation of 'sensors_plus'. Therefore, you have to include `sensors_plus_tizen` alongside 'sensors_plus' as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  sensors_plus: ^1.0.0
  sensors_plus_tizen: ^2.0.0
```

Then you can import 'sensors_plus' in your Dart code:

```dart
import 'package:sensors_plus/sensors_plus.dart';
```

For detailed usage, see https://github.com/fluttercommunity/plus_plugins/tree/main/packages/sensors_plus#usage.

## Supported devices

This plugin is supported on these types of devices:

- Galaxy Watch (running Tizen 4.0 or later)
