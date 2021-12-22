# connectivity_tizen

---

## Deprecation Notice

The `connectivity` plugin has been replaced by the [Flutter Community Plus Plugins](https://plus.fluttercommunity.dev) version, [`connectivity_plus`](https://pub.dev/packages/connectivity_plus). Consider migrating to `connectivity_plus` and its Tizen implementation [`connectivity_plus_tizen`](https://pub.dev/packages/connectivity_plus_tizen).

---

[![pub package](https://img.shields.io/pub/v/connectivity_tizen.svg)](https://pub.dev/packages/connectivity_tizen)

The Tizen implementation of [`connectivity`](https://pub.dev/packages/connectivity).

## Usage

This package is not an _endorsed_ implementation of `connectivity`. Therefore, you have to include `connectivity_tizen` alongside `connectivity` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  connectivity: ^3.0.3
  connectivity_tizen: ^2.0.2
```

Then you can import `connectivity` in your Dart code:

```dart
import 'package:connectivity/connectivity.dart';
```

For detailed usage, see https://pub.dev/packages/connectivity#usage.

## Required privileges

To get connectivity information using this plugin, add below lines under the `<manifest>` section in your `tizen-manifest.xml` file,

```xml
<privileges>
  <privilege>http://tizen.org/privilege/network.get</privilege>
</privileges>
```
