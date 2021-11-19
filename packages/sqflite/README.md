# sqflite_tizen

The Tizen implementation of [`sqflite`](https://github.com/tekartik/sqflite).

## Getting Started

 This package is not an _endorsed_ implementation of `sqflite`. Therefore, you have to include `sqflite_tizen` alongside `sqflite` as dependencies in your `pubspec.yaml` file.

 ```yaml
dependencies:
  sqflite: ^2.0.1
  sqflite_tizen: ^0.1.0
```

Then you can import `sqflite` in your Dart code:

```dart
import 'package:sqflite/sqflite.dart';
```

For more details, see [here](https://github.com/tekartik/sqflite/blob/master/sqflite/README.md).

## Supported devices

This plugin is supported on these types of devices:

- Galaxy Watch (running Tizen 4.0 or later)
