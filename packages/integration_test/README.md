# integration_test_tizen

[![pub package](https://img.shields.io/pub/v/integration_test_tizen.svg)](https://pub.dev/packages/integration_test_tizen)

The Tizen implementation of [`integration_test`](https://github.com/flutter/flutter/tree/master/packages/integration_test).

## Usage

This package is not an _endorsed_ implementation of `integration_test`. Therefore, you have to include `integration_test_tizen` alongside `integration_test` as dependencies in your `pubspec.yaml` file.

```yaml
dev_dependencies:
  integration_test:
    sdk: flutter
  integration_test_tizen: ^2.0.0
```

Then you can import `integration_test` in your Dart code:

```dart
import 'package:integration_test/integration_test.dart';
```

For detailed usage, see https://github.com/flutter/flutter/tree/master/packages/integration_test#usage.
