# camera_tizen

The Tizen implementation of [`camera`](https://github.com/flutter/plugins/tree/master/packages/camera).

## Supported devices

This plugin is an experimental plug-in for the future

- Nothing

## Required privileges

To use this plugin, add below lines under the `<manifest>` section in your `tizen-manifest.xml` file,

```xml
<privileges>
    <privilege>http://tizen.org/privilege/camera</privilege>
    <privilege>http://tizen.org/privilege/recorder</privilege>
</privileges>
```

## Usage

This package is not an _endorsed_ implementation of `camera`. Therefore, you have to include `camera_tizen` alongside `camera` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  camera: ^0.8.1
  camera_tizen: ^0.2.0
```

Then you can import `camera` in your Dart code:

```dart
import 'package:camera/camera.dart';
```
For detailed usage, see https://github.com/flutter/plugins/tree/master/packages/camera/camera#example.

## Notes
CameraPreview currently does not support other platforms except Android and iOS. Therefor the camera preview to orient properly, you have to modify the `camera_preview.dart`.

```dart
  Widget _wrapInRotatedBox({required Widget child}) {
    // if (defaultTargetPlatform != TargetPlatform.android) {
    //   return child;
    // }

    return RotatedBox(
      quarterTurns: _getQuarterTurns(),
      child: child,
    );
  }
```
