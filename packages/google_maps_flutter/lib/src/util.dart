// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//part of google_maps_flutter_tizen;
import 'dart:async';
import 'dart:convert';
import 'dart:math';
import 'package:google_maps_flutter_platform_interface/google_maps_flutter_platform_interface.dart';
import 'package:webview_flutter/webview_flutter.dart';

const LatLng nullLatLng = LatLng(0, 0);
final LatLngBounds nullLatLngBounds =
    LatLngBounds(southwest: nullLatLng, northeast: nullLatLng);

LatLng _convertToLatLng(String value) {
  try {
    final dynamic latlng = json.decode(value);
    if (latlng is Map<String, dynamic>) {
      assert(latlng['lat'] is num && latlng['lng'] is num);
      return LatLng((latlng['lat'] as num) + 0.0, (latlng['lng'] as num) + 0.0);
    }
  } catch (e) {
    print('Javascript Error: $e');
  }
  return nullLatLng;
}

class GMarkerOptions {
  GMarkerOptions();
  GPoint? anchorPoint;
  Animation? animation;
  bool? clickable;
  bool? crossOnDrag;
  String? cursor;
  bool? draggable;
  Object? /*String?|Icon?|GSymbol?*/ icon;
  Object? /*String?|MarkerLabel?*/ label;
  num? opacity;
  bool? optimized;
  LatLng? position;
  GMarkerShape? shape;
  String? title;
  bool? visible;
  num? zIndex;

  @override
  String toString() {
    return '{anchorPoint:$anchorPoint, animation:$animation, clickable:$clickable, crossOnDrag:$crossOnDrag, cursor:$cursor, draggable:$draggable, icon:$icon, label:$label, map, opacity:$opacity, optimized:$optimized, position:{lat:${position?.latitude}, lng:${position?.longitude}},shape: $shape, title:"$title", visible:$visible, zIndex:$zIndex}';
  }
}

class GMarkerShape {
  GMarkerShape();
  String? type;
  @override
  String toString() {
    return type ?? '';
  }
}

class GIcon {
  GIcon();

  String? url;
  GPoint? anchor;
  GPoint? labelOrigin;
  GPoint? origin;
  GSize? scaledSize;
  GSize? size;

  @override
  String toString() {
    return '{url: "$url", anchor: $anchor, labelOrigin:$labelOrigin, origin: $origin, scaledSize:$scaledSize, size: $size}';
  }
}

// 'google.maps.Size'
class GSize {
  GSize(
    num? this.width,
    num? this.height, [
    String? widthUnit, // ignore: unused_element
    String? heightUnit, // ignore: unused_element
  ]);

  num? width;
  num? height;

  @override
  String toString() {
    return 'new google.maps.Size($width, $height)';
  }

  String toValue() {
    return '{width:$width, height:$height}';
  }
}

// 'google.maps.Point'
class GPoint {
  GPoint(num? this.x, num? this.y);

  num? x;
  num? y;

  @override
  String toString() {
    return 'new google.maps.Point($x, $y)';
  }

  String toValue() {
    return '{x:$x, y:$y}';
  }
}

// 'google.maps.Animation'
enum Animation { bounce, drop }

class GMarkerLabel {
  GMarkerLabel();

  String? text;
  String? className;
  String? color;
  String? fontFamily;
  String? fontSize;
  String? fontWeight;
}

class GInfoWindowOptions {
  factory GInfoWindowOptions() {
    return _options;
  }
  GInfoWindowOptions._internal();
  static final GInfoWindowOptions _options = GInfoWindowOptions._internal();

  String? content;
  bool? disableAutoPan;
  num? maxWidth;
  num? minWidth;
  GSize? pixelOffset;
  LatLng? position;
  num? zIndex;

  @override
  String toString() {
    final String pos = position != null
        ? '{lat:${position?.latitude}, lng:${position?.longitude}}'
        : 'null';
    return '{content:$content, disableAutoPan:$disableAutoPan, maxWidth:$maxWidth, minWidth:$minWidth, pixelOffset:$pixelOffset, position:$pos, zIndex:$zIndex}';
  }
}

class GInfoWindow {
  GInfoWindow(GInfoWindowOptions? opts) : _id = _gid++ {
    _createInfoWindow(opts);
  }

  Future<void> _createInfoWindow(GInfoWindowOptions? opts) async {
    final String command =
        'var ${toString()} = new google.maps.InfoWindow($opts);';
    await (await webController!).evaluateJavascript(command);
  }

  final int _id;
  static int _gid = 0;

  void close() {
    _callCloseInfoWindow();
  }

  Future<void> _callCloseInfoWindow() async {
    await (await webController!).evaluateJavascript('${toString()}.close();');
  }

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
}

extension GInfoWindow$Ext on GInfoWindow {
  set content(Object? /*String?|Node?*/ content) => _setContent(content);
  set options(GInfoWindowOptions? options) => _setOptions(options);
  set position(LatLng? position) => _setPosition(position);
  set zIndex(num? zIndex) => _setZIndex(zIndex);
  set pixelOffset(GSize? size) => _setPixelOffset(size);

  void _setContent(Object? /*String?|Node?*/ content) {
    callMethod(this, 'setContent', [content]);
  }

  void _setOptions(GInfoWindowOptions? options) {
    callMethod(this, 'setOptions', [options]);
  }

  void _setPosition(LatLng? position) {
    callMethod(this, 'setPosition', [position]);
  }

  void _setZIndex(num? zIndex) {
    callMethod(this, 'setZIndex', [zIndex]);
  }

  void _setPixelOffset(GSize? size) {
    setProperty(this, 'pixelOffset', size?.toValue());
  }
}

// 'google.maps.Marker'
class GMarker {
  GMarker([
    GMarkerOptions? opts, // ignore: unused_element
  ]) : id = _gid++ {
    _createMarker(opts);
  }

  Future<void> _createMarker(GMarkerOptions? opts) async {
    final String command = 'var ${toString()} = new google.maps.Marker($opts);';
    await (await webController!).evaluateJavascript(command);
  }

  final int id;
  static int _gid = 0;

  GMarkerOptions? opts;

  @override
  String toString() {
    return 'marker$id';
  }
}

extension GMarker$Ext on GMarker {
  set animation(Animation? animation) => _setAnimation(animation);
  set clickable(bool? clickable) => _setClickable(clickable);
  set cursor(String? cursor) => _setCursor(cursor);
  set draggable(bool? draggable) => _setDraggable(draggable);
  set icon(Object? /*String?|Icon?|GSymbol?*/ icon) => _setIcon(icon);
  set label(Object? /*String?|MarkerLabel?*/ label) => _setLabel(label);
  set map(Object? /*GMap?|StreetViewPanorama?*/ map) => _setMap(map);
  set opacity(num? opacity) => _setOpacity(opacity);
  set options(GMarkerOptions? options) {
    opts = options;
    _setOptions(options);
  }

  set position(LatLng? position) => _setPosition(position);
  set shape(GMarkerShape? shape) => _setShape(shape);
  set title(String? title) => _setTitle(title);
  set visible(bool? visible) => _setVisible(visible);
  set zIndex(num? zIndex) => _setZIndex(zIndex);

  Future<void> _setAnimation(Animation? animation) async {
    await callMethod(this, 'setAnimation', [animation]);
  }

  Future<void> _setClickable(bool? flag) async {
    await callMethod(this, 'setClickable', [flag]);
  }

  Future<void> _setCursor(String? cursor) async {
    await callMethod(this, 'setCursor', [cursor]);
  }

  Future<void> _setDraggable(bool? flag) async {
    await callMethod(this, 'setDraggable', [flag]);
  }

  Future<void> _setIcon(Object? /*String?|Icon?|GSymbol?*/ icon) async {
    await callMethod(this, 'setIcon', [icon]);
  }

  Future<void> _setLabel(Object? /*String?|MarkerLabel?*/ label) async {
    await callMethod(this, 'setLabel', [label]);
  }

  Future<void> _setMap(Object? /*GMap?|StreetViewPanorama?*/ map) async {
    await callMethod(this, 'setMap', [map]);
  }

  Future<void> _setOpacity(num? opacity) async {
    await callMethod(this, 'setOpacity', [opacity]);
  }

  Future<void> _setOptions(GMarkerOptions? options) async {
    await callMethod(this, 'setOptions', [options]);
  }

  Future<void> _setPosition(LatLng? latlng) async {
    await callMethod(this, 'setPosition', [latlng]);
  }

  Future<void> _setShape(GMarkerShape? shape) async {
    await callMethod(this, 'setShape', [shape]);
  }

  Future<void> _setTitle(String? title) async {
    await callMethod(this, 'setTitle', [title]);
  }

  Future<void> _setVisible(bool? visible) async {
    await callMethod(this, 'setVisible', [visible]);
  }

  Future<void> _setZIndex(num? zIndex) async {
    await callMethod(this, 'setZIndex', [zIndex]);
  }
}

WebView? webview;
Future<WebViewController>? webController;

Future<String> getProperty(Object o, String method) async {
  assert(webController != null, 'mapController is null!!');

  final String command = 'JSON.stringify(${o.toString()}[\'$method\'])';
  print('${o.toString()}.getProperty: $command');
  return await (await webController!).evaluateJavascript(command);
}

Future<String> setProperty(Object o, String method, Object? value) async {
  assert(webController != null, 'mapController is null!!');

  final String command =
      'JSON.stringify(${o.toString()}[\'$method\'] = $value)';
  print('${o.toString()}.setProperty: $command');
  return await (await webController!).evaluateJavascript(command);
}

Future<String> callMethod(Object o, String method, List<Object?> args) async {
  assert(webController != null, 'webController is null!!');

  final String command =
      'JSON.stringify(${o.toString()}.$method.apply(${o.toString()}, $args))';
  print('[${o.toString()}][$method($args)] : $command');
  return await (await webController!).evaluateJavascript(command);
}
