// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/services.dart';

const String _channelName = 'flutter.wearable_rotary.channel';

const EventChannel _channel = EventChannel(_channelName);

/// A broadcast stream of events from the device rotary sensor.
Stream<RotaryEvent> get rotaryEvents {
  return _rotaryEvents ??= _channel.receiveBroadcastStream().map(
        (dynamic event) => _parseEvent(event),
      );
}

Stream<RotaryEvent>? _rotaryEvents;

/// A rotary event.
class RotaryEvent {
  /// Constructor
  const RotaryEvent({required this.direction, this.magnitude});

  /// The direction of the rotary event.
  final RotaryDirection direction;

  /// The magnitude of the rotation.
  ///
  /// Null on Tizen devices. Tizen devices only report the [direction] of the
  /// event, and the rotation angle may differ by device.
  final double? magnitude;
}

/// The direction of the rotary event.
enum RotaryDirection {
  /// A rotation event in the clockwise direction.
  clockwise,

  /// A rotation event in the counter clockwise direction.
  counterClockwise,
}

RotaryEvent _parseEvent(dynamic event) {
  if (event is Map) {
    return RotaryEvent(
      direction: RotaryDirection.values.byName(event['direction'] as String),
      magnitude: event['magnitude'] as double,
    );
  } else if (event is bool) {
    return event
        ? const RotaryEvent(direction: RotaryDirection.clockwise)
        : const RotaryEvent(direction: RotaryDirection.counterClockwise);
  } else {
    throw PlatformException(
      code: 'type_cast',
      details: 'Platform plugin returns invalid type for rotary event',
    );
  }
}
