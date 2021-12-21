# device_info_tizen

---

## Deprecation Notice

The `device_info` plugin has been replaced by the [Flutter Community Plus Plugins](https://plus.fluttercommunity.dev) version, [`device_info_plus`](https://pub.dev/packages/device_info_plus). Consider migrating to `device_info_plus` and its Tizen implementation [`device_info_plus_tizen`](https://pub.dev/packages/device_info_plus_tizen).

---

[![pub package](https://img.shields.io/pub/v/device_info_tizen.svg)](https://pub.dev/packages/device_info_tizen)

The Tizen implementation of [`device_info`](https://pub.dev/packages/device_info).

## Usage

```yaml
dependencies:
  device_info_tizen: ^2.0.2
```

You can import `device_info_tizen` in your Dart code:

```dart
import 'package:device_info_tizen/device_info_tizen.dart';

DeviceInfoPluginTizen deviceInfo = DeviceInfoPluginTizen();
TizenDeviceInfo tizenInfo = await deviceInfo.tizenInfo;
print('Running on ${tizenInfo.modelName}');  // e.g. "SM-R800"
```
