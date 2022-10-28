# wearable_rotary

 [![pub package](https://img.shields.io/pub/v/wearable_rotary.svg)](https://pub.dev/packages/wearable_rotary)

Flutter plugin that can listen to rotary events on Wear OS and Tizen Galaxy watch devices.

## Setup

### Android

Add the following to `MainActivity.kt`:

```kotlin
import android.view.MotionEvent
import com.samsung.wearable_rotary.WearableRotaryPlugin

class MainActivity : FlutterActivity() {
    override fun onGenericMotionEvent(event: MotionEvent?): Boolean {
        return when {
            WearableRotaryPlugin.onGenericMotionEvent(event) -> true
            else -> super.onGenericMotionEvent(event)
        }
    }
}
```

## Usage

To use this plugin, add `wearable_rotary` as a dependency in your `pubspec.yaml` file.

```yaml
dependencies:
  wearable_rotary: ^2.0.0
```

Then, import `wearable_rotary` in your Dart code.

```dart
// Import the package.
import 'package:wearable_rotary/wearable_rotary.dart';

// Be informed when an event (RotaryEvent.clockwise or RotaryEvent.counterClockwise) occurs.
StreamSubscription<RotaryEvent> rotarySubscription =
    rotaryEvents.listen((RotaryEvent event) {
  if (event.direction == RotaryEvent.clockwise) {
    // Do something.
  } else if (event.direction == RotaryEvent.counterClockwise) {
    // Do something.
  }
});

// Be sure to cancel on dispose.
rotarySubscription.cancel();

// Use [RotaryScrollController] to easily make scrolling widgets respond to rotary input.
ListView(controller: RotaryScrollController());
```

## Supported devices

- Wear OS devices with rotary input (Galaxy Watch 4, Pixel Watch, etc.)
- Galaxy Watch series (running Tizen 4.0 or later)
