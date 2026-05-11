// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of '../google_maps_flutter_tizen.dart';

/// This class manages a set of [GroundOverlayController]s associated to a
/// [GoogleMapController].
class GroundOverlaysController extends GeometryController {
  /// Initializes the cache. The [StreamController] comes from the
  /// [GoogleMapController], and is shared with other controllers.
  GroundOverlaysController({
    required StreamController<MapEvent<Object?>> stream,
  })  : _streamController = stream,
        _groundOverlayIdToController =
            <GroundOverlayId, GroundOverlayController>{},
        _idToGroundOverlayId = <int, GroundOverlayId>{};

  // A cache of [GroundOverlayController]s indexed by their [GroundOverlayId].
  final Map<GroundOverlayId, GroundOverlayController>
      _groundOverlayIdToController;
  final Map<int, GroundOverlayId> _idToGroundOverlayId;

  // The stream over which ground overlays broadcast events.
  final StreamController<MapEvent<Object?>> _streamController;

  /// Adds a set of [GroundOverlay] objects to the cache.
  ///
  /// Wraps each [GroundOverlay] into its corresponding
  /// [GroundOverlayController].
  void addGroundOverlays(Set<GroundOverlay> groundOverlaysToAdd) {
    groundOverlaysToAdd.forEach(_addGroundOverlay);
  }

  void _addGroundOverlay(GroundOverlay? groundOverlay) {
    if (groundOverlay == null) {
      return;
    }

    final util.GGroundOverlayOptions? populationOptions =
        _groundOverlayOptionsFromGroundOverlay(groundOverlay);
    if (populationOptions == null) {
      return;
    }

    final util.GGroundOverlay gGroundOverlay = util.GGroundOverlay(
      populationOptions,
    );
    final GroundOverlayController controller = GroundOverlayController(
      groundOverlay: gGroundOverlay,
      onTap: () {
        _onGroundOverlayTap(groundOverlay.groundOverlayId);
      },
      controller: util.webController,
    );
    _idToGroundOverlayId[gGroundOverlay.id] = groundOverlay.groundOverlayId;
    _groundOverlayIdToController[groundOverlay.groundOverlayId] = controller;
  }

  /// Updates a set of [GroundOverlay] objects with new options.
  void changeGroundOverlays(Set<GroundOverlay> groundOverlaysToChange) {
    groundOverlaysToChange.forEach(_changeGroundOverlay);
  }

  void _changeGroundOverlay(GroundOverlay groundOverlay) {
    final GroundOverlayController? groundOverlayController =
        _groundOverlayIdToController[groundOverlay.groundOverlayId];
    final util.GGroundOverlayOptions? options =
        _groundOverlayOptionsFromGroundOverlay(groundOverlay);
    if (options == null) {
      return;
    }
    groundOverlayController?.update(options);
  }

  /// Removes a set of [GroundOverlayId]s from the cache.
  void removeGroundOverlays(Set<GroundOverlayId> groundOverlayIdsToRemove) {
    groundOverlayIdsToRemove.forEach(_removeGroundOverlay);
  }

  void _removeGroundOverlay(GroundOverlayId groundOverlayId) {
    final GroundOverlayController? groundOverlayController =
        _groundOverlayIdToController[groundOverlayId];
    groundOverlayController?.remove();
    _groundOverlayIdToController.remove(groundOverlayId);
  }

  bool _onGroundOverlayTap(GroundOverlayId groundOverlayId) {
    _streamController.add(GroundOverlayTapEvent(mapId, groundOverlayId));
    return false;
  }
}
