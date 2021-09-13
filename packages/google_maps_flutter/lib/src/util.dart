// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//part of google_maps_flutter_tizen;
import 'dart:async';
import 'package:google_maps_flutter_platform_interface/google_maps_flutter_platform_interface.dart';
import 'package:webview_flutter/webview_flutter.dart';

/// Default LatLng.
const LatLng nullLatLng = LatLng(0, 0);

/// Default LatLngBounds.
final LatLngBounds nullLatLngBounds =
    LatLngBounds(southwest: nullLatLng, northeast: nullLatLng);

/// Default ScreenCoordinate.
const ScreenCoordinate nullScreenCoordinate = ScreenCoordinate(x: 0, y: 0);

/// This class defines marker's options.
class GMarkerOptions {
  /// GMarkerOptions Constructor.
  factory GMarkerOptions() {
    return _options;
  }
  GMarkerOptions._internal();
  static final GMarkerOptions _options = GMarkerOptions._internal();

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
  factory GInfoWindowOptions() {
    return _options;
  }
  GInfoWindowOptions._internal();
  static final GInfoWindowOptions _options = GInfoWindowOptions._internal();

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
    return '{content:$content, position:$pos, zIndex:$zIndex}';
  }
}

/// This class represents GMarker's InfoWindow.
class GInfoWindow {
  /// GInfoWindow Constructor.
  GInfoWindow(GInfoWindowOptions? opts) : _id = _gid++ {
    _createInfoWindow(opts);
  }

  Future<void> _createInfoWindow(GInfoWindowOptions? opts) async {
    await (await webController!).evaluateJavascript(
        'var ${toString()} = new google.maps.InfoWindow($opts);');
  }

  final int _id;
  static int _gid = 0;

  /// Closes InfoWindow
  void close() {
    _callCloseInfoWindow();
  }

  Future<void> _callCloseInfoWindow() async {
    await (await webController!).evaluateJavascript('${toString()}.close();');
  }

  /// Opens InfoWindow on the given map.
  void open([
    GMarker? anchor,
  ]) {
    _callOpenInfoWindow(anchor);
  }

  Future<void> _callOpenInfoWindow(GMarker? anchor) async {
    await (await webController!).evaluateJavascript(
        '${toString()}.open({anchor: ${anchor.toString()}, map});');
  }

  @override
  String toString() {
    return 'infoWindow$_id';
  }

  /// Sets the content to be displayed by InfoWindow.
  set content(Object? /*String?|Node?*/ content) => _setContent(content);

  /// Sets the offset of the tip of the info window from the point on the map.
  set pixelOffset(GSize? size) => _setPixelOffset(size);

  void _setContent(Object? /*String?|Node?*/ content) {
    callMethod(this, 'setContent', [content]);
  }

  void _setPixelOffset(GSize? size) {
    setProperty(this, 'pixelOffset', size?.toValue());
  }
}

/// This class represents a geographical location on the map as a Marker.
class GMarker {
  /// GMarker Constructor.
  GMarker([
    GMarkerOptions? opts,
  ])  : id = _gid++,
        _options = opts {
    _createMarker(opts);
  }

  Future<void> _createMarker(GMarkerOptions? opts) async {
    await (await webController!).evaluateJavascript(
        'var ${toString()} = new google.maps.Marker($opts);');
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
    _options = options;
    _setOptions(options);
  }

  Future<void> _setMap(Object? /*GMap?|StreetViewPanorama?*/ map) async {
    await callMethod(this, 'setMap', [map]);
  }

  Future<void> _setOptions(GMarkerOptions? options) async {
    await callMethod(this, 'setOptions', [options]);
  }

  Future<void> _setVisible(bool? visible) async {
    await callMethod(this, 'setVisible', [visible]);
  }
}

/// This class represents a linear overlay of connected line segments on the map.
class GPolyline {
  /// GPolyline Constructor.
  GPolyline([
    GPolylineOptions? opts,
  ]) : id = _gid++ {
    _createPolyline(opts);
  }

  Future<void> _createPolyline(GPolylineOptions? opts) async {
    await (await webController!).evaluateJavascript(
        'var ${toString()} = new google.maps.Polyline($opts);');
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
    await callMethod(this, 'setVisible', [visible]);
  }

  Future<void> _setMap(Object? /*GMap?|StreetViewPanorama?*/ map) async {
    await callMethod(this, 'setMap', [map]);
  }

  Future<void> _setOptions(GPolylineOptions? options) async {
    await callMethod(this, 'setOptions', [options]);
  }
}

/// This class defines polyline's options.
class GPolylineOptions {
  /// GPolylineOptions Constructor.
  factory GPolylineOptions() {
    return _options;
  }
  GPolylineOptions._internal();
  static final GPolylineOptions _options = GPolylineOptions._internal();

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
            'new google.maps.LatLng(${position.latitude},${position.longitude}), ');
      }
    }

    return '{geodesic:$geodesic, path:[${paths.toString()}], strokeColor:"$strokeColor",'
        ' strokeOpacity:$strokeOpacity, map: map, strokeWeight:$strokeWeight, visible:$visible, zIndex:$zIndex}';
  }
}

/// This class represents a polygon (like a polyline) that defines a series of
/// connected coordinates in an ordered sequence.
class GPolygon {
  /// GPolygon Constructor.
  GPolygon([
    GPolygonOptions? opts,
  ]) : id = _gid++ {
    _createPolygon(opts);
  }

  Future<void> _createPolygon(GPolygonOptions? opts) async {
    await (await webController!).evaluateJavascript(
        'var ${toString()} = new google.maps.Polygon($opts);');
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
    await callMethod(this, 'setVisible', [visible]);
  }

  Future<void> _setMap(Object? /*GMap?|StreetViewPanorama?*/ map) async {
    await callMethod(this, 'setMap', [map]);
  }

  Future<void> _setOptions(GPolygonOptions? options) async {
    await callMethod(this, 'setOptions', [options]);
  }
}

/// This class defines polygon's options.
class GPolygonOptions {
  /// GPolygonOptions Constructor.
  factory GPolygonOptions() {
    return _options;
  }
  GPolygonOptions._internal();
  static final GPolygonOptions _options = GPolygonOptions._internal();

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
              'new google.maps.LatLng(${position.latitude},${position.longitude}), ');
        }
      }
      str.write('], ');
    }

    return '{fillColor:"$fillColor", fillOpacity:$fillOpacity, geodesic:$geodesic, paths:[${str.toString()}],'
        ' strokeColor:"$strokeColor", strokeOpacity:$strokeOpacity, map: map,'
        ' strokeWeight:$strokeWeight, visible:$visible, zIndex:$zIndex}';
  }
}

/// This class represents a circle using the passed GCircleOptions.
class GCircle {
  /// GCircle Constructor.
  GCircle([
    GCircleOptions? opts,
  ]) : id = _gid++ {
    _createCircle(opts);
  }

  Future<void> _createCircle(GCircleOptions? opts) async {
    await (await webController!).evaluateJavascript(
        'var ${toString()} = new google.maps.Circle($opts);');
  }

  /// GCircle id.
  final int id;
  static int _gid = 0;

  @override
  String toString() {
    return 'polygon$id';
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
    await callMethod(this, 'setVisible', [visible]);
  }

  Future<void> _setRadius(num? radius) async {
    await callMethod(this, 'setRadius', [radius]);
  }

  Future<void> _setMap(Object? /*GMap?|StreetViewPanorama?*/ map) async {
    await callMethod(this, 'setMap', [map]);
  }

  Future<void> _setOptions(GCircleOptions? options) async {
    await callMethod(this, 'setOptions', [options]);
  }
}

/// This class defines circle's options.
class GCircleOptions {
  /// GCircleOptions Constructor.
  factory GCircleOptions() {
    return _options;
  }
  GCircleOptions._internal();
  static final GCircleOptions _options = GCircleOptions._internal();

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

/// Returns webview controller instance
Future<WebViewController>? webController;

/// Returns the property value of the object.
Future<String> getProperty(Object o, String property) async {
  assert(webController != null, 'mapController is null!!');
  final String command = 'JSON.stringify(${o.toString()}[\'$property\'])';
  return await (await webController!).evaluateJavascript(command);
}

/// Sets the value to property of the object.
Future<String> setProperty(Object o, String property, Object? value) async {
  assert(webController != null, 'mapController is null!!');
  final String command =
      'JSON.stringify(${o.toString()}[\'$property\'] = $value)';
  return await (await webController!).evaluateJavascript(command);
}

/// Calls the method of the object with the args.
Future<String> callMethod(Object o, String method, List<Object?> args) async {
  assert(webController != null, 'webController is null!!');
  final String command =
      'JSON.stringify(${o.toString()}.$method.apply(${o.toString()}, $args))';
  return await (await webController!).evaluateJavascript(command);
}
