# firebase_core_tizen

[![pub package](https://img.shields.io/pub/v/firebase_core_tizen.svg)](https://pub.dev/packages/firebase_core_tizen)

The Tizen implementation of [`firebase_core`](https://github.com/flutter/plugins/tree/master/packages/firebase_core).

## Usage

This package is not an _endorsed_ implementation of `firebase_core`. Therefore, you have to include `firebase_core_tizen` alongside `firebase_core` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  firebase_core: ^1.24.0
  firebase_core_tizen: ^0.1.0
```

Then you can import `firebase_core` in your Dart code:

```dart
import 'package:firebase_core/firebase_core.dart';
```

### Initialize Default App

Unlike `firebase_core` for iOS, Android, macOS and Web, there's no need for platform specific config files to initialize the default Firebase app, 
instead, add your configurations as options to `initializeApp` method without a name.
```dart
const firebaseOptions = FirebaseOptions(
  appId: '...',
  apiKey: '...',
  projectId: '...',
  messagingSenderId: '...',
  authDomain: '...',
);

await Firebase.initializeApp(options: firebaseOptions);
```

Note that initialization should happen before any other usage of FlutterFire plugins.

### Initialize Secondary Apps

To initialize a secondary app, provide the name to `initializeApp` method:
```dart
await Firebase.initializeApp(app: 'foo', options: firebaseOptions);
```
