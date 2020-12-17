# battery_tizen

The Tizen implementation of [`battery`](https://github.com/flutter/plugins/tree/master/packages/battery).

## Supported devices

This plugin is available on these types of devices:

- Galaxy Watch (running Tizen 5.5 or later)

# Usage

You can use the Battery to access various information about the battery of the device the app is running on.
This works on Tizen platform.

You can run an example like below.
$ cd example
$ flutter-tizen run -d tizen

```dart
// Import package
import 'package:battery/battery.dart';

// Instantiate it
var _battery = Battery();

// Access current battery level
print(await _battery.batteryLevel);

// Be informed when the state (full, charging, discharging) changes
_battery.onBatteryStateChanged.listen((BatteryState state) {
  // Do something with new state
});
```
