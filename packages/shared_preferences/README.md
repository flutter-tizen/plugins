# shared_preferences_tizen

The Tizen implementation of [`shared_preferences`](https://github.com/flutter/plugins/tree/master/packages/shared_preferences).

## Usage

This package is not an _endorsed_ implementation of `shared_preferences`. Therefore, you have to include `shared_preferences_tizen` alongside `shared_preferences` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  shared_preferences: ^0.5.12+4
  shared_preferences_tizen: ^1.0.1
```

Then you can import `shared_preferences` in your Dart code:

```dart
import 'package:shared_preferences/shared_preferences.dart';
```

For detailed usage, see https://github.com/flutter/plugins/tree/master/packages/shared_preferences/shared_preferences#usage.
