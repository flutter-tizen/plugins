// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of '../google_maps_flutter_tizen.dart';

/// The `GroundOverlayController` class wraps a [GGroundOverlay] and its
/// `onTap` behavior.
class GroundOverlayController {
  /// Creates a `GroundOverlayController` that wraps a [GGroundOverlay] object
  /// and its `onTap` behavior.
  GroundOverlayController({
    required util.GGroundOverlay groundOverlay,
    ui.VoidCallback? onTap,
    WebViewController? controller,
  })  : _groundOverlay = groundOverlay,
        tapEvent = onTap {
    _addGroundOverlayEvent(controller);
  }

  util.GGroundOverlay? _groundOverlay;

  /// Ground overlay component's tap event.
  ui.VoidCallback? tapEvent;

  Future<void> _addGroundOverlayEvent(WebViewController? controller) async {
    final String command =
        "$_groundOverlay.addListener('click', (event) => GroundOverlayClick.postMessage(JSON.stringify(${_groundOverlay?.id})));";
    await controller!.runJavaScript(command);
  }

  /// Updates the options of the wrapped [GGroundOverlay] object.
  ///
  /// This cannot be called after [remove].
  void update(util.GGroundOverlayOptions options) {
    assert(
      _groundOverlay != null,
      'Cannot `update` GroundOverlay after calling `remove`.',
    );
    _groundOverlay!.options = options;
  }

  /// Disposes of the currently wrapped [GGroundOverlay].
  void remove() {
    if (_groundOverlay != null) {
      _groundOverlay!.map = null;
      _groundOverlay = null;
    }
  }
}
