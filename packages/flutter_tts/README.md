# flutter_tts_tizen

[![pub package](https://img.shields.io/pub/v/flutter_tts_tizen.svg)](https://pub.dev/packages/flutter_tts_tizen)

The Tizen implementation of [`flutter_tts`](https://pub.dev/packages/flutter_tts).

## Getting Started

This package is not an _endorsed_ implementation of `flutter_tts`. Therefore, you have to include `flutter_tts_tizen` alongside `flutter_tts` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  flutter_tts: ^4.2.0
  flutter_tts_tizen: ^1.5.1
```

Then you can import `flutter_tts` in your Dart code:

```dart
import 'package:flutter_tts/flutter_tts.dart';
```

For detailed usage, see https://pub.dev/packages/flutter_tts#usage.

## Supported APIs

The features supported by Tizen are as follows. Other features are not supported.

 - [x] speak
 - [x] stop
 - [x] pause
 - [x] get languages
 - [x] set language
 - [x] is language available
 - [x] get voices
 - [x] set voice
 - [x] set speech rate
 - [x] set speech volume (requires privilege `http://tizen.org/privilege/volume.set` in `tizen_manifest.xml`)
 - [x] get default voice
 - [x] get max speech input length
