// Copyright 2026 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';

/// Possible visibility states for the software keyboard (input panel).
enum KeyboardState {
  /// The state has not yet been reported by the platform.
  unknown,

  /// The keyboard is fully visible.
  visible,

  /// The keyboard is animating from hidden to visible.
  visibling,

  /// The keyboard is fully hidden.
  hidden,

  /// The keyboard is animating from visible to hidden.
  ///
  /// On Tizen the platform does not emit a "will hide" event, so this state is
  /// not reached on Tizen devices. It is kept for API parity with the
  /// `keyboard_detection` package.
  hiding,
}

/// A callback invoked when the keyboard state changes.
///
/// Return `true` to keep receiving events, `false` to unregister automatically.
typedef KeyboardDetectionCallback = FutureOr<bool> Function(
    KeyboardState state);

/// Detects software keyboard visibility on Tizen by listening to the
/// `tizen/internal/inputpanel` event channel exposed by the flutter-tizen
/// embedder.
///
/// Mirrors the API of the [keyboard_detection](https://pub.dev/packages/keyboard_detection)
/// package so application code can be ported to Tizen with minimal changes.
class KeyboardDetectionController {
  /// Creates a controller and immediately starts listening to keyboard events.
  ///
  /// [onChanged] is called on every state change. Use [registerCallback] to
  /// add additional listeners that may auto-unregister.
  KeyboardDetectionController({this.onChanged}) {
    _subscription = _channel.receiveBroadcastStream().listen(
      _handleEvent,
      onError: (Object _) {
        // Stay in unknown state if the channel raises an error (e.g. when
        // running on a host that does not provide the input panel channel).
      },
    );
  }

  static const EventChannel _channel = EventChannel(
    'tizen/internal/inputpanel',
  );

  /// Called whenever the keyboard state changes.
  final void Function(KeyboardState state)? onChanged;

  late final StreamSubscription<dynamic> _subscription;
  final Map<KeyboardDetectionCallback, bool> _callbacks =
      <KeyboardDetectionCallback, bool>{};
  final StreamController<KeyboardState> _streamController =
      StreamController<KeyboardState>.broadcast();
  final Completer<bool> _sizeLoaded = Completer<bool>();

  KeyboardState _state = KeyboardState.unknown;
  double _x = 0;
  double _y = 0;
  double _width = 0;
  double _size = 0;

  /// Broadcast stream of keyboard state changes.
  Stream<KeyboardState> get stream => _streamController.stream;

  /// The latest reported keyboard state.
  KeyboardState get state => _state;

  /// The last observed keyboard height in physical pixels reported by the
  /// Tizen input method context.
  ///
  /// Returns `0` until the keyboard has been visible at least once.
  double get size => _size;

  /// The last observed keyboard width in physical pixels.
  double get width => _width;

  /// The last observed top-left position of the keyboard in physical pixels.
  Offset get position => Offset(_x, _y);

  /// Whether [size] has been observed at least once.
  bool get isSizeLoaded => _sizeLoaded.isCompleted;

  /// Resolves once [size] has a non-zero value.
  Future<void> get ensureSizeLoaded => _sizeLoaded.future;

  /// Returns the current visibility as a boolean.
  ///
  /// - `null` while the state is [KeyboardState.unknown].
  /// - When [includeTransitionalState] is `false` (default), the transitional
  ///   [KeyboardState.visibling] / [KeyboardState.hiding] states are mapped to
  ///   their stable counterpart so the value does not depend on prior calls.
  bool? stateAsBool([bool includeTransitionalState = false]) {
    return switch (_state) {
      KeyboardState.unknown => null,
      KeyboardState.visibling => includeTransitionalState,
      KeyboardState.visible => true,
      KeyboardState.hiding => !includeTransitionalState,
      KeyboardState.hidden => false,
    };
  }

  /// Registers a callback to be invoked on every keyboard state change.
  ///
  /// The callback is removed automatically the first time it returns `false`.
  void registerCallback(KeyboardDetectionCallback callback) {
    _callbacks[callback] = true;
  }

  /// Removes a previously registered callback.
  void unregisterCallback(KeyboardDetectionCallback callback) {
    _callbacks.remove(callback);
  }

  /// Removes all registered callbacks.
  void unregisterAllCallbacks() {
    _callbacks.clear();
  }

  /// Releases native subscriptions. Call when the controller is no longer
  /// needed, typically from a [State.dispose].
  Future<void> dispose() async {
    await _subscription.cancel();
    await _streamController.close();
  }

  void _handleEvent(dynamic event) {
    if (event is! Map) {
      return;
    }
    final Object? state = event['state'];
    if (state is! String) {
      return;
    }

    final double? width = _readNumber(event['width']);
    final double? height = _readNumber(event['height']);
    if (width != null && height != null && width > 0 && height > 0) {
      _width = width;
      _size = height;
      _x = _readNumber(event['x']) ?? 0;
      _y = _readNumber(event['y']) ?? 0;
      if (!_sizeLoaded.isCompleted) {
        _sizeLoaded.complete(true);
      }
    } else if (state == 'hide') {
      _width = 0;
      _size = 0;
      _x = 0;
      _y = 0;
    }

    final KeyboardState next = switch (state) {
      'will_show' => KeyboardState.visibling,
      'show' => KeyboardState.visible,
      'will_hide' => KeyboardState.hiding,
      'hide' => KeyboardState.hidden,
      _ => KeyboardState.unknown,
    };
    _setState(next);
  }

  static double? _readNumber(Object? value) {
    if (value is num) {
      return value.toDouble();
    }
    return null;
  }

  void _setState(KeyboardState next) {
    _state = next;
    if (!_streamController.isClosed) {
      _streamController.add(next);
    }
    onChanged?.call(next);
    unawaited(_executeCallbacks(next));
  }

  Future<void> _executeCallbacks(KeyboardState state) async {
    final List<KeyboardDetectionCallback> targets = _callbacks.keys.toList();
    for (final KeyboardDetectionCallback callback in targets) {
      if (!_callbacks.containsKey(callback)) {
        continue;
      }
      final bool keep = await callback(state);
      if (!_callbacks.containsKey(callback)) {
        continue;
      }
      if (!keep) {
        _callbacks.remove(callback);
      }
    }
  }
}
