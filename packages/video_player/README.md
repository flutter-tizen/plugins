# video_player_tizen

The Tizen implementation of [`video_player`](https://github.com/flutter/plugins/tree/master/packages/video_player).

## Required privileges

To use this plugin in a Tizen application, the mediastorage, externalstorage and internet privileges are required. Add below lines under the `<manifest>` section in your `tizen-manifest.xml` file.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/mediastorage</privilege>
  <privilege>http://tizen.org/privilege/externalstorage</privilege>
  <privilege>http://tizen.org/privilege/internet</privilege>
</privileges>
```

- The mediastorage privilege (`http://tizen.org/privilege/mediastorage`) must be added to play video files located in the internal storage.
- The externalstorage privilege (`http://tizen.org/privilege/externalstorage`) must be added to play video files located in the external storage.
- The internet privilege (`http://tizen.org/privilege/internet`) must be added to play any URLs from network.

For details, see [Security and API Privileges](https://docs.tizen.org/application/dotnet/tutorials/sec-privileges).

## Usage

This package is not an _endorsed_ implementation of `video_player`. Therefore, you have to include `video_player_tizen` alongside `video_player` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  video_player: ^1.0.1
  video_player_tizen: ^1.0.0
```

Then you can import `video_player` in your Dart code:

```dart
import 'package:video_player/video_player.dart';
```

For how to use the plugin, see https://github.com/flutter/plugins/tree/master/packages/video_player/video_player#usage.
