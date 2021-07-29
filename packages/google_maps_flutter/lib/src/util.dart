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
  factory GMarkerOptions() => create();
  Point? anchorPoint;
  Animation? animation;
  bool? clickable;
  bool? crossOnDrag;
  String? cursor;
  bool? draggable;
  Object? /*String?|Icon?|GSymbol?*/ icon;
  Object? /*String?|MarkerLabel?*/ label;
  Object? /*GMap?|StreetViewPanorama?*/ map;
  num? opacity;
  bool? optimized;
  LatLng? position;
  GMarkerShape? shape;
  String? title;
  bool? visible;
  num? zIndex;
}

class GMarkerShape {
  factory GMarkerShape() => create();
  String? type;
}

extension GMarkerShape$Ext on GMarkerShape {
  List<num?>? get coords {
    final String value = getProperty(this, 'coords') as String;
    print('[LEESS] MarkerShape 1: $value');
    final List<String> list = value.split(',');
    print('[LEESS] MarkerShape 2: $list');
    final List<num> result = list.map(num.parse).toList();
    print('[LEESS] MarkerShape 3: $result');
    return result;
  }

  set coords(List<num?>? value) {
    setProperty(this, 'coords', value);
  }
}

class GIcon {
  factory GIcon() => create();

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
    num? width,
    num? height, [
    String? widthUnit, // ignore: unused_element
    String? heightUnit, // ignore: unused_element
  ]);

  num? width;
  num? height;
}

// 'google.maps.Animation'
enum Animation { bounce, drop }

class GMarkerLabel {
  factory GMarkerLabel() => create();

  String? text;
  String? className;
  String? color;
  String? fontFamily;
  String? fontSize;
  String? fontWeight;
}

T create<T>() => const Object() as T;

class GInfoWindowOptions {
  factory GInfoWindowOptions() {
    return _options;
  }
  GInfoWindowOptions._internal();
  static final GInfoWindowOptions _options = GInfoWindowOptions._internal();

  Object? /*String?|Node?*/ content;
  bool? disableAutoPan;
  num? maxWidth;
  num? minWidth;
  GSize? pixelOffset;
  LatLng? position;
  num? zIndex;
}

class GInfoWindow {
  GInfoWindow(GInfoWindowOptions? opts) {
    _id++;
    _createInfoWindow([opts]);
  }

  Future<void> _createInfoWindow(List<Object?> opts) async {
    final String command =
        'var ${toString()} = new google.maps.InfoWindow($opts)';
    await controller!.evaluateJavascript(command);
  }

  static int _id = 0;

  void close() {
    // TODO: It should be implemented
  }

  void open([
    Object? /*GMap?|StreetViewPanorama?*/ map,
    Object? anchor,
  ]) {
    // TODO: It should be implemented
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
  set content(Object? /*String?|Node?*/ content) => _setContent(content);
  set options(GInfoWindowOptions? options) => _setOptions(options);
  set position(LatLng? position) => _setPosition(position);
  set zIndex(num? zIndex) => _setZIndex(zIndex);

  Object? /*String?|Node?*/ _getContent() => callMethod(this, 'getContent', []);
  LatLng? _getPosition() {
    final String value = callMethod(this, 'getPosition', []) as String;
    return _convertToLatLng(value);
  }

  num? _getZIndex() {
    final String value = callMethod(this, 'getZIndex', []) as String;
    return num.parse(value);
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
}

class MapMouseEvent {
  factory MapMouseEvent() => create();
  Object? /*MouseEvent?|TouchEvent?|PointerEvent?|KeyboardEvent?|Object?*/ domEvent;
  LatLng? latLng;
  void stop() {
    // TODO: Implement here!
  }
}

class MapsEventListener {
  factory MapsEventListener() => create();
  void remove() {
    // TODO: Implement here!
  }
}

class Event {
  static MapsEventListener addListener(
      Object? instance, String? eventName, Function? handler) {
    // [javascript example]
    // google.maps.event.addListener(marker, 'click', function (event){
    //   popInfoWindow(latlng);
    // });

    // TODO: Implement here!!!!!
    // final String value = callMethod('google.maps.event', 'addListener', [
    //   instance,
    //   eventName,
    //   handler == null ? null : allowInterop(handler)
    // ]) as String;

    print('[LEESS] addListener: $instance, $eventName, $handler');
    return MapsEventListener();
  }
}

// 'google.maps.Marker'
class GMarker {
  GMarker([
    GMarkerOptions? opts, // ignore: unused_element
  ]);
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
  set options(GMarkerOptions? options) => _setOptions(options);
  set position(LatLng? position) => _setPosition(position);
  set shape(GMarkerShape? shape) => _setShape(shape);
  set title(String? title) => _setTitle(title);
  set visible(bool? visible) => _setVisible(visible);
  set zIndex(num? zIndex) => _setZIndex(zIndex);

  Stream<MapMouseEvent> get onClick {
    late StreamController<MapMouseEvent> sc; // ignore: close_sinks
    late MapsEventListener mapsEventListener;
    void start() => mapsEventListener = Event.addListener(
          this,
          'click',
          (MapMouseEvent event) => sc.add(event),
        );
    void stop() => mapsEventListener.remove();
    sc = StreamController<MapMouseEvent>(
      onListen: start,
      onCancel: stop,
      onResume: start,
      onPause: stop,
    );
    return sc.stream;
  }

  Stream<MapMouseEvent> get onDragend {
    late StreamController<MapMouseEvent> sc; // ignore: close_sinks
    late MapsEventListener mapsEventListener;
    void start() => mapsEventListener = Event.addListener(
          this,
          'dragend',
          (MapMouseEvent event) => sc.add(event),
        );
    void stop() => mapsEventListener.remove();
    sc = StreamController<MapMouseEvent>(
      onListen: start,
      onCancel: stop,
      onResume: start,
      onPause: stop,
    );
    return sc.stream;
  }

  Animation? _getAnimation() {
    final String value = callMethod(this, 'getAnimation', []) as String;
    final Animation a =
        Animation.values.firstWhere((e) => e.toString() == value);
    print('[LEESS] _getAnimation: $a');
    return a;
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

  LatLng? _getPosition() {
    final String value = callMethod(this, 'getPosition', []) as String;
    final LatLng result = _convertToLatLng(value);
    print('[LEESS] _getPosition: $result');
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

  void _setAnimation(Animation? animation) {
    final String value =
        callMethod(this, 'setAnimation', [animation]) as String;
    print('[LEESS] _getShape: $value');
  }

  void _setClickable(bool? flag) {
    final String value = callMethod(this, 'setClickable', [flag]) as String;
    print('[LEESS] _setClickable: $value');
  }

  void _setCursor(String? cursor) {
    final String value = callMethod(this, 'setCursor', [cursor]) as String;
    print('[LEESS] _setCursor: $value');
  }

  void _setDraggable(bool? flag) {
    final String value = callMethod(this, 'setDraggable', [flag]) as String;
    print('[LEESS] _setDraggable: $value');
  }

  void _setIcon(Object? /*String?|Icon?|GSymbol?*/ icon) {
    final String value = callMethod(this, 'setIcon', [icon]) as String;
    print('[LEESS] _setIcon: $value');
  }

  void _setLabel(Object? /*String?|MarkerLabel?*/ label) {
    final String value = callMethod(this, 'setLabel', [label]) as String;
    print('[LEESS] _setLabel: $value');
  }

  void _setMap(Object? /*GMap?|StreetViewPanorama?*/ map) {
    final String value = callMethod(this, 'setMap', [map]) as String;
    print('[LEESS] _setMap: $value');
  }

  void _setOpacity(num? opacity) {
    final String value = callMethod(this, 'setOpacity', [opacity]) as String;
    print('[LEESS] _setOpacity: $value');
  }

  void _setOptions(GMarkerOptions? options) {
    final String value = callMethod(this, 'setOptions', [options]) as String;
    print('[LEESS] _setOptions: $value');
  }

  void _setPosition(LatLng? latlng) {
    final String value = callMethod(this, 'setPosition', [latlng]) as String;
    print('[LEESS] _setPosition: $value');
  }

  void _setShape(GMarkerShape? shape) {
    final String value = callMethod(this, 'setShape', [shape]) as String;
    print('[LEESS] _setShape: $value');
  }

  void _setTitle(String? title) {
    final String value = callMethod(this, 'setTitle', [title]) as String;
    print('[LEESS] _setTitle: $value');
  }

  void _setVisible(bool? visible) {
    final String value = callMethod(this, 'setVisible', [visible]) as String;
    print('[LEESS] _setVisible: $value');
  }

  void _setZIndex(num? zIndex) {
    final String value = callMethod(this, 'setZIndex', [zIndex]) as String;
    print('[LEESS] _setZIndex: $value');
  }
}

WebViewController? controller;

Future<String> getProperty(Object o, String method) async {
  assert(controller != null, 'mapController is null!!');

  // toString() or o.toString()??
  final String command = 'JSON.stringify(${o.toString()}[$method])';
  print('MarkerShape getProperty: $command');
  final String result = await controller!.evaluateJavascript(command);
  return result;
}

Future<String> setProperty(Object o, String method, Object? value) async {
  assert(controller != null, 'mapController is null!!');
  print('MarkerShape setProperty: $method($value)');

  // toString() or o.toString()??
  final String command = 'JSON.stringify(${o.toString()}[$method] = $value)';
  print('MarkerShape setProperty: $command');
  final String result = await controller!.evaluateJavascript(command);
  return result;
}

Future<String> callMethod(Object o, String method, List<Object?> args) async {
  assert(controller != null, 'mapController is null!!');
  print('InfoWindow callMethod: $method($args)');

  // toString() or o.toString()??
  final String command =
      'JSON.stringify(${o.toString()}.$method.apply(${o.toString()}, $args))';
  final String result = await controller!.evaluateJavascript(command);
  return result;
}
