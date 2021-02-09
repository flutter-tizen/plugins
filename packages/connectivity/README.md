# connectivity_tizen

The Tizen implementation of [`connectivity`](https://github.com/flutter/plugins/tree/master/packages/connectivity).

## Usage

This package is not an _endorsed_ implementation of `connectivity`. Therefore, you have to include `connectivity_tizen` alongside `connectivity` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  connectivity: ^2.0.2
  connectivity_tizen: ^1.0.0
```
Then you can import `connectivity` in your Dart code:

```dart
import 'package:connectivity/connectivity.dart';
```

For detailed usage, see https://github.com/flutter/plugins/tree/master/packages/connectivity/connectivity#usage.

## Supported devices

This plugin is supported on these types of devices:

- Galaxy Watch (running Tizen 4.0 or later)

## Required privileges

To get connectivity information using this plugin, add below lines under the `<manifest>` section in your `tizen-manifest.xml` file,

```xml
<privileges>
  <privilege>http://tizen.org/privilege/network.get</privilege>
</privileges>
```
