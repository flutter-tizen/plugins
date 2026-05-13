// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// ignore_for_file: public_member_api_docs

import 'package:flutter/material.dart';
import 'package:google_maps_flutter/google_maps_flutter.dart';

import 'page.dart';

class GroundOverlayPage extends GoogleMapExampleAppPage {
  const GroundOverlayPage({Key? key})
      : super(const Icon(Icons.map), 'Ground overlay', key: key);

  @override
  Widget build(BuildContext context) {
    return const GroundOverlayBody();
  }
}

class GroundOverlayBody extends StatefulWidget {
  const GroundOverlayBody({super.key});

  @override
  State<StatefulWidget> createState() => GroundOverlayBodyState();
}

class GroundOverlayBodyState extends State<GroundOverlayBody> {
  GoogleMapController? controller;
  GroundOverlay? _groundOverlay;
  int _groundOverlayIndex = 0;

  static const LatLng _mapCenter = LatLng(37.422026, -122.085329);

  final LatLngBounds _bounds1 = LatLngBounds(
    southwest: const LatLng(37.42, -122.09),
    northeast: const LatLng(37.423, -122.084),
  );
  final LatLngBounds _bounds2 = LatLngBounds(
    southwest: const LatLng(37.421, -122.091),
    northeast: const LatLng(37.424, -122.08),
  );

  late LatLngBounds _currentBounds = _bounds1;

  // ignore: use_setters_to_change_properties
  void _onMapCreated(GoogleMapController controller) {
    this.controller = controller;
  }

  Future<void> _addGroundOverlay() async {
    final AssetMapBitmap image = await AssetMapBitmap.create(
      createLocalImageConfiguration(context),
      'assets/red_square.png',
      bitmapScaling: MapBitmapScaling.none,
    );

    _groundOverlayIndex += 1;

    final GroundOverlay overlay = GroundOverlay.fromBounds(
      groundOverlayId: GroundOverlayId('ground_overlay_$_groundOverlayIndex'),
      image: image,
      bounds: _currentBounds,
      onTap: _toggleBounds,
    );

    setState(() {
      _groundOverlay = overlay;
    });
  }

  void _removeGroundOverlay() {
    setState(() {
      _groundOverlay = null;
    });
  }

  void _toggleTransparency() {
    if (_groundOverlay == null) {
      return;
    }
    setState(() {
      final double transparency =
          _groundOverlay!.transparency == 0.0 ? 0.5 : 0.0;
      _groundOverlay =
          _groundOverlay!.copyWith(transparencyParam: transparency);
    });
  }

  void _toggleVisible() {
    if (_groundOverlay == null) {
      return;
    }
    setState(() {
      _groundOverlay =
          _groundOverlay!.copyWith(visibleParam: !_groundOverlay!.visible);
    });
  }

  Future<void> _toggleBounds() async {
    setState(() {
      _currentBounds = _currentBounds == _bounds1 ? _bounds2 : _bounds1;
    });
    await _addGroundOverlay();
  }

  @override
  Widget build(BuildContext context) {
    final Set<GroundOverlay> overlays = <GroundOverlay>{
      if (_groundOverlay != null) _groundOverlay!,
    };
    return Column(
      mainAxisSize: MainAxisSize.min,
      mainAxisAlignment: MainAxisAlignment.spaceEvenly,
      crossAxisAlignment: CrossAxisAlignment.stretch,
      children: <Widget>[
        Expanded(
          child: GoogleMap(
            initialCameraPosition: const CameraPosition(
              target: _mapCenter,
              zoom: 14.0,
            ),
            groundOverlays: overlays,
            onMapCreated: _onMapCreated,
          ),
        ),
        Row(
          mainAxisAlignment: MainAxisAlignment.spaceEvenly,
          children: <Widget>[
            TextButton(
              onPressed: _groundOverlay == null ? _addGroundOverlay : null,
              child: const Text('Add'),
            ),
            TextButton(
              onPressed: _groundOverlay != null ? _removeGroundOverlay : null,
              child: const Text('Remove'),
            ),
            TextButton(
              onPressed: _groundOverlay == null ? null : _toggleTransparency,
              child: const Text('Toggle transparency'),
            ),
            TextButton(
              onPressed: _groundOverlay == null ? null : _toggleVisible,
              child: const Text('Toggle visible'),
            ),
            TextButton(
              onPressed: _groundOverlay == null ? null : _toggleBounds,
              child: const Text('Change bounds'),
            ),
          ],
        ),
      ],
    );
  }
}
