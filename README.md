# plugins

This repo contains Flutter plugins maintained by the flutter-tizen team. We're in process of adding Tizen platform support to existing first and third-party plugins on [pub.dev](https://pub.dev) based on their popularity. If the plugin you're looking for isn't implemented for Tizen yet, consider filing an [issue](../../issues) or creating a package by yourself. (We welcome your pull requests!)

To build Flutter applications with plugins, use the [flutter-tizen](https://github.com/flutter-tizen/flutter-tizen) tool.

For how to extend existing plugins for Tizen, see [Writing custom platform-specific code](https://flutter.dev/docs/development/platform-integration/platform-channels) and [Federated plugins](https://flutter.dev/docs/development/packages-and-plugins/developing-packages#federated-plugins) from the Flutter docs. If the original plugin uses the federated plugins approach, you can implement its platform interface either in Dart (inheriting directly) or C++ (using a fallback method channel).

## List of packages

The _"non-endorsed"_ status means that the plugin is not endorsed by the original author. In such case, you must set both `foobar` and `foobar_tizen` package dependencies in `pubspec.yaml` file to achieve full functionality.

| Package name | Original package | Pub | Endorsed |
|-|-|:-:|:-:|
| [**audioplayers_tizen**](packages/audioplayers) | [audioplayers](https://github.com/luanpotter/audioplayers) (3rd-party) | [![pub package](https://img.shields.io/pub/v/audioplayers_tizen.svg)](https://pub.dev/packages/audioplayers_tizen) | No |
| [**battery_tizen**](packages/battery) | [battery](https://github.com/flutter/plugins/tree/master/packages/battery) (1st-party) | [![pub package](https://img.shields.io/pub/v/battery_tizen.svg)](https://pub.dev/packages/battery_tizen) | No |
| [**battery_plus_tizen**](packages/battery_plus) | [battery_plus](https://github.com/fluttercommunity/plus_plugins/tree/main/packages/battery_plus) (3rd-party) | [![pub package](https://img.shields.io/pub/v/battery_plus_tizen.svg)](https://pub.dev/packages/battery_plus_tizen) | No |
| [**connectivity_tizen**](packages/connectivity) | [connectivity](https://github.com/flutter/plugins/tree/master/packages/connectivity) (1st-party) | [![pub package](https://img.shields.io/pub/v/connectivity_tizen.svg)](https://pub.dev/packages/connectivity_tizen) | No |
| [**device_info_tizen**](packages/device_info) | [device_info](https://github.com/flutter/plugins/tree/master/packages/device_info) (1st-party) | [![pub package](https://img.shields.io/pub/v/device_info_tizen.svg)](https://pub.dev/packages/device_info_tizen) | No |
| [**flutter_tts_tizen**](packages/flutter_tts) | [flutter_tts](https://github.com/dlutton/flutter_tts) (3rd-party) | [![pub package](https://img.shields.io/pub/v/flutter_tts_tizen.svg)](https://pub.dev/packages/flutter_tts_tizen) | No |
| [**image_picker_tizen**](packages/image_picker) | [image_picker](https://github.com/flutter/plugins/tree/master/packages/image_picker) (1st-party) | [![pub package](https://img.shields.io/pub/v/image_picker_tizen.svg)](https://pub.dev/packages/image_picker_tizen) | No |
| [**integration_test_tizen**](packages/integration_test) | [integration_test](https://github.com/flutter/flutter/tree/master/packages/integration_test) (1st-party) | [![pub package](https://img.shields.io/pub/v/integration_test_tizen.svg)](https://pub.dev/packages/integration_test_tizen) | No |
| [**package_info_tizen**](packages/package_info) | [package_info](https://github.com/flutter/plugins/tree/master/packages/package_info) (1st-party) | [![pub package](https://img.shields.io/pub/v/package_info_tizen.svg)](https://pub.dev/packages/package_info_tizen) | No |
| [**package_info_plus_tizen**](packages/package_info_plus) | [package_info_plus](https://github.com/fluttercommunity/plus_plugins/tree/main/packages/package_info_plus) (3rd-party) | [![pub package](https://img.shields.io/pub/v/package_info_plus_tizen.svg)](https://pub.dev/packages/package_info_plus_tizen) | No |
| [**path_provider_tizen**](packages/path_provider) | [path_provider](https://github.com/flutter/plugins/tree/master/packages/path_provider) (1st-party) | [![pub package](https://img.shields.io/pub/v/path_provider_tizen.svg)](https://pub.dev/packages/path_provider_tizen) | No |
| [**permission_handler_tizen**](packages/permission_handler) | [permission_handler](https://github.com/Baseflow/flutter-permission-handler) (3rd-party) | [![pub package](https://img.shields.io/pub/v/permission_handler_tizen.svg)](https://pub.dev/packages/permission_handler_tizen) | No |
| [**sensors_tizen**](packages/sensors) | [sensors](https://github.com/flutter/plugins/tree/master/packages/sensors) (1st-party) | [![pub package](https://img.shields.io/pub/v/sensors_tizen.svg)](https://pub.dev/packages/sensors_tizen) | No |
| [**sensors_plus_tizen**](packages/sensors_plus) | [sensors_plus](https://github.com/fluttercommunity/plus_plugins/tree/main/packages/sensors_plus) (3rd-party) | [![pub package](https://img.shields.io/pub/v/sensors_plus_tizen.svg)](https://pub.dev/packages/sensors_plus_tizen) | No |
| [**share_tizen**](packages/share) | [share](https://github.com/flutter/plugins/tree/master/packages/share) (1st-party) | [![pub package](https://img.shields.io/pub/v/share_tizen.svg)](https://pub.dev/packages/share_tizen) | No |
| [**shared_preferences_tizen**](packages/shared_preferences) | [shared_preferences](https://github.com/flutter/plugins/tree/master/packages/shared_preferences) (1st-party) | [![pub package](https://img.shields.io/pub/v/shared_preferences_tizen.svg)](https://pub.dev/packages/shared_preferences_tizen) | No |
| [**url_launcher_tizen**](packages/url_launcher) | [url_launcher](https://github.com/flutter/plugins/tree/master/packages/url_launcher) (1st-party) | [![pub package](https://img.shields.io/pub/v/url_launcher_tizen.svg)](https://pub.dev/packages/url_launcher_tizen) | No |
| [**video_player_tizen**](packages/video_player) | [video_player](https://github.com/flutter/plugins/tree/master/packages/video_player) (1st-party) | [![pub package](https://img.shields.io/pub/v/video_player_tizen.svg)](https://pub.dev/packages/video_player_tizen) | No |
| [**wakelock_tizen**](packages/wakelock) | [wakelock](https://github.com/creativecreatorormaybenot/wakelock) (3rd-party) | [![pub package](https://img.shields.io/pub/v/wakelock_tizen.svg)](https://pub.dev/packages/wakelock_tizen) | No |
| [**wearable_rotary**](packages/wearable_rotary) | _N/A_ | [![pub package](https://img.shields.io/pub/v/wearable_rotary.svg)](https://pub.dev/packages/wearable_rotary) | _N/A_ |
| [**webview_flutter_tizen**](packages/webview_flutter) | [webview_flutter](https://github.com/flutter/plugins/tree/master/packages/webview_flutter) (1st-party) | [![pub package](https://img.shields.io/pub/v/webview_flutter_tizen.svg)](https://pub.dev/packages/webview_flutter_tizen) | No |
| [**wifi_info_flutter_tizen**](packages/wifi_info_flutter) | [wifi_info_flutter](https://github.com/flutter/plugins/tree/master/packages/wifi_info_flutter) (1st-party) | [![pub package](https://img.shields.io/pub/v/wifi_info_flutter_tizen.svg)](https://pub.dev/packages/wifi_info_flutter_tizen) | No |

## Device limitations

| Package name | Watch | Watch<br>emulator | TV | TV<br>emulator | Remarks |
|-|:-:|:-:|:-:|:-:|-|
| [**audioplayers_tizen**](packages/audioplayers) | ✔️ | ✔️ | ✔️ | ✔️ |
| [**battery_tizen**](packages/battery) | ✔️ | ✔️ | ❌ | ❌ | No battery |
| [**battery_plus_tizen**](packages/battery_plus) | ✔️ | ✔️ | ❌ | ❌ | No battery |
| [**connectivity_tizen**](packages/connectivity) | ✔️ | ⚠️ | ✔️ | ✔️ | The return value is incorrect |
| [**device_info_tizen**](packages/device_info) | ✔️ | ✔️ | ✔️ | ✔️ |
| [**flutter_tts_tizen**](packages/flutter_tts) | ✔️ | ✔️ | ✔️ | ✔️ |
| [**image_picker_tizen**](packages/image_picker) | ⚠️ | ❌ | ❌ | ❌ | No camera<br>No file manager app |
| [**integration_test_tizen**](packages/integration_test) | ✔️ | ✔️ | ✔️ | ✔️ |
| [**package_info_tizen**](packages/package_info) | ✔️ | ✔️ | ✔️ | ✔️ |
| [**package_info_plus_tizen**](packages/package_info_plus) | ✔️ | ✔️ | ✔️ | ✔️ |
| [**path_provider_tizen**](packages/path_provider) | ⚠️ | ⚠️ | ⚠️ | ⚠️ | No external storage |
| [**permission_handler_tizen**](packages/permission_handler) | ✔️ | ✔️ | ❌ | ❌ | Not applicable for TV |
| [**sensors_tizen**](packages/sensors) | ✔️ | ✔️ | ❌ | ❌ | No sensor hardware |
| [**sensors_plus_tizen**](packages/sensors_plus) | ✔️ | ✔️ | ❌ | ❌ | No sensor hardware |
| [**share_tizen**](packages/share) | ⚠️ | ⚠️ | ❌ | ❌ | No SMS or e-mail app |
| [**shared_preferences_tizen**](packages/shared_preferences) | ✔️ | ✔️ | ✔️ | ✔️ |
| [**url_launcher_tizen**](packages/url_launcher) | ✔️ | ❌ | ✔️ | ❌ | No browser app |
| [**video_player_tizen**](packages/video_player) | ✔️ | ✔️ | ✔️ | ❌ | TV emulator issue |
| [**wakelock_tizen**](packages/wakelock) | ✔️ | ✔️ | ❌ | ❌ | Cannot override system display setting|
| [**wearable_rotary**](packages/wearable_rotary) | ✔️ | ✔️ | ❌ | ❌ | Not applicable for TV  |
| [**webview_flutter_tizen**](packages/webview_flutter) | ✔️ | ❌ | ✔️ | ❌ | Dependent library unavailable |
| [**wifi_info_flutter_tizen**](packages/wifi_info_flutter) | ✔️ | ❌ | ✔️ | ❌ | API unsupported by emulators |
