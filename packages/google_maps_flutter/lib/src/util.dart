// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: avoid_setters_without_getters

import 'dart:async';
import 'package:google_maps_flutter_platform_interface/google_maps_flutter_platform_interface.dart';

import 'google_maps_js_bridge.dart';

/// Default LatLng.
const LatLng nullLatLng = LatLng(0, 0);

/// Default LatLngBounds.
final LatLngBounds nullLatLngBounds = LatLngBounds(
  southwest: nullLatLng,
  northeast: nullLatLng,
);

/// Default ScreenCoordinate.
const ScreenCoordinate nullScreenCoordinate = ScreenCoordinate(x: 0, y: 0);

/// This class defines marker's options.
class GMarkerOptions {
  /// GMarkerOptions Constructor.
  GMarkerOptions();

  /// The offset from the marker's position to the tip of an InfoWindow.
  GPoint? anchorPoint;

  /// If true, the marker can be dragged. Default value is false.
  bool? draggable;

  /// Icon for the foreground.
  Object? /*String?|Icon?|GSymbol?*/ icon;

  /// A number between 0.0, transparent, and 1.0, opaque.
  num? opacity;

  /// The marker position.
  LatLng? position;

  /// Text displayed in an info window.
  String? title;

  /// If true, the marker is visible.
  bool? visible;

  /// All markers are displayed on the map in order of their zIndex.
  num? zIndex;

  @override
  String toString() {
    return '{anchorPoint:$anchorPoint, draggable:$draggable, icon:$icon, map: map, '
        ' opacity:$opacity, position:new google.maps.LatLng(${position?.latitude}, ${position?.longitude}),'
        ' title:"$title", visible:$visible, zIndex:$zIndex}';
  }
}

/// This class represents a Marker icon image.
class GIcon {
  /// GIcon Constructor.
  GIcon();

  /// The URL of the image.
  String? url;

  /// The size of the entire image after scaling, if any.
  GSize? scaledSize;

  /// The display size of the sprite or image.
  GSize? size;

  @override
  String toString() {
    return '{url: "$url", scaledSize:$scaledSize, size: $size}';
  }
}

/// This class represents 'google.maps.Size'
class GSize {
  /// GSize Constructor.
  GSize(this.width, this.height);

  /// The width along the x-axis, in pixels.
  num? width;

  /// The height along the y-axis, in pixels.
  num? height;

  @override
  String toString() {
    return 'new google.maps.Size($width, $height)';
  }

  /// Returns width and height values.
  String toValue() {
    return '{width:$width, height:$height}';
  }
}

/// This class represents 'google.maps.Point'
class GPoint {
  /// GPoint Constructor.
  GPoint(this.x, this.y);

  /// The X coordinate.
  num? x;

  /// The Y coordinate.
  num? y;

  @override
  String toString() {
    return 'new google.maps.Point($x, $y)';
  }

  /// Returns x and y values.
  String toValue() {
    return '{x:$x, y:$y}';
  }
}

/// This class defines info window's options.
class GInfoWindowOptions {
  /// GInfoWindowOptions Constructor.
  GInfoWindowOptions();

  /// Content to display in the InfoWindow.
  String? content;

  /// The LatLng at which to display this InfoWindow.
  LatLng? position;

  /// All InfoWindows are displayed on the map in order of their zIndex.
  num? zIndex;

  @override
  String toString() {
    final String pos = position != null
        ? '{lat:${position?.latitude}, lng:${position?.longitude}}'
        : 'null';
    return '{content:$content, pixelOffset:null , position:$pos, zIndex:$zIndex}';
  }
}

/// This class represents GMarker's InfoWindow.
class GInfoWindow {
  /// GInfoWindow Constructor.
  GInfoWindow(GoogleMapsJsBridge bridge, GInfoWindowOptions? opts)
      : _bridge = bridge,
        _id = _gid++ {
    _createInfoWindow(opts);
  }

  final GoogleMapsJsBridge _bridge;

  Future<void> _createInfoWindow(GInfoWindowOptions? opts) async {
    await _bridge.createObject(
      toString(),
      'new google.maps.InfoWindow($opts)',
    );
  }

  final int _id;
  static int _gid = 0;

  /// Closes InfoWindow
  void close() {
    _callCloseInfoWindow();
  }

  Future<void> _callCloseInfoWindow() async {
    await _bridge.runJavaScript('${toString()}.close();');
  }

  /// Opens InfoWindow on the given map.
  void open([GMarker? anchor]) {
    _callOpenInfoWindow(anchor);
  }

  Future<void> _callOpenInfoWindow(GMarker? anchor) async {
    await _bridge.runJavaScript('${toString()}.open({anchor: $anchor, map});');
  }

  @override
  String toString() {
    return 'infoWindow$_id';
  }

  /// Sets the content to be displayed by InfoWindow.
  set content(Object? /*String?|Node?*/ content) => _setContent(content);

  /// Sets the offset of the tip of the info window from the point on the map.
  set pixelOffset(GSize? size) => _setPixelOffset(size);

  Future<void> _setContent(Object? /*String?|Node?*/ content) async {
    await _bridge.callMethod(JsRef(toString()), 'setContent', <Object?>[
      content,
    ]);
  }

  Future<void> _setPixelOffset(GSize? size) async {
    await _bridge.setProperty(
      JsRef(toString()),
      'pixelOffset',
      size?.toValue(),
    );
  }
}

/// This class represents a geographical location on the map as a Marker.
class GMarker {
  /// GMarker Constructor.
  GMarker(GoogleMapsJsBridge bridge, [GMarkerOptions? opts])
      : _bridge = bridge,
        id = _gid++,
        _options = opts {
    _createMarker(opts);
  }

  final GoogleMapsJsBridge _bridge;

  Future<void> _createMarker(GMarkerOptions? opts) async {
    final String command =
        'var ${toString()} = new google.maps.Marker($opts); ${toString()}.id = $id;';
    await _bridge.runJavaScript(command);
  }

  /// GMarker id.
  final int id;
  static int _gid = 0;
  GMarkerOptions? _options;

  /// Caches GMarker's options
  GMarkerOptions? get options => _options;

  @override
  String toString() {
    return 'marker$id';
  }

  /// Sets map.
  set map(Object? /*GMap?|StreetViewPanorama?*/ map) => _setMap(map);

  /// Sets if the Marker is visible.
  set visible(bool? visible) => _setVisible(visible);

  /// Sets marker options.
  set options(GMarkerOptions? options) {
    if (_options == null || options == null) {
      _options = options;
      _setOptions(options);
      return;
    }

    if (_options!.draggable != options.draggable) {
      _options!.draggable = options.draggable;
      _setDraggable(options.draggable);
    }
    if (_options!.icon != options.icon) {
      _options!.icon = options.icon;
      _setIcon(options.icon);
    }
    if (_options!.opacity != options.opacity) {
      _options!.opacity = options.opacity;
      _setOpacity(options.opacity);
    }
    if (_options!.position != options.position) {
      _options!.position = options.position;
      _setPosition(options.position);
    }
    if (_options!.title != options.title) {
      _options!.title = options.title;
      _setTitle(options.title);
    }
    if (_options!.visible != options.visible) {
      _options!.visible = options.visible;
      _setVisible(options.visible);
    }
    if (_options!.zIndex != options.zIndex) {
      _options!.zIndex = options.zIndex;
      _setZIndex(options.zIndex);
    }
  }

  Future<void> _setMap(Object? /*GMap?|StreetViewPanorama?*/ map) async {
    await _bridge.callMethod(JsRef(toString()), 'setMap', <Object?>[map]);
  }

  Future<void> _setOptions(GMarkerOptions? options) async {
    await _bridge.callMethod(JsRef(toString()), 'setOptions', <GMarkerOptions?>[
      options,
    ]);
  }

  Future<void> _setVisible(bool? visible) async {
    await _bridge.callMethod(JsRef(toString()), 'setVisible', <bool?>[
      visible,
    ]);
  }

  Future<void> _setDraggable(bool? visible) async {
    await _bridge.callMethod(JsRef(toString()), 'setDraggable', <bool?>[
      visible,
    ]);
  }

  Future<void> _setIcon(Object? icon) async {
    await _bridge.callMethod(JsRef(toString()), 'setIcon', <Object?>[icon]);
  }

  Future<void> _setOpacity(num? opacity) async {
    await _bridge.callMethod(JsRef(toString()), 'setOpacity', <num?>[
      opacity,
    ]);
  }

  Future<void> _setPosition(LatLng? position) async {
    await _bridge.callMethod(JsRef(toString()), 'setPosition', <Object?>[
      'new google.maps.LatLng(${position!.latitude},${position.longitude})',
    ]);
  }

  Future<void> _setTitle(String? title) async {
    await _bridge.callMethod(JsRef(toString()), 'setTitle', <String?>[title]);
  }

  Future<void> _setZIndex(num? zIndex) async {
    await _bridge.callMethod(JsRef(toString()), 'setZIndex', <num?>[zIndex]);
  }
}

/// This class represents a linear overlay of connected line segments on the
/// map.
class GPolyline {
  /// GPolyline Constructor.
  GPolyline(GoogleMapsJsBridge bridge, [GPolylineOptions? opts])
      : _bridge = bridge,
        id = _gid++ {
    _createPolyline(opts);
  }

  final GoogleMapsJsBridge _bridge;

  Future<void> _createPolyline(GPolylineOptions? opts) async {
    await _bridge.createObject(
      toString(),
      'new google.maps.Polyline($opts)',
    );
  }

  /// GPolyline id.
  final int id;
  static int _gid = 0;

  @override
  String toString() {
    return 'polyline$id';
  }

  /// Sets if the Polyline is visible.
  set visible(bool? visible) => _setVisible(visible);

  /// Sets map.
  set map(Object? /*GMap?|StreetViewPanorama?*/ map) => _setMap(map);

  /// Sets options.
  set options(GPolylineOptions? options) {
    _setOptions(options);
  }

  Future<void> _setVisible(bool? visible) async {
    await _bridge.callMethod(JsRef(toString()), 'setVisible', <bool?>[
      visible,
    ]);
  }

  Future<void> _setMap(Object? /*GMap?|StreetViewPanorama?*/ map) async {
    await _bridge.callMethod(JsRef(toString()), 'setMap', <Object?>[map]);
  }

  Future<void> _setOptions(GPolylineOptions? options) async {
    await _bridge.callMethod(
      JsRef(toString()),
      'setOptions',
      <GPolylineOptions?>[options],
    );
  }
}

/// This class defines polyline's options.
class GPolylineOptions {
  /// GPolylineOptions Constructor.
  GPolylineOptions();

  /// When true, the polyline are interpreted as geodesic and will follow the
  /// curvature of the Earth.
  bool? geodesic;

  /// The ordered sequence of coordinates of the Polyline.
  List<LatLng?>? path;

  /// The stroke color.
  String? strokeColor;

  /// The stroke opacity between 0.0 and 1.0.
  num? strokeOpacity;

  /// The stroke width in pixels.
  num? strokeWeight;

  /// Whether this polyline is visible on the map.
  bool? visible;

  /// The zIndex compared to other polys.
  num? zIndex;

  @override
  String toString() {
    final StringBuffer paths = StringBuffer();
    for (final LatLng? position in path!) {
      if (position != null) {
        paths.write(
          'new google.maps.LatLng(${position.latitude},${position.longitude}), ',
        );
      }
    }

    return '{geodesic:$geodesic, path:[$paths], strokeColor:"$strokeColor",'
        ' strokeOpacity:$strokeOpacity, map: map, strokeWeight:$strokeWeight, visible:$visible, zIndex:$zIndex}';
  }
}

/// This class represents a polygon (like a polyline) that defines a series of
/// connected coordinates in an ordered sequence.
class GPolygon {
  /// GPolygon Constructor.
  GPolygon(GoogleMapsJsBridge bridge, [GPolygonOptions? opts])
      : _bridge = bridge,
        id = _gid++ {
    _createPolygon(opts);
  }

  final GoogleMapsJsBridge _bridge;

  Future<void> _createPolygon(GPolygonOptions? opts) async {
    await _bridge.createObject(toString(), 'new google.maps.Polygon($opts)');
  }

  /// GPolygon id.
  final int id;
  static int _gid = 0;

  @override
  String toString() {
    return 'polygon$id';
  }

  /// Sets if the Polygon is visible.
  set visible(bool? visible) => _setVisible(visible);

  /// Sets map.
  set map(Object? /*GMap?|StreetViewPanorama?*/ map) => _setMap(map);

  /// Sets options.
  set options(GPolygonOptions? options) {
    _setOptions(options);
  }

  Future<void> _setVisible(bool? visible) async {
    await _bridge.callMethod(JsRef(toString()), 'setVisible', <bool?>[
      visible,
    ]);
  }

  Future<void> _setMap(Object? /*GMap?|StreetViewPanorama?*/ map) async {
    await _bridge.callMethod(JsRef(toString()), 'setMap', <Object?>[map]);
  }

  Future<void> _setOptions(GPolygonOptions? options) async {
    await _bridge.callMethod(
      JsRef(toString()),
      'setOptions',
      <GPolygonOptions?>[options],
    );
  }
}

/// This class defines polygon's options.
class GPolygonOptions {
  /// GPolygonOptions Constructor.
  GPolygonOptions();

  /// The fill color.
  String? fillColor;

  /// The fill opacity between 0.0 and 1.0
  num? fillOpacity;

  /// When true, edges of the polygon are interpreted as geodesic and will
  /// follow the curvature of the Earth.
  bool? geodesic;

  /// The ordered sequence of coordinates that designates a closed loop.
  List<List<LatLng?>?>? paths;

  /// The stroke color.
  String? strokeColor;

  /// The stroke opacity between 0.0 and 1.0.
  num? strokeOpacity;

  /// The stroke width in pixels.
  num? strokeWeight;

  /// Whether this polygon is visible on the map.
  bool? visible;

  /// The zIndex compared to other polys.
  num? zIndex;

  @override
  String toString() {
    final StringBuffer str = StringBuffer();
    for (final List<LatLng?>? latlng in paths!) {
      str.write('[');
      for (final LatLng? position in latlng!) {
        if (position != null) {
          str.write(
            'new google.maps.LatLng(${position.latitude},${position.longitude}), ',
          );
        }
      }
      str.write('], ');
    }

    return '{fillColor:"$fillColor", fillOpacity:$fillOpacity, geodesic:$geodesic, paths:[$str],'
        ' strokeColor:"$strokeColor", strokeOpacity:$strokeOpacity, map: map,'
        ' strokeWeight:$strokeWeight, visible:$visible, zIndex:$zIndex}';
  }
}

/// This class represents a circle using the passed GCircleOptions.
class GCircle {
  /// GCircle Constructor.
  GCircle(GoogleMapsJsBridge bridge, [GCircleOptions? opts])
      : _bridge = bridge,
        id = _gid++ {
    _createCircle(opts);
  }

  final GoogleMapsJsBridge _bridge;

  Future<void> _createCircle(GCircleOptions? opts) async {
    await _bridge.createObject(toString(), 'new google.maps.Circle($opts)');
  }

  /// GCircle id.
  final int id;
  static int _gid = 0;

  @override
  String toString() {
    return 'circle$id';
  }

  /// Sets if the circle is visible.
  set visible(bool? visible) => _setVisible(visible);

  /// Sets the radius of the circle.
  set radius(num? radius) => _setRadius(radius);

  /// Sets map.
  set map(Object? /*GMap?|StreetViewPanorama?*/ map) => _setMap(map);

  /// Sets options.
  set options(GCircleOptions? options) {
    _setOptions(options);
  }

  Future<void> _setVisible(bool? visible) async {
    await _bridge.callMethod(JsRef(toString()), 'setVisible', <bool?>[
      visible,
    ]);
  }

  Future<void> _setRadius(num? radius) async {
    await _bridge.callMethod(JsRef(toString()), 'setRadius', <num?>[radius]);
  }

  Future<void> _setMap(Object? /*GMap?|StreetViewPanorama?*/ map) async {
    await _bridge.callMethod(JsRef(toString()), 'setMap', <Object?>[map]);
  }

  Future<void> _setOptions(GCircleOptions? options) async {
    await _bridge.callMethod(
      JsRef(toString()),
      'setOptions',
      <GCircleOptions?>[options],
    );
  }
}

/// This class defines circle's options.
class GCircleOptions {
  /// GCircleOptions Constructor.
  GCircleOptions();

  /// The center of the Circle.
  LatLng? center;

  /// The fill color.
  String? fillColor;

  /// The fill opacity between 0.0 and 1.0.
  num? fillOpacity;

  /// The radius in meters on the Earth's surface.
  num? radius;

  /// The stroke color.
  String? strokeColor;

  /// The stroke opacity between 0.0 and 1.0.
  num? strokeOpacity;

  /// The stroke width in pixels.
  num? strokeWeight;

  /// Whether this circle is visible on the map.
  bool? visible;

  /// The zIndex compared to other polys.
  num? zIndex;

  @override
  String toString() {
    return '{center: new google.maps.LatLng(${center?.latitude}, ${center?.longitude}), fillColor:"$fillColor",'
        ' fillOpacity:$fillOpacity, radius:$radius, strokeColor:"$strokeColor", strokeOpacity:$strokeOpacity,'
        ' map: map, strokeWeight:$strokeWeight, visible:$visible, zIndex:$zIndex}';
  }
}

/// The [GMarkerClusterer] object used to cluster markers on the map.
class GMarkerClusterer {
  /// GMarkerCluster Constructor.
  GMarkerClusterer(GoogleMapsJsBridge bridge, [GMarkerClustererOptions? opts])
      : _bridge = bridge,
        id = _gid++ {
    _createMarkerClusterer(opts);
  }

  final GoogleMapsJsBridge _bridge;

  void _createMarkerClusterer(GMarkerClustererOptions? opts) {
    final String command =
        'var ${toString()} = new markerClusterer.MarkerClusterer($opts);';
    _bridge.runJavaScript(command);
  }

  /// GCircle id.
  final int id;
  static int _gid = 0;

  /// Adds a marker to be clustered by the [GMarkerClusterer].
  void addMarker(GMarker marker, bool? noDraw) {
    _bridge.runJavaScript('${toString()}.addMarker($marker, $noDraw);');
  }

  /// Removes a marker from the [GMarkerClusterer].
  Future<bool> removeMarker(GMarker marker, bool? noDraw) async {
    final bool result = await _bridge.runJavaScriptReturningResult(
      '${toString()}.removeMarker($marker, $noDraw);',
    ) as bool;

    return result;
  }

  /// Adds a list of markers to be clustered by the [GMarkerClusterer].
  void addMarkers(List<GMarker>? markers, bool? noDraw) {
    final String command =
        'JSON.stringify($this.addMarkers.call($this, $markers, $noDraw))';
    _bridge.runJavaScript(command);
  }

  /// Removes a list of markers from the [GMarkerClusterer].
  bool removeMarkers(List<GMarker>? markers, bool? noDraw) {
    final String command =
        'JSON.stringify($this.removeMarkers.call($this, $markers, $noDraw))';
    return _bridge.runJavaScriptReturningResult(command) as bool;
  }

  /// Clears all the markers from the [GMarkerClusterer].
  void clearMarkers(bool? noDraw) {
    _bridge.runJavaScript('${toString()}.clearMarkers($noDraw);');
  }

  /// Returns the list of clusters.
  List<Map<String, dynamic>> get clusters {
    final List<Map<String, dynamic>> results =
        _bridge.runJavaScriptReturningResult('${toString()}.clusters')
            as List<Map<String, dynamic>>;
    return results;
  }

  /// Called when the [GMarkerClusterer] is added to the map.
  void onAdd() {
    _bridge.runJavaScript('${toString()}.onAdd();');
  }

  /// Called when the [MarkerClusterer] is removed from the map.
  void onRemove() {
    _bridge.runJavaScript('${toString()}.onRemove();');
  }

  /// Recalculates and draws all the marker clusters.
  void render() {
    _bridge.runJavaScript('${toString()}.render();');
  }

  @override
  String toString() {
    return 'markerClusterer$id';
  }
}

/// This class defines MarkerClusterer's options.
class GMarkerClustererOptions {
  /// GMarkerClustererOptions Constructor.
  GMarkerClustererOptions({List<GMarker>? markers, String? onClusterClick}) {
    _markers = markers;
    if (onClusterClick != null) {
      _onClusterClick = onClusterClick;
    }
  }

  List<GMarker>? _markers;
  String? _onClusterClick;

  /// List of markers in the cluster.
  List<GMarker>? get markers => _markers;

  /// Returns the onClusterClick handler.
  String? get onClusterClick => _onClusterClick;

  @override
  String toString() {
    String options = '{map: map';
    if (markers != null) {
      options += ', markers: $markers';
    }
    if (onClusterClick != null) {
      options += ', onClusterClick: $onClusterClick';
    }
    options += '}';

    return options;
  }
}

/// This class represents an image overlay drawn on the map and oriented
/// against the Earth's surface.
class GGroundOverlay {
  /// GGroundOverlay Constructor.
  GGroundOverlay(GoogleMapsJsBridge bridge, [GGroundOverlayOptions? opts])
      : _bridge = bridge,
        id = _gid++,
        _options = opts {
    _createGroundOverlay(opts);
  }

  final GoogleMapsJsBridge _bridge;

  Future<void> _createGroundOverlay(GGroundOverlayOptions? opts) async {
    final String url = opts?.url ?? "''";
    final String bounds = opts?.bounds ?? '{}';
    final String command =
        'var ${toString()} = new google.maps.GroundOverlay($url, $bounds, $opts);'
        ' ${toString()}.id = $id;';
    await _bridge.runJavaScript(command);
  }

  /// GGroundOverlay id.
  final int id;
  static int _gid = 0;
  GGroundOverlayOptions? _options;

  /// Caches GGroundOverlay's options.
  GGroundOverlayOptions? get options => _options;

  @override
  String toString() {
    return 'groundOverlay$id';
  }

  /// Sets map.
  set map(Object? /*GMap?*/ map) => _setMap(map);

  /// Sets ground overlay options.
  ///
  /// The JavaScript Maps GroundOverlay only exposes `setMap` and `setOpacity`,
  /// so only [GGroundOverlayOptions.opacity] and [GGroundOverlayOptions.visible]
  /// are reflected here. Changes to `url`, `bounds`, or `clickable` require a
  /// new [GGroundOverlay] instance.
  set options(GGroundOverlayOptions? options) {
    if (_options == null || options == null) {
      _options = options;
      return;
    }
    if (_options!.opacity != options.opacity) {
      _options!.opacity = options.opacity;
      _setOpacity(options.opacity);
    }
    if (_options!.visible != options.visible) {
      _options!.visible = options.visible;
      _setMap(options.visible == false ? null : 'map');
    }
  }

  Future<void> _setMap(Object? map) async {
    await _bridge.callMethod(JsRef(toString()), 'setMap', <Object?>[map]);
  }

  Future<void> _setOpacity(num? opacity) async {
    await _bridge.callMethod(JsRef(toString()), 'setOpacity', <num?>[
      opacity,
    ]);
  }
}

/// This class defines GroundOverlay's options.
class GGroundOverlayOptions {
  /// GGroundOverlayOptions Constructor.
  GGroundOverlayOptions();

  /// The image URL passed to the JS GroundOverlay constructor, already quoted
  /// as a JS string literal (e.g. `'data:image/png;base64,...'`).
  String? url;

  /// The bounds passed to the JS GroundOverlay constructor as a JS object
  /// literal: `{south:.., west:.., north:.., east:..}`.
  String? bounds;

  /// Whether click events are handled.
  bool? clickable;

  /// Opacity (0.0 transparent through 1.0 opaque).
  num? opacity;

  /// Whether this ground overlay is visible on the map.
  bool? visible;

  @override
  String toString() {
    return '{clickable:$clickable, opacity:$opacity,'
        ' map: ${visible == false ? 'null' : 'map'}}';
  }
}
