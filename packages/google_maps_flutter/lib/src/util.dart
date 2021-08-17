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
  Point? anchorPoint;
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

extension GMarkerShape$Ext on GMarkerShape {
  List<num?>? get coords {
    final String value = getProperty(this, 'coords') as String;
    final List<String> list = value.split(',');
    final List<num> result = list.map(num.parse).toList();
    return result;
  }

  set coords(List<num?>? value) {
    setProperty(this, 'coords', value);
  }
}

class GIcon {
  GIcon();

  String? url;
  Point? anchor;
  Point? labelOrigin;
  Point? origin;
  GSize? scaledSize;
  GSize? size;
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
    await webController!.evaluateJavascript(command);
  }

  final int _id;
  static int _gid = 0;

  void close() {
    _callCloseInfoWindow();
  }

  Future<void> _callCloseInfoWindow() async {
    await webController!.evaluateJavascript('${toString()}.close();');
  }

  void open([
    GMarker? anchor,
  ]) {
    _callOpenInfoWindow(anchor);
  }

  Future<void> _callOpenInfoWindow(GMarker? anchor) async {
    await webController!.evaluateJavascript(
        '${toString()}.open({anchor: ${anchor.toString()}, map});');
  }

  @override
  String toString() {
    return 'infoWindow$_id';
  }
}

extension GInfoWindow$Ext on GInfoWindow {
  Object? /*String?|Node?*/ get content => _getContent();
  LatLng? get position => _getPosition();
  num? get zIndex => _getZIndex();
  GSize? get pixelOffset => _getPixelOffset();
  set content(Object? /*String?|Node?*/ content) => _setContent(content);
  set options(GInfoWindowOptions? options) => _setOptions(options);
  set position(LatLng? position) => _setPosition(position);
  set zIndex(num? zIndex) => _setZIndex(zIndex);
  set pixelOffset(GSize? size) => _setPixelOffset(size);

  Object? /*String?|Node?*/ _getContent() => callMethod(this, 'getContent', []);

  // LatLng? _getPosition() {
  //   print('[LEESS] 2._getPosition');
  //   final String value = callMethod(this, 'getPosition', []) as String;
  //   // return _convertToLatLng(value);
  //   final LatLng result = _convertToLatLng(value);
  //   print('[LEESS] 2._getPosition: $result');
  //   return result;
  // }

  LatLng? _getPosition() {
    print('[LEESS] 2._getPosition');
    final String value = callMethod(this, 'getPosition', []) as String;
    final LatLng result = _convertToLatLng(value);
    print('[LEESS] 2._getPosition: $result');
    return result;
  }

  num? _getZIndex() {
    final String value = callMethod(this, 'getZIndex', []) as String;
    return num.parse(value);
  }

  GSize? _getPixelOffset() {
    return null;
  }

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

// class GMap {
//   @override
//   String toString() {
//     return 'map';
//   }
// }

// 'google.maps.Marker'
class GMarker {
  GMarker([
    GMarkerOptions? opts, // ignore: unused_element
  ]) : id = _gid++ {
    _createMarker(opts);
  }

  Future<void> _createMarker(GMarkerOptions? opts) async {
    final String command = 'var ${toString()} = new google.maps.Marker($opts);';
    await webController!.evaluateJavascript(command);
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
  Animation? get animation => _getAnimation();
  bool? get clickable => _getClickable();
  String? get cursor => _getCursor();
  bool? get draggable => _getDraggable();
  Object? /*String?|Icon?|GSymbol?*/ get icon => _getIcon();
  GMarkerLabel? get label => _getLabel();
  Object? /*GMap?|StreetViewPanorama?*/ get map => _getMap();
  num? get opacity => _getOpacity();
  LatLng? get position => _getPosition();
  GMarkerShape? get shape => _getShape();
  String? get title => _getTitle();
  bool? get visible => _getVisible();
  num? get zIndex => _getZIndex();
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

  Animation? _getAnimation() {
    final String value = callMethod(this, 'getAnimation', []) as String;
    final Animation animation =
        Animation.values.firstWhere((Animation e) => e.toString() == value);
    print('[LEESS] _getAnimation: $animation');
    return animation;
  }

  bool? _getClickable() {
    final String value = callMethod(this, 'getClickable', []) as String;
    bool b = value.toLowerCase() == 'true';
    print('[LEESS] _getClickable: $b');
    return b;
  }

  String? _getCursor() {
    final String value = callMethod(this, 'getCursor', []) as String;
    print('[LEESS] _getCursor: $value');
    return value;
  }

  bool? _getDraggable() {
    final String value = callMethod(this, 'getDraggable', []) as String;
    final bool b = value.toLowerCase() == 'true';
    print('[LEESS] _getDraggable: $b');
    return b;
  }

  Object? /*String?|Icon?|GSymbol?*/ _getIcon() {
    final String value = callMethod(this, 'getIcon', []) as String;
    print('[LEESS] _getIcon: $value');
    return value;
  }

  GMarkerLabel? _getLabel() {
    final String value = callMethod(this, 'getLabel', []) as String;
    print('[LEESS] _getLabel: $value');
    return GMarkerLabel();
  }

  Object? /*GMap?|StreetViewPanorama?*/ _getMap() {
    final String value = callMethod(this, 'getMap', []) as String;
    print('[LEESS] _getMap: $value');
    return Object();
  }

  num? _getOpacity() {
    final String value = callMethod(this, 'getOpacity', []) as String;
    final num result = num.parse(value);
    print('[LEESS] _getOpacity: $result');
    return result;
  }

  // LatLng? _getPosition() {
  //   print('[LEESS] 1._getPosition');
  //   final Future<String> value = callMethod(this, 'getPosition', []);
  //   LatLng result = nullLatLng;
  //   value.then((String value) {
  //     result = _convertToLatLng(value);
  //   });
  //   print('[LEESS] 1._getPosition: $value');
  //   return result;
  // }

  LatLng? _getPosition() {
    print('[LEESS] 1._getPosition');
    LatLng result = nullLatLng;
    final Future<LatLng> value = _callMethod();
    value.then((LatLng value) => result = value);
    // final LatLng result = _convertToLatLng2(value) as LatLng;
    // print('[LEESS] 1._getPosition: $value');
    return result;
  }

  Future<LatLng> _callMethod() async {
    final String value = await _callMethod2(this, 'getPosition', []);
    final LatLng result = _convertToLatLng(value);
    return result;
  }

  Future<String> _callMethod2(
      Object o, String method, List<Object?> args) async {
    assert(webController != null, 'webController is null!!');
    final String command =
        'JSON.stringify(${o.toString()}.$method.apply(${o.toString()}, $args))';
    print('[LEESS] 3.InfoWindow [$method($args)] : $command');
    final String result = await webController!.evaluateJavascript(command);
    print('[LEESS] 4.InfoWindow [$method] result: $result');
    return result;
  }

  GMarkerShape? _getShape() {
    final String value = callMethod(this, 'getShape', []) as String;
    print('[LEESS] _getShape: $value');
    return Object() as GMarkerShape;
  }

  String? _getTitle() {
    final String value = callMethod(this, 'getTitle', []) as String;
    print('[LEESS] _getTitle: $value');
    return value;
  }

  bool? _getVisible() {
    final String value = callMethod(this, 'getVisible', []) as String;
    final bool b = value.toLowerCase() == 'true';
    print('[LEESS] _getVisible: $b');
    return b;
  }

  num? _getZIndex() {
    final String value = callMethod(this, 'getZIndex', []) as String;
    final num result = num.parse(value);
    print('[LEESS] _getZIndex: $result');
    return result;
  }

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
WebViewController? webController;

Future<String> getProperty(Object o, String method) async {
  assert(webController != null, 'mapController is null!!');

  final String command = 'JSON.stringify(${o.toString()}[\'$method\'])';
  print('${o.toString()}.getProperty: $command');
  return await webController!.evaluateJavascript(command);
}

Future<String> setProperty(Object o, String method, Object? value) async {
  assert(webController != null, 'mapController is null!!');

  final String command =
      'JSON.stringify(${o.toString()}[\'$method\'] = $value)';
  print('${o.toString()}.setProperty: $command');
  return await webController!.evaluateJavascript(command);
}

Future<String> callMethod(Object o, String method, List<Object?> args) async {
  assert(webController != null, 'webController is null!!');

  final String command =
      'JSON.stringify(${o.toString()}.$method.apply(${o.toString()}, $args))';
  print('[${o.toString()}][$method($args)] : $command');
  return await webController!.evaluateJavascript(command);
}
