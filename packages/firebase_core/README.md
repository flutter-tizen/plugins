# firebase_core_tizen

[![pub package](https://img.shields.io/pub/v/firebase_core_tizen.svg)](https://pub.dev/packages/firebase_core_tizen)

The Tizen implementation of [`firebase_core`](https://pub.dev/packages/firebase_core).

## Usage

This package is not an _endorsed_ implementation of `firebase_core`. Therefore, you have to include `firebase_core_tizen` alongside `firebase_core` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  firebase_core: ^2.4.0
  firebase_core_tizen: ^0.1.1
```

Then you can import `firebase_core` in your Dart code:

```dart
import 'package:firebase_core/firebase_core.dart';
```

For detailed usage, see https://github.com/invertase/flutterfire_desktop#flutterfire-desktop.

## Tizen integration

The implementation of `firebase_core_tizen` is based on [FlutterFire Desktop Core](https://pub.dev/packages/firebase_core_desktop). FlutterFire Desktop provides Dart implementations of the Firebase modules.
