// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: avoid_dynamic_calls

part of '../google_maps_flutter_tizen.dart';

/// This class implements a Map Controller and its events
class GoogleMapsController {
  /// Initializes the GoogleMapsController.
  GoogleMapsController({
    required int mapId,
    required StreamController<MapEvent<Object?>> streamController,
    required CameraPosition initialCameraPosition,
    Set<Marker> markers = const <Marker>{},
    Set<Polygon> polygons = const <Polygon>{},
    Set<Polyline> polylines = const <Polyline>{},
    Set<Circle> circles = const <Circle>{},
    Set<ClusterManager> clusterManagers = const <ClusterManager>{},
    Set<GroundOverlay> groundOverlays = const <GroundOverlay>{},
    Map<String, dynamic> mapOptions = const <String, dynamic>{},
  })  : _mapId = mapId,
        _streamController = streamController,
        _initialCameraPosition = initialCameraPosition,
        _markers = markers,
        _polygons = polygons,
        _polylines = polylines,
        _circles = circles,
        _clusterManagers = clusterManagers,
        _groundOverlays = groundOverlays,
        _rawMapOptions = mapOptions {
    _circlesController = CirclesController(
      stream: _streamController,
      bridge: _bridge,
    );
    _polygonsController = PolygonsController(
      stream: _streamController,
      bridge: _bridge,
    );
    _polylinesController = PolylinesController(
      stream: _streamController,
      bridge: _bridge,
    );
    _clusterManagersController = ClusterManagersController(
      stream: _streamController,
      bridge: _bridge,
    );
    _markersController = MarkersController(
      stream: _streamController,
      clusterManagersController: _clusterManagersController!,
      bridge: _bridge,
    );
    _groundOverlaysController = GroundOverlaysController(
      stream: _streamController,
      bridge: _bridge,
    );
  }

  // The internal ID of the map. Used to broadcast events, DOM IDs and everything where a unique ID is needed.
  final int _mapId;

  bool _isFirst = false;

  final CameraPosition _initialCameraPosition;
  final Set<Marker> _markers;
  final Set<Polygon> _polygons;
  final Set<Polyline> _polylines;
  final Set<Circle> _circles;
  final Set<ClusterManager> _clusterManagers;
  final Set<GroundOverlay> _groundOverlays;
  WebViewWidget? _webview;
  // The raw options passed by the user, before converting to maps.
  // Caching this allows us to re-create the map faithfully when needed.
  Map<String, dynamic> _rawMapOptions = <String, dynamic>{};

  /// The bridge mediating all interaction with the Google Maps JavaScript
  /// API running inside the WebView.
  final GoogleMapsJsBridge _bridge = WebViewGoogleMapsJsBridge();

  /// The Flutter widget that will contain the rendered Map. Used for caching.
  WebViewWidget? get webview => _webview;

  /// Returns min-max zoom levels. Test only.
  @visibleForTesting
  Future<MinMaxZoomPreference> getMinMaxZoomLevels() async {
    final String value = await _bridge.runJavaScriptReturningResult(
      'JSON.stringify([map.minZoom, map.maxZoom])',
    ) as String;
    final dynamic bound = json.decode(value);
    double min = 0, max = 0;
    if (bound is List<dynamic>) {
      if (bound[0] is num) {
        min = (bound[0] is double)
            ? (bound[0] as double)
            : (bound[0] as int).toDouble();
      }
      if (bound[1] is num) {
        max = (bound[1] is double)
            ? (bound[1] as double)
            : (bound[1] as int).toDouble();
      }
      return MinMaxZoomPreference(min, max);
    }
    return const MinMaxZoomPreference(0, 0);
  }

  /// Returns if zoomGestures property is enabled. Test only.
  @visibleForTesting
  Future<bool> isZoomGesturesEnabled() async {
    final String value = await _bridge
        .runJavaScriptReturningResult('map.gestureHandling') as String;
    return value != 'none';
  }

  /// Returns if zoomControls property is enabled. Test only.
  @visibleForTesting
  Future<bool> isZoomControlsEnabled() async {
    final String value =
        await _bridge.runJavaScriptReturningResult('map.zoomControl') as String;
    return value != 'false';
  }

  /// Returns if scrollGestures property is enabled. Test only.
  @visibleForTesting
  Future<bool> isScrollGesturesEnabled() async {
    final String value = await _bridge
        .runJavaScriptReturningResult('map.gestureHandling') as String;
    return value != 'none';
  }

  /// Returns if traffic layer is enabled. Test only.
  @visibleForTesting
  Future<bool> isTrafficEnabled() async {
    return _isTrafficLayerEnabled(_rawMapOptions);
  }

  void _getWebview() {
    _bridge.events.listen(_onJsEvent);
    _webview = WebViewWidget(controller: _bridge.controller);
  }

  Future<void> _onJsEvent(MapsJsEvent event) async {
    switch (event) {
      case BoundsChangedJsEvent():
        await _onBoundsChanged();
      case IdleJsEvent():
        _onIdle();
      case TilesLoadedJsEvent():
        _onTilesloaded();
      case ClickJsEvent(message: final String message):
        _onClick(message);
      case LongPressJsEvent(message: final String message):
        _onLongPress(message);
      case MarkerClickJsEvent(message: final String message):
        _onMarkerClick(message);
      case ClusterClickJsEvent(message: final String message):
        _onClusterClick(message);
      case MarkerDragStartJsEvent(message: final String message):
        _onMarkerDragStart(message);
      case MarkerDragJsEvent(message: final String message):
        _onMarkerDrag(message);
      case MarkerDragEndJsEvent(message: final String message):
        _onMarkerDragEnd(message);
      case PolylineClickJsEvent(message: final String message):
        _onPolylineClick(message);
      case PolygonClickJsEvent(message: final String message):
        _onPolygonClick(message);
      case CircleClickJsEvent(message: final String message):
        _onCircleClick(message);
      case GroundOverlayClickJsEvent(message: final String message):
        _onGroundOverlayClick(message);
    }
  }

  Future<void> _createMap() async {
    await _bridge.createMap(_createOptions());
  }

  String _createOptions() {
    String options = _rawOptionsToString(_rawMapOptions);
    options = _applyInitialPosition(_initialCameraPosition, options);
    return '{$options}';
  }

  // The StreamController used by this controller and the geometry ones.
  final StreamController<MapEvent<Object?>> _streamController;

  /// The Stream over which this controller broadcasts events.
  Stream<MapEvent<Object?>> get events => _streamController.stream;

  // // Geometry controllers, for different features of the map.
  CirclesController? _circlesController;
  PolygonsController? _polygonsController;
  PolylinesController? _polylinesController;
  MarkersController? _markersController;
  ClusterManagersController? _clusterManagersController;
  GroundOverlaysController? _groundOverlaysController;

  // Keeps track if _attachGeometryControllers has been called or not.
  bool _controllersBoundToMap = false;
  // Keeps track if the map is moving or not.
  bool _mapIsMoving = false;

  Future<void> _onBoundsChanged() async {
    final LatLng center = await getCenter();
    final num zoom = await getZoomLevel();

    if (!_streamController.isClosed) {
      if (!_mapIsMoving) {
        _mapIsMoving = true;
        _streamController.add(CameraMoveStartedEvent(_mapId));
      }

      _streamController.add(
        CameraMoveEvent(
          _mapId,
          CameraPosition(target: center, zoom: zoom.toDouble()),
        ),
      );
    }
  }

  void _onIdle() {
    _mapIsMoving = false;
    _streamController.add(CameraIdleEvent(_mapId));
  }

  void _onTilesloaded() {
    try {
      if (_isFirst) {
        return;
      }
      _streamController.add(MapReadyEvent(_mapId));
      _isFirst = true;
    } catch (e) {
      debugPrint('JavaScript Error: $e');
    }
  }

  void _onClick(String message) {
    try {
      final dynamic event = json.decode(message);
      if (event is Map<String, dynamic>) {
        assert(event['latLng'] != null);
        final LatLng position = LatLng(
          event['latLng']['lat'] as double,
          event['latLng']['lng'] as double,
        );
        _streamController.add(MapTapEvent(_mapId, position));
      }
    } catch (e) {
      debugPrint('JavaScript Error: $e');
    }
  }

  void _onLongPress(String message) {
    try {
      final dynamic event = json.decode(message);
      if (event is Map<String, dynamic>) {
        assert(event['latLng'] != null);
        final LatLng position = LatLng(
          event['latLng']['lat'] as double,
          event['latLng']['lng'] as double,
        );
        _streamController.add(MapLongPressEvent(_mapId, position));
      }
    } catch (e) {
      debugPrint('JavaScript Error: $e');
    }
  }

  void _onClusterClick(String message) {
    try {
      final dynamic result = json.decode(message);

      final String id = result['id'] as String;
      final ClusterManagerId? clusterManagerId =
          _clusterManagersController?.idToClusterManagerId[id];
      if (clusterManagerId == null) {
        return;
      }

      final Map<String, dynamic> markerClustererCluster =
          result['cluster'] as Map<String, dynamic>;

      _clusterManagersController?.clusterClicked(
        clusterManagerId,
        markerClustererCluster,
      );
    } catch (e) {
      debugPrint('JavaScript Error: $e');
    }
  }

  void _onMarkerClick(String message) {
    try {
      final dynamic id = json.decode(message);
      if (_markersController != null && id is int) {
        final MarkerId? markerId = _markersController!._idToMarkerId[id];
        final MarkerController? marker =
            _markersController!._markerIdToController[markerId];
        if (marker?.tapEvent != null) {
          marker?.tapEvent!();
        }
      }
    } catch (e) {
      debugPrint('JavaScript Error: $e');
    }
  }

  void _onMarkerDragStart(String message) {
    try {
      final dynamic result = json.decode(message);
      if (result is Map<String, dynamic>) {
        assert(result['id'] != null && result['event'] != null);
        if (_markersController != null && result['id'] is int) {
          final MarkerId? markerId =
              _markersController!._idToMarkerId[result['id']];
          final MarkerController? marker =
              _markersController!._markerIdToController[markerId];

          final LatLng position = LatLng(
            result['event']['latLng']['lat'] as double,
            result['event']['latLng']['lng'] as double,
          );

          if (marker?.dragStartEvent != null) {
            marker?.dragStartEvent!(position);
          }
        }
      }
    } catch (e) {
      debugPrint('JavaScript Error: $e');
    }
  }

  void _onMarkerDrag(String message) {
    try {
      final dynamic result = json.decode(message);
      if (result is Map<String, dynamic>) {
        assert(result['id'] != null && result['event'] != null);
        if (_markersController != null && result['id'] is int) {
          final MarkerId? markerId =
              _markersController!._idToMarkerId[result['id']];
          final MarkerController? marker =
              _markersController!._markerIdToController[markerId];

          final LatLng position = LatLng(
            result['event']['latLng']['lat'] as double,
            result['event']['latLng']['lng'] as double,
          );

          if (marker?.dragEvent != null) {
            marker?.dragEvent!(position);
          }
        }
      }
    } catch (e) {
      debugPrint('JavaScript Error: $e');
    }
  }

  void _onMarkerDragEnd(String message) {
    try {
      final dynamic result = json.decode(message);
      if (result is Map<String, dynamic>) {
        assert(result['id'] != null && result['event'] != null);
        if (_markersController != null && result['id'] is int) {
          final MarkerId? markerId =
              _markersController!._idToMarkerId[result['id']];
          final MarkerController? marker =
              _markersController!._markerIdToController[markerId];

          final LatLng position = LatLng(
            result['event']['latLng']['lat'] as double,
            result['event']['latLng']['lng'] as double,
          );

          if (marker?.dragEndEvent != null) {
            marker?.dragEndEvent!(position);
          }
        }
      }
    } catch (e) {
      debugPrint('JavaScript Error: $e');
    }
  }

  void _onPolylineClick(String message) {
    try {
      final dynamic id = json.decode(message);
      if (_polylinesController != null && id is int) {
        final PolylineId? polylineId =
            _polylinesController!._idToPolylineId[id];
        final PolylineController? polyline =
            _polylinesController!._polylineIdToController[polylineId];
        if (polyline?.tapEvent != null) {
          polyline?.tapEvent!();
        }
      }
    } catch (e) {
      debugPrint('JavaScript Error: $e');
    }
  }

  void _onPolygonClick(String message) {
    try {
      final dynamic id = json.decode(message);
      if (_polygonsController != null && id is int) {
        final PolygonId? polygonId = _polygonsController!._idToPolygonId[id];
        final PolygonController? polygon =
            _polygonsController!._polygonIdToController[polygonId];
        if (polygon?.tapEvent != null) {
          polygon?.tapEvent!();
        }
      }
    } catch (e) {
      debugPrint('JavaScript Error: $e');
    }
  }

  void _onCircleClick(String message) {
    try {
      final dynamic id = json.decode(message);
      if (_polygonsController != null && id is int) {
        final CircleId? circleId = _circlesController!._idToCircleId[id];
        final CircleController? circle =
            _circlesController!._circleIdToController[circleId];
        if (circle?.tapEvent != null) {
          circle?.tapEvent!();
        }
      }
    } catch (e) {
      debugPrint('JavaScript Error: $e');
    }
  }

  void _onGroundOverlayClick(String message) {
    try {
      final dynamic id = json.decode(message);
      if (_groundOverlaysController != null && id is int) {
        final GroundOverlayId? groundOverlayId =
            _groundOverlaysController!._idToGroundOverlayId[id];
        final GroundOverlayController? groundOverlay =
            _groundOverlaysController!
                ._groundOverlayIdToController[groundOverlayId];
        if (groundOverlay?.tapEvent != null) {
          groundOverlay?.tapEvent!();
        }
      }
    } catch (e) {
      debugPrint('JavaScript Error: $e');
    }
  }

  /// Initializes the map from the stored `rawOptions`.
  ///
  /// This is called by the [GoogleMapsPlugin.init] method when appropriate.
  ///
  /// Failure to call this method would result in not rendering at all,
  /// and most of the public methods on this class no-op'ing.
  Future<void> init() async {
    if (_webview == null && !_streamController.isClosed) {
      _getWebview();
      await _bridge.load();
      await _createMap();
    }
    await _attachGeometryControllers();
    _renderInitialGeometry(
      markers: _markers,
      circles: _circles,
      polygons: _polygons,
      polylines: _polylines,
      groundOverlays: _groundOverlays,
    );

    _initClustering(_clusterManagers);

    await _setTrafficLayer(_isTrafficLayerEnabled(_rawMapOptions));
  }

  // Binds the Geometry controllers to a map instance
  Future<void> _attachGeometryControllers() async {
    // Now we can add the initial geometry.
    // And bind the (ready) map instance to the other geometry controllers.
    assert(
      _circlesController != null,
      'Cannot attach a map to a null CirclesController instance.',
    );
    assert(
      _polygonsController != null,
      'Cannot attach a map to a null PolygonsController instance.',
    );
    assert(
      _polylinesController != null,
      'Cannot attach a map to a null PolylinesController instance.',
    );
    assert(
      _markersController != null,
      'Cannot attach a map to a null MarkersController instance.',
    );
    assert(
      _clusterManagersController != null,
      'Cannot attach a map to a null ClusterManagersController instance.',
    );
    assert(
      _groundOverlaysController != null,
      'Cannot attach a map to a null GroundOverlaysController instance.',
    );

    _circlesController!.bindToMap(_mapId, _webview!);
    _polygonsController!.bindToMap(_mapId, _webview!);
    _polylinesController!.bindToMap(_mapId, _webview!);
    _markersController!.bindToMap(_mapId, _webview!);
    _clusterManagersController!.bindToMap(_mapId, _webview!);
    _groundOverlaysController!.bindToMap(_mapId, _webview!);

    _controllersBoundToMap = true;
  }

  void _initClustering(Set<ClusterManager> clusterManagers) {
    _clusterManagersController!.addClusterManagers(clusterManagers);
  }

  // Renders the initial sets of geometry.
  void _renderInitialGeometry({
    Set<Marker> markers = const <Marker>{},
    Set<Circle> circles = const <Circle>{},
    Set<Polygon> polygons = const <Polygon>{},
    Set<Polyline> polylines = const <Polyline>{},
    Set<GroundOverlay> groundOverlays = const <GroundOverlay>{},
  }) {
    assert(
      _controllersBoundToMap,
      'Geometry controllers must be bound to a map before any geometry can '
      'be added to them. Ensure _attachGeometryControllers is called first.',
    );
    // The above assert will only succeed if the controllers have been bound to a map
    // in the [_attachGeometryControllers] method, which ensures that all these
    // controllers below are *not* null.
    _markersController!.addMarkers(markers);
    _circlesController!.addCircles(circles);
    _polygonsController!.addPolygons(polygons);
    _polylinesController!.addPolylines(polylines);
    _groundOverlaysController!.addGroundOverlays(groundOverlays);
  }

  // Merges new options coming from the plugin into the _rawMapOptions map.
  //
  // Returns the updated _rawMapOptions object.
  Map<String, dynamic> _mergeRawOptions(Map<String, dynamic> newOptions) {
    _rawMapOptions = <String, dynamic>{..._rawMapOptions, ...newOptions};
    return _rawMapOptions;
  }

  /// Updates the map options from a `Map<String, dynamic>`.
  ///
  /// This method converts the map into the proper [MapOptions]
  void updateRawOptions(Map<String, dynamic> optionsUpdate) {
    assert(_webview != null, 'Cannot update options on a null map.');

    final Map<String, dynamic> newOptions = _mergeRawOptions(optionsUpdate);
    final String options = _rawOptionsToString(newOptions);

    _setOptions('{$options}');
    _setTrafficLayer(_isTrafficLayerEnabled(newOptions));
  }

  Future<void> _setOptions(String options) async {
    await _callMethod('setOptions', <String>[options]);
  }

  Future<void> _setZoom(String options) async {
    await _callMethod('setZoom', <String>[options]);
  }

  // Attaches/detaches a Traffic Layer on the `map` if `attach` is true/false.
  Future<void> _setTrafficLayer(bool attach) async {
    final String command = '''
      var trafficLayer;
      if ($attach == true && trafficLayer == null) {
        trafficLayer = new google.maps.TrafficLayer();
        trafficLayer.setMap(map);
        console.log('trafficLayer attached!!');
      }
      if ($attach == false && trafficLayer != null) {
        trafficLayer.setMap(null);
        trafficLayer = null;
        console.log('trafficLayer detached!!');
      }
    ''';
    await _bridge.runJavaScript(command);
  }

  Future<void> _setMoveCamera(String options) async {
    await _callMethod('moveCamera', <String>[options]);
  }

  Future<void> _setPanTo(String options) async {
    await _callMethod('panTo', <String>[options]);
  }

  Future<void> _setPanBy(String options) async {
    await _callMethod('panBy', <String>[options]);
  }

  Future<void> _setFitBounds(String options) async {
    await _callMethod('fitBounds', <String>[options]);
  }

  Future<Object> _callMethod(String method, List<String?> args) async {
    return _bridge.runJavaScriptReturningResult(
      'JSON.stringify(map.$method.apply(map, $args))',
    );
  }

  Future<double> _getZoom() async {
    try {
      return (await _callMethod('getZoom', <String>[]) as num) + 0.0;
    } catch (e) {
      debugPrint('JavaScript Error: $e');
      return 0.0;
    }
  }

  /// Returns the [LatLngBounds] of the current viewport.
  Future<LatLngBounds> getVisibleRegion() async {
    return _convertToBounds(
      await _callMethod('getBounds', <String>[]) as String,
    );
  }

  /// Returns the [LatLng] at the center of the map.
  Future<LatLng> getCenter() async {
    return _convertToLatLng(
      await _callMethod('getCenter', <String>[]) as String,
    );
  }

  /// Returns the [ScreenCoordinate] for a given viewport [LatLng].
  Future<ScreenCoordinate> getScreenCoordinate(LatLng latLng) async {
    return _convertToPoint(await _latLngToPoint(latLng));
  }

  /// Returns the [LatLng] for a `screenCoordinate` (in pixels) of the viewport.
  Future<LatLng> getLatLng(ScreenCoordinate screenCoordinate) async {
    return _convertToLatLng(
      await _pixelToLatLng(screenCoordinate.x + 0.0, screenCoordinate.y + 0.0),
    );
  }

  /// Applies a `cameraUpdate` to the current viewport.
  Future<void> moveCamera(CameraUpdate cameraUpdate) async {
    await _applyCameraUpdate(cameraUpdate);
  }

  // Translates a [CameraUpdate] into operations on a [Javascript].
  Future<void> _applyCameraUpdate(CameraUpdate update) async {
    final List<dynamic> json = update.toJson() as List<dynamic>;
    switch (json[0]) {
      case 'newCameraPosition':
        await _setMoveCamera(
          '{heading: ${json[1]['bearing']}, zoom: ${json[1]['zoom']}, tilt: ${json[1]['tilt']}}',
        );
        await _setPanTo(
          '{lat:${json[1]['target'][0]}, lng: ${json[1]['target'][1]}}',
        );
      case 'newLatLng':
        await _setPanTo('{lat:${json[1][0]}, lng:${json[1][1]}}');
      case 'newLatLngZoom':
        await _setMoveCamera('{zoom: ${json[2]}}');
        await _setPanTo('{lat:${json[1][0]}, lng: ${json[1][1]}}');
      case 'newLatLngBounds':
        await _setFitBounds(
          '{south:${json[1][0][0]}, west:${json[1][0][1]}, north:${json[1][1][0]}, east:${json[1][1][1]}}, ${json[2]}',
        );
      case 'scrollBy':
        await _setPanBy('${json[1]}, ${json[2]}');
      case 'zoomBy':
        String? focusLatLng;
        double zoomDelta = 0.0;
        if (json[1] != null) {
          zoomDelta = (json[1] as num) + 0.0;
        }
        // Web only supports integer changes...
        final int newZoomDelta =
            zoomDelta < 0 ? zoomDelta.floor() : zoomDelta.ceil();
        if (json.length == 3) {
          // With focus
          try {
            focusLatLng = await _pixelToLatLng(
              json[2][0] as double,
              json[2][1] as double,
            );
          } catch (e) {
            debugPrint('Error computing focus LatLng. JS Error: $e');
          }
        }
        await _setZoom('${(await getZoomLevel()) + newZoomDelta}');
        if (focusLatLng != null) {
          await _setPanTo(focusLatLng);
        }
      case 'zoomIn':
        await _setZoom('${(await getZoomLevel()) + 1}');
      case 'zoomOut':
        await _setZoom('${(await getZoomLevel()) - 1}');
      case 'zoomTo':
        await _setZoom('${json[1]}');
      default:
        throw UnimplementedError('Unimplemented CameraMove: ${json[0]}.');
    }
  }

  Future<String> _pixelToLatLng(double x, double y) async {
    final String command = '''
      function getPixelToLatLng() {
        var projection = map.getProjection();
        var ne = map.getBounds().getNorthEast();
        var sw = map.getBounds().getSouthWest();
        var topRight = projection.fromLatLngToPoint(ne);
        var bottomLeft = projection.fromLatLngToPoint(sw);
        var scale = 1 << map.zoom;
        var point = new google.maps.Point(($x / scale) + bottomLeft.x, ($y / scale) + topRight.y);
        return projection.fromPointToLatLng(point);
      }
      JSON.stringify(getPixelToLatLng());
    ''';

    return await _bridge.runJavaScriptReturningResult(command) as String;
  }

  Future<String> _latLngToPoint(LatLng latLng) async {
    final String command = '''
      function getLatLngToPixel() {
        var ne = map.getBounds().getNorthEast();
        var sw = map.getBounds().getSouthWest();
        var projection = map.getProjection();
        var topRight = projection.fromLatLngToPoint(ne);
        var bottomLeft = projection.fromLatLngToPoint(sw);
        var scale = 1 << map.getZoom();
        var newLatlng = projection.fromLatLngToPoint(new google.maps.LatLng(${latLng.latitude}, ${latLng.longitude}));
        var xx = (newLatlng.x - bottomLeft.x) * scale;
        var yy = (newLatlng.y - topRight.y) * scale;
        return new google.maps.Point(xx, yy);
      }
      JSON.stringify(getLatLngToPixel());
    ''';

    return await _bridge.runJavaScriptReturningResult(command) as String;
  }

  /// Returns the zoom level of the current viewport.
  Future<double> getZoomLevel() async {
    return _getZoom();
  }

  // Geometry manipulation

  /// Applies [CircleUpdates] to the currently managed circles.
  void updateCircles(CircleUpdates updates) {
    assert(
      _circlesController != null,
      'Cannot update circles after dispose().',
    );
    _circlesController?.addCircles(updates.circlesToAdd);
    _circlesController?.changeCircles(updates.circlesToChange);
    _circlesController?.removeCircles(updates.circleIdsToRemove);
  }

  /// Applies [PolygonUpdates] to the currently managed polygons.
  void updatePolygons(PolygonUpdates updates) {
    assert(
      _polygonsController != null,
      'Cannot update polygons after dispose().',
    );
    _polygonsController?.addPolygons(updates.polygonsToAdd);
    _polygonsController?.changePolygons(updates.polygonsToChange);
    _polygonsController?.removePolygons(updates.polygonIdsToRemove);
  }

  /// Applies [PolylineUpdates] to the currently managed lines.
  void updatePolylines(PolylineUpdates updates) {
    assert(
      _polylinesController != null,
      'Cannot update polylines after dispose().',
    );
    _polylinesController?.addPolylines(updates.polylinesToAdd);
    _polylinesController?.changePolylines(updates.polylinesToChange);
    _polylinesController?.removePolylines(updates.polylineIdsToRemove);
  }

  /// Applies [MarkerUpdates] to the currently managed markers.
  void updateMarkers(MarkerUpdates updates) {
    assert(
      _markersController != null,
      'Cannot update markers after dispose().',
    );
    _markersController?.addMarkers(updates.markersToAdd);
    _markersController?.changeMarkers(updates.markersToChange);
    _markersController?.removeMarkers(updates.markerIdsToRemove);
  }

  /// Applies [ClusterManagerUpdates] to the currently managed cluster managers.
  void updateClusterManagers(ClusterManagerUpdates updates) {
    assert(
      _clusterManagersController != null,
      'Cannot update markers after dispose().',
    );
    _clusterManagersController?.addClusterManagers(
      updates.clusterManagersToAdd,
    );
    _clusterManagersController?.removeClusterManagers(
      updates.clusterManagerIdsToRemove,
    );
  }

  /// Applies [GroundOverlayUpdates] to the currently managed ground overlays.
  void updateGroundOverlays(GroundOverlayUpdates updates) {
    assert(
      _groundOverlaysController != null,
      'Cannot update ground overlays after dispose().',
    );
    _groundOverlaysController?.addGroundOverlays(updates.groundOverlaysToAdd);
    _groundOverlaysController?.changeGroundOverlays(
      updates.groundOverlaysToChange,
    );
    _groundOverlaysController?.removeGroundOverlays(
      updates.groundOverlayIdsToRemove,
    );
  }

  /// Shows the [InfoWindow] of the marker identified by its [MarkerId].
  void showInfoWindow(MarkerId markerId) {
    assert(
      _markersController != null,
      'Cannot show infowindow of marker [${markerId.value}] after dispose().',
    );
    _markersController?.showMarkerInfoWindow(markerId);
  }

  /// Hides the [InfoWindow] of the marker identified by its [MarkerId].
  void hideInfoWindow(MarkerId markerId) {
    assert(
      _markersController != null,
      'Cannot hide infowindow of marker [${markerId.value}] after dispose().',
    );
    _markersController?.hideMarkerInfoWindow(markerId);
  }

  /// Returns true if the [InfoWindow] of the marker identified by [MarkerId] is shown.
  bool isInfoWindowShown(MarkerId markerId) {
    return _markersController?.isInfoWindowShown(markerId) ?? false;
  }

  // Cleanup

  /// Disposes of this controller and its resources.
  ///
  /// You won't be able to call many of the methods on this controller after
  /// calling `dispose`!
  void dispose() {
    _bridge.dispose();
    _webview = null;
    _circlesController = null;
    _polygonsController = null;
    _polylinesController = null;
    _markersController = null;
    _clusterManagersController = null;
    _groundOverlaysController = null;
    _streamController.close();
  }
}

/// A MapEvent event fired when a [mapId] on web is interactive.
class MapReadyEvent extends MapEvent<Object?> {
  /// Build a WebMapReady Event for the map represented by `mapId`.
  MapReadyEvent(int mapId) : super(mapId, null);
}
