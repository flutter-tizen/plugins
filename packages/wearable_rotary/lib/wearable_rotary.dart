import 'package:flutter/services.dart';

const String _channelName = 'flutter.wearable_rotary.channel';

const EventChannel _channel = EventChannel(_channelName);

/// A broadcast stream of events from the device rotary sensor.
Stream<RotaryEvent> get rotaryEvents {
  _rotaryEvents ??= _channel
      .receiveBroadcastStream()
      .map((dynamic event) => _parseEvent(event));
  return _rotaryEvents!;
}

Stream<RotaryEvent>? _rotaryEvents;

/// Discrete reading from an rotary sensor. Rotary sensor measure the rotation
/// of the wearable device. The event refers to a single "click" of rotatation
/// angle which may differ by device.
enum RotaryEvent {
  /// A roation angle in the clockwise direction.
  clockwise,

  /// A roation angle in the counter clockwise direction.
  counterClockwise,
}

RotaryEvent _parseEvent(dynamic event) {
  if (event is bool) {
    return event ? RotaryEvent.clockwise : RotaryEvent.counterClockwise;
  } else {
    throw PlatformException(
        code: 'type_cast',
        details: 'Platform plugin returns non-bool type for rotary event');
  }
}
