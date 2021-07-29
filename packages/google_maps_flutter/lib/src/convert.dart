// Copyright 2013 The Flutter Authors. All rights reserved.
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
// The following options are not handled here, for various reasons:
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

String _mapStyles(String? mapStyleJson) {
  return mapStyleJson ?? 'null';
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

util.GInfoWindowOptions? _infoWindowOptionsFromMarker(Marker marker) {
  final markerTitle = marker.infoWindow.title ?? '';
  final markerSnippet = marker.infoWindow.snippet ?? '';

  // If both the title and snippet of an infowindow are empty, we don't really
  // want an infowindow...
  if ((markerTitle.isEmpty) && (markerSnippet.isEmpty)) {
    return null;
  }

  // Add an outer wrapper to the contents of the infowindow, we need it to listen
  // to click events...
  final HtmlElement container = DivElement()
    ..id = 'gmaps-marker-${marker.markerId.value}-infowindow';

  if (markerTitle.isNotEmpty) {
    final HtmlElement title = HeadingElement.h3()
      ..className = 'infowindow-title'
      ..innerText = markerTitle;
    container.children.add(title);
  }
  if (markerSnippet.isNotEmpty) {
    final HtmlElement snippet = DivElement()
      ..className = 'infowindow-snippet'
      ..setInnerHtml(
        sanitizeHtml(markerSnippet),
        treeSanitizer: NodeTreeSanitizer.trusted,
      );
    container.children.add(snippet);
  }

  return util.GInfoWindowOptions()
    ..content = container
    ..zIndex = marker.zIndex;
  // TODO: Compute the pixelOffset of the infoWindow, from the size of the Marker,
  // and the marker.infoWindow.anchor property.
}

// Computes the options for a new [gmaps.Marker] from an incoming set of options
// [marker], and the existing marker registered with the map: [currentMarker].
// Preserves the position from the [currentMarker], if set.
util.GMarkerOptions _markerOptionsFromMarker(
  Marker marker,
  util.GMarker? currentMarker,
) {
  final iconConfig = marker.icon.toJson() as List;
  util.GIcon? icon;

  if (iconConfig != null || iconConfig.isEmpty) {
    if (iconConfig[0] == 'fromAssetImage') {
      assert(iconConfig.length >= 2);
      // iconConfig[2] contains the DPIs of the screen, but that information is
      // already encoded in the iconConfig[1]

      // TODO(seungsoo): temporally make hard code
      icon = util.GIcon()..url = '${iconConfig[1]}';
      print('[LEESS] util.GIcon()..url: ${iconConfig[1]}');
      // ..url = ui.webOnlyAssetManager.getAssetUrl(iconConfig[1]);

      // iconConfig[3] may contain the [width, height] of the image, if passed!
      if (iconConfig.length >= 4 && iconConfig[3] != null) {
        final size =
            util.GSize(iconConfig[3][0] as num, iconConfig[3][1] as num);
        icon
          ..size = size
          ..scaledSize = size;
      }
    } else if (iconConfig[0] == 'fromBytes') {
      // TODO: temporally make hard code
      // // Grab the bytes, and put them into a blob
      // List<int> bytes = iconConfig[1] as List<int>;
      // final blob = Blob(bytes); // Let the browser figure out the encoding
      // icon = gmaps.Icon()..url = Url.createObjectUrlFromBlob(blob);
    }
  }
  return util.GMarkerOptions()
    ..position = currentMarker?.position ??
        LatLng(
          marker.position.latitude,
          marker.position.longitude,
        )
    ..title = sanitizeHtml(marker.infoWindow.title ?? "")
    ..zIndex = marker.zIndex
    ..visible = marker.visible
    ..opacity = marker.alpha
    ..draggable = marker.draggable
    ..icon = icon;
  // TODO: Compute anchor properly, otherwise infowindows attach to the wrong spot.
  // Flat and Rotation are not supported directly on the web.
}
