// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_tizen_plugin_tools/src/tizen_sdk.dart';
import 'package:test/test.dart';

void main() {
  test('correctly parses device types', () {
    expect(DeviceType.fromString('wearable'), DeviceType.wearable);
    expect(DeviceType.fromString('tv'), DeviceType.tv);
    expect(DeviceType.fromString('mobile'), DeviceType.mobile);
  });
  test('throws argument exception for unsupported device types.', () {
    expect(() => DeviceType.fromString('invalid'), throwsArgumentError);
  });

  test('correctly parses profile', () {
    final Profile profile = Profile.fromString('wearable-5.5');
    expect(profile.deviceType, DeviceType.wearable);
    expect(profile.version!.major, 5);
    expect(profile.version!.minor, 5);
  });

  test('throws argument exception on invalid profile string.', () {
    expect(() => Profile.fromString(''), throwsArgumentError);
    expect(() => Profile.fromString('wear'), throwsArgumentError);
    expect(() => Profile.fromString('tv-3xx'), throwsArgumentError);
  });
}
