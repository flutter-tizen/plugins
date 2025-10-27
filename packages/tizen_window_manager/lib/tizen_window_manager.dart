// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/services.dart';

/// A plugin for managing windows on Tizen platform.
///
/// This plugin provides methods to control window behavior such as
/// activating, raising, lowering, showing, hiding, and getting window geometry.
class WindowManager {
  WindowManager._();

  static const MethodChannel _channel = MethodChannel('tizen/window_manager');

  /// Activates the window.
  static Future<void> activate() => _channel.invokeMethod('activate');

  /// Lowers the window.
  static Future<void> lower() => _channel.invokeMethod('lower');

  /// Gets the geometry (position and size) of the window.
  ///
  /// Returns a map containing:
  /// - 'x': The x coordinate of the window
  /// - 'y': The y coordinate of the window
  /// - 'width': The width of the window
  /// - 'height': The height of the window
  static Future<Map<String, int>> getGeometry() async {
    final dynamic result = await _channel.invokeMethod('getGeometry');
    if (result is Map) {
      return Map<String, int>.from(result);
    }
    throw Exception(
        'Unexpected result type from getGeometry: ${result.runtimeType}');
  }
}
