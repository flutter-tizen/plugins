// Copyright 2022 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter_tizen_plugin_tools/src/tizen_sdk.dart';
import 'package:test/test.dart';

void main() {
  test('correctly parses device types', () {
    expect(DeviceType.fromString('tv'), DeviceType.tv);
    expect(DeviceType.fromString('mobile'), DeviceType.mobile);
  });
  test('throws argument exception for unsupported device types.', () {
    expect(() => DeviceType.fromString('invalid'), throwsArgumentError);
  });

  test('correctly parses profile', () {
    final Profile profile = Profile.fromString('tv-6.0');
    expect(profile.deviceType, DeviceType.tv);
    expect(profile.version!.major, 6);
    expect(profile.version!.minor, 0);
  });

  test('throws argument exception on invalid profile string.', () {
    expect(() => Profile.fromString(''), throwsArgumentError);
    expect(() => Profile.fromString('wear'), throwsArgumentError);
    expect(() => Profile.fromString('tv-3xx'), throwsArgumentError);
  });
}
