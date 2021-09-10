// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of google_maps_flutter_tizen;

/// The `PolygonController` class wraps a [GPolyline] and its `onTap` behavior.
class PolylineController {
  /// Creates a `PolylineController` that wraps a [GPolyline] object and its `onTap` behavior.
  PolylineController({
    required util.GPolyline polyline,
    bool consumeTapEvents = false,
    ui.VoidCallback? onTap,
    Future<WebViewController>? controller,
  })  : _polyline = polyline,
        _consumeTapEvents = consumeTapEvents,
        tapEvent = onTap {
    _addPolylineEvent(controller);
  }

  util.GPolyline? _polyline;
  final bool _consumeTapEvents;

  /// Polyline component's tap event.
  ui.VoidCallback? tapEvent;

  Future<void> _addPolylineEvent(Future<WebViewController>? _controller) async {
    final String command =
        "$_polyline.addListener('click', (event) => PolylineClick.postMessage(JSON.stringify(${_polyline?.id})));";
    await (await _controller!).evaluateJavascript(command);
  }

  /// Returns `true` if this Controller will use its own `onTap` handler to consume events.
  bool get consumeTapEvents => _consumeTapEvents;

  /// Updates the options of the wrapped [GPolyline] object.
  ///
  /// This cannot be called after [remove].
  void update(util.GPolylineOptions options) {
    assert(
        _polyline != null, 'Cannot `update` Polyline after calling `remove`.');
    _polyline!.options = options;
  }

  /// Disposes of the currently wrapped [GPolyline].
  void remove() {
    if (_polyline != null) {
      _polyline!.visible = false;
      _polyline!.map = null;
      _polyline = null;
    }
  }
}
