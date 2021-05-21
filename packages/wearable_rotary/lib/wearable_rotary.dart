import 'package:flutter/services.dart';

const String _ROTARY_CHANNEL_NAME = 'flutter.wearable_rotary.channel';

const EventChannel _channel = EventChannel(_ROTARY_CHANNEL_NAME);

Stream<RotaryEvent> get rotaryEvent {
  _rotaryEvent ??= _channel
      .receiveBroadcastStream()
      .map((dynamic event) => _parseEvent(event as bool));
  return _rotaryEvent!;
}

Stream<RotaryEvent>? _rotaryEvent;

enum RotaryEvent {
  CLOCKWISE,
  COUNTER_CLOCKWISE,
}

RotaryEvent _parseEvent(bool clockwise) =>
    clockwise ? RotaryEvent.CLOCKWISE : RotaryEvent.COUNTER_CLOCKWISE;
