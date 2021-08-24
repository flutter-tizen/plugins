// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of google_maps_flutter_tizen;

/// This class manages a set of [PolylinesController]s associated to a [GoogleMapController].
class PolylinesController extends GeometryController {
  // A cache of [PolylineController]s indexed by their [PolylineId].
  final Map<PolylineId, PolylineController> _polylineIdToController;
  final Map<int, PolylineId> _idToPolylineId;

  // The stream over which polylines broadcast their events
  StreamController<MapEvent> _streamController;

  /// Initializes the cache. The [StreamController] comes from the [GoogleMapController], and is shared with other controllers.
  PolylinesController({
    required StreamController<MapEvent> stream,
  })  : _streamController = stream,
        _polylineIdToController = Map<PolylineId, PolylineController>(),
        _idToPolylineId = Map<int, PolylineId>();

  /// Adds a set of [Polyline] objects to the cache.
  ///
  /// Wraps each line into its corresponding [PolylineController].
  void addPolylines(Set<Polyline> polylinesToAdd) {
    polylinesToAdd.forEach((polyline) {
      _addPolyline(polyline);
    });
  }

  void _addPolyline(Polyline polyline) {
    if (polyline == null) {
      return;
    }

    final polylineOptions = _polylineOptionsFromPolyline(polyline);
    util.GPolyline gPolyline = util.GPolyline(polylineOptions);
    PolylineController controller = PolylineController(
        polyline: gPolyline,
        consumeTapEvents: polyline.consumeTapEvents,
        onTap: () {
          _onPolylineTap(polyline.polylineId);
        },
        controller: util.webController);
    _idToPolylineId[gPolyline.id] = polyline.polylineId;
    _polylineIdToController[polyline.polylineId] = controller;
  }

  /// Updates a set of [Polyline] objects with new options.
  void changePolylines(Set<Polyline> polylinesToChange) {
    polylinesToChange.forEach((polylineToChange) {
      _changePolyline(polylineToChange);
    });
  }

  void _changePolyline(Polyline polyline) {
    PolylineController? polylineController =
        _polylineIdToController[polyline.polylineId];
    polylineController?.update(_polylineOptionsFromPolyline(polyline));
  }

  /// Removes a set of [PolylineId]s from the cache.
  void removePolylines(Set<PolylineId> polylineIdsToRemove) {
    polylineIdsToRemove.forEach((polylineId) {
      final PolylineController? polylineController =
          _polylineIdToController[polylineId];
      polylineController?.remove();
      _polylineIdToController.remove(polylineId);
    });
  }

  // Handle internal events

  bool _onPolylineTap(PolylineId polylineId) {
    // Have you ended here on your debugging? Is this wrong?
    // Comment here: https://github.com/flutter/flutter/issues/64084
    _streamController.add(PolylineTapEvent(mapId, polylineId));
    return _polylineIdToController[polylineId]?.consumeTapEvents ?? false;
  }
}
