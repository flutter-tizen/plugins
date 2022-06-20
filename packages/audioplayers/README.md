# audioplayers_tizen

[![pub package](https://img.shields.io/pub/v/audioplayers_tizen.svg)](https://pub.dev/packages/audioplayers_tizen)

The Tizen implementation of [`audioplayers`](https://github.com/luanpotter/audioplayers).

## Usage

This package is not an _endorsed_ implementation of `audioplayers`. Therefore, you have to include `audioplayers_tizen` alongside `audioplayers` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  audioplayers: ^0.20.1
  audioplayers_tizen: ^1.1.0

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

- [x] `AudioPlayer.play` (supported arguments: `url`, `volume`, `position`)
- [x] `AudioPlayer.playBytes` (supported arguments: `bytes`, `volume`, `position`)
- [x] `AudioPlayer.pause`
- [x] `AudioPlayer.stop`
- [x] `AudioPlayer.resume`
- [x] `AudioPlayer.release`
- [x] `AudioPlayer.seek`
- [x] `AudioPlayer.setVolume`
- [x] `AudioPlayer.setReleaseMode`
- [x] `AudioPlayer.setPlaybackRate`
- [x] `AudioPlayer.setUrl` (supported arguments: `url`)
- [x] `AudioPlayer.getDuration`
- [x] `AudioPlayer.getCurrentPosition`
- [x] `AudioPlayer.dispose`
- [ ] `AudioPlayer.earpieceOrSpeakersToggle` (not supported by Tizen)
- [ ] `Logger.changeLogLevel` (not implemented)
- [ ] `NotificationService` (iOS-only)

Note: In order to use the `AudioPlayer.playBytes` method, you need to manually modify the source code (`audioplayers.dart`) of your cached audioplayers package.

```dart
Future<int> playBytes(
  Uint8List bytes, {
  ...
}) async {
  // Delete or comment out the following lines.
  // if (!_isAndroid()) {
  //   throw PlatformException(
  //     code: 'Not supported',
  //     message: 'Only Android is currently supported',
  //   );
  // }

  final result = await _invokeMethod(
    'playBytes',
    <String, dynamic>{
  ...
}
```

## Limitations

- `onPlayerComplete` event will not be fired when `ReleaseMode` is set to loop which differs from the behavior specified in the [documentation](https://github.com/bluefireteam/audioplayers/blob/e6532371a3372c0f208abb24f13e1e82a9c1e040/packages/audioplayers/lib/src/audioplayer.dart#L64). And playback rate will reset to 1.0 when audio is replayed.
- `setVolume` will have no effect on TV devices.