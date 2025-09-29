# video_player_avplay

[![pub package](https://img.shields.io/pub/v/video_player_avplay.svg)](https://pub.dev/packages/video_player_avplay)

A downloadable plugin which supports MMPlayer and PlusPlayer(PlusPlayer is a new multimedia player object-oriented designed) on Tizen TV devices.

This plugin is only supported on Tizen TV devices. If you are targeting other types of devices or are not interested in playing DRM content in your app, use [`video_player`](https://pub.dev/packages/video_player) and [`video_player_tizen`](https://pub.dev/packages/video_player_tizen) instead.

## Usage

To use this package, add `video_player_avplay` as a dependency in your `pubspec.yaml` file.

```yaml
dependencies:
  video_player_avplay: ^0.7.2
```

Then you can import `video_player_avplay` in your Dart code:

```dart
import 'package:video_player_avplay/video_player.dart';
```

Note that `video_player_avplay` is not compatible with the original `video_player` plugin. If you're writing a cross-platform app for Tizen and other platforms, it is recommended to create two separate source files and import `video_player` and `video_player_avplay` in the files respectively.

Change api-version in tizen-manifest.xml according to your TV version.

Note that `video_player_avplay` uses a compiled dynamic library, change the api-version according to your TV version in tizen-manifest.xml :

```xml
<manifest package="xxx" version="1.0.0" api-version="6.0">
```

> [!NOTE]
> This plugin for a specific api-version does not provide OS version compatibility.
> | `api-version` | TizenOS version |
> |:-:|:-:|
> |6.0|6.0|
> |6.5|6.5 ~ 9.0|
> |7.0|7.0 ~ 9.0|
> |8.0|8.0 ~ 9.0|
> |9.0|9.0|
> |10.0|10.0|
>
> When you build an application with this plugin, version-specific [dynamic libraries](https://github.com/flutter-tizen/plugins/tree/master/packages/video_player_avplay/tizen/lib/armel) are packaged together based on the api-version information in tizen-manifest.xml. 
> If you are planning to distribute an application that includes this plugin, you will need to build a TPK package for each TizenOS version (api-version in tizen-manifest.xml). Please refer to the [Samsung Developers](https://developer.samsung.com/smarttv/develop) for information on TizenOS versions by [TV model groups](https://developer.samsung.com/smarttv/develop/specifications/tv-model-groups.html).
> 
> If you plan to distribute from TizenOS version 6.0 to 10.0, it should be packaged as follows.
> - `<.... api-version="6.0" version="1.0.0" ...> # for TizenOS 6.0.`
> - `<.... api-version="6.5" version="1.0.1" ...> # for TizenOS 6.5 ~ 9.0.`
> - `<.... api-version="10.0" version="1.0.2" ...> # for TizenOS 10.0.`
>
> If you plan to distribute from TizenOS version 7.0 to 10.0, it should be packaged as follows.
> - `<.... api-version="7.0" version="1.0.0" ...> # for TizenOS 7.0 ~ 9.0.`
> - `<.... api-version="10.0" version="1.0.1" ...> # for TizenOS 10.0.`

Note that if you play dash streams, please add dash format when creating the player:
```dart
    VideoPlayerController.network(
      'https://xxx.mpd',
      formatHint: VideoFormat.dash);
```

### Example

```dart
import 'package:flutter/material.dart';
import 'package:video_player_avplay/video_player.dart';

class RemoteVideo extends StatefulWidget {
  const RemoteVideo({Key? key}) : super(key: key);

  @override
  State<RemoteVideo> createState() => _RemoteVideoState();
}

class _RemoteVideoState extends State<RemoteVideo> {
  late VideoPlayerController _controller;

  @override
  void initState() {
    super.initState();
    _controller = VideoPlayerController.network(
      'https://media.w3.org/2010/05/bunny/trailer.mp4',
      drmConfigs: const DrmConfigs(
        type: DrmType.playready,
        licenseServerUrl:
            'http://test.playready.microsoft.com/service/rightsmanager.asmx',
      ),
    );
    _controller.addListener(() => setState(() {}));
    _controller.initialize().then((_) => setState(() {}));
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Center(
      child: AspectRatio(
        aspectRatio: _controller.value.aspectRatio,
        child: Stack(
          alignment: Alignment.bottomCenter,
          children: <Widget>[
            VideoPlayer(_controller),
            ClosedCaption(text: _controller.value.caption.text),
            GestureDetector(
              onTap: () {
                _controller.value.isPlaying
                    ? _controller.pause()
                    : _controller.play();
              },
            ),
            VideoProgressIndicator(_controller, allowScrubbing: true),
          ],
        ),
      ),
    );
  }
}
```

## Required privileges

To use this plugin, you may need to declare the following privileges in your `tizen-manifest.xml` file.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/mediastorage</privilege>
  <privilege>http://tizen.org/privilege/externalstorage</privilege>
  <privilege>http://tizen.org/privilege/internet</privilege>
  <privilege>http://developer.samsung.com/privilege/drmplay</privilege>
</privileges>
```

- The mediastorage privilege (`http://tizen.org/privilege/mediastorage`) is required to play video files located in the internal storage.
- The externalstorage privilege (`http://tizen.org/privilege/externalstorage`) is required to play video files located in the external storage.
- The internet privilege (`http://tizen.org/privilege/internet`) is required to play any URL from the network.
- The drmplay privilege (`http://developer.samsung.com/privilege/drmplay`) is required to play DRM content. The app must be signed with a [partner-level certificate](https://docs.tizen.org/application/dotnet/get-started/certificates/creating-certificates) to use this privilege.

For detailed information on Tizen privileges, see [Tizen Docs: API Privileges](https://docs.tizen.org/application/dotnet/get-started/api-privileges).

## Limitations

This plugin is not supported on TV emulators.

The following options are not currently supported.

- `VideoPlayerOptions.allowBackgroundPlayback`
- `VideoPlayerOptions.mixWithOthers`

This plugin has the following limitations.

- The `httpHeaders` option of `VideoPlayerController.network` only support `Cookie` and `User-Agent`.
- The `setPlaybackSpeed` method will fail if triggered within the last 3 seconds of the video.
- The playback speed will reset to 1.0 when the video is replayed in loop mode.
- The `seekTo` method works only when the playback speed is 1.0, and it sets the video position to the nearest keyframe, not the exact value passed.
- The `setLooping` method only works when the player's DataSourceType is DataSourceType.asset.
