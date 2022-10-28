import 'dart:async';

import 'package:flutter/material.dart';
import 'package:wearable_rotary/wearable_rotary.dart';

/// A [ScrollController] that responds to [RotaryEvent]s.
class RotaryScrollController extends ScrollController {
  /// Constructor
  RotaryScrollController({this.fallbackIncrement = 50});

  StreamSubscription<RotaryEvent>? _subscription;

  /// The amount to scroll if the [RotaryEvent.magnitude] is null.
  final double fallbackIncrement;

  void _onRotaryEvent(RotaryEvent event) {
    final double scrollIncrement = event.magnitude ?? fallbackIncrement;
    if (event.direction == RotaryDirection.clockwise) {
      jumpTo(offset + scrollIncrement);
    } else {
      jumpTo(offset - scrollIncrement);
    }
  }

  @override
  void attach(ScrollPosition position) {
    _subscription ??= rotaryEvents.listen(_onRotaryEvent);
    super.attach(position);
  }

  @override
  void detach(ScrollPosition position) {
    _subscription?.cancel();
    super.detach(position);
  }

  @override
  void dispose() {
    _subscription?.cancel();
    super.dispose();
  }
}
