// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of '../google_maps_flutter_tizen.dart';

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
    LatLngCallback? onDragStart,
    LatLngCallback? onDrag,
    LatLngCallback? onDragEnd,
    ui.VoidCallback? onTap,
    ClusterManagerId? clusterManagerId,
    WebViewController? controller,
  })  : _marker = marker,
        _infoWindow = infoWindow,
        _consumeTapEvents = consumeTapEvents,
        _clusterManagerId = clusterManagerId,
        tapEvent = onTap,
        dragStartEvent = onDragStart,
        dragEvent = onDrag,
        dragEndEvent = onDragEnd {
    if (controller != null) {
      _addMarkerEvent(controller);
    }
  }

  util.GMarker? _marker;
  final bool _consumeTapEvents;
  final ClusterManagerId? _clusterManagerId;
  final util.GInfoWindow? _infoWindow;
  bool _infoWindowShown = false;

  /// Marker component's tap event.
  ui.VoidCallback? tapEvent;

  /// Marker component's drag start event.
  LatLngCallback? dragStartEvent;

  /// Marker component's drag event.
  LatLngCallback? dragEvent;

  /// Marker component's drag end event.
  LatLngCallback? dragEndEvent;

  Future<void> _addMarkerEvent(WebViewController? controller) async {
    final String command = '''
        $marker.addListener("click", (event) => MarkerClick.postMessage(JSON.stringify(${marker?.id})));
        $marker.addListener("dragstart", (event) => MarkerDragStart.postMessage(JSON.stringify({id:${marker?.id}, event:event})));
        $marker.addListener("drag", (event) => MarkerDrag.postMessage(JSON.stringify({id:${marker?.id}, event:event})));
        $marker.addListener("dragend", (event) => MarkerDragEnd.postMessage(JSON.stringify({id:${marker?.id}, event:event})));''';
    await controller!.runJavaScript(command);
  }

  /// Returns `true` if this Controller will use its own `onTap` handler to consume events.
  bool get consumeTapEvents => _consumeTapEvents;

  /// Returns `true` if the [GInfoWindow] associated to this marker is being shown.
  bool get infoWindowShown => _infoWindowShown;

  /// Returns [ClusterManagerId] if marker belongs to cluster.
  ClusterManagerId? get clusterManagerId => _clusterManagerId;

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
    _marker!.options = options;
    if (_infoWindow != null && newInfoWindowContent != null) {
      _infoWindow.content = newInfoWindowContent;
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
      _infoWindow.close();
      _infoWindowShown = false;
    }
  }

  /// Show the associated [GInfoWindow].
  ///
  /// This cannot be called after [remove].
  void showInfoWindow() {
    assert(_marker != null, 'Cannot `showInfoWindow` on a `remove`d Marker.');
    if (_infoWindow != null) {
      _infoWindow.open(_marker);
      _infoWindowShown = true;
    }
  }
}
