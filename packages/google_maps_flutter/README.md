# google_maps_flutter_tizen

The Tizen implementation of [`google_maps_flutter`](https://github.com/flutter/plugins/tree/master/packages/google_maps_flutter).

## Required privileges

To use this plugin, add below lines under the `<manifest>` section in your `tizen-manifest.xml` file,

```xml
<privileges>
  <privilege>http://tizen.org/privilege/internet</privilege>
</privileges>
```

## Usage

This package is not an _endorsed_ implementation of `google_maps_flutter`. Therefore, you have to include `google_maps_flutter_tizen` alongside `google_maps_flutter` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  google_maps_flutter: ^2.0.1
  google_maps_flutter_tizen: ^0.0.1
```
Also, you need to modify some parts of original 'google_maps_flutter' to use 'google_maps_flutter_tizen'
because 'google_maps_flutter' plugin doesn't support the tizen platform.

```dart
in package:google_maps_flutter_platform_interface/src/method_channel/method_channel_google_maps_flutter.dart

import '../types/tile_overlay_updates.dart';
import '../types/utils/tile_overlay.dart';
+ import 'package:google_maps_flutter_tizen/google_maps_flutter_tizen.dart';

...

} else if (defaultTargetPlatform == TargetPlatform.iOS) {
    return UiKitView(
    viewType: 'plugins.flutter.io/google_maps',
    onPlatformViewCreated: onPlatformViewCreated,
    gestureRecognizers: gestureRecognizers,
    creationParams: creationParams,
    creationParamsCodec: const StandardMessageCodec(),
    );
+ } else if (defaultTargetPlatform == TargetPlatform.linux) {
+     return TizenView(
+     viewType: 'plugins.flutter.io/google_maps',
+     onPlatformViewCreated: onPlatformViewCreated,
+     gestureRecognizers: gestureRecognizers,
+     creationParams: creationParams,
+     creationParamsCodec: const StandardMessageCodec(),
+     );
}

```
