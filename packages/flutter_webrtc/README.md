# flutter_webrtc_tizen

[![pub package](https://img.shields.io/pub/v/flutter_webrtc_tizen.svg)](https://pub.dev/packages/flutter_webrtc_tizen)

The Tizen implementation of [`flutter_webrtc`](https://github.com/flutter-webrtc/flutter-webrtc).

## Supported devices

This plugin is only supported devices running Tizen 6.5 or later.

## Required privileges

To use this plugin, add below lines under the `<manifest>` section in your `tizen-manifest.xml` file,

```xml
<privileges>
  <privilege>http://tizen.org/privilege/camera</privilege>
  <privilege>http://tizen.org/privilege/internet</privilege>
  <privilege>http://tizen.org/privilege/recorder</privilege>
</privileges>
```

## Usage

This package is not an _endorsed_ implementation of `flutter_webrtc`. 

```yaml
dependencies:
    flutter_webrtc_tizen: ^0.0.1
```
