// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of '../google_maps_flutter_tizen.dart';

/// A controller class for managing marker clustering.
///
/// This class maps [ClusterManager] objects to javascript [GMarkerClusterer]
/// objects and provides an interface for adding and removing markers from
/// clusters.
class ClusterManagersController extends GeometryController {
  /// Creates a new [ClusterManagersController] instance.
  ///
  /// The [stream] parameter is a required [StreamController] used for
  /// emitting map events.
  ClusterManagersController({
    required StreamController<MapEvent<Object?>> stream,
  })  : _streamController = stream,
        _idToClusterManagerId = <String, ClusterManagerId>{},
        _clusterManagerIdToMarkerClusterer =
            <ClusterManagerId, util.GMarkerClusterer>{};

  // The stream over which cluster managers broadcast their events
  final StreamController<MapEvent<Object?>> _streamController;

  // A cache of [MarkerClusterer]s indexed by their [ClusterManagerId].
  final Map<ClusterManagerId, util.GMarkerClusterer>
      _clusterManagerIdToMarkerClusterer;
  final Map<String, ClusterManagerId> _idToClusterManagerId;

  /// A cache of [ClusterManagerId]s indexed by [GMarkerClusterer.id].
  Map<String, ClusterManagerId> get idToClusterManagerId =>
      _idToClusterManagerId;

  /// Adds a set of [ClusterManager] objects to the cache.
  void addClusterManagers(Set<ClusterManager> clusterManagersToAdd) {
    clusterManagersToAdd.forEach(_addClusterManager);
  }

  void _addClusterManager(ClusterManager clusterManager) {
    final String onClusterClickHandler =
        '(event, cluster, map) => ClusterClick.postMessage(makeClusterEvent("${clusterManager.clusterManagerId.value}", event, cluster))';

    final util.GMarkerClustererOptions options = util.GMarkerClustererOptions(
      onClusterClick: onClusterClickHandler,
    );

    final util.GMarkerClusterer markerClusterer = util.GMarkerClusterer(
      options,
    );

    _clusterManagerIdToMarkerClusterer[clusterManager.clusterManagerId] =
        markerClusterer;
    _idToClusterManagerId[clusterManager.clusterManagerId.value] =
        clusterManager.clusterManagerId;
    markerClusterer.onAdd();
  }

  /// Removes a set of [ClusterManagerId]s from the cache.
  void removeClusterManagers(Set<ClusterManagerId> clusterManagerIdsToRemove) {
    clusterManagerIdsToRemove.forEach(_removeClusterManager);
  }

  void _removeClusterManager(ClusterManagerId clusterManagerId) {
    final util.GMarkerClusterer? markerClusterer =
        _clusterManagerIdToMarkerClusterer[clusterManagerId];
    if (markerClusterer != null) {
      markerClusterer.clearMarkers(true);
      markerClusterer.onRemove();
    }
    _clusterManagerIdToMarkerClusterer.remove(clusterManagerId);
  }

  /// Adds given [gmaps.Marker] to the [MarkerClusterer] with given
  /// [ClusterManagerId].
  void addItem(ClusterManagerId clusterManagerId, util.GMarker marker) {
    final util.GMarkerClusterer? markerClusterer =
        _clusterManagerIdToMarkerClusterer[clusterManagerId];
    if (markerClusterer != null) {
      markerClusterer.addMarker(marker, true);
      markerClusterer.render();
    }
  }

  /// Removes given [gmaps.Marker] from the [MarkerClusterer] with given
  /// [ClusterManagerId].
  void removeItem(ClusterManagerId clusterManagerId, util.GMarker? marker) {
    if (marker != null) {
      final util.GMarkerClusterer? markerClusterer =
          _clusterManagerIdToMarkerClusterer[clusterManagerId];
      if (markerClusterer != null) {
        markerClusterer.removeMarker(marker, true);
        markerClusterer.render();
      }
    }
  }

  /// Returns list of clusters in [MarkerClusterer] with given
  /// [ClusterManagerId].
  List<Cluster> getClusters(ClusterManagerId clusterManagerId) {
    final util.GMarkerClusterer? markerClusterer =
        _clusterManagerIdToMarkerClusterer[clusterManagerId];
    if (markerClusterer != null) {
      return markerClusterer.clusters
          .map(
            (Map<String, dynamic> cluster) =>
                _convertCluster(clusterManagerId, cluster),
          )
          .toList();
    }
    return <Cluster>[];
  }

  /// Call ClusterTapEvent when [MarkerClusterer] with given [ClusterManagerId]
  /// is clicked.
  void clusterClicked(
    ClusterManagerId clusterManagerId,
    Map<String, dynamic> markerClustererCluster,
  ) {
    if (markerClustererCluster['count'] as int > 0 &&
        markerClustererCluster['bounds'] != null) {
      final Cluster cluster = _convertCluster(
        clusterManagerId,
        markerClustererCluster,
      );
      _streamController.add(ClusterTapEvent(mapId, cluster));
    }
  }

  /// Converts [MarkerClustererCluster] to [Cluster].
  Cluster _convertCluster(
    ClusterManagerId clusterManagerId,
    Map<String, dynamic> markerClustererCluster,
  ) {
    final LatLng position = LatLng(
      (markerClustererCluster['position'] as dynamic)['lat'] as double,
      (markerClustererCluster['position'] as dynamic)['lng'] as double,
    );

    final LatLngBounds bounds = LatLngBounds(
      southwest: LatLng(
        (markerClustererCluster['bounds'] as dynamic)['south'] as double,
        (markerClustererCluster['bounds'] as dynamic)['west'] as double,
      ),
      northeast: LatLng(
        (markerClustererCluster['bounds'] as dynamic)['north'] as double,
        (markerClustererCluster['bounds'] as dynamic)['east'] as double,
      ),
    );

    final List<MarkerId> markerIds =
        (markerClustererCluster['markers']! as List<dynamic>)
            .map<MarkerId>(
              (dynamic markerId) => MarkerId((markerId as int).toString()),
            )
            .toList();

    return Cluster(
      clusterManagerId,
      markerIds,
      position: position,
      bounds: bounds,
    );
  }
}
