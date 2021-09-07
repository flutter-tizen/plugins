# geolocator_tizen

The Tizen implementation of [`geolocator`](https://github.com/Baseflow/flutter-geolocator/tree/master/geolocator).

## Getting Started

 This package is not an _endorsed_ implementation of `geolocator`. Therefore, you have to include `geolocator_tizen` alongside `geolocator` as dependencies in your `pubspec.yaml` file.

 ```yaml
dependencies:
  geolocator: ^7.4.0
  geolocator_tizen: ^1.0.0
```

Then you can import `geolocator` in your Dart code:

```dart
import 'package:geolocator/geolocator.dart';
```

For more details, see [here](https://github.com/Baseflow/flutter-geolocator/tree/master/geolocator#usage).

## Required privileges

To use this plugin, you need to declare privileges in `tizen-manifest.xml` of your application.

``` xml
<privileges>
  <privilege>http://tizen.org/privilege/appmanager.launch</privilege>
  <privilege>http://tizen.org/privilege/location</privilege>
  <privilege>http://tizen.org/privilege/location.coarse</privilege>
</privileges>
```

## Supported devices

This plugin is supported on these types of devices:

- Galaxy Watch (running Tizen 4.0 or later)
