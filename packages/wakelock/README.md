# wakelock_tizen

The Tizen implementation of [`wakelock`](https://github.com/creativecreatorormaybenot/wakelock).

## Usage

This package is not an _endorsed_ implementation of `wakelock`. Therefore, you have to include `wakelock_tizen` alongside `wakelock` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  wakelock: ^0.5.6
  wakelock_tizen: ^1.0.1
```

Then you can import `wakelock` in your Dart code:

```dart
import 'package:wakelock/wakelock.dart';
```

For detailed usage, see https://pub.dev/packages/wakelock#usage.

## Required privileges

To use the wakelock plugin in a Tizen application, the display privilege must be added in your `tizen-manifest.xml` file. If you don't know where to place the privilege, see the `example/tizen/tizen-manifest.xml` file.

```xml
<privileges>
  <privilege>http://tizen.org/privilege/display</privilege>
</privileges>
```
