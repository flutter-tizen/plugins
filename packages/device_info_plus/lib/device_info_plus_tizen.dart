// Copyright 2020 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';

import 'package:flutter/services.dart';

/// Information derived from `system_info`.
///
/// See: https://docs.tizen.org/application/native/guides/device/system
class TizenDeviceInfo {
  TizenDeviceInfo._({
    required this.data,
    required this.modelName,
    required this.cpuArch,
    required this.nativeApiVersion,
    required this.platformVersion,
    required this.webApiVersion,
    required this.profile,
    required this.buildDate,
    required this.buildId,
    required this.buildString,
    required this.buildTime,
    required this.buildType,
    required this.buildVariant,
    required this.buildRelease,
    required this.deviceType,
    required this.manufacturer,
    required this.platformName,
    required this.platformProcessor,
    required this.tizenId,
    required this.screenWidth,
    required this.screenHeight,
  });

  /// Device information data.
  final Map<String, dynamic> data;

  /// http://tizen.org/system/model_name
  final String? modelName;

  /// http://tizen.org/feature/platform.core.cpu.arch
  final String? cpuArch;

  /// http://tizen.org/feature/platform.native.api.version
  final String? nativeApiVersion;

  /// http://tizen.org/feature/platform.version
  final String? platformVersion;

  /// http://tizen.org/feature/platform.web.api.version
  final String? webApiVersion;

  /// http://tizen.org/feature/profile
  final String? profile;

  /// http://tizen.org/system/build.date
  final String? buildDate;

  /// http://tizen.org/system/build.id
  final String? buildId;

  /// http://tizen.org/system/build.string
  final String? buildString;

  /// http://tizen.org/system/build.time
  final String? buildTime;

  /// http://tizen.org/system/build.type
  final String? buildType;

  /// http://tizen.org/system/build.variant
  final String? buildVariant;

  /// http://tizen.org/system/build.release
  final String? buildRelease;

  /// http://tizen.org/system/device_type
  final String? deviceType;

  /// http://tizen.org/system/manufacturer
  final String? manufacturer;

  /// http://tizen.org/system/platform.name
  final String? platformName;

  /// http://tizen.org/system/platform.processor
  final String? platformProcessor;

  /// http://tizen.org/system/tizenid
  final String? tizenId;

  /// http://tizen.org/feature/screen.width
  final int screenWidth;

  /// http://tizen.org/feature/screen.height
  final int screenHeight;

  /// Creates a [TizenDeviceInfo] from the [map].
  static TizenDeviceInfo fromMap(Map<String, dynamic> map) {
    return TizenDeviceInfo._(
      data: map,
      modelName: map['modelName'],
      cpuArch: map['cpuArch'],
      nativeApiVersion: map['nativeApiVersion'],
      platformVersion: map['platformVersion'],
      webApiVersion: map['webApiVersion'],
      profile: map['profile'],
      buildDate: map['buildDate'],
      buildId: map['buildId'],
      buildString: map['buildString'],
      buildTime: map['buildTime'],
      buildType: map['buildType'],
      buildVariant: map['buildVariant'],
      buildRelease: map['buildRelease'],
      deviceType: map['deviceType'],
      manufacturer: map['manufacturer'],
      platformName: map['platformName'],
      platformProcessor: map['platformProcessor'],
      tizenId: map['tizenId'],
      screenWidth: map['screenWidth'],
      screenHeight: map['screenHeight'],
    );
  }

  @override
  String toString() {
    return 'TizenDeviceInfo{data: $data}';
  }
}

class _MethodChannelDeviceInfo {
  /// The method channel used to interact with the native platform.
  MethodChannel channel = const MethodChannel('tizen/device_info_plus');

  /// Method channel for Tizen devices.
  Future<TizenDeviceInfo> tizenInfo() async {
    return TizenDeviceInfo.fromMap(
      (await channel.invokeMethod(
        'getTizenDeviceInfo',
      )).cast<String, dynamic>(),
    );
  }
}

/// Provides device and operating system information of a Tizen device.
class DeviceInfoPluginTizen {
  /// No work is done when instantiating the plugin. It's safe to call this
  /// repeatedly or in performance-sensitive blocks.
  DeviceInfoPluginTizen();

  static final _MethodChannelDeviceInfo _platform = _MethodChannelDeviceInfo();

  /// This information does not change from call to call. Cache it.
  TizenDeviceInfo? _cachedTizenDeviceInfo;

  /// Information derived from `system_info`.
  ///
  /// See: https://docs.tizen.org/application/native/guides/device/system
  Future<TizenDeviceInfo> get tizenInfo async =>
      _cachedTizenDeviceInfo ??= await _platform.tizenInfo();
}
