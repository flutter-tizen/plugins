# audioplayers_tizen

The Tizen implementation of [`audioplayers`](https://github.com/luanpotter/audioplayers).

## Usage

This package is not an _endorsed_ implementation of `audioplayers`. Therefore, you have to include `audioplayers_tizen` alongside `audioplayers` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  audioplayers: ^0.18.3
  audioplayers_tizen: ^1.0.1
```

Then you can import `audioplayers` in your Dart code:

```dart
import 'package:audioplayers/audio_cache.dart';
import 'package:audioplayers/audioplayers.dart';
```

For detailed usage, see https://github.com/luanpotter/audioplayers#usage.

## Required privileges

To use this plugin in a Tizen application, the mediastorage, externalstorage and internet privileges may be required. Add below lines under the `<manifest>` section in your `tizen-manifest.xml` file.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/mediastorage</privilege>
  <privilege>http://tizen.org/privilege/externalstorage</privilege>
  <privilege>http://tizen.org/privilege/internet</privilege>
</privileges>
```

- The mediastorage privilege (`http://tizen.org/privilege/mediastorage`) must be added to play audio files located in the internal storage.
- The externalstorage privilege (`http://tizen.org/privilege/externalstorage`) must be added to play audio files located in the external storage.
- The internet privilege (`http://tizen.org/privilege/internet`) must be added to play any URLs from network.

For details about Tizen privileges, see [Security and API Privileges](https://docs.tizen.org/application/dotnet/tutorials/sec-privileges).

## Some notes

`earpieceOrSpeakersToggle` isn't supported on Tizen. `AudioPlay` will change stream routing automatically on Tizen, you can't choose speakers or earpiece.

`AudioPlay` supports `playBytes` on Tizen, to use this api, you have to modify the `audioplayers.dart`.

```dart
  Future<int> playBytes(
    Uint8List bytes, {
    double volume = 1.0,
    // position must be null by default to be compatible with radio streams
    Duration position,
    bool respectSilence = false,
    bool stayAwake = false,
    bool duckAudio = false,
    bool recordingActive = false,
  }) async {
    volume ??= 1.0;
    respectSilence ??= false;
    stayAwake ??= false;

    // To use this api on Tizen, you have to delete the following code
    ///if (!Platform.isAndroid) {
    ///  throw PlatformException(
    ///    code: 'Not supported',
    ///    message: 'Only Android is currently supported',
    ///  );
    ///}

    final int result = await _invokeMethod('playBytes', {
      'bytes': bytes,
      'volume': volume,
      'position': position?.inMilliseconds,
      'respectSilence': respectSilence,
      'stayAwake': stayAwake,
      'duckAudio': duckAudio,
      'recordingActive': recordingActive,
    });

    if (result == 1) {
      state = AudioPlayerState.PLAYING;
    }

    return result;
  }
```

## Limitations

This plugin has some limitations on TV:

- Don't change playback speed at last 3 senconds when playing audio, otherwise it will be failed.
- If playing a audio in a loop and change the playback speed(not 1.0) successfully, the playback speed will be recovered to 1.0 when play video again.
- Don't use seekTo if playback speed isn't 1.0, seekTo doesn't work after change playback speed.
- The audio has some key frames, seekTo will set position to the key frame. For example, the audio has key frame on 0 second and 3 second, actually the position is at 3 second when seek the position to 2 second.
