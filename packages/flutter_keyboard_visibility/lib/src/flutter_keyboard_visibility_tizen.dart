// Copyright 2025 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'package:flutter/services.dart';
import 'package:flutter_keyboard_visibility_platform_interface/flutter_keyboard_visibility_platform_interface.dart';

/// The Tizen implementation of [FlutterKeyboardVisibilityPlugin].
class FlutterKeyboardVisibilityPlugin
    extends FlutterKeyboardVisibilityPlatform {
  /// Registers this class as the default instance of [FlutterKeyboardVisibilityPlatform].
  static void register() {
    FlutterKeyboardVisibilityPlatform.instance =
        FlutterKeyboardVisibilityPlugin();
  }

  static const EventChannel _internalChannel =
      EventChannel('tizen/internal/inputpanel');

  @override
  Stream<bool> get onChange =>
      _internalChannel.receiveBroadcastStream().map((dynamic event) {
        final String? state =
            (event as Map<Object?, Object?>)['state'] as String?;
        if (state == 'show' || state == 'will_show') {
          return true;
        } else if (state == 'hide') {
          return false;
        } else {
          return false;
        }
      });
}
