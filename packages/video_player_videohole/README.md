# video_player_videohole

A video_player_videohole flutter plugin only for Tizen TV devices, supports play DRM(playready & widevine).

## Required privileges

To use this plugin in a Tizen application, you may need to add the following privileges in your `tizen-manifest.xml` file.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/mediastorage</privilege>
  <privilege>http://tizen.org/privilege/externalstorage</privilege>
  <privilege>http://tizen.org/privilege/internet</privilege>
  <privilege>http://tizen.org/privilege/network.get</privilege>
  <privilege>http://tizen.org/privilege/download</privilege>
  <privilege>http://tizen.org/privilege/push</privilege>
  <privilege>http://developer.samsung.com/privilege/drmplay</privilege>
</privileges>
```

- The mediastorage privilege (http://tizen.org/privilege/mediastorage) is required to play video files located in the internal storage.
- The externalstorage privilege (http://tizen.org/privilege/externalstorage) is required to play video files located in the external storage.
- The internet privilege (http://tizen.org/privilege/internet) is required to play any URLs from network.
- The drmplay privilege (http://developer.samsung.com/privilege/drmplay) is required to play DRM video files.

To play DRM with this streaming player, you need to have [partner level certificate](https://developer.samsung.com/tv-seller-office/guides/membership/becoming-partner.html).

For detailed information on Tizen privileges, see [Tizen Docs: API Privileges](https://docs.tizen.org/application/dotnet/get-started/api-privileges/).

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
      'https://media.w3.org/2010/05/bunny/trailer.mp4',
    );

    _controller.addListener(() {
      setState(() {});
    });
    _controller.setLooping(true);
    _controller.initialize().then((_) => setState(() {}));
    _controller.play();
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return SingleChildScrollView(
      child: Column(
        children: <Widget>[
          Container(padding: const EdgeInsets.only(top: 20.0)),
          const Text('Video Demo'),
          Container(
            padding: const EdgeInsets.all(20),
            child: AspectRatio(
              aspectRatio: _controller.value.aspectRatio,
              child: Stack(
                alignment: Alignment.bottomCenter,
                children: <Widget>[
                  VideoPlayer(_controller),
                  // Set internal subtitle from stream.
                  ClosedCaption(
                      text: _controller.value.subtitleText,
                      isSubtitle: _controller.value.isSubtitle),
                  _ControlsOverlay(controller: _controller),
                  VideoProgressIndicator(_controller, allowScrubbing: true),
                ],
              ),
            ),
          ),
        ],
      ),
    );
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

## Limitations

The 'httpheaders' option for 'VideoPlayerController.network' and 'mixWithOthers' option of 'VideoPlayerOptions' will be silently ignored in Tizen platform.

This plugin has some limitations on TV:

- The 'setPlaybackSpeed' method will fail if triggered within last 3 seconds.
- The playback speed will reset to 1.0 when video is replayed in loop mode.
- The 'seekTo' method works only when playback speed is 1.0, and it sets video position to the nearest key frame which may differ from the passed argument.
- The 'player_set_subtitle_updated_cb' can't support Dash sidecar subtitle on Tizen 6.0/6.5, but support on Tizen 7.0.
