// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/services.dart';

const String _channelName = 'flutter.wearable_rotary.channel';

const EventChannel _channel = EventChannel(_channelName);

/// A broadcast stream of events from the device rotary sensor.
Stream<RotaryEvent> get rotaryEvents {
  return _rotaryEvents ??= _channel
      .receiveBroadcastStream()
      .map((dynamic event) => _parseEvent(event));
}

Stream<RotaryEvent>? _rotaryEvents;

/// A single "click" of rotation. The rotation angle may differ by device.
enum RotaryEvent {
  /// A rotation event in the clockwise direction.
  clockwise,

  /// A rotation event in the counter clockwise direction.
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
