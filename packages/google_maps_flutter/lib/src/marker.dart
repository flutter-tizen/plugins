// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of google_maps_flutter_tizen;

// Marker Size
const int markerWidth = 24;
const int markerHeight = 43;

/// The `MarkerController` class wraps a [GMarker], how it handles events, and its associated (optional) [GInfoWindow] widget.
class MarkerController {
  util.GMarker? _marker;

  final bool _consumeTapEvents;

  final util.GInfoWindow? _infoWindow;

  bool _infoWindowShown = false;

  ui.VoidCallback? _onTop;
  LatLngCallback? _onDragEnd;

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
        _consumeTapEvents = consumeTapEvents {
    if (controller != null) {
      _createMarkerController(controller);
      _onTop = onTap;
      _onDragEnd = onDragEnd;
    }
  }

  Future<void> _createMarkerController(
      Future<WebViewController>? _controller) async {
    String command =
        "$marker.addListener('click', (event) => MarkerClick.postMessage(JSON.stringify(${marker?.id})));";
    command +=
        "$marker.addListener('dragend', (event) => MarkerDragEnd.postMessage(JSON.stringify({id:${marker?.id}, event:event})));";
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
          (marker.infoWindow.anchor.dx - 0.5) * markerWidth,
          marker.infoWindow.anchor.dy * markerHeight);
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
