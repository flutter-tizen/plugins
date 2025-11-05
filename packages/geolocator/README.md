# geolocator_tizen

[![pub package](https://img.shields.io/pub/v/geolocator_tizen.svg)](https://pub.dev/packages/geolocator_tizen)

The Tizen implementation of [`geolocator`](https://pub.dev/packages/geolocator).

## Usage

 This package is not an _endorsed_ implementation of `geolocator`. Therefore, you have to include `geolocator_tizen` alongside `geolocator` as dependencies in your `pubspec.yaml` file.

 ```yaml
dependencies:
  geolocator: ^8.0.0
  geolocator_tizen: ^1.0.7
```

Then you can import `geolocator` in your Dart code:

```dart
import 'package:geolocator/geolocator.dart';
```

For detailed usage, see https://pub.dev/packages/geolocator#usage.

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

- Galaxy Watch series (running Tizen 5.5)

## Supported APIs

- [x] `Geolocator.isLocationServiceEnabled`
- [x] `Geolocator.getServiceStatusStream`
- [x] `Geolocator.checkPermission`
- [x] `Geolocator.requestPermission`
- [x] `Geolocator.getLastKnownPosition`
- [x] `Geolocator.getCurrentPosition` (supported arguments: `timeLimit`)
- [x] `Geolocator.getPositionStream` (supported arguments: `locationSettings.timeLimit`)
- [ ] `Geolocator.getLocationAccuracy`
- [ ] `Geolocator.requestTemporaryFullAccuracy` (iOS-only)
- [x] `Geolocator.openAppSettings` (not supported on emulators)
- [x] `Geolocator.openLocationSettings` (not supported on emulators)
