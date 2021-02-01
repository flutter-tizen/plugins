# wifi_info_flutter_tizen

The Tizen implementation of [`wifi_info_flutter`](https://github.com/flutter/plugins/tree/master/packages/wifi_info_flutter).

## Usage

This package is not an _endorsed_ implementation of `wifi_info_flutter`. Therefore, you have to include `wifi_info_flutter_tizen` alongside `wifi_info_flutter` as dependencies in your `pubspec.yaml` file.
This package also requires `connectivity_tizen`. So it should be used like this:

```yaml
dependencies:
  connectivity: 0.4.9+3
  connectivity_tizen:
    path: ../../../connectivity/connectivity_tizen/
  wifi_info_flutter: ^1.0.1
  wifi_info_flutter_tizen:
    path: ../```

Then you can import `wifi_info_flutter` in your Dart code:

```dart
import 'package:wifi_info_flutter/wifi_info_flutter.dart';
```

For detailed usage, see https://github.com/flutter/plugins/tree/master/packages/wifi_info_flutter/wifi_info_flutter#usage.

## Supported devices

This plugin is supported on these types of devices:

- Galaxy Watch (running Tizen 4.0 or later)
