# video_player_videohole

A video_player flutter plugin for Tizen TV devices,also supports play DRM type.

## Required privileges

To use this plugin in a Tizen application, the internet privileges are required. Add below lines under the `<manifest>` section in your `tizen-manifest.xml` file.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/mediastorage</privilege>
  <privilege>http://tizen.org/privilege/externalstorage</privilege>
  <privilege>http://tizen.org/privilege/internet</privilege>
</privileges>
```

- The mediastorage privilege (http://tizen.org/privilege/mediastorage) is required to play video files located in the internal storage.
  The externalstorage privilege (http://tizen.org/privilege/externalstorage) is required to play video files located in the external storage.
  The internet privilege (http://tizen.org/privilege/internet) is required to play any URLs from network.

For details, see [Security and API Privileges](https://docs.tizen.org/application/dotnet/tutorials/sec-privileges).

## Example

```dart
import 'package:video_player_videohole/video_player.dart';
import 'package:flutter/material.dart';

void main() => runApp(VideoApp());

class VideoApp extends StatefulWidget {
  @override
  _VideoAppState createState() => _VideoAppState();
}

class _VideoAppState extends State<VideoApp> {
  VideoPlayerController _controller;

  @override
  void initState() {
    super.initState();
    _controller = VideoPlayerController.network(
        'https://flutter.github.io/assets-for-api-docs/assets/videos/bee.mp4')
      ..initialize().then((_) {
        // Ensure the first frame is shown after the video is initialized, even before the play button has been pressed.
        setState(() {});
      });
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Video Demo',
      home: Scaffold(
        body: Center(
          child: _controller.value.isInitialized
              ? AspectRatio(
                  aspectRatio: _controller.value.aspectRatio,
                  child: VideoPlayer(_controller),
                )
              : Container(),
        ),
        floatingActionButton: FloatingActionButton(
          onPressed: () {
            setState(() {
              _controller.value.isPlaying
                  ? _controller.pause()
                  : _controller.play();
            });
          },
          child: Icon(
            _controller.value.isPlaying ? Icons.pause : Icons.play_arrow,
          ),
        ),
      ),
    );
  }

  @override
  void dispose() {
    super.dispose();
    _controller.dispose();
  }
}
```

## Usage

```yaml
dependencies:
  video_player_videohole: ^1.0.0
```

Then you can import `video_player_videohole` in your Dart code:

```dart
import 'package:video_player_videhole/video_player.dart';
```

For how to use the plugin, see https://github.com/flutter/plugins/tree/master/packages/video_player/video_player#usage.

## Limitations

The 'httpheaders' option for 'VideoPlayerController.network' and 'mixWithOthers' option of 'VideoPlayerOptions' will be silently ignored in Tizen platform.

This plugin has some limitations on TV:

- The 'setPlaybackSpeed' method will fail if triggered within last 3 seconds.
- The playback speed will reset to 1.0 when video is replayed in loop mode.
- The 'seekTo' method works only when playback speed is 1.0, and it sets video position to the nearest key frame which may differ from the passed argument.
