// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of google_maps_flutter_tizen;

// Marker Size
const int _markerWidth = 24;
const int _markerHeight = 43;

/// The `MarkerController` class wraps a [GMarker], how it handles events, and its associated (optional) [GInfoWindow] widget.
class MarkerController {
  /// Creates a `MarkerController`, which wraps a [GMarker] object, its `onTap`/`onDrag` behavior, and its associated [GInfoWindow].
  MarkerController({
    required util.GMarker marker,
    util.GInfoWindow? infoWindow,
    bool consumeTapEvents = false,
    LatLngCallback? onDragEnd,
    ui.VoidCallback? onTap,
    Future<WebViewController>? controller,
  })  : _marker = marker,
        _infoWindow = infoWindow,
        _consumeTapEvents = consumeTapEvents,
        tapEvent = onTap,
        dragEndEvent = onDragEnd {
    if (controller != null) {
      _addMarkerEvent(controller);
    }
  }

  util.GMarker? _marker;
  final bool _consumeTapEvents;
  final util.GInfoWindow? _infoWindow;
  bool _infoWindowShown = false;

  /// Marker component's tap event.
  ui.VoidCallback? tapEvent;

  /// Marker component's drag end event.
  LatLngCallback? dragEndEvent;

  Future<void> _addMarkerEvent(Future<WebViewController>? _controller) async {
    final String command =
        '''$marker.addListener("click", (event) => MarkerClick.postMessage(JSON.stringify(${marker?.id})));
        $marker.addListener("dragend", (event) => MarkerDragEnd.postMessage(JSON.stringify({id:${marker?.id}, event:event})));''';
    await (await _controller!).evaluateJavascript(command);
  }

  /// Returns `true` if this Controller will use its own `onTap` handler to consume events.
  bool get consumeTapEvents => _consumeTapEvents;

  /// Returns `true` if the [GInfoWindow] associated to this marker is being shown.
  bool get infoWindowShown => _infoWindowShown;

  /// Returns the [GMarker] associated to this controller.
  util.GMarker? get marker => _marker;

  /// Updates the options of the wrapped [GMarker] object.
  ///
  /// This cannot be called after [remove].
  void update(
    Marker marker,
    util.GMarkerOptions options, {
    String? newInfoWindowContent,
  }) {
    assert(_marker != null, 'Cannot `update` Marker after calling `remove`.');
    if (_infoWindow != null && newInfoWindowContent != null) {
      _infoWindow!.content = newInfoWindowContent;
      _infoWindow!.pixelOffset = util.GSize(
          (marker.infoWindow.anchor.dx - 0.5) * _markerWidth,
          marker.infoWindow.anchor.dy * _markerHeight);
    }
    _marker!.options = options;
    if (!marker.visible) {
      hideInfoWindow();
    }
  }

  /// Disposes of the currently wrapped [GMarker].
  void remove() {
    if (_marker != null) {
      _infoWindowShown = false;
      _marker!.visible = false;
      _marker!.map = null;
      _marker = null;
    }
  }

  /// Hide the associated [GInfoWindow].
  ///
  /// This cannot be called after [remove].
  void hideInfoWindow() {
    assert(_marker != null, 'Cannot `hideInfoWindow` on a `remove`d Marker.');
    if (_infoWindow != null) {
      _infoWindow!.close();
      _infoWindowShown = false;
    }
  }

  /// Show the associated [GInfoWindow].
  ///
  /// This cannot be called after [remove].
  void showInfoWindow() {
    assert(_marker != null, 'Cannot `showInfoWindow` on a `remove`d Marker.');
    if (_infoWindow != null) {
      _infoWindow!.open(_marker);
      _infoWindowShown = true;
    }
  }
}
