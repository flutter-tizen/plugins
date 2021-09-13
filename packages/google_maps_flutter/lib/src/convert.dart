// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of google_maps_flutter_tizen;

// Indices in the plugin side don't match with the ones
Map<int, String> _mapTypeToMapTypeId = {
  0: 'roadmap', // None
  1: 'roadmap',
  2: 'satellite',
  3: 'terrain',
  4: 'hybrid',
};

String? _getCameraBounds(dynamic option) {
  if (option is! List<Object?> || option.first == null) {
    return null;
  }

  final List<Object> bound = option[0] as List<Object>;
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
// myLocationButtonEnabled Widget not available in web yet, it needs to be built on top of the maps widget
//   See: https://developers.google.com/maps/documentation/javascript/examples/control-custom
// myLocationEnabled needs to be built through dart:html navigator.geolocation
//   See: https://api.dart.dev/stable/2.8.4/dart-html/Geolocation-class.html
// trackCameraPosition is just a boolan value that indicates if the map has an onCameraMove handler.
// indoorViewEnabled seems to not have an equivalent in web
// buildingsEnabled seems to not have an equivalent in web
// padding seems to behave differently in web than mobile. You can't move UI elements in web.
String _rawOptionsToString(Map<String, dynamic> rawOptions) {
  // These don't have any rawOptions entry, but they seem to be off in the native maps.
  String options =
      'mapTypeControl: false, fullscreenControl: false, streetViewControl: false';

  if (_mapTypeToMapTypeId.containsKey(rawOptions['mapType'])) {
    options += ', mapTypeId: \'${_mapTypeToMapTypeId[rawOptions['mapType']]}\'';
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

  if (rawOptions['styles'] != null) {
    options += ', styles: ${rawOptions['styles']}';
  } else {
    options += ', styles: null';
  }

  if (rawOptions['scrollGesturesEnabled'] == false ||
      rawOptions['zoomGesturesEnabled'] == false) {
    options += ', gestureHandling: \'none\'';
  } else {
    options += ', gestureHandling: \'auto\'';
  }

  return options;
}

String _applyInitialPosition(
  CameraPosition initialPosition,
  String options,
) {
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
bool _isJsonMapStyle(Map value) {
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
          .decode(mapStyleJson, reviver: (key, value) {
            if (value is Map && _isJsonMapStyle(value)) {
              return MapTypeStyle()
                ..elementType = value['elementType'] as String?
                ..featureType = value['featureType'] as String?
                ..stylers = (value['stylers'] as List<dynamic>)
                    .map<dynamic>((dynamic e) => e)
                    .toList();
            }
            return value;
          })
          .cast<MapTypeStyle>()
          .toList() as List<MapTypeStyle>;
    } catch (e) {
      throw MapStyleException('Invalid Map Style JSON: ${e.toString()}');
    }
    return mapStyleJson;
  }
  return 'null';
}

LatLngBounds _convertToBounds(String value) {
  try {
    final dynamic bound = json.decode(value);
    if (bound is Map<String, dynamic>) {
      assert(bound['south'] is double &&
          bound['west'] is double &&
          bound['north'] is double &&
          bound['east'] is double);
      return LatLngBounds(
          southwest: LatLng(bound['south'] as double, bound['west'] as double),
          northeast: LatLng(bound['north'] as double, bound['east'] as double));
    }
  } catch (e) {
    print('Javascript Error: $e');
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
    print('Javascript Error: $e');
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
    print('Javascript Error: $e');
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
  buffer.write('</div>\'');

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
  final iconConfig = marker.icon.toJson() as List;
  util.GIcon? icon;

  if (iconConfig.isNotEmpty) {
    if (iconConfig[0] == 'fromAssetImage') {
      assert(iconConfig.length >= 2);
      // iconConfig[2] contains the DPIs of the screen, but that information is
      // already encoded in the iconConfig[1]
      icon = util.GIcon()..url = '../${iconConfig[1]}';

      // iconConfig[3] may contain the [width, height] of the image, if passed!
      if (iconConfig.length >= 4 && iconConfig[3] != null) {
        final util.GSize size =
            util.GSize(iconConfig[3][0] as num, iconConfig[3][1] as num);
        icon
          ..size = size
          ..scaledSize = size;
      }
    } else if (iconConfig[0] == 'fromBytes') {
      throw UnimplementedError('Not Implemented');
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
  return '#' + color.value.toRadixString(16).padLeft(8, '0').substring(2);
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
  final List<List<LatLng>> paths = [path];
  int holeIndex = 0;
  polygon.holes.forEach((List<LatLng> hole) {
    List<LatLng> holePath = hole;
    if (_isPolygonClockwise(holePath) == polygonDirection) {
      holePath = holePath.reversed.toList();
      if (kDebugMode) {
        print(
            'Hole [$holeIndex] in Polygon [${polygon.polygonId.value}] has been reversed.'
            ' Ensure holes in polygons are "wound in the opposite direction to the outer path."'
            ' More info: https://github.com/flutter/flutter/issues/74096');
      }
    }
    paths.add(holePath);
    holeIndex++;
  });
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
