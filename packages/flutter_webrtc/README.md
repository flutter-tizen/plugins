# flutter_webrtc_tizen

[![pub package](https://img.shields.io/pub/v/flutter_webrtc_tizen.svg)](https://pub.dev/packages/flutter_webrtc_tizen)

The Tizen implementation of [`flutter_webrtc`](https://github.com/flutter-webrtc/flutter-webrtc).

## Required privileges

To use this plugin in a Tizen application, you may need to declare the following privileges in your `tizen-manifest.xml` file.

```xml
<privileges>
    <privilege>http://developer.samsung.com/privilege/camera</privilege>
    <privilege>http://tizen.org/privilege/internet</privilege>
    <privilege>http://tizen.org/privilege/camera</privilege>
    <privilege>http://tizen.org/privilege/recorder</privilege>
</privileges>
```

- The internet privilege (`http://tizen.org/privilege/internet`) is required to access the internet.
- The camera priviege (`http://tizen.org/privilege/internet`) is required to use camera.
- The recorder priviege (`http://tizen.org/privilege/recorder`) is required to record video and audio.

 To use camera on Tizen TV devices, you need to add developer camera privilege (`http://developer.samsung.com/privilege/camera`) and have a [partner level certificate](https://developer.samsung.com/tv-seller-office/guides/membership/becoming-partner.html).
 For details, see [Security and API Privileges](https://docs.tizen.org/application/dotnet/tutorials/sec-privileges).

## Usage

 This package is not an _endorsed_ implementation of `flutter_webrtc`. Therefore, you have to include `flutter_webrtc_tizen` alongside `flutter_webrtc` as dependencies in your `pubspec.yaml` file.

 ```yaml
dependencies:
  flutter_webrtc: ^0.9.11
  flutter_webrtc_tizen: ^0.9.11
```

## Functionality

|      Feature       |       Tizen        |
| :----------------: | :----------------: |
|    Audio/Video     | :heavy_check_mark: |
|    Data Channel    |       [WIP]        |
|   Screen Capture   |       [WIP]        |
|    Unified-Plan    | :heavy_check_mark: |
|     Simulcast      |       [WIP]        |
|   MediaRecorder    |       [WIP]        |
| Insertable Streams |       [WIP]        |
