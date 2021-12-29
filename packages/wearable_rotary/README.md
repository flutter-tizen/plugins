# wearable_rotary

 [![pub package](https://img.shields.io/pub/v/wearable_rotary.svg)](https://pub.dev/packages/wearable_rotary)

Plugin that can listen to rotary events on Galaxy watch devices.

## Usage

To use this plugin, add `wearable_rotary` as a dependency in your `pubspec.yaml` file.

```yaml
dependencies:
  wearable_rotary: ^1.0.2
```

Then, import `wearable_rotary` in your Dart code.

```dart
// Import the package.
import 'package:wearable_rotary/wearable_rotary.dart';

// Be informed when an event (RotaryEvent.clockwise or RotaryEvent.counterClockwise) occurs.
StreamSubscription<RotaryEvent> rotarySubscription =
    rotaryEvents.listen((RotaryEvent event) {
  if (event == RotaryEvent.clockwise) {
    // Do something.
  } else if (event == RotaryEvent.counterClockwise) {
    // Do something.
  }
});

// Be sure to cancel on dispose.
rotarySubscription.cancel();
```

## Supported devices

- Galaxy Watch series (running Tizen 4.0 or later)
