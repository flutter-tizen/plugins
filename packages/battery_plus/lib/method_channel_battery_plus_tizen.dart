// Copyright 2021 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:battery_plus_platform_interface/method_channel_battery_plus.dart'
    as battery_plus;
// ignore: implementation_imports
import 'package:battery_plus_platform_interface/src/enums.dart';

import 'battery_plus_tizen.dart';

class MethodChannelBattery extends BatteryPlugin {
  MethodChannelBattery()
      : _methodChannelBattery = battery_plus.MethodChannelBattery();

  final battery_plus.MethodChannelBattery _methodChannelBattery;

  @override
  Future<int> get batteryLevel => _methodChannelBattery.batteryLevel;

  @override
  Stream<BatteryState> get onBatteryStateChanged {
    return _methodChannelBattery.onBatteryStateChanged;
  }
}
