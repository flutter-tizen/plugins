# device_info_plus_tizen

[![pub package](https://img.shields.io/pub/v/device_info_plus_tizen.svg)](https://pub.dev/packages/device_info_plus_tizen)

The Tizen implementation of [`device_info_plus`](https://github.com/fluttercommunity/plus_plugins/tree/main/packages/device_info_plus).

## Usage

```yaml
dependencies:
  device_info_plus_tizen: ^1.0.0
```

You can import `device_info_plus_tizen` in your Dart code:

```dart
import 'package:device_info_plus_tizen/device_info_plus_tizen.dart';

DeviceInfoPluginTizen deviceInfo = DeviceInfoPluginTizen();
TizenDeviceInfo tizenInfo = await deviceInfo.tizenInfo;
print('Running on ${tizenInfo.modelName}');  // e.g. "SM-R800"
```
