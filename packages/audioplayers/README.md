# audioplayers_tizen

[![pub package](https://img.shields.io/pub/v/audioplayers_tizen.svg)](https://pub.dev/packages/audioplayers_tizen)

The Tizen implementation of [`audioplayers`](https://pub.dev/packages/audioplayers).

## Usage

This package is not an _endorsed_ implementation of `audioplayers`. Therefore, you have to include `audioplayers_tizen` alongside `audioplayers` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  audioplayers: ^5.1.0
  audioplayers_tizen: ^3.0.0

```

Then you can import `audioplayers` in your Dart code:

```dart
import 'package:audioplayers/audioplayers.dart';
```

For detailed usage, see https://pub.dev/packages/audioplayers#usage.

## Required privileges

To use this plugin in a Tizen application, you may need to declare the following privileges in your `tizen-manifest.xml` file.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/mediastorage</privilege>
  <privilege>http://tizen.org/privilege/externalstorage</privilege>
  <privilege>http://tizen.org/privilege/internet</privilege>
</privileges>
```

- The mediastorage privilege (`http://tizen.org/privilege/mediastorage`) is required to play audio files located in the internal storage.
- The externalstorage privilege (`http://tizen.org/privilege/externalstorage`) is required to play audio files located in the external storage.
- The internet privilege (`http://tizen.org/privilege/internet`) is required to play any URLs from network.

For detailed information on Tizen privileges, see [Tizen Docs: API Privileges](https://docs.tizen.org/application/dotnet/get-started/api-privileges).

## Supported APIs

- [x] `AudioPlayer.play` (`AudioContext` not supported)
- [ ] `AudioPlayer.setAudioContext` (not supported)
- [x] `AudioPlayer.setPlayerMode`
- [x] `AudioPlayer.pause`
- [x] `AudioPlayer.stop`
- [x] `AudioPlayer.resume`
- [x] `AudioPlayer.release`
- [x] `AudioPlayer.seek`
- [ ] `AudioPlayer.setBalance` (not supported)
- [x] `AudioPlayer.setVolume`
- [x] `AudioPlayer.setReleaseMode`
- [x] `AudioPlayer.setPlaybackRate`
- [x] `AudioPlayer.setSource`
- [x] `AudioPlayer.setSourceUrl`
- [x] `AudioPlayer.setSourceDeviceFile`
- [x] `AudioPlayer.setSourceAsset`
- [x] `AudioPlayer.setSourceBytes`
- [x] `AudioPlayer.getDuration`
- [x] `AudioPlayer.getCurrentPosition`
- [x] `AudioPlayer.dispose`
- [ ] `AudioLogger.logLevel` (not supported)
- [ ] `AudioPlayer.global.setAudioContext` (not supported)

## Limitations

- `onPlayerComplete` event will not be fired when `ReleaseMode` is set to loop which differs from the behavior specified in the [documentation](https://pub.dev/documentation/audioplayers/latest/audioplayers/AudioPlayer/onPlayerComplete.html). And playback rate will reset to 1.0 when audio is replayed.
- `setVolume` will have no effect on TV devices.