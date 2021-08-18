// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of google_maps_flutter_tizen;

/// A void function that handles a [gmaps.LatLng] as a parameter.
///
/// Similar to [ui.VoidCallback], but specific for Marker drag events.
typedef LatLngCallback = void Function(LatLng latLng);

/// The base class for all "geometry" group controllers.
///
/// This lets all Geometry controllers ([MarkersController], [CirclesController],
/// [PolygonsController], [PolylinesController]) to be bound to a [gmaps.GMap]
/// instance and our internal `mapId` value.
abstract class GeometryController {
  /// The WebViewController instance that this controller operates on.
  late WebViewController viewController;

  /// The map ID for events.
  late int mapId;

  /// Binds a `mapId` and the [gmaps.GMap] instance to this controller.
  void bindToMap(int mapId, WebViewController viewController) {
    this.mapId = mapId;
    this.viewController = viewController;
  }
}
