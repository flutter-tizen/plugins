# device_info_tizen

The Tizen implementation of [`device_info`](https://github.com/flutter/plugins/tree/master/packages/device_info).

## Usage

```yaml
dependencies:
  device_info_tizen: ^2.0.0
```

You can import `device_info_tizen` in your Dart code:

```dart
import 'package:device_info_tizen/device_info_tizen.dart';

DeviceInfoPluginTizen deviceInfo = DeviceInfoPluginTizen();
TizenDeviceInfo tizenInfo = await deviceInfo.tizenInfo;
print('Running on ${tizenInfo.modelName}');  // e.g. "SM-R800"
```

## Supported devices

This plugin is supported on these types of devices:

- Galaxy Watch (running Tizen 4.0 or later)
