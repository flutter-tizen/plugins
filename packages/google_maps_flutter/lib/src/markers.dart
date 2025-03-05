// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of '../google_maps_flutter_tizen.dart';

/// This class manages a set of [MarkerController]s associated to a [GoogleMapController].
class MarkersController extends GeometryController {
  /// Initialize the cache. The [StreamController] comes from the [GoogleMapController], and is shared with other controllers.
  MarkersController({
    required StreamController<MapEvent<Object?>> stream,
    required ClusterManagersController clusterManagersController,
  }) : _streamController = stream,
       _clusterManagersController = clusterManagersController,
       _idToMarkerId = <int, MarkerId>{},
       _markerIdToController = <MarkerId, MarkerController>{};

  // A cache of [MarkerController]s indexed by their [MarkerId].
  final Map<MarkerId, MarkerController> _markerIdToController;
  final Map<int, MarkerId> _idToMarkerId;

  // The stream over which markers broadcast their events
  final StreamController<MapEvent<Object?>> _streamController;

  final ClusterManagersController _clusterManagersController;

  /// Adds a set of [Marker] objects to the cache.
  ///
  /// Wraps each [Marker] into its corresponding [MarkerController].
  void addMarkers(Set<Marker> markersToAdd) {
    markersToAdd.forEach(_addMarker);
  }

  void _addMarker(Marker? marker) {
    if (marker == null) {
      return;
    }

    final util.GInfoWindowOptions? infoWindowOptions =
        _infoWindowOptionsFromMarker(marker);
    util.GInfoWindow? infoWindow;

    if (infoWindowOptions != null) {
      infoWindow = util.GInfoWindow(infoWindowOptions);
    }

    final util.GMarkerOptions populationOptions = _markerOptionsFromMarker(
      marker,
      _markerIdToController[marker.markerId]?.marker,
    );
    final util.GMarker gMarker = util.GMarker(populationOptions);

    if (marker.clusterManagerId != null) {
      _clusterManagersController.addItem(marker.clusterManagerId!, gMarker);
    }

    final MarkerController markerController = MarkerController(
      marker: gMarker,
      clusterManagerId: marker.clusterManagerId,
      infoWindow: infoWindow,
      consumeTapEvents: marker.consumeTapEvents,
      onTap: () {
        showMarkerInfoWindow(marker.markerId);
        _onMarkerTap(marker.markerId);
      },
      onDragStart: (LatLng latLng) {
        _onMarkerDragStart(marker.markerId, latLng);
      },
      onDrag: (LatLng latLng) {
        _onMarkerDrag(marker.markerId, latLng);
      },
      onDragEnd: (LatLng latLng) {
        _onMarkerDragEnd(marker.markerId, latLng);
      },
      controller: util.webController,
    );
    _idToMarkerId[gMarker.id] = marker.markerId;
    _markerIdToController[marker.markerId] = markerController;
  }

  /// Updates a set of [Marker] objects with new options.
  void changeMarkers(Set<Marker> markersToChange) {
    markersToChange.forEach(_changeMarker);
  }

  void _changeMarker(Marker marker) {
    final MarkerController? markerController =
        _markerIdToController[marker.markerId];

    if (markerController != null) {
      final util.GMarkerOptions markerOptions = _markerOptionsFromMarker(
        marker,
        markerController.marker,
      );
      final util.GInfoWindowOptions? infoWindow = _infoWindowOptionsFromMarker(
        marker,
      );
      markerController.update(
        marker,
        markerOptions,
        newInfoWindowContent: infoWindow?.content,
      );
    }
  }

  /// Removes a set of [MarkerId]s from the cache.
  void removeMarkers(Set<MarkerId> markerIdsToRemove) {
    markerIdsToRemove.forEach(_removeMarker);
  }

  void _removeMarker(MarkerId markerId) {
    final MarkerController? markerController = _markerIdToController[markerId];

    if (markerController?.clusterManagerId != null) {
      _clusterManagersController.removeItem(
        markerController!.clusterManagerId!,
        markerController.marker,
      );
    }

    markerController?.remove();
    _markerIdToController.remove(markerId);
  }

  /// Shows the [InfoWindow] of a [MarkerId].
  ///
  /// See also [hideMarkerInfoWindow] and [isInfoWindowShown].
  void showMarkerInfoWindow(MarkerId markerId) {
    _hideAllMarkerInfoWindow();
    _markerIdToController[markerId]?.showInfoWindow();
  }

  /// Hides the [InfoWindow] of a [MarkerId].
  ///
  /// See also [showMarkerInfoWindow] and [isInfoWindowShown].
  void hideMarkerInfoWindow(MarkerId markerId) {
    _markerIdToController[markerId]?.hideInfoWindow();
  }

  /// Returns whether or not the [InfoWindow] of a [MarkerId] is shown.
  ///
  /// See also [showMarkerInfoWindow] and [hideMarkerInfoWindow].
  bool isInfoWindowShown(MarkerId markerId) {
    return _markerIdToController[markerId]?.infoWindowShown ?? false;
  }

  // Handle internal events

  bool _onMarkerTap(MarkerId markerId) {
    // Have you ended here on your debugging? Is this wrong?
    // Comment here: https://github.com/flutter/flutter/issues/64084
    _streamController.add(MarkerTapEvent(mapId, markerId));
    return _markerIdToController[markerId]?.consumeTapEvents ?? false;
  }

  // ignore: unused_element
  void _onInfoWindowTap(MarkerId markerId) {
    _streamController.add(InfoWindowTapEvent(mapId, markerId));
  }

  void _onMarkerDragStart(MarkerId markerId, LatLng latLng) {
    _streamController.add(MarkerDragStartEvent(mapId, latLng, markerId));
  }

  void _onMarkerDrag(MarkerId markerId, LatLng latLng) {
    _streamController.add(MarkerDragEvent(mapId, latLng, markerId));
  }

  void _onMarkerDragEnd(MarkerId markerId, LatLng latLng) {
    _streamController.add(MarkerDragEndEvent(mapId, latLng, markerId));
  }

  void _hideAllMarkerInfoWindow() {
    _markerIdToController.values
        .where(
          (MarkerController? controller) =>
              controller?.infoWindowShown ?? false,
        )
        .forEach((MarkerController controller) {
          controller.hideInfoWindow();
        });
  }
}
