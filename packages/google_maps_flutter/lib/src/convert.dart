// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: avoid_dynamic_calls

part of '../google_maps_flutter_tizen.dart';

// Indices in the plugin side don't match with the ones
Map<int, String> _mapTypeToMapTypeId = <int, String>{
  0: 'roadmap', // None
  1: 'roadmap',
  2: 'satellite',
  3: 'terrain',
  4: 'hybrid',
};

// Builds the raw map options map from a [MapConfiguration].
//
// This mirrors the platform interface's `jsonForMapConfiguration` helper
// (which is not exported), plus [MapConfiguration.colorScheme].
//
// Intentionally omitted:
// * `cloudMapId` — deprecated alias for `mapId`, which is already serialized.
// * `markerType` — only relevant to advanced markers, which are not supported
//   on Tizen.
Map<String, dynamic> _mapOptionsFromConfiguration(MapConfiguration config) {
  final EdgeInsets? padding = config.padding;
  return <String, dynamic>{
    if (config.compassEnabled != null) 'compassEnabled': config.compassEnabled,
    if (config.mapToolbarEnabled != null)
      'mapToolbarEnabled': config.mapToolbarEnabled,
    if (config.cameraTargetBounds != null)
      'cameraTargetBounds': config.cameraTargetBounds!.toJson(),
    if (config.mapType != null) 'mapType': config.mapType!.index,
    if (config.minMaxZoomPreference != null)
      'minMaxZoomPreference': config.minMaxZoomPreference!.toJson(),
    if (config.rotateGesturesEnabled != null)
      'rotateGesturesEnabled': config.rotateGesturesEnabled,
    if (config.scrollGesturesEnabled != null)
      'scrollGesturesEnabled': config.scrollGesturesEnabled,
    if (config.tiltGesturesEnabled != null)
      'tiltGesturesEnabled': config.tiltGesturesEnabled,
    if (config.zoomControlsEnabled != null)
      'zoomControlsEnabled': config.zoomControlsEnabled,
    if (config.zoomGesturesEnabled != null)
      'zoomGesturesEnabled': config.zoomGesturesEnabled,
    if (config.liteModeEnabled != null)
      'liteModeEnabled': config.liteModeEnabled,
    if (config.trackCameraPosition != null)
      'trackCameraPosition': config.trackCameraPosition,
    if (config.myLocationEnabled != null)
      'myLocationEnabled': config.myLocationEnabled,
    if (config.myLocationButtonEnabled != null)
      'myLocationButtonEnabled': config.myLocationButtonEnabled,
    if (padding != null)
      'padding': <double>[
        padding.top,
        padding.left,
        padding.bottom,
        padding.right,
      ],
    if (config.indoorViewEnabled != null)
      'indoorEnabled': config.indoorViewEnabled,
    if (config.trafficEnabled != null) 'trafficEnabled': config.trafficEnabled,
    if (config.buildingsEnabled != null)
      'buildingsEnabled': config.buildingsEnabled,
    if (config.mapId != null) 'mapId': config.mapId,
    if (config.style != null) 'style': config.style,
    if (config.colorScheme != null) 'colorScheme': config.colorScheme!.index,
  };
}

String? _getCameraBounds(dynamic option) {
  if (option is! List<Object?> || option.first == null) {
    return null;
  }

  final List<Object> bound = option[0]! as List<Object>;
  final LatLng? southwest = LatLng.fromJson(bound[0]);
  final LatLng? northeast = LatLng.fromJson(bound[1]);

  final String restrictedBound =
      '{south:${southwest?.latitude}, west:${southwest?.longitude}, north:${northeast?.latitude}, east:${northeast?.longitude}}';

  return restrictedBound;
}

// Converts options into String that can be used by a webview.
// The following options are not handled here due to various reasons:
// The following are not available in web, because the map doesn't rotate there:
//   compassEnabled
//   rotateGesturesEnabled
//   tiltGesturesEnabled
// mapToolbarEnabled is unused in web, there's no "map toolbar"
// myLocationButtonEnabled Widget not available in web yet, it needs to be built
// on top of the maps widget
//   See:
//   https://developers.google.com/maps/documentation/javascript/examples/control-custom
// myLocationEnabled needs to be built through dart:html navigator.geolocation
//   See: https://api.dart.dev/stable/2.8.4/dart-html/Geolocation-class.html
// trackCameraPosition is just a boolan value that indicates if the map has an
// onCameraMove handler. indoorViewEnabled seems to not have an equivalent in
// web buildingsEnabled seems to not have an equivalent in web padding seems to
// behave differently in web than mobile. You can't move UI elements in web.
String _rawOptionsToString(Map<String, dynamic> rawOptions) {
  // These don't have any rawOptions entry, but they seem to be off in the
  // native maps.
  String options =
      'mapTypeControl: false, fullscreenControl: false, streetViewControl: false';

  if (_mapTypeToMapTypeId.containsKey(rawOptions['mapType'])) {
    options += ", mapTypeId: '${_mapTypeToMapTypeId[rawOptions['mapType']]}'";
  }

  if (rawOptions['minMaxZoomPreference'] != null) {
    options += ', minZoom: ${rawOptions['minMaxZoomPreference'][0]}';
    options += ', maxZoom: ${rawOptions['minMaxZoomPreference'][1]}';
  }

  if (rawOptions['cameraTargetBounds'] != null) {
    final String? bound = _getCameraBounds(rawOptions['cameraTargetBounds']);
    if (bound != null) {
      options += ', restriction: { latLngBounds: $bound, strictBounds: false }';
    } else {
      options += ', restriction: null';
    }
  }

  if (rawOptions['zoomControlsEnabled'] != null) {
    options += ', zoomControl: ${rawOptions['zoomControlsEnabled']}';
  }

  if (rawOptions['style'] != null &&
      (rawOptions['style'] as String).isNotEmpty) {
    options += ', styles: ${rawOptions['style']}';
  } else {
    options += ', styles: null';
  }

  if (rawOptions['scrollGesturesEnabled'] == false ||
      rawOptions['zoomGesturesEnabled'] == false) {
    options += ", gestureHandling: 'none'";
  } else {
    options += ", gestureHandling: 'auto'";
  }

  final String? colorScheme = _colorSchemeToJs(rawOptions['colorScheme']);
  if (colorScheme != null) {
    options += ', colorScheme: $colorScheme';
  }

  return options;
}

// Maps a serialized MapColorScheme value from the platform interface to the
// JavaScript google.maps.ColorScheme enum.
String? _colorSchemeToJs(Object? value) {
  if (value is! int) {
    return null;
  }
  // The platform interface serializes MapColorScheme as its enum index:
  // 0 = light, 1 = dark, 2 = followSystem.
  switch (value) {
    case 0:
      return 'google.maps.ColorScheme.LIGHT';
    case 1:
      return 'google.maps.ColorScheme.DARK';
    case 2:
      return 'google.maps.ColorScheme.FOLLOW_SYSTEM';
  }
  return null;
}

String _applyInitialPosition(CameraPosition initialPosition, String options) {
  options += ', zoom: ${initialPosition.zoom}';
  options +=
      ', center: {lat: ${initialPosition.target.latitude} ,lng: ${initialPosition.target.longitude}}';
  return options;
}

// Extracts the status of the traffic layer from the rawOptions map.
bool _isTrafficLayerEnabled(Map<String, dynamic> rawOptions) {
  if (rawOptions['trafficEnabled'] != null) {
    return rawOptions['trafficEnabled'] as bool;
  }
  return false;
}

// The keys we'd expect to see in a serialized MapTypeStyle JSON object.
final Set<String> _mapStyleKeys = <String>{
  'elementType',
  'featureType',
  'stylers',
};

// Checks if the passed in Map contains some of the _mapStyleKeys.
bool _isJsonMapStyle(Map<String, Object?> value) {
  return _mapStyleKeys.intersection(value.keys.toSet()).isNotEmpty;
}

// ignore: public_member_api_docs
class MapTypeStyle {
  // ignore: public_member_api_docs
  String? elementType;
  // ignore: public_member_api_docs
  String? featureType;
  // ignore: public_member_api_docs
  List<Object?>? stylers;
}

// Checks if there is un-parsable styling JSON, unrecognized feature type,
// unrecognized element type, or invalid styler keys in mapStyleJson.
String _mapStyles(String? mapStyleJson) {
  if (mapStyleJson != null) {
    try {
      json
          .decode(
            mapStyleJson,
            reviver: (Object? key, Object? value) {
              if (value is Map &&
                  _isJsonMapStyle(value as Map<String, Object?>)) {
                return MapTypeStyle()
                  ..elementType = value['elementType'] as String?
                  ..featureType = value['featureType'] as String?
                  ..stylers = (value['stylers']! as List<dynamic>)
                      .map<dynamic>((dynamic e) => e)
                      .toList();
              }
              return value;
            },
          )
          .cast<MapTypeStyle>()
          .toList() as List<MapTypeStyle>;
    } catch (e) {
      throw MapStyleException('Invalid Map Style JSON: $e');
    }
    return mapStyleJson;
  }
  return 'null';
}

LatLngBounds _convertToBounds(String value) {
  try {
    final dynamic bound = json.decode(value);
    if (bound is Map<String, dynamic>) {
      assert(
        bound['south'] is double &&
            bound['west'] is double &&
            bound['north'] is double &&
            bound['east'] is double,
      );
      return LatLngBounds(
        southwest: LatLng(bound['south'] as double, bound['west'] as double),
        northeast: LatLng(bound['north'] as double, bound['east'] as double),
      );
    }
  } catch (e) {
    debugPrint('Javascript Error: $e');
  }
  return util.nullLatLngBounds;
}

LatLng _convertToLatLng(String value) {
  try {
    final dynamic latlng = json.decode(value);
    if (latlng is Map<String, dynamic>) {
      assert(latlng['lat'] is num && latlng['lng'] is num);
      return LatLng((latlng['lat'] as num) + 0.0, (latlng['lng'] as num) + 0.0);
    }
  } catch (e) {
    debugPrint('Javascript Error: $e');
  }
  return util.nullLatLng;
}

ScreenCoordinate _convertToPoint(String value) {
  try {
    final dynamic latlng = json.decode(value);
    int x = 0, y = 0;

    if (latlng is Map<String, dynamic>) {
      x = latlng['x'] is int
          ? latlng['x'] as int
          : (latlng['x'] as double).toInt();
      y = latlng['y'] is int
          ? latlng['y'] as int
          : (latlng['y'] as double).toInt();

      return ScreenCoordinate(x: x, y: y);
    }
  } catch (e) {
    debugPrint('Javascript Error: $e');
  }
  return util.nullScreenCoordinate;
}

util.GInfoWindowOptions? _infoWindowOptionsFromMarker(Marker marker) {
  final String markerTitle = marker.infoWindow.title ?? '';
  final String markerSnippet = marker.infoWindow.snippet ?? '';

  // If both the title and snippet of an infowindow are empty, we don't really
  // want an infowindow...
  if ((markerTitle.isEmpty) && (markerSnippet.isEmpty)) {
    return null;
  }

  // Add an outer wrapper to the contents of the infowindow
  final StringBuffer buffer = StringBuffer();
  buffer.write('\'<div id="marker-${marker.markerId.value}-infowindow">');
  if (markerTitle.isNotEmpty) {
    buffer.write('<h3 class="infowindow-title">');
    buffer.write(markerTitle);
    buffer.write('</h3>');
  }
  if (markerSnippet.isNotEmpty) {
    buffer.write('<div class="infowindow-snippet">');
    buffer.write(markerSnippet);
    buffer.write('</div>');
  }
  buffer.write("</div>'");

  // Need to add Click Event to infoWindow's content
  return util.GInfoWindowOptions()
    ..content = buffer.toString()
    ..zIndex = marker.zIndex;
}

// Computes the options for a new [GMarker] from an incoming set of options
// [marker], and the existing marker registered with the map: [currentMarker].
// Preserves the position from the [currentMarker], if set.
util.GMarkerOptions _markerOptionsFromMarker(
  Marker marker,
  util.GMarker? currentMarker,
) {
  final List<Object?> iconConfig = marker.icon.toJson() as List<Object?>;
  util.GIcon? icon;

  if (iconConfig.isNotEmpty) {
    if (iconConfig[0] == 'asset') {
      assert(iconConfig.length >= 2);
      final Map<String, Object?> assetConfig =
          iconConfig[1]! as Map<String, Object?>;
      icon = util.GIcon()..url = '../${assetConfig['assetName']}';
      if (assetConfig['width'] != null || assetConfig['height'] != null) {
        icon.size = util.GSize(
          assetConfig['width'] != null
              ? double.parse(assetConfig['width']!.toString())
              : null,
          assetConfig['height'] != null
              ? double.parse(assetConfig['height']!.toString())
              : null,
        );
      }
    } else if (iconConfig[0] == 'bytes') {
      assert(iconConfig.length >= 2);
      final Map<String, Object?> assetConfig =
          iconConfig[1]! as Map<String, Object?>;
      icon = util.GIcon()
        ..url =
            'data:image/png;base64,${base64Encode(assetConfig['byteData']! as List<int>)}';
      if (assetConfig['width'] != null || assetConfig['height'] != null) {
        icon.size = util.GSize(
          assetConfig['width'] != null
              ? double.parse(assetConfig['width']!.toString())
              : null,
          assetConfig['height'] != null
              ? double.parse(assetConfig['height']!.toString())
              : null,
        );
      }
    }
  }

  final LatLng? position;
  if (currentMarker?.options?.position == null ||
      currentMarker?.options?.position != marker.position) {
    position = marker.position;
  } else {
    position = currentMarker?.options?.position;
  }

  // Flat and Rotation are not supported directly on the web.
  return util.GMarkerOptions()
    ..position = position
    ..title = marker.infoWindow.title ?? ''
    ..zIndex = marker.zIndex
    ..visible = marker.visible
    ..opacity = marker.alpha
    ..draggable = marker.draggable
    ..icon = icon;
}

// Converts a [Color] into a valid CSS value #RRGGBB.
String _getCssColor(Color color) {
  return '#${color.value.toRadixString(16).padLeft(8, '0').substring(2)}';
}

// Extracts the opacity from a [Color].
double _getCssOpacity(Color color) {
  return color.opacity;
}

util.GPolylineOptions _polylineOptionsFromPolyline(Polyline polyline) {
  // Some properties of polyline (endCap, jointType, patterns, startCap) are not
  // directly supported on the web.
  return util.GPolylineOptions()
    ..path = polyline.points
    ..strokeWeight = polyline.width
    ..strokeColor = _getCssColor(polyline.color)
    ..strokeOpacity = _getCssOpacity(polyline.color)
    ..visible = polyline.visible
    ..zIndex = polyline.zIndex
    ..geodesic = polyline.geodesic;
}

util.GPolygonOptions _polygonOptionsFromPolygon(Polygon polygon) {
  final List<LatLng> path = polygon.points;
  final bool polygonDirection = _isPolygonClockwise(path);
  final List<List<LatLng>> paths = <List<LatLng>>[path];
  int holeIndex = 0;

  for (int i = 0; i < polygon.holes.length; i++) {
    List<LatLng> holePath = polygon.holes[i];
    if (_isPolygonClockwise(holePath) == polygonDirection) {
      holePath = holePath.reversed.toList();
      debugPrint(
        'Hole [$holeIndex] in Polygon [${polygon.polygonId.value}] has been reversed.'
        ' Ensure holes in polygons are "wound in the opposite direction to the outer path."'
        ' More info: https://github.com/flutter/flutter/issues/74096',
      );
    }
    paths.add(holePath);
    holeIndex++;
  }

  return util.GPolygonOptions()
    ..paths = paths
    ..strokeColor = _getCssColor(polygon.strokeColor)
    ..strokeOpacity = _getCssOpacity(polygon.strokeColor)
    ..strokeWeight = polygon.strokeWidth
    ..fillColor = _getCssColor(polygon.fillColor)
    ..fillOpacity = _getCssOpacity(polygon.fillColor)
    ..visible = polygon.visible
    ..zIndex = polygon.zIndex
    ..geodesic = polygon.geodesic;
}

/// Calculates the direction of a given Polygon
/// based on: https://stackoverflow.com/a/1165943
///
/// returns [true] if clockwise [false] if counterclockwise
///
/// This method expects that the incoming [path] is a `List` of well-formed,
/// non-null [LatLng] objects.
///
/// Currently, this method is only called from [_polygonOptionsFromPolygon], and
/// the `path` is a transformed version of [Polygon.points] or each of the
/// [Polygon.holes], guaranteeing that `lat` and `lng` can be accessed with `!`.
bool _isPolygonClockwise(List<LatLng> path) {
  double direction = 0.0;
  for (int i = 0; i < path.length; i++) {
    direction = direction +
        ((path[(i + 1) % path.length].latitude - path[i].latitude) *
            (path[(i + 1) % path.length].longitude + path[i].longitude));
  }
  return direction >= 0;
}

util.GCircleOptions _circleOptionsFromCircle(Circle circle) {
  return util.GCircleOptions()
    ..strokeColor = _getCssColor(circle.strokeColor)
    ..strokeOpacity = _getCssOpacity(circle.strokeColor)
    ..strokeWeight = circle.strokeWidth
    ..fillColor = _getCssColor(circle.fillColor)
    ..fillOpacity = _getCssOpacity(circle.fillColor)
    ..center = LatLng(circle.center.latitude, circle.center.longitude)
    ..radius = circle.radius
    ..visible = circle.visible
    ..zIndex = circle.zIndex;
}

util.GGroundOverlayOptions? _groundOverlayOptionsFromGroundOverlay(
  GroundOverlay groundOverlay,
) {
  // The JS Maps GroundOverlay only supports bounds-based positioning. Skip
  // position-only overlays — the platform interface allows the field but the
  // JS API has no equivalent.
  final LatLngBounds? bounds = groundOverlay.bounds;
  if (bounds == null) {
    debugPrint(
      'GroundOverlay ${groundOverlay.groundOverlayId.value} skipped: '
      'the Google Maps JavaScript API only supports bounds-based '
      'ground overlays.',
    );
    return null;
  }
  final String? imageUrl = _imageUrlFromMapBitmap(groundOverlay.image);
  if (imageUrl == null) {
    debugPrint(
      'GroundOverlay ${groundOverlay.groundOverlayId.value} skipped: '
      'unsupported image source.',
    );
    return null;
  }
  return util.GGroundOverlayOptions()
    ..url = "'$imageUrl'"
    ..bounds = '{south:${bounds.southwest.latitude},'
        ' west:${bounds.southwest.longitude},'
        ' north:${bounds.northeast.latitude},'
        ' east:${bounds.northeast.longitude}}'
    ..clickable = groundOverlay.clickable
    ..opacity = 1.0 - groundOverlay.transparency
    ..visible = groundOverlay.visible;
}

String? _imageUrlFromMapBitmap(MapBitmap bitmap) {
  final List<Object?> iconConfig = bitmap.toJson() as List<Object?>;
  if (iconConfig.isEmpty) {
    return null;
  }
  if (iconConfig[0] == 'asset' && iconConfig.length >= 2) {
    final Map<String, Object?> assetConfig =
        iconConfig[1]! as Map<String, Object?>;
    return '../${assetConfig['assetName']}';
  }
  if (iconConfig[0] == 'bytes' && iconConfig.length >= 2) {
    final Map<String, Object?> assetConfig =
        iconConfig[1]! as Map<String, Object?>;
    final List<int> bytes = assetConfig['byteData']! as List<int>;
    return 'data:image/png;base64,${base64Encode(bytes)}';
  }
  return null;
}
