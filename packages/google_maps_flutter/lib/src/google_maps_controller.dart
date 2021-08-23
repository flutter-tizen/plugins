// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of google_maps_flutter_tizen;

class GoogleMapController {
  // The internal ID of the map. Used to broadcast events, DOM IDs and everything where a unique ID is needed.
  final int _mapId;

  final CameraPosition _initialCameraPosition;
  final Set<Marker> _markers;
  final Set<Polygon> _polygons;
  final Set<Polyline> _polylines;
  final Set<Circle> _circles;
  // The raw options passed by the user, before converting to gmaps.
  // Caching this allows us to re-create the map faithfully when needed.
  Map<String, dynamic> _rawMapOptions = <String, dynamic>{};

  // Creates the 'viewType' for the _widget
  String _getViewType(int mapId) => 'plugins.flutter.io/google_maps_$mapId';

  WebView? _widget;
  final Completer<WebViewController> _controller =
      Completer<WebViewController>();
  Future<WebViewController> get controller => _controller.future;

  late WebViewController temp;
  void _getWebview() {
    print('_getWebview()');

    // TODO(seungsoo47): We need to check if the variable's value exists.
    // If the variable does not exist, we must find other alternatives.
    String path = Platform.environment['AUL_ROOT_PATH'] ?? '';
    path += '/res/flutter_assets/assets/map.html';

    _widget = WebView(
      initialUrl: path,
      javascriptMode: JavascriptMode.unrestricted,
      onWebViewCreated: (WebViewController webViewController) async {
        temp = webViewController;
      },
      onProgress: (int progress) {
        print('Google map is loading (progress : $progress%)');
      },
      javascriptChannels: <JavascriptChannel>{
        _onBoundsChanged(),
        _onIdle(),
        _onClick(),
        _onRightClick(),
        _onMarkerClick(),
        _onMarkerDragEnd(),
      },
      onPageStarted: (String url) {
        print('Google map started loading');
      },
      onPageFinished: (String url) async {
        _controller.complete(temp);
        print('Google map finished loading');
      },
      gestureNavigationEnabled: true,
    );
  }

  Future<void> _createMap() async {
    final String options = _createOptions();
    final String command = '''
      map = new google.maps.Map(document.getElementById('map'), $options);
      map.addListener('bounds_changed', BoundChanged.postMessage);
      map.addListener('idle', Idle.postMessage);
      map.addListener('click', (event) => Click.postMessage(JSON.stringify(event)));
      map.addListener('rightclick', (event) => RightClick.postMessage(JSON.stringify(event)));
    ''';
    await (await controller).evaluateJavascript(command);
  }

  /// The Flutter widget that will contain the rendered Map. Used for caching.
  Widget? get widget {
    if (_widget == null && !_streamController.isClosed) {
      _getWebview();
      _createMap();
    }
    return _widget;
  }

  String _createOptions() {
    String options = _rawOptionsToString(_rawMapOptions);
    options = _applyInitialPosition(_initialCameraPosition, options);
    return '{$options}';
  }

  // The StreamController used by this controller and the geometry ones.
  final StreamController<MapEvent> _streamController;

  /// The Stream over which this controller broadcasts events.
  Stream<MapEvent> get events => _streamController.stream;

  // TODO : implement for google_map_tizen if necessary
  // // Geometry controllers, for different features of the map.
  // CirclesController? _circlesController;
  // PolygonsController? _polygonsController;
  // PolylinesController? _polylinesController;
  MarkersController? _markersController;

  // Keeps track if _attachGeometryControllers has been called or not.
  bool _controllersBoundToMap = false;
  // Keeps track if the map is moving or not.
  bool _mapIsMoving = false;

  /// Initializes the GoogleMapController.
  GoogleMapController({
    required int mapId,
    required StreamController<MapEvent> streamController,
    required CameraPosition initialCameraPosition,
    Set<Marker> markers = const <Marker>{},
    Set<Polygon> polygons = const <Polygon>{},
    Set<Polyline> polylines = const <Polyline>{},
    Set<Circle> circles = const <Circle>{},
    Set<TileOverlay> tileOverlays = const <TileOverlay>{},
    Set<Factory<OneSequenceGestureRecognizer>> gestureRecognizers =
        const <Factory<OneSequenceGestureRecognizer>>{},
    Map<String, dynamic> mapOptions = const <String, dynamic>{},
  })  : _mapId = mapId,
        _streamController = streamController,
        _initialCameraPosition = initialCameraPosition,
        _markers = markers,
        _polygons = polygons,
        _polylines = polylines,
        _circles = circles,
        _rawMapOptions = mapOptions {
    // TODO : implement for google_map_tizen if necessary
    // _circlesController = CirclesController(stream: this._streamController);
    // _polygonsController = PolygonsController(stream: this._streamController);
    // _polylinesController = PolylinesController(stream: this._streamController);
    _markersController = MarkersController(stream: _streamController);
  }

  JavascriptChannel _onBoundsChanged() {
    return JavascriptChannel(
        name: 'BoundChanged',
        onMessageReceived: (JavascriptMessage message) async {
          final LatLng center = await getCenter();
          final double zoom = await getZoomLevel();
          if (!_mapIsMoving) {
            _mapIsMoving = true;
            _streamController.add(CameraMoveStartedEvent(_mapId));
          }
          _streamController.add(
            CameraMoveEvent(_mapId, CameraPosition(target: center, zoom: zoom)),
          );
        });
  }

  JavascriptChannel _onIdle() {
    return JavascriptChannel(
        name: 'Idle',
        onMessageReceived: (JavascriptMessage message) async {
          _mapIsMoving = false;
          _streamController.add(CameraIdleEvent(_mapId));
        });
  }

  JavascriptChannel _onClick() {
    return JavascriptChannel(
        name: 'Click',
        onMessageReceived: (JavascriptMessage message) async {
          try {
            final dynamic event = json.decode(message.message);
            if (event is Map<String, dynamic>) {
              assert(event['latLng'] != null);
              final LatLng position = LatLng(event['latLng']['lat'] as double,
                  event['latLng']['lng'] as double);
              _streamController.add(MapTapEvent(_mapId, position));
            }
          } catch (e) {
            print('Javascript Error: $e');
          }
        });
  }

  JavascriptChannel _onMarkerClick() {
    return JavascriptChannel(
        name: 'MarkerClick',
        onMessageReceived: (JavascriptMessage message) async {
          try {
            final dynamic id = json.decode(message.message);
            if (_markersController != null && id is int) {
              final MarkerId? markerId = _markersController!._idToMarkerId[id];
              final MarkerController? marker =
                  _markersController!._markerIdToController[markerId];
              marker?._onTop!();
            }
          } catch (e) {
            print('Javascript Error: $e');
          }
        });
  }

  JavascriptChannel _onMarkerDragEnd() {
    return JavascriptChannel(
        name: 'MarkerDragEnd',
        onMessageReceived: (JavascriptMessage message) async {
          try {
            // TODO(seungsoo47): Implement properly.
          } catch (e) {
            print('Javascript Error: $e');
          }
        });
  }

  JavascriptChannel _onRightClick() {
    // NOTE: LWE does not support a long press event.
    return JavascriptChannel(
        name: 'RightClick',
        onMessageReceived: (JavascriptMessage message) async {
          try {
            final dynamic event = json.decode(message.message);
            if (event is Map<String, dynamic>) {
              assert(event['latLng'] != null);
              final LatLng position = LatLng(event['latLng']['lat'] as double,
                  event['latLng']['lng'] as double);
              _streamController.add(MapLongPressEvent(_mapId, position));
            }
          } catch (e) {
            print('Javascript Error: $e');
          }
        });
  }

  // TODO : implement for google_map_tizen if necessary
  // gmaps.GMap _createMap(HtmlElement div, gmaps.MapOptions options) {
  //   if (_overrideCreateMap != null) {
  //     return _overrideCreateMap!(div, options);
  //   }
  //   return gmaps.GMap(div, options);
  // }

  /// Initializes the [gmaps.GMap] instance from the stored `rawOptions`.
  ///
  /// This method actually renders the GMap into the cached `_div`. This is
  /// called by the [GoogleMapsPlugin.init] method when appropriate.
  ///
  /// Failure to call this method would result in the GMap not rendering at all,
  /// and most of the public methods on this class no-op'ing.
  void init() {
    // TODO : implement for google_map_tizen if necessary
    // var options = _rawOptionsToGmapsOptions(_rawMapOptions);
    // // Initial position can only to be set here!
    // options = _applyInitialPosition(_initialCameraPosition, options);
    // // Create the map...
    // final map = _createMap(_div, options);
    // _googleMap = map;
    // _attachMapEvents(map);
    // _attachGeometryControllers(map);
    // _renderInitialGeometry(
    //   markers: _markers,
    //   circles: _circles,
    //   polygons: _polygons,
    //   polylines: _polylines,
    // );
    // _setTrafficLayer(map, _isTrafficLayerEnabled(_rawMapOptions));
    //

    print('init()');
    if (_widget == null && !_streamController.isClosed) {
      _getWebview();
      _createMap();
    }
    _attachGeometryControllers();
    _renderInitialGeometry(
      markers: _markers,
      circles: _circles,
      polygons: _polygons,
      polylines: _polylines,
    );
    _setTrafficLayer(_isTrafficLayerEnabled(_rawMapOptions));
  }

  // Binds the Geometry controllers to a map instance
  void _attachGeometryControllers() {
    // Now we can add the initial geometry.
    // And bind the (ready) map instance to the other geometry controllers.
    //
    // These controllers are either created in the constructor of this class, or
    // overriden (for testing) by the [debugSetOverrides] method. They can't be
    // null.
    //   assert(_circlesController != null,
    //       'Cannot attach a map to a null CirclesController instance.');
    //   assert(_polygonsController != null,
    //       'Cannot attach a map to a null PolygonsController instance.');
    //   assert(_polylinesController != null,
    //       'Cannot attach a map to a null PolylinesController instance.');
    assert(_markersController != null,
        'Cannot attach a map to a null MarkersController instance.');
    //   _circlesController!.bindToMap(_mapId, map);
    //   _polygonsController!.bindToMap(_mapId, map);
    //   _polylinesController!.bindToMap(_mapId, map);
    _markersController!.bindToMap(_mapId, _widget!);
    util.webview = _widget!;
    util.webController = controller;
    _controllersBoundToMap = true;
  }

  // Renders the initial sets of geometry.
  void _renderInitialGeometry({
    Set<Marker> markers = const {},
    Set<Circle> circles = const {},
    Set<Polygon> polygons = const {},
    Set<Polyline> polylines = const {},
  }) {
    assert(
        _controllersBoundToMap,
        'Geometry controllers must be bound to a map before any geometry can ' +
            'be added to them. Ensure _attachGeometryControllers is called first.');
    // The above assert will only succeed if the controllers have been bound to a map
    // in the [_attachGeometryControllers] method, which ensures that all these
    // controllers below are *not* null.
    _markersController!.addMarkers(markers);
    //   _circlesController!.addCircles(circles);
    //   _polygonsController!.addPolygons(polygons);
    //   _polylinesController!.addPolylines(polylines);
  }

  // Merges new options coming from the plugin into the _rawMapOptions map.
  //
  // Returns the updated _rawMapOptions object.
  Map<String, dynamic> _mergeRawOptions(Map<String, dynamic> newOptions) {
    _rawMapOptions = <String, dynamic>{
      ..._rawMapOptions,
      ...newOptions,
    };
    return _rawMapOptions;
  }

  /// Updates the map options from a `Map<String, dynamic>`.
  ///
  /// This method converts the map into the proper [gmaps.MapOptions]
  void updateRawOptions(Map<String, dynamic> optionsUpdate) {
    assert(_widget != null, 'Cannot update options on a null map.');

    final Map<String, dynamic> newOptions = _mergeRawOptions(optionsUpdate);
    final String options = _rawOptionsToString(newOptions);

    _setOptions('{$options}');
    _setTrafficLayer(_isTrafficLayerEnabled(newOptions));
  }

  Future<void> _setOptions(String options) async {
    if (_controller.isCompleted) {
      await _callMethod(await controller, 'setOptions', [options]);
    }
  }

  Future<void> _setZoom(String options) async {
    await _callMethod(await controller, 'setZoom', [options]);
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
    await (await controller).evaluateJavascript(command);
  }

  Future<void> _setMoveCamera(String options) async {
    await _callMethod(await controller, 'moveCamera', [options]);
  }

  Future<void> _setPanTo(String options) async {
    await _callMethod(await controller, 'panTo', [options]);
  }

  Future<void> _setPanBy(String options) async {
    await _callMethod(await controller, 'panBy', [options]);
  }

  Future<void> _setFitBounds(String options) async {
    await _callMethod(await controller, 'fitBounds', [options]);
  }

  Future<String> _callMethod(
      WebViewController mapView, String method, List<Object?> args) async {
    final String command = 'JSON.stringify(map.$method.apply(map, $args))';
    print('callMethod: $command');
    final String result = await mapView.evaluateJavascript(command);
    return result;
  }

  Future<LatLngBounds> _getBounds(WebViewController c) async {
    final String value = await _callMethod(c, 'getBounds', []);
    return _convertToBounds(value);
  }

  Future<LatLng> _getCenter(WebViewController c) async {
    final String value = await _callMethod(c, 'getCenter', []);
    return _convertToLatLng(value);
  }

  Future<double> _getZoom(WebViewController c) async {
    try {
      final String value = await _callMethod(c, 'getZoom', []);
      return double.parse(value);
    } catch (e) {
      print('Javascript Error: $e');
      return 0.0;
    }
  }

  /// Returns the [LatLngBounds] of the current viewport.
  Future<LatLngBounds> getVisibleRegion() async {
    assert(_widget != null, 'Cannot get the visible region of a null map.');
    return await _getBounds(await controller);
  }

  Future<LatLng> getCenter() async {
    assert(_widget != null, 'Cannot get the visible region of a null map.');
    return await _getCenter(await controller);
  }

  /// Returns the [ScreenCoordinate] for a given viewport [LatLng].
  Future<ScreenCoordinate> getScreenCoordinate(LatLng latLng) async {
    // TODO : implement for google_map_tizen if necessary
    // assert(_googleMap != null,
    //     'Cannot get the screen coordinates with a null map.');
    // assert(_googleMap!.projection != null,
    //     'Cannot compute screen coordinate with a null map or projection.');
    // final point =
    //     _googleMap!.projection!.fromLatLngToPoint!(_latLngToGmLatLng(latLng))!;
    // assert(point.x != null && point.y != null,
    //     'The x and y of a ScreenCoordinate cannot be null.');
    // return ScreenCoordinate(x: point.x!.toInt(), y: point.y!.toInt());

    assert(
        _widget != null, 'Cannot get the screen coordinates with a null map.');
    return ScreenCoordinate(x: 0, y: 0);
  }

  /// Returns the [LatLng] for a `screenCoordinate` (in pixels) of the viewport.
  Future<LatLng> getLatLng(ScreenCoordinate screenCoordinate) async {
    // TODO : implement for google_map_tizen if necessary
    // assert(_googleMap != null,
    //     'Cannot get the lat, lng of a screen coordinate with a null map.');
    // final gmaps.LatLng latLng =
    //     _pixelToLatLng(_googleMap!, screenCoordinate.x, screenCoordinate.y);
    // return _gmLatLngToLatLng(latLng);

    assert(_widget != null,
        'Cannot get the lat, lng of a screen coordinate with a null map.');
    return LatLng(0, 0);
  }

  /// Applies a `cameraUpdate` to the current viewport.
  Future<void> moveCamera(CameraUpdate cameraUpdate) async {
    assert(_widget != null, 'Cannot update the camera of a null map.');
    _applyCameraUpdate(cameraUpdate);
  }

  // Translates a [CameraUpdate] into operations on a [Javascript].
  Future<void> _applyCameraUpdate(CameraUpdate update) async {
    final List<dynamic> json = update.toJson() as List<dynamic>;
    switch (json[0]) {
      case 'newCameraPosition':
        _setMoveCamera(
            '{heading: ${json[1]['bearing']}, zoom: ${json[1]['zoom']}, tilt: ${json[1]['tilt']}}');
        _setPanTo(
            '{lat:${json[1]['target'][0]}, lng: ${json[1]['target'][1]}}');
        break;
      case 'newLatLng':
        _setPanTo('{lat:${json[1][0]}, lng:${json[1][1]}}');
        break;
      case 'newLatLngZoom':
        _setMoveCamera('{zoom: ${json[2]}}');
        _setPanTo('{lat:${json[1][0]}, lng: ${json[1][1]}}');
        break;
      case 'newLatLngBounds':
        _setFitBounds(
            '{south:${json[1][0][0]}, west:${json[1][0][1]}, north:${json[1][1][0]}, east:${json[1][1][1]}}, ${json[2]}');
        break;
      case 'scrollBy':
        _setPanBy('${json[1]}, ${json[2]}');
        break;
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
                json[2][0] as double, json[2][1] as double);
          } catch (e) {
            print('Error computing focus LatLng. JS Error: ' + e.toString());
          }
        }
        _setZoom('${(await getZoomLevel()) + newZoomDelta}');
        if (focusLatLng != null) {
          _setPanTo(focusLatLng);
        }
        break;
      case 'zoomIn':
        _setZoom('${(await getZoomLevel()) + 1}');
        break;
      case 'zoomOut':
        _setZoom('${(await getZoomLevel()) - 1}');
        break;
      case 'zoomTo':
        _setZoom('${json[1]}');
        break;
      default:
        throw UnimplementedError('Unimplemented CameraMove: ${json[0]}.');
    }
  }

  Future<String> _pixelToLatLng(double x, double y) async {
    final String command = '''
      function assert(condition, message) {
        if (!condition) {
          throw new Error(message || "Assertion failed");
        }
      }

      function getPixelToLatLng() {
        var bounds = map.getBounds();
        var projection = map.getProjection();
        var zoom = map.zoom;

        assert(bounds != null, 'Map Bounds required to compute LatLng of screen x/y.');
        assert(projection != null, 'Map Projection required to compute LatLng of screen x/y');
        assert(zoom != null, 'Current map zoom level required to compute LatLng of screen x/y');

        var ne = bounds.getNorthEast();
        var sw = bounds.getSouthWest();
        var topRight = projection.fromLatLngToPoint(ne);
        var bottomLeft = projection.fromLatLngToPoint(sw);
        var scale = 1 << parseInt(zoom); // 2 ^ zoom

        var point = new google.maps.Point(topRight.x - ($x / scale), bottomLeft.y - ($y / scale));
        return projection.fromPointToLatLng(point);
      }

      JSON.stringify(getPixelToLatLng());
    ''';

    return await (await controller).evaluateJavascript(command);
  }

  /// Returns the zoom level of the current viewport.
  Future<double> getZoomLevel() async {
    assert(_widget != null, 'Cannot get zoom level of a null map.');
    return await _getZoom(await controller);
  }

  // Geometry manipulation

  /// Applies [CircleUpdates] to the currently managed circles.
  void updateCircles(CircleUpdates updates) {
    // TODO : implement for google_map_tizen if necessary
    // assert(
    //     _circlesController != null, 'Cannot update circles after dispose().');
    // _circlesController?.addCircles(updates.circlesToAdd);
    // _circlesController?.changeCircles(updates.circlesToChange);
    // _circlesController?.removeCircles(updates.circleIdsToRemove);
  }

  /// Applies [PolygonUpdates] to the currently managed polygons.
  void updatePolygons(PolygonUpdates updates) {
    // TODO : implement for google_map_tizen if necessary
    // assert(
    //     _polygonsController != null, 'Cannot update polygons after dispose().');
    // _polygonsController?.addPolygons(updates.polygonsToAdd);
    // _polygonsController?.changePolygons(updates.polygonsToChange);
    // _polygonsController?.removePolygons(updates.polygonIdsToRemove);
  }

  /// Applies [PolylineUpdates] to the currently managed lines.
  void updatePolylines(PolylineUpdates updates) {
    // TODO : implement for google_map_tizen if necessary
    // assert(_polylinesController != null,
    //     'Cannot update polylines after dispose().');
    // _polylinesController?.addPolylines(updates.polylinesToAdd);
    // _polylinesController?.changePolylines(updates.polylinesToChange);
    // _polylinesController?.removePolylines(updates.polylineIdsToRemove);
  }

  /// Applies [MarkerUpdates] to the currently managed markers.
  void updateMarkers(MarkerUpdates updates) {
    // TODO : implement for google_map_tizen if necessary
    assert(
        _markersController != null, 'Cannot update markers after dispose().');
    _markersController?.addMarkers(updates.markersToAdd);
    _markersController?.changeMarkers(updates.markersToChange);
    _markersController?.removeMarkers(updates.markerIdsToRemove);
  }

  /// Shows the [InfoWindow] of the marker identified by its [MarkerId].
  void showInfoWindow(MarkerId markerId) {
    // TODO : implement for google_map_tizen if necessary
    assert(_markersController != null,
        'Cannot show infowindow of marker [${markerId.value}] after dispose().');
    _markersController?.showMarkerInfoWindow(markerId);
  }

  /// Hides the [InfoWindow] of the marker identified by its [MarkerId].
  void hideInfoWindow(MarkerId markerId) {
    // TODO : implement for google_map_tizen if necessary
    assert(_markersController != null,
        'Cannot hide infowindow of marker [${markerId.value}] after dispose().');
    _markersController?.hideMarkerInfoWindow(markerId);
  }

  /// Returns true if the [InfoWindow] of the marker identified by [MarkerId] is shown.
  bool isInfoWindowShown(MarkerId markerId) {
    // TODO : implement for google_map_tizen if necessary
    return _markersController?.isInfoWindowShown(markerId) ?? false;

    return false;
  }

  // Cleanup

  /// Disposes of this controller and its resources.
  ///
  /// You won't be able to call many of the methods on this controller after
  /// calling `dispose`!
  void dispose() {
    // TODO : implement for google_map_tizen if necessary
    // _widget = null;
    // _googleMap = null;
    // _circlesController = null;
    // _polygonsController = null;
    // _polylinesController = null;
    _markersController = null;
    // _streamController.close();

    _widget = null;
    _streamController.close();
  }
}
