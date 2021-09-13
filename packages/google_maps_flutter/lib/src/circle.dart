// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of google_maps_flutter_tizen;

/// The `CircleController` class wraps a [GCircle] and its `onTap` behavior.
class CircleController {
  /// Creates a `CircleController`, which wraps a [GCircle] object and its `onTap` behavior.
  CircleController({
    required util.GCircle circle,
    bool consumeTapEvents = false,
    ui.VoidCallback? onTap,
    Future<WebViewController>? controller,
  })  : _circle = circle,
        _consumeTapEvents = consumeTapEvents,
        tapEvent = onTap {
    _addCircleEvent(controller);
  }

  util.GCircle? _circle;
  final bool _consumeTapEvents;

  /// Circle component's tap event.
  ui.VoidCallback? tapEvent;

  Future<void> _addCircleEvent(Future<WebViewController>? _controller) async {
    final String command =
        "$_circle.addListener('click', (event) => CircleClick.postMessage(JSON.stringify(${_circle?.id})));";
    await (await _controller!).evaluateJavascript(command);
  }

  /// Returns `true` if this Controller will use its own `onTap` handler to consume events.
  bool get consumeTapEvents => _consumeTapEvents;

  /// Updates the options of the wrapped [GCircle] object.
  /// This cannot be called after [remove].
  void update(util.GCircleOptions options) {
    assert(_circle != null, 'Cannot `update` Circle after calling `remove`.');
    _circle!.options = options;
  }

  /// Disposes of the currently wrapped [GCircle].
  void remove() {
    if (_circle != null) {
      _circle!.visible = false;
      _circle!.radius = 0;
      _circle!.map = null;
      _circle = null;
    }
  }
}
