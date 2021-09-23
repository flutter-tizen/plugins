// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of google_maps_flutter_tizen;

/// This class manages a set of [PolygonController]s associated to a [GoogleMapController].
class PolygonsController extends GeometryController {
  /// Initializes the cache. The [StreamController] comes from the [GoogleMapController], and is shared with other controllers.
  PolygonsController({
    required StreamController<MapEvent> stream,
  })  : _streamController = stream,
        _polygonIdToController = <PolygonId, PolygonController>{},
        _idToPolygonId = <int, PolygonId>{};

  // A cache of [PolygonController]s indexed by their [PolygonId].
  final Map<PolygonId, PolygonController> _polygonIdToController;
  final Map<int, PolygonId> _idToPolygonId;

  // The stream over which polygons broadcast events
  final StreamController<MapEvent> _streamController;

  /// Adds a set of [Polygon] objects to the cache.
  ///
  /// Wraps each Polygon into its corresponding [PolygonController].
  void addPolygons(Set<Polygon> polygonsToAdd) {
    polygonsToAdd.forEach(_addPolygon);
  }

  void _addPolygon(Polygon? polygon) {
    if (polygon == null) {
      return;
    }

    final util.GPolygonOptions populationOptions =
        _polygonOptionsFromPolygon(polygon);
    final util.GPolygon gPolygon = util.GPolygon(populationOptions);
    final PolygonController controller = PolygonController(
        polygon: gPolygon,
        consumeTapEvents: polygon.consumeTapEvents,
        onTap: () {
          _onPolygonTap(polygon.polygonId);
        },
        controller: util.webController);
    _idToPolygonId[gPolygon.id] = polygon.polygonId;
    _polygonIdToController[polygon.polygonId] = controller;
  }

  /// Updates a set of [Polygon] objects with new options.
  void changePolygons(Set<Polygon> polygonsToChange) {
    polygonsToChange.forEach(_changePolygon);
  }

  void _changePolygon(Polygon polygon) {
    final PolygonController? polygonController =
        _polygonIdToController[polygon.polygonId];
    polygonController?.update(_polygonOptionsFromPolygon(polygon));
  }

  /// Removes a set of [PolygonId]s from the cache.
  void removePolygons(Set<PolygonId> polygonIdsToRemove) {
    polygonIdsToRemove.forEach(_removePolygon);
  }

  void _removePolygon(PolygonId polygonId) {
    final PolygonController? polygonController =
        _polygonIdToController[polygonId];
    polygonController?.remove();
    _polygonIdToController.remove(polygonId);
  }

  // Handle internal events
  bool _onPolygonTap(PolygonId polygonId) {
    // Have you ended here on your debugging? Is this wrong?
    // Comment here: https://github.com/flutter/flutter/issues/64084
    _streamController.add(PolygonTapEvent(mapId, polygonId));
    return _polygonIdToController[polygonId]?.consumeTapEvents ?? false;
  }
}
