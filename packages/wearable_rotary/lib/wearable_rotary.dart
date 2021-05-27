import 'package:flutter/services.dart';

const String _channelName = 'flutter.wearable_rotary.channel';

const EventChannel _channel = EventChannel(_channelName);

Stream<RotaryEvent> get rotaryEvent {
  _rotaryEvent ??= _channel
      .receiveBroadcastStream()
      .map((dynamic event) => _parseEvent(event));
  return _rotaryEvent!;
}

Stream<RotaryEvent>? _rotaryEvent;

enum RotaryEvent {
  clockwise,
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
