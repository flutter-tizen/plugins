// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of '../google_maps_flutter_tizen.dart';

/// The Tizen implementation of [GoogleMapsFlutterPlatform].
///
/// This class implements the `package:google_maps_flutter` functionality for the Tizen.
class GoogleMapsPlugin extends GoogleMapsFlutterPlatform {
  /// Registers this class as the default instance of [GoogleMapsFlutterPlatform].
  static void register() {
    GoogleMapsFlutterPlatform.instance = GoogleMapsPlugin();
  }

  // A cache of map controllers by map Id.
  Map<int, GoogleMapsController> _mapById = <int, GoogleMapsController>{};

  /// Get controller for debugging.
  @visibleForTesting
  GoogleMapsController? debugGetMapById(int mapId) {
    return _mapById[mapId];
  }

  @visibleForTesting
  // ignore: use_setters_to_change_properties, public_member_api_docs
  void debugSetMapById(Map<int, GoogleMapsController> mapById) {
    _mapById = mapById;
  }

  // Convenience getter for a stream of events filtered by their mapId.
  Stream<MapEvent<Object?>> _events(int mapId) => _map(mapId).events;

  // Convenience getter for a map controller by its mapId.
  GoogleMapsController _map(int mapId) {
    final GoogleMapsController? controller = _mapById[mapId];
    assert(controller != null,
        'Maps cannot be retrieved before calling buildView!');
    return controller!;
  }

  @override
  Future<void> init(int mapId) async {
    await _map(mapId).init();
  }

  /// Updates the options of a given `mapId`.
  ///
  /// This attempts to merge the new `optionsUpdate` passed in, with the previous
  /// options passed to the map (in other updates, or when creating it).
  @override
  Future<void> updateMapOptions(
    Map<String, dynamic> optionsUpdate, {
    required int mapId,
  }) async {
    _map(mapId).updateRawOptions(optionsUpdate);
  }

  /// Applies the passed in `markerUpdates` to the `mapId`.
  @override
  Future<void> updateMarkers(
    MarkerUpdates markerUpdates, {
    required int mapId,
  }) async {
    _map(mapId).updateMarkers(markerUpdates);
  }

  /// Applies the passed in `polygonUpdates` to the `mapId`.
  @override
  Future<void> updatePolygons(
    PolygonUpdates polygonUpdates, {
    required int mapId,
  }) async {
    _map(mapId).updatePolygons(polygonUpdates);
  }

  /// Applies the passed in `polylineUpdates` to the `mapId`.
  @override
  Future<void> updatePolylines(
    PolylineUpdates polylineUpdates, {
    required int mapId,
  }) async {
    _map(mapId).updatePolylines(polylineUpdates);
  }

  /// Applies the passed in `circleUpdates` to the `mapId`.
  @override
  Future<void> updateCircles(
    CircleUpdates circleUpdates, {
    required int mapId,
  }) async {
    _map(mapId).updateCircles(circleUpdates);
  }

  @override
  Future<void> updateTileOverlays({
    required Set<TileOverlay> newTileOverlays,
    required int mapId,
  }) async {
    return; // Noop for now!
  }

  @override
  Future<void> updateHeatmaps(
    HeatmapUpdates heatmapUpdates, {
    required int mapId,
  }) async {
    return; // Noop for now!
  }

  @override
  Future<void> updateClusterManagers(
    ClusterManagerUpdates clusterManagerUpdates, {
    required int mapId,
  }) async {
    return; // Noop for now!
  }

  @override
  Future<void> clearTileCache(
    TileOverlayId tileOverlayId, {
    required int mapId,
  }) async {
    return; // Noop for now!
  }

  /// Applies the given `cameraUpdate` to the current viewport (with animation).
  @override
  Future<void> animateCamera(
    CameraUpdate cameraUpdate, {
    required int mapId,
  }) async {
    return moveCamera(cameraUpdate, mapId: mapId);
  }

  /// Applies the given `cameraUpdate` to the current viewport.
  @override
  Future<void> moveCamera(
    CameraUpdate cameraUpdate, {
    required int mapId,
  }) async {
    return _map(mapId).moveCamera(cameraUpdate);
  }

  /// Sets the passed-in `mapStyle` to the map.
  ///
  /// This function just adds a 'styles' option to the current map options.
  ///
  /// Subsequent calls to this method override previous calls, you need to
  /// pass full styles.
  @override
  Future<void> setMapStyle(
    String? mapStyle, {
    required int mapId,
  }) async {
    _map(mapId).updateRawOptions(<String, dynamic>{
      'styles': _mapStyles(mapStyle),
    });
  }

  /// Returns the bounds of the current viewport.
  @override
  Future<LatLngBounds> getVisibleRegion({
    required int mapId,
  }) {
    return _map(mapId).getVisibleRegion();
  }

  /// Returns the screen coordinate (in pixels) of a given `latLng`.
  @override
  Future<ScreenCoordinate> getScreenCoordinate(
    LatLng latLng, {
    required int mapId,
  }) {
    return _map(mapId).getScreenCoordinate(latLng);
  }

  /// Returns the [LatLng] of a [ScreenCoordinate] of the viewport.
  @override
  Future<LatLng> getLatLng(
    ScreenCoordinate screenCoordinate, {
    required int mapId,
  }) {
    return _map(mapId).getLatLng(screenCoordinate);
  }

  /// Shows the [InfoWindow] (if any) of the [Marker] identified by `markerId`.
  ///
  /// See also:
  ///   * [hideMarkerInfoWindow] to hide the info window.
  ///   * [isMarkerInfoWindowShown] to check if the info window is visible/hidden.
  @override
  Future<void> showMarkerInfoWindow(
    MarkerId markerId, {
    required int mapId,
  }) async {
    _map(mapId).showInfoWindow(markerId);
  }

  /// Hides the [InfoWindow] (if any) of the [Marker] identified by `markerId`.
  ///
  /// See also:
  ///   * [showMarkerInfoWindow] to show the info window.
  ///   * [isMarkerInfoWindowShown] to check if the info window is shown.
  @override
  Future<void> hideMarkerInfoWindow(
    MarkerId markerId, {
    required int mapId,
  }) async {
    _map(mapId).hideInfoWindow(markerId);
  }

  /// Returns true if the [InfoWindow] of the [Marker] identified by `markerId` is shown.
  ///
  /// See also:
  ///   * [showMarkerInfoWindow] to show the info window.
  ///   * [hideMarkerInfoWindow] to hide the info window.
  @override
  Future<bool> isMarkerInfoWindowShown(
    MarkerId markerId, {
    required int mapId,
  }) async {
    return _map(mapId).isInfoWindowShown(markerId);
  }

  /// Returns the zoom level of the `mapId`.
  @override
  Future<double> getZoomLevel({
    required int mapId,
  }) {
    return _map(mapId).getZoomLevel();
  }

  // The following are the 11 possible streams of data from the native side
  // into the plugin

  @override
  Stream<CameraMoveStartedEvent> onCameraMoveStarted({required int mapId}) {
    return _events(mapId).whereType<CameraMoveStartedEvent>();
  }

  @override
  Stream<CameraMoveEvent> onCameraMove({required int mapId}) {
    return _events(mapId).whereType<CameraMoveEvent>();
  }

  @override
  Stream<CameraIdleEvent> onCameraIdle({required int mapId}) {
    return _events(mapId).whereType<CameraIdleEvent>();
  }

  @override
  Stream<MarkerTapEvent> onMarkerTap({required int mapId}) {
    return _events(mapId).whereType<MarkerTapEvent>();
  }

  @override
  Stream<InfoWindowTapEvent> onInfoWindowTap({required int mapId}) {
    return _events(mapId).whereType<InfoWindowTapEvent>();
  }

  @override
  Stream<MarkerDragStartEvent> onMarkerDragStart({required int mapId}) {
    return _events(mapId).whereType<MarkerDragStartEvent>();
  }

  @override
  Stream<MarkerDragEvent> onMarkerDrag({required int mapId}) {
    return _events(mapId).whereType<MarkerDragEvent>();
  }

  @override
  Stream<MarkerDragEndEvent> onMarkerDragEnd({required int mapId}) {
    return _events(mapId).whereType<MarkerDragEndEvent>();
  }

  @override
  Stream<PolylineTapEvent> onPolylineTap({required int mapId}) {
    return _events(mapId).whereType<PolylineTapEvent>();
  }

  @override
  Stream<PolygonTapEvent> onPolygonTap({required int mapId}) {
    return _events(mapId).whereType<PolygonTapEvent>();
  }

  @override
  Stream<CircleTapEvent> onCircleTap({required int mapId}) {
    return _events(mapId).whereType<CircleTapEvent>();
  }

  @override
  Stream<MapTapEvent> onTap({required int mapId}) {
    return _events(mapId).whereType<MapTapEvent>();
  }

  @override
  Stream<MapLongPressEvent> onLongPress({required int mapId}) {
    return _events(mapId).whereType<MapLongPressEvent>();
  }

  @override
  Stream<ClusterTapEvent> onClusterTap({required int mapId}) {
    return _events(mapId).whereType<ClusterTapEvent>();
  }

  /// Disposes of the current map. It can't be used afterwards!
  @override
  void dispose({required int mapId}) {
    _map(mapId).dispose();
    _mapById.remove(mapId);
  }

  @override
  Widget buildView(
    int creationId,
    PlatformViewCreatedCallback onPlatformViewCreated, {
    required CameraPosition initialCameraPosition,
    Set<Marker> markers = const <Marker>{},
    Set<Polygon> polygons = const <Polygon>{},
    Set<Polyline> polylines = const <Polyline>{},
    Set<Circle> circles = const <Circle>{},
    Set<TileOverlay> tileOverlays = const <TileOverlay>{},
    Set<Factory<OneSequenceGestureRecognizer>>? gestureRecognizers =
        const <Factory<OneSequenceGestureRecognizer>>{},
    Map<String, dynamic> mapOptions = const <String, dynamic>{},
  }) {
    // Bail fast if we've already rendered this map ID...
    if (_mapById[creationId]?.webview != null) {
      return _mapById[creationId]!.webview!;
    }

    final StreamController<MapEvent<Object?>> controller =
        StreamController<MapEvent<Object?>>.broadcast();

    final GoogleMapsController mapController = GoogleMapsController(
      initialCameraPosition: initialCameraPosition,
      mapId: creationId,
      streamController: controller,
      markers: markers,
      polygons: polygons,
      polylines: polylines,
      circles: circles,
      mapOptions: mapOptions,
    )..init(); // Initialize the controller

    _mapById[creationId] = mapController;

    mapController.events
        .whereType<MapReadyEvent>()
        .first
        .then((MapReadyEvent event) {
      assert(creationId == event.mapId,
          'Received MapReadyEvent for the wrong map');
      // Notify the plugin now that there's a fully initialized controller.
      onPlatformViewCreated.call(event.mapId);
    });
    assert(mapController.webview != null,
        'The widget of a GoogleMapsController cannot be null before calling dispose on it.');

    return mapController.webview!;
  }
}
