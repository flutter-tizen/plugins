# sensors_plus_tizen

[![pub package](https://img.shields.io/pub/v/sensors_plus_tizen.svg)](https://pub.dev/packages/sensors_plus_tizen)

The Tizen implementation of [`sensors_plus`](https://pub.dev/packages/sensors_plus).

## Usage

This package is not an _endorsed_ implementation of 'sensors_plus'. Therefore, you have to include `sensors_plus_tizen` alongside `sensors_plus` as dependencies in your `pubspec.yaml` file.

```yaml
dependencies:
  sensors_plus: ^1.2.1
  sensors_plus_tizen: ^1.1.2
```

Then you can import `sensors_plus` in your Dart code:

```dart
import 'package:sensors_plus/sensors_plus.dart';
```

For detailed usage, see https://pub.dev/packages/sensors_plus#usage.

## Supported devices

- Galaxy Watch series (running Tizen 5.5)

## Supported APIs

- [x] `accelerometerEvents` (maps to [`SENSOR_ACCELEROMETER`](https://docs.tizen.org/application/native/guides/location-sensors/device-sensors/#accelerometer))
- [x] `gyroscopeEvents` (maps to [`SENSOR_GYROSCOPE`](https://docs.tizen.org/application/native/guides/location-sensors/device-sensors/#gyroscope))
- [x] `userAccelerometerEvents` (maps to [`SENSOR_LINEAR_ACCELERATION`](https://docs.tizen.org/application/native/guides/location-sensors/device-sensors/#linear-acceleration-sensor))
- [ ] `magnetometerEvents` (no supported devices)

## Notes

You need to declare one or more of the following features in your `tizen-manifest.xml` if you plan to release your app on the app store (to enable [feature-based filtering](https://docs.tizen.org/application/native/tutorials/details/app-filtering)).

```xml
<feature name="http://tizen.org/feature/sensor.accelerometer"/>
<feature name="http://tizen.org/feature/sensor.gyroscope"/>
<feature name="http://tizen.org/feature/sensor.linear_acceleration"/>
```
