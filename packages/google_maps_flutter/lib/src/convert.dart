// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of google_maps_flutter_tizen;

const LatLng _nullLatLng = LatLng(0, 0);
final LatLngBounds _nullLatLngBounds =
    LatLngBounds(southwest: _nullLatLng, northeast: _nullLatLng);

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

  // ex: [[[-34.022631, 150.62068499999998], [-33.571835, 151.32595200000003]]]
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
