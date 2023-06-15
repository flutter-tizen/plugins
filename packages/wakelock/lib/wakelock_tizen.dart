// Copyright 2023 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/services.dart';
import 'package:wakelock_platform_interface/wakelock_platform_interface.dart';

const MethodChannel _channel = MethodChannel('tizen/wakelock_plugin');

/// A Tizen implementation of [WakelockPlatformInterface].
class WakelockTizen extends WakelockPlatformInterface {
  /// Registers this class as the default platform implementation.
  static void register() {
    WakelockPlatformInterface.instance = WakelockTizen();
  }

  @override
  Future<void> toggle({required bool enable}) async {
    await _channel.invokeMethod<void>('toggle', enable);
  }

  @override
  Future<bool> get enabled async =>
      await _channel.invokeMethod<bool>('isEnabled') ?? false;
}
