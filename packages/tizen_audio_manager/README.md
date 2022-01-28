# tizen_audio_manager

[![pub package](https://img.shields.io/pub/v/tizen_audio_manager.svg)](https://pub.dev/packages/tizen_audio_manager)

A Flutter plugin that allows setting and getting the volume level for different audio types as well as getting current playback type.

## Usage

To use this package, add `tizen_audio_manager` as a dependency in your `pubspec.yaml` file.

```yaml
dependencies:
  tizen_audio_manager: ^0.1.0
```

## Required privileges

In order to set the volume, add the following privileges to your `tizen-manifest.xml` file:

```xml
<privileges>
  <privilege>http://tizen.org/privilege/volume.set</privilege>
</privileges>
```

## Getting and setting the volume level

Get the maximum volume level for `alarm` media type:

```dart
final level = await AudioManager.volumeController.getMaxLevel(AudioVolumeType.alarm);
```

Get the current level for `alarm` media type:

```dart
final level = await AudioManager.volumeController.getLevel(AudioVolumeType.alarm);
```

Set new volume level for `alarm` media type:

```dart
final newLevel = 10;
AudioManager.volumeController.setLevel(AudioVolumeType.alarm, newLevel);
```

## Listening to volume changes

You can detect volume changes using `AudioManager.volumeController.onChanged`.

```dart
_subscription = AudioManager.volumeController.onChanged.listen((event) {
    final mediaType = event.type;
    final newLevel = event.level;
});
_subscription.cancel();
```

## Getting current playback type

Use the following code to get currently playing playback type:

```dart
final type = await AudioManager.volumeController.currentPlaybackType;
print(type);
```

## Available types

You can get and set the volume level for the following audio types:

```dart
enum AudioVolumeType {
  system,
  notification,
  alarm,
  ringtone,
  media,
  call,
  voip,
  voice,
  none
}
```
