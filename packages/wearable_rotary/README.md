# wearable_rotary

## Usage

To use this plugin, add wearable_rotary as a dependency in your pubspec.yaml file:

```yaml
dependencies:
  wearable_rotary: ^1.0.0
```

## Example

```dart
// Import package
import 'package:wearable_rotary/wearable_rotary.dart';

// Be informed when the event (RotaryEvent.CLOCKWISE, RotaryEvent.COUNTER_CLOCKWISE) occurs
StreamSubscription<RotaryEvent> rotarySubscription =
    rotaryEvent.listen((RotaryEvent event) {
  if (event == RotaryEvent.CLOCKWISE) {
    // Do something
  } else if (event == RotaryEvent.COUNTER_CLOCKWISE) {
    // Do something
  }
});

// Be sure to cancel on dispose
rotarySubscription.cancel();
```

## Supported devices

This plugin is supported only on Galaxy watches.
