// Copyright 2013 The Flutter Authors. All rights reserved.
// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of google_maps_flutter_tizen;

/// A void function that handles a [LatLng] as a parameter.
///
/// Similar to [ui.VoidCallback], but specific for Marker drag events.
typedef LatLngCallback = void Function(LatLng latLng);

/// The base class for all "geometry" group controllers.
///
/// This lets all Geometry controllers ([MarkersController], [CirclesController],
/// [PolygonsController], [PolylinesController]) to be bound to a map
/// instance and our internal `mapId` value.
abstract class GeometryController {
  /// The WebView instance that this controller operates on.
  late WebView webview;

  /// The map ID for events.
  late int mapId;

  /// Binds a `mapId` and the map instance to this controller.
  void bindToMap(int mapId, WebView webview) {
    this.mapId = mapId;
    this.webview = webview;
  }
}
