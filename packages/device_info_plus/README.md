# device_info_plus_tizen

[![pub package](https://img.shields.io/pub/v/device_info_plus_tizen.svg)](https://pub.dev/packages/device_info_plus_tizen)

The Tizen implementation of [`device_info_plus`](https://pub.dev/packages/device_info_plus).

## Usage

Add `device_info_plus_tizen` as a dependency in your `pubspec.yaml` file.

```yaml
dependencies:
  device_info_plus_tizen: ^1.2.1
```

Then you can import `device_info_plus_tizen` in your Dart code.

```dart
import 'package:device_info_plus_tizen/device_info_plus_tizen.dart';

DeviceInfoPluginTizen deviceInfo = DeviceInfoPluginTizen();
TizenDeviceInfo tizenInfo = await deviceInfo.tizenInfo;

String modelName = tizenInfo.modelName;
```

## Supported properties

| Property | Feature or system key |
|-|-|
| `modelName` | `http://tizen.org/system/model_name` |
| `cpuArch` | `http://tizen.org/feature/platform.core.cpu.arch` |
| `nativeApiVersion` | `http://tizen.org/feature/platform.native.api.version` |
| `platformVersion` | `http://tizen.org/feature/platform.version` |
| `webApiVersion` | `http://tizen.org/feature/platform.web.api.version` |
| `profile` | `http://tizen.org/feature/profile` |
| `buildDate` | `http://tizen.org/system/build.date` |
| `buildId` | `http://tizen.org/system/build.id` |
| `buildString` | `http://tizen.org/system/build.string` |
| `buildTime` | `http://tizen.org/system/build.time` |
| `buildType` | `http://tizen.org/system/build.type` |
| `buildVariant` | `http://tizen.org/system/build.variant` |
| `buildRelease` | `http://tizen.org/system/build.release` |
| `deviceType` | `http://tizen.org/system/device_type` |
| `manufacturer` | `http://tizen.org/system/manufacturer` |
| `platformName` | `http://tizen.org/system/platform.name` |
| `platformProcessor` | `http://tizen.org/system/platform.processor` |
| `tizenId` | `http://tizen.org/system/tizenid` |

For a description of each feature or system key in the list, see https://docs.tizen.org/application/native/guides/device/system.
