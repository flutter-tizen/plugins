// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of google_maps_flutter_tizen;

/// Callback method for when the map is ready to be used.
///
/// Pass to [GoogleMap.onMapCreated] to receive a [GoogleMapController] when the
/// map is created.
typedef void MapCreatedCallback(GoogleMapController controller);

// This counter is used to provide a stable "constant" initialization id
// to the buildView function, so the web implementation can use it as a
// cache key. This needs to be provided from the outside, because web
// views seem to re-render much more often that mobile platform views.
int _nextMapCreationId = 0;

/// Error thrown when an unknown map object ID is provided to a method.
class UnknownMapObjectIdError extends Error {
  /// Creates an assertion error with the provided [message].
  UnknownMapObjectIdError(this.objectType, this.objectId, [this.context]);

  /// The name of the map object whose ID is unknown.
  final String objectType;

  /// The unknown maps object ID.
  final MapsObjectId objectId;

  /// The context where the error occurred.
  final String? context;

  String toString() {
    if (context != null) {
      return 'Unknown $objectType ID "${objectId.value}" in $context';
    }
    return 'Unknown $objectType ID "${objectId.value}"';
  }
}

/// A widget which displays a map with data obtained from the Google Maps service.
class GoogleMap extends StatefulWidget {
  /// Creates a widget displaying data from Google Maps services.
  ///
  /// [AssertionError] will be thrown if [initialCameraPosition] is null;
  const GoogleMap({
    Key? key,
    required this.initialCameraPosition,
    this.onMapCreated,
    this.gestureRecognizers = const <Factory<OneSequenceGestureRecognizer>>{},
    this.compassEnabled = true,
    this.mapToolbarEnabled = true,
    this.cameraTargetBounds = CameraTargetBounds.unbounded,
    this.mapType = MapType.normal,
    this.minMaxZoomPreference = MinMaxZoomPreference.unbounded,
    this.rotateGesturesEnabled = true,
    this.scrollGesturesEnabled = true,
    this.zoomControlsEnabled = true,
    this.zoomGesturesEnabled = true,
    this.liteModeEnabled = false,
    this.tiltGesturesEnabled = true,
    this.myLocationEnabled = false,
    this.myLocationButtonEnabled = true,

    /// If no padding is specified default padding will be 0.
    this.padding = const EdgeInsets.all(0),
    this.indoorViewEnabled = false,
    this.trafficEnabled = false,
    this.buildingsEnabled = true,
    this.markers = const <Marker>{},
    this.polygons = const <Polygon>{},
    this.polylines = const <Polyline>{},
    this.circles = const <Circle>{},
    this.onCameraMoveStarted,
    this.tileOverlays = const <TileOverlay>{},
    this.onCameraMove,
    this.onCameraIdle,
    this.onTap,
    this.onLongPress,
  })  : assert(initialCameraPosition != null),
        super(key: key);

  /// Callback method for when the map is ready to be used.
  ///
  /// Used to receive a [GoogleMapController] for this [GoogleMap].
  final MapCreatedCallback? onMapCreated;

  /// The initial position of the map's camera.
  final CameraPosition initialCameraPosition;

  /// True if the map should show a compass when rotated.
  final bool compassEnabled;

  /// True if the map should show a toolbar when you interact with the map. Android only.
  final bool mapToolbarEnabled;

  /// Geographical bounding box for the camera target.
  final CameraTargetBounds cameraTargetBounds;

  /// Type of map tiles to be rendered.
  final MapType mapType;

  /// Preferred bounds for the camera zoom level.
  ///
  /// Actual bounds depend on map data and device.
  final MinMaxZoomPreference minMaxZoomPreference;

  /// True if the map view should respond to rotate gestures.
  final bool rotateGesturesEnabled;

  /// True if the map view should respond to scroll gestures.
  final bool scrollGesturesEnabled;

  /// True if the map view should show zoom controls. This includes two buttons
  /// to zoom in and zoom out. The default value is to show zoom controls.
  ///
  /// This is only supported on Android. And this field is silently ignored on iOS.
  final bool zoomControlsEnabled;

  /// True if the map view should respond to zoom gestures.
  final bool zoomGesturesEnabled;

  /// True if the map view should be in lite mode. Android only.
  ///
  /// See https://developers.google.com/maps/documentation/android-sdk/lite#overview_of_lite_mode for more details.
  final bool liteModeEnabled;

  /// True if the map view should respond to tilt gestures.
  final bool tiltGesturesEnabled;

  /// Padding to be set on map. See https://developers.google.com/maps/documentation/android-sdk/map#map_padding for more details.
  final EdgeInsets padding;

  /// Markers to be placed on the map.
  final Set<Marker> markers;

  /// Polygons to be placed on the map.
  final Set<Polygon> polygons;

  /// Polylines to be placed on the map.
  final Set<Polyline> polylines;

  /// Circles to be placed on the map.
  final Set<Circle> circles;

  /// Tile overlays to be placed on the map.
  final Set<TileOverlay> tileOverlays;

  /// Called when the camera starts moving.
  ///
  /// This can be initiated by the following:
  /// 1. Non-gesture animation initiated in response to user actions.
  ///    For example: zoom buttons, my location button, or marker clicks.
  /// 2. Programmatically initiated animation.
  /// 3. Camera motion initiated in response to user gestures on the map.
  ///    For example: pan, tilt, pinch to zoom, or rotate.
  final VoidCallback? onCameraMoveStarted;

  /// Called repeatedly as the camera continues to move after an
  /// onCameraMoveStarted call.
  ///
  /// This may be called as often as once every frame and should
  /// not perform expensive operations.
  final CameraPositionCallback? onCameraMove;

  /// Called when camera movement has ended, there are no pending
  /// animations and the user has stopped interacting with the map.
  final VoidCallback? onCameraIdle;

  /// Called every time a [GoogleMap] is tapped.
  final ArgumentCallback<LatLng>? onTap;

  /// Called every time a [GoogleMap] is long pressed.
  final ArgumentCallback<LatLng>? onLongPress;

  /// True if a "My Location" layer should be shown on the map.
  ///
  /// This layer includes a location indicator at the current device location,
  /// as well as a My Location button.
  /// * The indicator is a small blue dot if the device is stationary, or a
  /// chevron if the device is moving.
  /// * The My Location button animates to focus on the user's current location
  /// if the user's location is currently known.
  ///
  /// Enabling this feature requires adding location permissions to both native
  /// platforms of your app.
  /// * On Android add either
  /// `<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />`
  /// or `<uses-permission android:name="android.permission.ACCESS_COARSE_LOCATION" />`
  /// to your `AndroidManifest.xml` file. `ACCESS_COARSE_LOCATION` returns a
  /// location with an accuracy approximately equivalent to a city block, while
  /// `ACCESS_FINE_LOCATION` returns as precise a location as possible, although
  /// it consumes more battery power. You will also need to request these
  /// permissions during run-time. If they are not granted, the My Location
  /// feature will fail silently.
  /// * On iOS add a `NSLocationWhenInUseUsageDescription` key to your
  /// `Info.plist` file. This will automatically prompt the user for permissions
  /// when the map tries to turn on the My Location layer.
  final bool myLocationEnabled;

  /// Enables or disables the my-location button.
  ///
  /// The my-location button causes the camera to move such that the user's
  /// location is in the center of the map. If the button is enabled, it is
  /// only shown when the my-location layer is enabled.
  ///
  /// By default, the my-location button is enabled (and hence shown when the
  /// my-location layer is enabled).
  ///
  /// See also:
  ///   * [myLocationEnabled] parameter.
  final bool myLocationButtonEnabled;

  /// Enables or disables the indoor view from the map
  final bool indoorViewEnabled;

  /// Enables or disables the traffic layer of the map
  final bool trafficEnabled;

  /// Enables or disables showing 3D buildings where available
  final bool buildingsEnabled;

  /// Which gestures should be consumed by the map.
  ///
  /// It is possible for other gesture recognizers to be competing with the map on pointer
  /// events, e.g if the map is inside a [ListView] the [ListView] will want to handle
  /// vertical drags. The map will claim gestures that are recognized by any of the
  /// recognizers on this list.
  ///
  /// When this set is empty, the map will only handle pointer events for gestures that
  /// were not claimed by any other gesture recognizer.
  final Set<Factory<OneSequenceGestureRecognizer>> gestureRecognizers;

  /// Creates a [State] for this [GoogleMap].
  @override
  State createState() => _GoogleMapState();
}

class _GoogleMapState extends State<GoogleMap> {
  final _mapId = _nextMapCreationId++;

  final Completer<GoogleMapController> _controller =
      Completer<GoogleMapController>();

  Map<MarkerId, Marker> _markers = <MarkerId, Marker>{};
  Map<PolygonId, Polygon> _polygons = <PolygonId, Polygon>{};
  Map<PolylineId, Polyline> _polylines = <PolylineId, Polyline>{};
  Map<CircleId, Circle> _circles = <CircleId, Circle>{};
  late _GoogleMapOptions _googleMapOptions;

  @override
  Widget build(BuildContext context) {
    return GoogleMapsFlutterPlatform.instance.buildView(
      _mapId,
      onPlatformViewCreated,
      initialCameraPosition: widget.initialCameraPosition,
      markers: widget.markers,
      polygons: widget.polygons,
      polylines: widget.polylines,
      circles: widget.circles,
      gestureRecognizers: widget.gestureRecognizers,
      mapOptions: _googleMapOptions.toMap(),
    );
  }

  @override
  void initState() {
    super.initState();
    _googleMapOptions = _GoogleMapOptions.fromWidget(widget);
    _markers = keyByMarkerId(widget.markers);
    _polygons = keyByPolygonId(widget.polygons);
    _polylines = keyByPolylineId(widget.polylines);
    _circles = keyByCircleId(widget.circles);
  }

  @override
  void dispose() async {
    super.dispose();
    GoogleMapController controller = await _controller.future;
    controller.dispose();
  }

  @override
  void didUpdateWidget(GoogleMap oldWidget) {
    super.didUpdateWidget(oldWidget);
    _updateOptions();
    _updateMarkers();
    _updatePolygons();
    _updatePolylines();
    _updateCircles();
    _updateTileOverlays();
  }

  void _updateOptions() async {
    final _GoogleMapOptions newOptions = _GoogleMapOptions.fromWidget(widget);
    final Map<String, dynamic> updates =
        _googleMapOptions.updatesMap(newOptions);
    if (updates.isEmpty) {
      return;
    }
    final GoogleMapController controller = await _controller.future;
    // ignore: unawaited_futures
    controller._updateMapOptions(updates);
    _googleMapOptions = newOptions;
  }

  void _updateMarkers() async {
    final GoogleMapController controller = await _controller.future;
    // ignore: unawaited_futures
    controller._updateMarkers(
        MarkerUpdates.from(_markers.values.toSet(), widget.markers));
    _markers = keyByMarkerId(widget.markers);
  }

  void _updatePolygons() async {
    final GoogleMapController controller = await _controller.future;
    // ignore: unawaited_futures
    controller._updatePolygons(
        PolygonUpdates.from(_polygons.values.toSet(), widget.polygons));
    _polygons = keyByPolygonId(widget.polygons);
  }

  void _updatePolylines() async {
    final GoogleMapController controller = await _controller.future;
    // ignore: unawaited_futures
    controller._updatePolylines(
        PolylineUpdates.from(_polylines.values.toSet(), widget.polylines));
    _polylines = keyByPolylineId(widget.polylines);
  }

  void _updateCircles() async {
    final GoogleMapController controller = await _controller.future;
    // ignore: unawaited_futures
    controller._updateCircles(
        CircleUpdates.from(_circles.values.toSet(), widget.circles));
    _circles = keyByCircleId(widget.circles);
  }

  void _updateTileOverlays() async {
    final GoogleMapController controller = await _controller.future;
    // ignore: unawaited_futures
    controller._updateTileOverlays(widget.tileOverlays);
  }

  Future<void> onPlatformViewCreated(int id) async {
    final GoogleMapController controller = await GoogleMapController.init(
      id,
      widget.initialCameraPosition,
      this,
    );
    _controller.complete(controller);
    _updateTileOverlays();
    final MapCreatedCallback? onMapCreated = widget.onMapCreated;
    if (onMapCreated != null) {
      onMapCreated(controller);
    }
  }

  void onMarkerTap(MarkerId markerId) {
    assert(markerId != null);
    final Marker? marker = _markers[markerId];
    if (marker == null) {
      throw UnknownMapObjectIdError('marker', markerId, 'onTap');
    }
    final VoidCallback? onTap = marker.onTap;
    if (onTap != null) {
      onTap();
    }
  }

  void onMarkerDragEnd(MarkerId markerId, LatLng position) {
    assert(markerId != null);
    final Marker? marker = _markers[markerId];
    if (marker == null) {
      throw UnknownMapObjectIdError('marker', markerId, 'onDragEnd');
    }
    final ValueChanged<LatLng>? onDragEnd = marker.onDragEnd;
    if (onDragEnd != null) {
      onDragEnd(position);
    }
  }

  void onPolygonTap(PolygonId polygonId) {
    assert(polygonId != null);
    final Polygon? polygon = _polygons[polygonId];
    if (polygon == null) {
      throw UnknownMapObjectIdError('polygon', polygonId, 'onTap');
    }
    final VoidCallback? onTap = polygon.onTap;
    if (onTap != null) {
      onTap();
    }
  }

  void onPolylineTap(PolylineId polylineId) {
    assert(polylineId != null);
    final Polyline? polyline = _polylines[polylineId];
    if (polyline == null) {
      throw UnknownMapObjectIdError('polyline', polylineId, 'onTap');
    }
    final VoidCallback? onTap = polyline.onTap;
    if (onTap != null) {
      onTap();
    }
  }

  void onCircleTap(CircleId circleId) {
    assert(circleId != null);
    final Circle? circle = _circles[circleId];
    if (circle == null) {
      throw UnknownMapObjectIdError('marker', circleId, 'onTap');
    }
    final VoidCallback? onTap = circle.onTap;
    if (onTap != null) {
      onTap();
    }
  }

  void onInfoWindowTap(MarkerId markerId) {
    assert(markerId != null);
    final Marker? marker = _markers[markerId];
    if (marker == null) {
      throw UnknownMapObjectIdError('marker', markerId, 'InfoWindow onTap');
    }
    final VoidCallback? onTap = marker.infoWindow.onTap;
    if (onTap != null) {
      onTap();
    }
  }

  void onTap(LatLng position) {
    assert(position != null);
    final ArgumentCallback<LatLng>? onTap = widget.onTap;
    if (onTap != null) {
      onTap(position);
    }
  }

  void onLongPress(LatLng position) {
    assert(position != null);
    final ArgumentCallback<LatLng>? onLongPress = widget.onLongPress;
    if (onLongPress != null) {
      onLongPress(position);
    }
  }
}

/// Configuration options for the GoogleMaps user interface.
class _GoogleMapOptions {
  _GoogleMapOptions.fromWidget(GoogleMap map)
      : compassEnabled = map.compassEnabled,
        mapToolbarEnabled = map.mapToolbarEnabled,
        cameraTargetBounds = map.cameraTargetBounds,
        mapType = map.mapType,
        minMaxZoomPreference = map.minMaxZoomPreference,
        rotateGesturesEnabled = map.rotateGesturesEnabled,
        scrollGesturesEnabled = map.scrollGesturesEnabled,
        tiltGesturesEnabled = map.tiltGesturesEnabled,
        trackCameraPosition = map.onCameraMove != null,
        zoomControlsEnabled = map.zoomControlsEnabled,
        zoomGesturesEnabled = map.zoomGesturesEnabled,
        liteModeEnabled = map.liteModeEnabled,
        myLocationEnabled = map.myLocationEnabled,
        myLocationButtonEnabled = map.myLocationButtonEnabled,
        padding = map.padding,
        indoorViewEnabled = map.indoorViewEnabled,
        trafficEnabled = map.trafficEnabled,
        buildingsEnabled = map.buildingsEnabled,
        assert(!map.liteModeEnabled || Platform.isAndroid);

  final bool compassEnabled;

  final bool mapToolbarEnabled;

  final CameraTargetBounds cameraTargetBounds;

  final MapType mapType;

  final MinMaxZoomPreference minMaxZoomPreference;

  final bool rotateGesturesEnabled;

  final bool scrollGesturesEnabled;

  final bool tiltGesturesEnabled;

  final bool trackCameraPosition;

  final bool zoomControlsEnabled;

  final bool zoomGesturesEnabled;

  final bool liteModeEnabled;

  final bool myLocationEnabled;

  final bool myLocationButtonEnabled;

  final EdgeInsets padding;

  final bool indoorViewEnabled;

  final bool trafficEnabled;

  final bool buildingsEnabled;

  Map<String, dynamic> toMap() {
    return <String, dynamic>{
      'compassEnabled': compassEnabled,
      'mapToolbarEnabled': mapToolbarEnabled,
      'cameraTargetBounds': cameraTargetBounds.toJson(),
      'mapType': mapType.index,
      'minMaxZoomPreference': minMaxZoomPreference.toJson(),
      'rotateGesturesEnabled': rotateGesturesEnabled,
      'scrollGesturesEnabled': scrollGesturesEnabled,
      'tiltGesturesEnabled': tiltGesturesEnabled,
      'zoomControlsEnabled': zoomControlsEnabled,
      'zoomGesturesEnabled': zoomGesturesEnabled,
      'liteModeEnabled': liteModeEnabled,
      'trackCameraPosition': trackCameraPosition,
      'myLocationEnabled': myLocationEnabled,
      'myLocationButtonEnabled': myLocationButtonEnabled,
      'padding': <double>[
        padding.top,
        padding.left,
        padding.bottom,
        padding.right,
      ],
      'indoorEnabled': indoorViewEnabled,
      'trafficEnabled': trafficEnabled,
      'buildingsEnabled': buildingsEnabled,
    };
  }

  Map<String, dynamic> updatesMap(_GoogleMapOptions newOptions) {
    final Map<String, dynamic> prevOptionsMap = toMap();

    return newOptions.toMap()
      ..removeWhere(
          (String key, dynamic value) => prevOptionsMap[key] == value);
  }
}
