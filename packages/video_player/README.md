# video_player_tizen

[![pub package](https://img.shields.io/pub/v/video_player_tizen.svg)](https://pub.dev/packages/video_player_tizen)

The Tizen implementation of [`video_player`](https://github.com/flutter/plugins/tree/master/packages/video_player).

## Required privileges

To use this plugin in a Tizen application, you may need to declare the following privileges in your `tizen-manifest.xml` file.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/mediastorage</privilege>
  <privilege>http://tizen.org/privilege/externalstorage</privilege>
  <privilege>http://tizen.org/privilege/internet</privilege>
</privileges>
```

- The mediastorage privilege (`http://tizen.org/privilege/mediastorage`) is required to play video files located in the internal storage.
- The externalstorage privilege (`http://tizen.org/privilege/externalstorage`) is required to play video files located in the external storage.
- The internet privilege (`http://tizen.org/privilege/internet`) is required to play any URLs from network.

For detailed information on Tizen privileges, see [Tizen Docs: API Privileges](https://docs.tizen.org/application/dotnet/get-started/api-privileges).

## Usage

This package is not an _endorsed_ implementation of `video_player`. Therefore, you have to include `video_player_tizen` alongside `video_player` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  video_player: ^2.2.6
  video_player_tizen: ^2.4.0
```

Then you can import `video_player` in your Dart code:

```dart
import 'package:video_player/video_player.dart';
```

For how to use the plugin, see https://github.com/flutter/plugins/tree/master/packages/video_player/video_player#example.

## Limitations

The `httpHeaders` option of `VideoPlayerController.network` and the `mixWithOthers` option of `VideoPlayerOptions` will be silently ignored in Tizen platform.

This plugin has some limitations on TV devices.

- The `setPlaybackSpeed` method will fail if triggered within last 3 seconds.
- The playback speed will reset to 1.0 when video is replayed in loop mode.
- The `seekTo` method works only when playback speed is 1.0, and it sets video position to the nearest key frame which may differ from the passed argument.
