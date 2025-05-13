# video_player_tizen

[![pub package](https://img.shields.io/pub/v/video_player_tizen.svg)](https://pub.dev/packages/video_player_tizen)

The Tizen implementation of [`video_player`](https://pub.dev/packages/video_player) based on the Tizen [Media Player](https://docs.tizen.org/application/native/api/iot-headed/latest/group__CAPI__MEDIA__PLAYER__MODULE.html) API.

## Supported devices

This plugin is **NOT** supported on TV emulators. This plugin is supported on Galaxy Watch devices and Smart TVs running Tizen 5.5 or above.

## Usage

This package is not an _endorsed_ implementation of `video_player`. Therefore, you have to include `video_player_tizen` alongside `video_player` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  video_player: ^2.9.2
  video_player_tizen: ^2.5.5
```

Then you can import `video_player` in your Dart code:

```dart
import 'package:video_player/video_player.dart';
```

For detailed usage, see https://pub.dev/packages/video_player#example.

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
- The internet privilege (`http://tizen.org/privilege/internet`) is required to play any URL from the network.

For detailed information on Tizen privileges, see [Tizen Docs: API Privileges](https://docs.tizen.org/application/dotnet/get-started/api-privileges).

## Limitations

This plugin is not supported on TV emulators.

The following options are not currently supported.

- `VideoPlayerOptions.allowBackgroundPlayback`
- `VideoPlayerOptions.mixWithOthers`

This plugin has the following limitations.

- The `httpHeaders` option of `VideoPlayerController.networkUrl` only support `Cookie` and `User-Agent`.
- The `setPlaybackSpeed` method will fail if triggered within the last 3 seconds of the video.
- The playback speed will reset to 1.0 when the video is replayed in loop mode.
- The `seekTo` method works only when the playback speed is 1.0, and it sets the video position to the nearest keyframe, not the exact value passed.
