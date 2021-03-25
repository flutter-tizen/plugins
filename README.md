# plugins

This repo contains Flutter plugins maintained by the flutter-tizen team. We're in process of adding Tizen platform support to existing first and third-party plugins on [pub.dev](https://pub.dev) based on their popularity. If the plugin you're looking for isn't implemented for Tizen yet, consider filing an [issue](../../issues) or creating a package by yourself. (We welcome your pull requests!)

To build Flutter applications with plugins, use the [flutter-tizen](https://github.com/flutter-tizen/flutter-tizen) tool.

For how to extend existing plugins for Tizen, see [Writing custom platform-specific code](https://flutter.dev/docs/development/platform-integration/platform-channels) and [Federated plugins](https://flutter.dev/docs/development/packages-and-plugins/developing-packages#federated-plugins) from the Flutter docs. If the original plugin uses the federated plugins approach, you can implement its platform interface either in Dart (inheriting directly) or C++ (using a fallback method channel).

## List of packages

The _"non-endorsed"_ status means that the plugin is not endorsed by the original author. In such case, you must set both `foobar` and `foobar_tizen` package dependencies in `pubspec.yaml` file to achieve full functionality.

| Package name | Original package | Pub | Endorsed |
|-|-|:-:|:-:|
| [**battery_tizen**](packages/battery) | [battery](https://github.com/flutter/plugins/tree/master/packages/battery) (1st-party) | [![pub package](https://img.shields.io/pub/v/battery_tizen.svg)](https://pub.dev/packages/battery_tizen) | No |
| [**connectivity_tizen**](packages/connectivity) | [connectivity](https://github.com/flutter/plugins/tree/master/packages/connectivity) (1st-party) | [![pub package](https://img.shields.io/pub/v/connectivity_tizen.svg)](https://pub.dev/packages/connectivity_tizen) | No |
| [**device_info_tizen**](packages/device_info) | [device_info](https://github.com/flutter/plugins/tree/master/packages/device_info) (1st-party) | [![pub package](https://img.shields.io/pub/v/device_info_tizen.svg)](https://pub.dev/packages/device_info_tizen) | No |
| [**image_picker_tizen**](packages/image_picker) | [image_picker](https://github.com/flutter/plugins/tree/master/packages/image_picker) (1st-party) | [![pub package](https://img.shields.io/pub/v/image_picker_tizen.svg)](https://pub.dev/packages/image_picker_tizen) | No |
| [**integration_test_tizen**](packages/integration_test) | [integration_test](https://github.com/flutter/flutter/tree/master/packages/integration_test) (1st-party) | [![pub package](https://img.shields.io/pub/v/integration_test_tizen.svg)](https://pub.dev/packages/integration_test_tizen) | No |
| [**package_info_tizen**](packages/package_info) | [package_info](https://github.com/flutter/plugins/tree/master/packages/package_info) (1st-party) | [![pub package](https://img.shields.io/pub/v/package_info_tizen.svg)](https://pub.dev/packages/package_info_tizen) | No |
| [**path_provider_tizen**](packages/path_provider) | [path_provider](https://github.com/flutter/plugins/tree/master/packages/path_provider) (1st-party) | [![pub package](https://img.shields.io/pub/v/path_provider_tizen.svg)](https://pub.dev/packages/path_provider_tizen) | No |
| [**sensors_tizen**](packages/sensors) | [sensors](https://github.com/flutter/plugins/tree/master/packages/sensors) (1st-party) | [![pub package](https://img.shields.io/pub/v/sensors_tizen.svg)](https://pub.dev/packages/sensors_tizen) | No |
| [**share_tizen**](packages/share) | [share](https://github.com/flutter/plugins/tree/master/packages/share) (1st-party) | [![pub package](https://img.shields.io/pub/v/share_tizen.svg)](https://pub.dev/packages/share_tizen) | No |
| [**shared_preferences_tizen**](packages/path_provider) | [shared_preferences](https://github.com/flutter/plugins/tree/master/packages/shared_preferences) (1st-party) | [![pub package](https://img.shields.io/pub/v/shared_preferences_tizen.svg)](https://pub.dev/packages/shared_preferences_tizen) | No |
| [**url_launcher_tizen**](packages/url_launcher) | [url_launcher](https://github.com/flutter/plugins/tree/master/packages/url_launcher) (1st-party) | [![pub package](https://img.shields.io/pub/v/url_launcher_tizen.svg)](https://pub.dev/packages/url_launcher_tizen) | No |
| [**wifi_info_flutter_tizen**](packages/wifi_info_flutter) | [wifi_info_flutter](https://github.com/flutter/plugins/tree/master/packages/wifi_info_flutter) (1st-party) | [![pub package](https://img.shields.io/pub/v/wifi_info_flutter_tizen.svg)](https://pub.dev/packages/wifi_info_flutter_tizen) | No |

## Device limitations

| Package name | Watch | Watch emulator | TV | TV emulator | Remarks |
|-|:-:|:-:|:-:|:-:|-|
| [**battery_tizen**](packages/battery) | ✔️ | ✔️ | ❌ | ❌ | No battery |
| [**connectivity_tizen**](packages/connectivity) | ✔️ | ⚠️ | ✔️ | ✔️ | The return value is incorrect |
| [**device_info_tizen**](packages/device_info) | ✔️ | ✔️ | ✔️ | ✔️ |
| [**image_picker_tizen**](packages/image_picker) | ⚠️ | ❌ | ❌ | ❌ | No camera<br>No file manager app |
| [**integration_test_tizen**](packages/integration_test) | ✔️ | ✔️ | ✔️ | ✔️ |
| [**package_info_tizen**](packages/package_info) | ✔️ | ✔️ | ✔️ | ✔️ |
| [**path_provider_tizen**](packages/path_provider) | ⚠️ | ⚠️ | ⚠️ | ⚠️ | No external storage |
| [**sensors_tizen**](packages/sensors) | ✔️ | ✔️ | ❌ | ❌ | No hardware |
| [**share_tizen**](packages/share) | ⚠️ | ⚠️ | ❌ | ❌ | No SMS or e-mail app |
| [**shared_preferences_tizen**](packages/path_provider) | ✔️ | ✔️ | ✔️ | ✔️ |
| [**url_launcher_tizen**](packages/url_launcher) | ✔️ | ❌ | ✔️ | ❌ | No browser app |
| [**wifi_info_flutter_tizen**](packages/wifi_info_flutter) | ✔️ | ❌ | ✔️ | ❌ |
