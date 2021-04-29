// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of google_maps_flutter_tizen;

// TODO : implement for google_map_tizen if necessary
// /// Type used when passing an override to the _createMap function.
// @visibleForTesting
// typedef DebugCreateMapFunction = gmaps.GMap Function(
//     HtmlElement div, gmaps.MapOptions options);

final _nullLatLng = LatLng(0, 0);
final _nullLatLngBounds =
    LatLngBounds(southwest: _nullLatLng, northeast: _nullLatLng);

/// Encapsulates a [gmaps.GMap], its events, and where in the DOM it's rendered.
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

  // TODO : implement for google_map_tizen if necessary
  // // The Flutter widget that contains the rendered Map.
  // HtmlElementView? _widget;
  // late HtmlElement _div;
  WebView? _widget;
  Completer<WebViewController> _controller = Completer<WebViewController>();

  // TODO : implement for google_map_tizen if necessary
  /// The Flutter widget that will contain the rendered Map. Used for caching.
  Widget? get widget {
    if (_widget == null && !_streamController.isClosed) {
      // TODO : implement for google_map_tizen if necessary
      // _widget = HtmlElementView(
      //   viewType: _getViewType(_mapId),
      // );

      _widget = WebView(
        initialUrl: 'https://seungsoo47.github.io/map.html',
        javascriptMode: JavascriptMode.unrestricted,
        onWebViewCreated: (WebViewController webViewController) {
          _controller.complete(webViewController);
        },
        onProgress: (int progress) {
          print("WebView is loading (progress : $progress%)");
        },
        javascriptChannels: <JavascriptChannel>{
          _toasterJavascriptChannel(),
        },
        navigationDelegate: (NavigationRequest request) {
          if (request.url.startsWith('https://www.youtube.com/')) {
            print('blocking navigation to $request}');
            return NavigationDecision.prevent;
          }
          print('allowing navigation to $request');
          return NavigationDecision.navigate;
        },
        onPageStarted: (String url) {
          print('Page started loading: $url');
        },
        onPageFinished: (String url) {
          print('Page finished loading: $url');
        },
        gestureNavigationEnabled: true,
      );
    }
    return _widget;
  }

  JavascriptChannel _toasterJavascriptChannel() {
    return JavascriptChannel(
        name: 'Toaster',
        onMessageReceived: (JavascriptMessage message) {
          print('ToasterMessage : ${message.message}');
          // // ignore: deprecated_member_use
          // Scaffold.of(context).showSnackBar(
          //   SnackBar(content: Text(message.message)),
          // );
        });
  }

  // TODO : implement for google_map_tizen if necessary
  // // The currently-enabled traffic layer.
  // gmaps.TrafficLayer? _trafficLayer;
  // /// A getter for the current traffic layer. Only for tests.
  // @visibleForTesting
  // gmaps.TrafficLayer? get trafficLayer => _trafficLayer;
  // // The underlying GMap instance. This is the interface with the JS SDK.
  // gmaps.GMap? _googleMap;

  // The StreamController used by this controller and the geometry ones.
  final StreamController<MapEvent> _streamController;

  /// The Stream over which this controller broadcasts events.
  Stream<MapEvent> get events => _streamController.stream;

  // TODO : implement for google_map_tizen if necessary
  // // Geometry controllers, for different features of the map.
  // CirclesController? _circlesController;
  // PolygonsController? _polygonsController;
  // PolylinesController? _polylinesController;
  // MarkersController? _markersController;

  // TODO : implement for google_map_tizen if necessary
  // // Keeps track if _attachGeometryControllers has been called or not.
  // bool _controllersBoundToMap = false;
  // // Keeps track if the map is moving or not.
  // bool _mapIsMoving = false;

  /// Initializes the GMap, and the sub-controllers related to it. Wires events.
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
    // _markersController = MarkersController(stream: this._streamController);

    // TODO : implement for google_map_tizen if necessary
    // // Register the view factory that will hold the `_div` that holds the map in the DOM.
    // // The `_div` needs to be created outside of the ViewFactory (and cached!) so we can
    // // use it to create the [gmaps.GMap] in the `init()` method of this class.
    // _div = DivElement()..id = _getViewType(mapId);
    // ui.platformViewRegistry.registerViewFactory(
    //   _getViewType(mapId),
    //   (int viewId) => _div,
    // );
  }

  // TODO : implement for google_map_tizen if necessary
  // /// Overrides certain properties to install mocks defined during testing.
  // @visibleForTesting
  // void debugSetOverrides({
  //   DebugCreateMapFunction? createMap,
  //   MarkersController? markers,
  //   CirclesController? circles,
  //   PolygonsController? polygons,
  //   PolylinesController? polylines,
  // }) {
  //   _overrideCreateMap = createMap;
  //   _markersController = markers ?? _markersController;
  //   _circlesController = circles ?? _circlesController;
  //   _polygonsController = polygons ?? _polygonsController;
  //   _polylinesController = polylines ?? _polylinesController;
  // }
  // DebugCreateMapFunction? _overrideCreateMap;

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
  }

  // TODO : implement for google_map_tizen if necessary
  // // Funnels map gmap events into the plugin's stream controller.
  // void _attachMapEvents(gmaps.GMap map) {
  //   map.onClick.listen((event) {
  //     assert(event.latLng != null);
  //     _streamController.add(
  //       MapTapEvent(_mapId, _gmLatLngToLatLng(event.latLng!)),
  //     );
  //   });
  //   map.onRightclick.listen((event) {
  //     assert(event.latLng != null);
  //     _streamController.add(
  //       MapLongPressEvent(_mapId, _gmLatLngToLatLng(event.latLng!)),
  //     );
  //   });
  //   map.onBoundsChanged.listen((event) {
  //     if (!_mapIsMoving) {
  //       _mapIsMoving = true;
  //       _streamController.add(CameraMoveStartedEvent(_mapId));
  //     }
  //     _streamController.add(
  //       CameraMoveEvent(_mapId, _gmViewportToCameraPosition(map)),
  //     );
  //   });
  //   map.onIdle.listen((event) {
  //     _mapIsMoving = false;
  //     _streamController.add(CameraIdleEvent(_mapId));
  //   });
  // }

  // TODO : implement for google_map_tizen if necessary
  // // Binds the Geometry controllers to a map instance
  // void _attachGeometryControllers(gmaps.GMap map) {
  //   // Now we can add the initial geometry.
  //   // And bind the (ready) map instance to the other geometry controllers.
  //   //
  //   // These controllers are either created in the constructor of this class, or
  //   // overriden (for testing) by the [debugSetOverrides] method. They can't be
  //   // null.
  //   assert(_circlesController != null,
  //       'Cannot attach a map to a null CirclesController instance.');
  //   assert(_polygonsController != null,
  //       'Cannot attach a map to a null PolygonsController instance.');
  //   assert(_polylinesController != null,
  //       'Cannot attach a map to a null PolylinesController instance.');
  //   assert(_markersController != null,
  //       'Cannot attach a map to a null MarkersController instance.');
  //   _circlesController!.bindToMap(_mapId, map);
  //   _polygonsController!.bindToMap(_mapId, map);
  //   _polylinesController!.bindToMap(_mapId, map);
  //   _markersController!.bindToMap(_mapId, map);
  //   _controllersBoundToMap = true;
  // }

  // TODO : implement for google_map_tizen if necessary
  // // Renders the initial sets of geometry.
  // void _renderInitialGeometry({
  //   Set<Marker> markers = const {},
  //   Set<Circle> circles = const {},
  //   Set<Polygon> polygons = const {},
  //   Set<Polyline> polylines = const {},
  // }) {
  //   assert(
  //       _controllersBoundToMap,
  //       'Geometry controllers must be bound to a map before any geometry can ' +
  //           'be added to them. Ensure _attachGeometryControllers is called first.');
  //   // The above assert will only succeed if the controllers have been bound to a map
  //   // in the [_attachGeometryControllers] method, which ensures that all these
  //   // controllers below are *not* null.
  //   _markersController!.addMarkers(markers);
  //   _circlesController!.addCircles(circles);
  //   _polygonsController!.addPolygons(polygons);
  //   _polylinesController!.addPolylines(polylines);
  // }

  // TODO : implement for google_map_tizen if necessary
  // // Merges new options coming from the plugin into the _rawMapOptions map.
  // //
  // // Returns the updated _rawMapOptions object.
  // Map<String, dynamic> _mergeRawOptions(Map<String, dynamic> newOptions) {
  //   _rawMapOptions = <String, dynamic>{
  //     ..._rawMapOptions,
  //     ...newOptions,
  //   };
  //   return _rawMapOptions;
  // }

  /// Updates the map options from a `Map<String, dynamic>`.
  ///
  /// This method converts the map into the proper [gmaps.MapOptions]
  void updateRawOptions(Map<String, dynamic> optionsUpdate) {
    // TODO : implement for google_map_tizen if necessary
    // assert(_googleMap != null, 'Cannot update options on a null map.');
    // final newOptions = _mergeRawOptions(optionsUpdate);
    // _setOptions(_rawOptionsToGmapsOptions(newOptions));
    // _setTrafficLayer(_googleMap!, _isTrafficLayerEnabled(newOptions));

    assert(_widget != null, 'Cannot update options on a null map.');
  }

  // TODO : implement for google_map_tizen if necessary
  // // Sets new [gmaps.MapOptions] on the wrapped map.
  // void _setOptions(gmaps.MapOptions options) {
  //   _googleMap?.options = options;
  // }

  // TODO : implement for google_map_tizen if necessary
  // // Attaches/detaches a Traffic Layer on the passed `map` if `attach` is true/false.
  // void _setTrafficLayer(gmaps.GMap map, bool attach) {
  //   if (attach && _trafficLayer == null) {
  //     _trafficLayer = gmaps.TrafficLayer()..set('map', map);
  //   }
  //   if (!attach && _trafficLayer != null) {
  //     _trafficLayer!.set('map', null);
  //     _trafficLayer = null;
  //   }
  // }

  Future<String> _callMethod(
      WebViewController c, String method, List<Object?> args) async {
    String command = 'JSON.stringify(map.$method.apply(map, $args))';
    String result = await c.evaluateJavascript(command);

    return result;
  }

  LatLngBounds _convertToBounds(String value) {
    try {
      Map<String, dynamic> map = json.decode(value);
      return LatLngBounds(
          southwest: LatLng(map['south'], map['west']),
          northeast: LatLng(map['north'], map['east']));
    } catch (e) {
      print('Javascript Error: ${e}');
      return _nullLatLngBounds;
    }
  }

  Future<LatLngBounds> _getBounds(WebViewController c) async {
    final String value = await _callMethod(c, 'getBounds', []);
    LatLngBounds result = _convertToBounds(value);
    return result;
  }

  /// Returns the [LatLngBounds] of the current viewport.
  Future<LatLngBounds> getVisibleRegion() async {
    assert(_widget != null, 'Cannot get the visible region of a null map.');

    WebViewController controller = await _controller.future;
    LatLngBounds result = await _getBounds(controller);

    return result;
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
    // TODO : implement for google_map_tizen if necessary
    // assert(_googleMap != null, 'Cannot update the camera of a null map.');
    // return _applyCameraUpdate(_googleMap!, cameraUpdate);

    assert(_widget != null, 'Cannot update the camera of a null map.');
  }

  /// Returns the zoom level of the current viewport.
  Future<double> getZoomLevel() async {
    // TODO : implement for google_map_tizen if necessary
    // assert(_googleMap != null, 'Cannot get zoom level of a null map.');
    // assert(_googleMap!.zoom != null,
    //     'Zoom level should not be null. Is the map correctly initialized?');
    // return _googleMap!.zoom!.toDouble();

    assert(_widget != null, 'Cannot get zoom level of a null map.');
    return 0.0;
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
    // assert(
    //     _markersController != null, 'Cannot update markers after dispose().');
    // _markersController?.addMarkers(updates.markersToAdd);
    // _markersController?.changeMarkers(updates.markersToChange);
    // _markersController?.removeMarkers(updates.markerIdsToRemove);
  }

  /// Shows the [InfoWindow] of the marker identified by its [MarkerId].
  void showInfoWindow(MarkerId markerId) {
    // TODO : implement for google_map_tizen if necessary
    // assert(_markersController != null,
    //     'Cannot show infowindow of marker [${markerId.value}] after dispose().');
    // _markersController?.showMarkerInfoWindow(markerId);
  }

  /// Hides the [InfoWindow] of the marker identified by its [MarkerId].
  void hideInfoWindow(MarkerId markerId) {
    // TODO : implement for google_map_tizen if necessary
    // assert(_markersController != null,
    //     'Cannot hide infowindow of marker [${markerId.value}] after dispose().');
    // _markersController?.hideMarkerInfoWindow(markerId);
  }

  /// Returns true if the [InfoWindow] of the marker identified by [MarkerId] is shown.
  bool isInfoWindowShown(MarkerId markerId) {
    // TODO : implement for google_map_tizen if necessary
    // return _markersController?.isInfoWindowShown(markerId) ?? false;

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
    // _markersController = null;
    // _streamController.close();

    _widget = null;
    _streamController.close();
  }
}
