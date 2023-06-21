# plugins

[![Build](https://github.com/flutter-tizen/plugins/actions/workflows/build.yml/badge.svg)](https://github.com/flutter-tizen/plugins/actions/workflows/build.yml)

This repo contains Flutter plugins maintained by the flutter-tizen team. We're in process of adding Tizen platform support to existing first and third-party plugins on [pub.dev](https://pub.dev) based on their popularity. If the plugin you're looking for isn't implemented for Tizen yet, consider filing an [issue](../../issues) or creating a package by yourself. (We welcome your pull requests!)

To build Flutter applications with plugins, use the [flutter-tizen](https://github.com/flutter-tizen/flutter-tizen) tool.

Every plugin in this repo is written in either C++, C#, or Dart. For how to write a new plugin or extend existing plugins for Tizen, see [this page](https://github.com/flutter-tizen/flutter-tizen/blob/master/doc/develop-plugin.md).

## List of packages

The _"non-endorsed"_ status means that the plugin is not endorsed by the original author. In such case, you must set both `foobar` and `foobar_tizen` package dependencies in `pubspec.yaml` file to achieve full functionality.

| Tizen package | Frontend package | Pub | Endorsed |
|-|-|:-:|:-:|
| [**audioplayers_tizen**](packages/audioplayers) | [audioplayers](https://pub.dev/packages/audioplayers) (3rd-party) | [![pub package](https://img.shields.io/pub/v/audioplayers_tizen.svg)](https://pub.dev/packages/audioplayers_tizen) | No |
| [**battery_plus_tizen**](packages/battery_plus) | [battery_plus](https://pub.dev/packages/battery_plus) (1st-party) | [![pub package](https://img.shields.io/pub/v/battery_plus_tizen.svg)](https://pub.dev/packages/battery_plus_tizen) | No |
| [**camera_tizen**](packages/camera) | [camera](https://pub.dev/packages/camera) (1st-party) | [![pub package](https://img.shields.io/pub/v/camera_tizen.svg)](https://pub.dev/packages/camera_tizen) | No |
| [**connectivity_plus_tizen**](packages/connectivity_plus) | [connectivity_plus](https://pub.dev/packages/connectivity_plus) (1st-party) | [![pub package](https://img.shields.io/pub/v/connectivity_plus_tizen.svg)](https://pub.dev/packages/connectivity_plus_tizen) | No |
| [**device_info_plus_tizen**](packages/device_info_plus) | [device_info_plus](https://pub.dev/packages/device_info_plus) (1st-party) | [![pub package](https://img.shields.io/pub/v/device_info_plus_tizen.svg)](https://pub.dev/packages/device_info_plus_tizen) | No |
| [**firebase_core_tizen**](packages/firebase_core) | [firebase_core](https://pub.dev/packages/firebase_core) | [![pub package](https://img.shields.io/pub/v/firebase_core_tizen.svg)](https://pub.dev/packages/firebase_core_tizen) | No |
| [**flutter_app_badger_tizen**](packages/flutter_app_badger) | [flutter_app_badger](https://pub.dev/packages/flutter_app_badger) (3rd-party) | [![pub package](https://img.shields.io/pub/v/flutter_app_badger_tizen.svg)](https://pub.dev/packages/flutter_app_badger_tizen) | No |
| [**flutter_secure_storage_tizen**](packages/flutter_secure_storage) | [flutter_secure_storage](https://pub.dev/packages/flutter_secure_storage) (3rd-party) | [![pub package](https://img.shields.io/pub/v/flutter_secure_storage_tizen.svg)](https://pub.dev/packages/flutter_secure_storage_tizen) | No |
| [**flutter_tts_tizen**](packages/flutter_tts) | [flutter_tts](https://pub.dev/packages/flutter_tts) (3rd-party) | [![pub package](https://img.shields.io/pub/v/flutter_tts_tizen.svg)](https://pub.dev/packages/flutter_tts_tizen) | No |
| [**flutter_webrtc_tizen**](packages/flutter_webrtc) | [flutter_webrtc](https://pub.dev/packages/flutter_webrtc) (3rd-party) | [![pub package](https://img.shields.io/pub/v/flutter_webrtc_tizen.svg)](https://pub.dev/packages/flutter_webrtc_tizen) | No |
| [**geolocator_tizen**](packages/geolocator) | [geolocator](https://pub.dev/packages/geolocator) (3rd-party) | [![pub package](https://img.shields.io/pub/v/geolocator_tizen.svg)](https://pub.dev/packages/geolocator_tizen) | No |
| [**google_maps_flutter_tizen**](packages/google_maps_flutter) | [google_maps_flutter](https://pub.dev/packages/google_maps_flutter) (1st-party) | [![pub package](https://img.shields.io/pub/v/google_maps_flutter_tizen.svg)](https://pub.dev/packages/google_maps_flutter_tizen) | No |
| [**google_sign_in_tizen**](packages/google_sign_in) | [google_sign_in](https://pub.dev/packages/google_sign_in) (1st-party) | [![pub package](https://img.shields.io/pub/v/google_sign_in_tizen.svg)](https://pub.dev/packages/google_sign_in_tizen) | No |
| [**image_picker_tizen**](packages/image_picker) | [image_picker](https://pub.dev/packages/image_picker) (1st-party) | [![pub package](https://img.shields.io/pub/v/image_picker_tizen.svg)](https://pub.dev/packages/image_picker_tizen) | No |
| [**in_app_purchase_tizen**](packages/in_app_purchase) | [in_app_purchase](https://pub.dev/packages/in_app_purchase) (1st-party) | [![pub package](https://img.shields.io/pub/v/in_app_purchase_tizen.svg)](https://pub.dev/packages/in_app_purchase_tizen) | No |
| [**integration_test_tizen**](packages/integration_test) | [integration_test](https://github.com/flutter/flutter/tree/main/packages/integration_test) (1st-party) | [![pub package](https://img.shields.io/pub/v/integration_test_tizen.svg)](https://pub.dev/packages/integration_test_tizen) | No |
| [**messageport_tizen**](packages/messageport) | (Tizen-only) | [![pub package](https://img.shields.io/pub/v/messageport_tizen.svg)](https://pub.dev/packages/messageport_tizen) | N/A |
| [**network_info_plus_tizen**](packages/network_info_plus) | [network_info_plus](https://pub.dev/packages/network_info_plus) (1st-party) | [![pub package](https://img.shields.io/pub/v/network_info_plus_tizen.svg)](https://pub.dev/packages/network_info_plus_tizen) | No |
| [**package_info_plus_tizen**](packages/package_info_plus) | [package_info_plus](https://pub.dev/packages/package_info_plus) (1st-party) | [![pub package](https://img.shields.io/pub/v/package_info_plus_tizen.svg)](https://pub.dev/packages/package_info_plus_tizen) | No |
| [**path_provider_tizen**](packages/path_provider) | [path_provider](https://pub.dev/packages/path_provider) (1st-party) | [![pub package](https://img.shields.io/pub/v/path_provider_tizen.svg)](https://pub.dev/packages/path_provider_tizen) | No |
| [**permission_handler_tizen**](packages/permission_handler) | [permission_handler](https://pub.dev/packages/permission_handler) (3rd-party) | [![pub package](https://img.shields.io/pub/v/permission_handler_tizen.svg)](https://pub.dev/packages/permission_handler_tizen) | No |
| [**sensors_plus_tizen**](packages/sensors_plus) | [sensors_plus](https://pub.dev/packages/sensors_plus) (1st-party) | [![pub package](https://img.shields.io/pub/v/sensors_plus_tizen.svg)](https://pub.dev/packages/sensors_plus_tizen) | No |
| [**shared_preferences_tizen**](packages/shared_preferences) | [shared_preferences](https://pub.dev/packages/shared_preferences) (1st-party) | [![pub package](https://img.shields.io/pub/v/shared_preferences_tizen.svg)](https://pub.dev/packages/shared_preferences_tizen) | No |
| [**sqflite_tizen**](packages/sqflite) | [sqflite](https://pub.dev/packages/sqflite) (3rd-party) | [![pub package](https://img.shields.io/pub/v/sqflite_tizen.svg)](https://pub.dev/packages/sqflite_tizen) | No |
| [**tizen_app_control**](packages/tizen_app_control) | (Tizen-only) | [![pub package](https://img.shields.io/pub/v/tizen_app_control.svg)](https://pub.dev/packages/tizen_app_control) | N/A |
| [**tizen_app_manager**](packages/tizen_app_manager) | (Tizen-only) | [![pub package](https://img.shields.io/pub/v/tizen_app_manager.svg)](https://pub.dev/packages/tizen_app_manager) | N/A |
| [**tizen_audio_manager**](packages/tizen_audio_manager) | (Tizen-only) | [![pub package](https://img.shields.io/pub/v/tizen_audio_manager.svg)](https://pub.dev/packages/tizen_audio_manager) | N/A |
| [**tizen_bundle**](packages/tizen_bundle) | (Tizen-only) | [![pub package](https://img.shields.io/pub/v/tizen_bundle.svg)](https://pub.dev/packages/tizen_bundle) | N/A |
| [**tizen_log**](packages/tizen_log) | (Tizen-only) | [![pub package](https://img.shields.io/pub/v/tizen_log.svg)](https://pub.dev/packages/tizen_log) | N/A |
| [**tizen_notification**](packages/tizen_notification) | (Tizen-only) | [![pub package](https://img.shields.io/pub/v/tizen_notification.svg)](https://pub.dev/packages/tizen_notification) | N/A |
| [**tizen_package_manager**](packages/tizen_package_manager) | (Tizen-only) | [![pub package](https://img.shields.io/pub/v/tizen_package_manager.svg)](https://pub.dev/packages/tizen_package_manager) | N/A |
| [**tizen_rpc_port**](packages/tizen_rpc_port) | (Tizen-only) | [![pub package](https://img.shields.io/pub/v/tizen_rpc_port.svg)](https://pub.dev/packages/tizen_rpc_port) | N/A |
| [**url_launcher_tizen**](packages/url_launcher) | [url_launcher](https://pub.dev/packages/url_launcher) (1st-party) | [![pub package](https://img.shields.io/pub/v/url_launcher_tizen.svg)](https://pub.dev/packages/url_launcher_tizen) | No |
| [**video_player_tizen**](packages/video_player) | [video_player](https://pub.dev/packages/video_player) (1st-party) | [![pub package](https://img.shields.io/pub/v/video_player_tizen.svg)](https://pub.dev/packages/video_player_tizen) | No |
| [**video_player_videohole**](packages/video_player_videohole) | (Tizen-only) | [![pub package](https://img.shields.io/pub/v/video_player_videohole.svg)](https://pub.dev/packages/video_player_videohole) | N/A |
| [**wakelock_tizen**](packages/wakelock) | [wakelock](https://pub.dev/packages/wakelock) (3rd-party) | [![pub package](https://img.shields.io/pub/v/wakelock_tizen.svg)](https://pub.dev/packages/wakelock_tizen) | No |
| [**wearable_rotary**](packages/wearable_rotary) | (Tizen-only) | [![pub package](https://img.shields.io/pub/v/wearable_rotary.svg)](https://pub.dev/packages/wearable_rotary) | N/A |
| [**webview_flutter_lwe**](packages/webview_flutter_lwe) | [webview_flutter](https://pub.dev/packages/webview_flutter) (1st-party) | [![pub package](https://img.shields.io/pub/v/webview_flutter_lwe.svg)](https://pub.dev/packages/webview_flutter_lwe) | No |
| [**webview_flutter_tizen**](packages/webview_flutter) | [webview_flutter](https://pub.dev/packages/webview_flutter) (1st-party) | [![pub package](https://img.shields.io/pub/v/webview_flutter_tizen.svg)](https://pub.dev/packages/webview_flutter_tizen) | No |

## Device limitations

| Package name | API level | Watch | Watch<br>emulator | TV | TV<br>emulator | Remarks |
|-|:-:|:-:|:-:|:-:|:-:|-|
| [**audioplayers_tizen**](packages/audioplayers) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**battery_plus_tizen**](packages/battery_plus) | 5.5 | ✔️ | ✔️ | ❌ | ❌ | No battery |
| [**camera_tizen**](packages/camera) | 5.5 | ❌ | ❌ | ❌ | ❌ | No camera |
| [**connectivity_plus_tizen**](packages/connectivity_plus) | 5.5 | ✔️ | ⚠️ | ✔️ | ✔️ | Returns incorrect connection status |
| [**device_info_plus_tizen**](packages/device_info_plus) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**firebase_core**](packages/firebase_core) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**flutter_app_badger_tizen**](packages/flutter_app_badger) | 5.5 | ✔️ | ✔️ | ❌ | ❌ | API not supported |
| [**flutter_secure_storage_tizen**](packages/flutter_secure_storage) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**flutter_tts_tizen**](packages/flutter_tts) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**flutter_webrtc_tizen**](packages/flutter_webrtc) | 6.0 | ❌ | ❌ | ✔️ | ❌ | No camera |
| [**geolocator_tizen**](packages/geolocator) | 5.5 | ✔️ | ✔️ | ❌ | ❌ | Not applicable for TV |
| [**google_maps_flutter_tizen**](packages/google_maps_flutter) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**google_sign_in_tizen**](packages/google_sign_in) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**image_picker_tizen**](packages/image_picker) | 5.5 | ⚠️ | ❌ | ❌ | ❌ | No camera,<br>No file manager app |
| [**in_app_purchase_tizen**](packages/in_app_purchase) | 5.5 | ❌ | ❌ | ✔️ | ❌ | Only applicable for TV |
| [**integration_test_tizen**](packages/integration_test) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**messageport_tizen**](packages/messageport) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**network_info_plus_tizen**](packages/network_info_plus) | 5.5 | ✔️ | ❌ | ✔️ | ❌ | API not supported on emulator |
| [**package_info_plus_tizen**](packages/package_info_plus) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**path_provider_tizen**](packages/path_provider) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**permission_handler_tizen**](packages/permission_handler) | 5.5 | ✔️ | ✔️ | ⚠️ | ⚠️ | Not applicable for TV |
| [**sensors_plus_tizen**](packages/sensors_plus) | 5.5 | ✔️ | ✔️ | ❌ | ❌ | No sensor hardware |
| [**shared_preferences_tizen**](packages/shared_preferences) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**sqflite_tizen**](packages/sqflite) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**tizen_app_control**](packages/tizen_app_control) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**tizen_app_manager**](packages/tizen_app_manager) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**tizen_audio_manager**](packages/tizen_audio_manager) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**tizen_bundle**](packages/tizen_bundle) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**tizen_log**](packages/tizen_log) | 5.5 | ✔️ | ✔️ | ❌ | ❌ | Not applicable for TV |
| [**tizen_notification**](packages/tizen_notification) | 5.5 | ❌ | ✔️ | ✔️ | ✔️ | API not supported |
| [**tizen_package_manager**](packages/tizen_package_manager) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**tizen_rpc_port**](packages/tizen_rpc_port) | 6.5 | ✔️ | ✔️ | ✔️ | ✔️ |
| [**url_launcher_tizen**](packages/url_launcher) | 5.5 | ✔️ | ❌ | ✔️ | ❌ | No browser app |
| [**video_player_tizen**](packages/video_player) | 5.5 | ✔️ | ✔️ | ✔️ | ❌ | TV emulator issue |
| [**video_player_videohole**](packages/video_player_videohole) | 5.5 | ❌ | ❌ | ✔️ | ❌ | Only for TV devices |
| [**wakelock_tizen**](packages/wakelock) | 5.5 | ✔️ | ✔️ | ❌ | ❌ | Cannot override system settings |
| [**wearable_rotary**](packages/wearable_rotary) | 5.5 | ✔️ | ✔️ | ❌ | ❌ | Not applicable for TV |
| [**webview_flutter_lwe**](packages/webview_flutter_lwe) | 5.5 | ✔️ | ✔️ | ✔️ | ✔️ | Not for production use |
| [**webview_flutter_tizen**](packages/webview_flutter) | 5.5 | ❌ | ❌ | ✔️ | ✔️ | API not supported |
