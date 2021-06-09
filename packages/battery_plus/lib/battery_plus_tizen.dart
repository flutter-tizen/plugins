// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:battery_plus_platform_interface/battery_plus_platform_interface.dart';
import 'package:battery_plus_platform_interface/method_channel_battery_plus.dart';

/// The Tizen implementation of [BatteryPlatform].
///
/// This class sets the default instance of [BatteryPlatform] to
/// Tizen's implementation of [BatteryPlatform].
/// This is required to avoid platform interface being set to Linux's.
/// https://github.com/fluttercommunity/plus_plugins/blob/06dd9fd986d52ccd72754b2e787198f7c82878b2/packages/battery_plus/lib/battery_plus.dart#L43-L47
class BatteryPlugin extends BatteryPlatform {
  /// Registers this class as the default instance of [BatteryPlatform].
  static void register() {
    BatteryPlatform.instance = BatteryPlugin();
  }

  final MethodChannelBattery _methodChannelBattery = MethodChannelBattery();

  @override
  Future<int> get batteryLevel => _methodChannelBattery.batteryLevel;

  @override
  Stream<BatteryState> get onBatteryStateChanged =>
      _methodChannelBattery.onBatteryStateChanged;
}
