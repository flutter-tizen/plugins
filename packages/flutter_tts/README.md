# flutter_tts_tizen

[![pub package](https://img.shields.io/pub/v/flutter_tts_tizen.svg)](https://pub.dev/packages/flutter_tts_tizen)

The tizen implementation of [`flutter_tts`](https://github.com/dlutton/flutter_tts).

## Getting Started

This package is not an _endorsed_ implementation of `flutter_tts`. Therefore, you have to include `flutter_tts_tizen` alongside `flutter_tts` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  flutter_tts: ^3.2.2
  flutter_tts_tizen: ^1.2.0
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
 - [x] set speech rate
 - [x] set speech volume (requires privilege `http://tizen.org/privilege/volume.set` in `tizen_manifest.xml`)
