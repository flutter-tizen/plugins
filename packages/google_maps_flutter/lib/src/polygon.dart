// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of google_maps_flutter_tizen;

/// The `PolygonController` class wraps a [GPolygon] and its `onTap` behavior.
class PolygonController {
  /// Creates a `PolygonController` that wraps a [GPolygon] object and its `onTap` behavior.
  PolygonController({
    required util.GPolygon polygon,
    bool consumeTapEvents = false,
    ui.VoidCallback? onTap,
    Future<WebViewController>? controller,
  })  : _polygon = polygon,
        _consumeTapEvents = consumeTapEvents,
        tapEvent = onTap {
    _addPolygonEvent(controller);
  }

  util.GPolygon? _polygon;
  final bool _consumeTapEvents;

  /// Polygon component's tap event.
  ui.VoidCallback? tapEvent;

  Future<void> _addPolygonEvent(Future<WebViewController>? _controller) async {
    final String command =
        "$_polygon.addListener('click', (event) => PolygonClick.postMessage(JSON.stringify(${_polygon?.id})));";
    await (await _controller!).evaluateJavascript(command);
  }

  /// Returns `true` if this Controller will use its own `onTap` handler to consume events.
  bool get consumeTapEvents => _consumeTapEvents;

  /// Updates the options of the wrapped [GPolygon] object.
  ///
  /// This cannot be called after [remove].
  void update(util.GPolygonOptions options) {
    assert(_polygon != null, 'Cannot `update` Polygon after calling `remove`.');
    _polygon!.options = options;
  }

  /// Disposes of the currently wrapped [GPolygon].
  void remove() {
    if (_polygon != null) {
      _polygon!.visible = false;
      _polygon!.map = null;
      _polygon = null;
    }
  }
}
