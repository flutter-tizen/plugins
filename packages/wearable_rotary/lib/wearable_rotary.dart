import 'package:flutter/services.dart';

const String _ROTARY_CHANNEL_NAME = 'flutter.wearable_rotary.channel';

const EventChannel _channel = EventChannel(_ROTARY_CHANNEL_NAME);

Stream<RotaryEvent> get rotaryEvent {
  _rotaryEvent ??= _channel
      .receiveBroadcastStream()
      .map((dynamic event) => _parseEvent(event));
  return _rotaryEvent!;
}

Stream<RotaryEvent>? _rotaryEvent;

enum RotaryEvent {
  CLOCKWISE,
  COUNTER_CLOCKWISE,
}

RotaryEvent _parseEvent(dynamic event) {
  if (event is bool) {
    return event ? RotaryEvent.CLOCKWISE : RotaryEvent.COUNTER_CLOCKWISE;
  } else {
    throw PlatformException(
        code: '_parseEvent',
        details: 'Platform plugin returns non-bool type for rotary event');
  }
}
